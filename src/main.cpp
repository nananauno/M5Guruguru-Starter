#include <M5Unified.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <LittleFS.h>
#include <Preferences.h>
#include <M5GuruguruAvatar.h>
#include <qrcode.h>  // ricmoo/QRCode

void displayQRCode(const String& data, int yCenter);
void displayConnected();
void drawFileStatus();
void markAvatarReady();
bool checkAvatarFilesReady();

WebServer server(80);
Preferences prefs;
M5GuruguruAvatar avatar;

String bootMode;  // "setup" or "avatar"
bool avatarReady = false;

const char* ap_ssid = "M5Guruguru-Starter";
const char* ap_pass = "guruguru123";

uint8_t detectRotation(float ax, float ay) {
  if (fabsf(ax) > fabsf(ay)) {
    return (ax > 0) ? 2 : 0;
  }
  return M5.Display.getRotation();
}

void setupUploadMode() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE);

  LittleFS.begin(true);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);

  IPAddress apIP = WiFi.softAPIP();
  Serial.printf("AP Started! SSID: %s, IP: %s\n", ap_ssid, apIP.toString().c_str());

  String qrData = "WIFI:S:" + String(ap_ssid) + ";T:WPA;P:" + String(ap_pass) + ";;";

  const int cx      = M5.Display.width() / 2;
  const int headerY = 50;
  const int qrCenter = 190;
  const int footerY  = 310;

  M5.Display.setTextDatum(TC_DATUM);
  M5.Display.setTextSize(2);
  M5.Display.drawString("M5Guruguru-Starter", cx, headerY);

  displayQRCode(qrData, qrCenter);

  M5.Display.setTextSize(3);
  M5.Display.drawString("Scan QR to connect", cx, footerY);

  M5.Display.setTextSize(2);
  M5.Display.drawString("SSID: " + String(ap_ssid), cx, footerY + 36);
  M5.Display.drawString("Pass: " + String(ap_pass), cx, footerY + 58);

  MDNS.begin("m5guru");

  server.on("/", HTTP_GET, []() {
    File f = LittleFS.open("/index.html", "r");
    if (!f) {
      server.send(404, "text/plain", "index.html not found");
      return;
    }
    server.streamFile(f, "text/html");
    f.close();
  });

  server.on("/upload", HTTP_POST,
    []() {
      server.send(200, "text/plain", "Upload complete");
    },
    []() {
      HTTPUpload& upload = server.upload();
      static File uploadFile;

      if (upload.status == UPLOAD_FILE_START) {
        String filename = "/" + upload.filename;
        if (!filename.endsWith(".png")) {
          server.send(400, "text/plain", "PNG only");
          return;
        }
        uploadFile = LittleFS.open(filename, "w");
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (uploadFile) uploadFile.write(upload.buf, upload.currentSize);
      } else if (upload.status == UPLOAD_FILE_END) {
        if (uploadFile) uploadFile.close();
        drawFileStatus();
        if (!avatarReady && checkAvatarFilesReady()) {
          markAvatarReady();
          const int cx = M5.Display.width() / 2;
          const int readyY = 290 + 60;  // footerY + 60
          M5.Display.setTextDatum(TC_DATUM);
          M5.Display.setTextColor(TFT_GREEN);
          M5.Display.setTextSize(2);
          M5.Display.drawString("Ready to guruguru!", cx, readyY);
          M5.Display.drawString("Restart the device.", cx, readyY + 22);
        }
      }
    }
  );

  server.begin();

  if (checkAvatarFilesReady()) markAvatarReady();
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(0);

  // Display boot screen
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextDatum(MC_DATUM);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(3);
  M5.Display.drawString("M5Guruguru-Starter", M5.Display.width() / 2, M5.Display.height() / 2);

  prefs.begin("boot", true);  // read-only
  bootMode = prefs.getString("mode", "setup");
  prefs.end();

  Serial.printf("Boot mode: %s\n", bootMode.c_str());

  if (bootMode == "avatar") {
    M5.Imu.begin();

    // Get the size of the avatar image from the PNG file header (24 bytes)
    int imgW = 240, imgH = 240;
    LittleFS.begin(false);
    File f = LittleFS.open("/dir0.png", "r");
    if (f && f.size() >= 24) {
      uint8_t buf[24];
      f.read(buf, 24);
      imgW = (buf[16] << 24) | (buf[17] << 16) | (buf[18] << 8) | buf[19];
      imgH = (buf[20] << 24) | (buf[21] << 16) | (buf[22] << 8) | buf[23];
      f.close();
    }
    Serial.printf("Avatar image size: %dx%d\n", imgW, imgH);
    avatar.init(imgW, imgH);
  } else {
    setupUploadMode();
  }
}

