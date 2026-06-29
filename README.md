# M5Guruguru-Starter

Quick & easy starter for cute guruguru avatar on M5 devices

Built on the [M5GuruguruAvatar](https://github.com/nananauno/M5GuruguruAvatar) library, M5Guruguru-Starter is designed so that anyone can enjoy a custom avatar on their M5 device — no development environment required.

## Supported Devices

- [M5Stack StopWatch](https://shop.m5stack.com/products/m5stack-stopwatch-dev-kit-esp32-s3) (ESP32-S3 + 1.75" round AMOLED touch)

## Setup

### 1. Flash the firmware

1. Download and open [M5Burner](https://docs.m5stack.com/en/uiflow/m5burner/intro)
2. Select **STOPWATCH** from the device list
3. Find **M5Guruguru-Starter** and click **Download**, then **Burn**

### 2. Upload image files

1. Boot the device — it starts in **Setup Mode** and launches a Wi-Fi AP
2. Scan the QR code on screen to connect, or enter the credentials manually:
   - SSID: `M5Guruguru-Starter`
   - Password: `guruguru123`
3. Open `http://m5guru.local/` in a browser
4. Upload the following image files:

#### Required files (dir0.png – dir8.png)

Nine PNG files, one per direction. Any size works, but ~250×240px is recommended. The LittleFS partition is ~3.4 MB total, so keep all 18 files (Steps 1 & 2 combined) under that limit — roughly 180 KB per file on average:

| File | Direction |
|---|---|
| dir0.png | Top-Left |
| dir1.png | Top |
| dir2.png | Top-Right |
| dir3.png | Left |
| dir4.png | Center (Front) — Default |
| dir5.png | Right |
| dir6.png | Bottom-Left |
| dir7.png | Bottom |
| dir8.png | Bottom-Right |

#### Optional files (dir0_close.png – dir8_close.png)

Eyes-closed variants for each direction. Same naming as above with a `_close` suffix.

> For instructions on how to create avatar images, see the [M5GuruguruAvatar](https://github.com/nananauno/M5GuruguruAvatar) repository.

5. Once dir0–dir8 are all uploaded, **"Ready to guruguru!"** appears on screen
6. Restart the device — it will boot into Avatar Mode

## Modes

### Setup Mode

- Starts a Wi-Fi AP and accepts image uploads from a browser
- When a device connects, the screen switches to show upload status (green = uploaded, red = missing)
- Once dir0–dir8 are complete, the device remembers the setting so the next boot goes straight to Avatar Mode

### Avatar Mode

- Launches M5GuruguruAvatar and displays the character
- Auto-rotates based on IMU orientation
- Touch the screen to control the face direction

### Returning to Setup Mode

In Avatar Mode, **hold Button B for 2 seconds** to reset to Setup Mode and reboot automatically.

## License

This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

Character assets (images) follow their own license. Please respect the original creators.