void displayQRCode(const String& data, int yCenter) {
  const uint8_t version = 5;
  QRCode qrcode;
  uint8_t qrcodeData[qrcode_getBufferSize(version)];
  qrcode_initText(&qrcode, qrcodeData, version, ECC_LOW, data.c_str());

  int qrSize = qrcode.size;
  int scale = 185 / qrSize;
  int actualSize = scale * qrSize;
  int margin = 4;

  int xOffset = (M5.Display.width() - actualSize) / 2;
  int yOffset = yCenter - actualSize / 2;

  M5.Display.fillRect(xOffset - margin, yOffset - margin,
                      actualSize + margin * 2, actualSize + margin * 2, TFT_WHITE);

  for (int y = 0; y < qrSize; y++) {
    for (int x = 0; x < qrSize; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        M5.Display.fillRect(xOffset + x * scale, yOffset + y * scale, scale, scale, TFT_BLACK);
      }
    }
  }
}

void drawFileStatus() {
  const int col1X  = 91;
  const int col2X  = 207;   // 91+96+20
  const int startY = 105;
  const int rowH   = 18;

  M5.Display.setTextSize(2);
  M5.Display.setTextDatum(TL_DATUM);

  for (int i = 0; i < 9; i++) {
    int y = startY + i * rowH;

    String name1 = "dir" + String(i) + ".png";
    String name2 = "dir" + String(i) + "_close.png";

    M5.Display.setTextColor(LittleFS.exists("/" + name1) ? TFT_GREEN : TFT_RED);
    M5.Display.drawString(name1, col1X, y);

    M5.Display.setTextColor(LittleFS.exists("/" + name2) ? TFT_GREEN : TFT_RED);
    M5.Display.drawString(name2, col2X, y);
  }
}

void displayConnected() {
  const int cx      = M5.Display.width() / 2;
  const int footerY = 290;

  M5.Display.fillRect(0, 74, M5.Display.width(), M5.Display.height() - 74, TFT_BLACK);

  drawFileStatus();

  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextDatum(TC_DATUM);
  M5.Display.setTextSize(3);
  M5.Display.drawString("Open m5guru.local", cx, footerY);
  M5.Display.setTextSize(2);
  M5.Display.drawString("to upload", cx, footerY + 34);

  if (avatarReady) {
    M5.Display.setTextColor(TFT_GREEN);
    M5.Display.drawString("Ready to guruguru!", cx, footerY + 60);
    M5.Display.drawString("Restart the device.", cx, footerY + 82);
  }
}

void markAvatarReady() {
  avatarReady = true;
  prefs.begin("boot", false);
  prefs.putString("mode", "avatar");
  prefs.end();
  Serial.println("All avatar files ready. Boot mode set to avatar.");
}

bool checkAvatarFilesReady() {
  for (int i = 0; i < 9; i++) {
    String path = "/dir" + String(i) + ".png";
    if (!LittleFS.exists(path)) return false;
  }
  return true;
}

void loop() {
  M5.update();

  if (bootMode == "avatar") {
    float ax, ay, az;
    if (M5.Imu.getAccel(&ax, &ay, &az)) {
      avatar.setRotation(detectRotation(ax, ay));
    }
    if (M5.Touch.getCount() > 0) {
      auto p = M5.Touch.getDetail(0);
      if (p.isPressed()) avatar.trackFace(p.x, p.y);
    }

    // Push button B to return to setup mode (2-second press to reset and restart)
    static uint32_t btnBPressStart = 0;
    if (M5.BtnB.isPressed()) {
      if (btnBPressStart == 0) btnBPressStart = millis();
      if (millis() - btnBPressStart > 2000) {
        prefs.begin("boot", false);
        prefs.putString("mode", "setup");
        prefs.end();
        ESP.restart();
      }
    } else {
      btnBPressStart = 0;
    }
  } else {
    server.handleClient();

    static int prevStations = 0;
    int stations = WiFi.softAPgetStationNum();
    if (stations > 0 && prevStations == 0) {
      displayConnected();
    }
    prevStations = stations;

    delay(10);
  }
}
