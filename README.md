# G8 Display Controller

ESP32-S3 touch controller for a Samsung Odyssey OLED G8 display.

The firmware uses an 800×480 LVGL interface and sends HTTP requests to a Raspberry Pi backend that handles display control and HDMI-switch IR commands.

## Hardware

- Waveshare ESP32-S3-Touch-LCD-4.3
- Samsung Odyssey OLED G8
- Raspberry Pi running the controller backend
- Optional BroadLink RM4 Mini + HDMI switch

## UI controls

- PC
- Mac
- PS5
- Switch
- Display power
- Current source highlighting
- Backend online/offline status

## Backend API

The firmware expects these endpoints:

- `GET /api/status`
- `POST /api/source/pc`
- `POST /api/source/mac`
- `POST /api/source/ps5`
- `POST /api/source/switch`
- `POST /api/power/toggle`

## Arduino setup

Tested with:

- ESP32 by Espressif Systems: `3.0.7`
- Board: `ESP32S3 Dev Module`
- Flash Size: `16MB (128Mb)`
- PSRAM: `OPI PSRAM`
- Flash Mode: `QIO 80MHz`
- CPU Frequency: `240MHz (WiFi)`
- Partition Scheme: `16M Flash (3MB APP / 9.9MB FATFS)`
- USB CDC On Boot: `Disabled` when using the board's USB-to-UART port

Use the matching Waveshare demo libraries for LVGL and `ESP32_Display_Panel` rather than upgrading them independently.

## Configuration

Edit `secrets.h` before compiling:

```cpp
#define WIFI_SSID     "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define G8_API_BASE   "http://YOUR_PI_IP:5000"
```

Change `G8_API_BASE` to the Raspberry Pi address used on your network.

## Notes

The LVGL port uses double buffering with full refresh to avoid RGB framebuffer tearing/vertical wrap on this panel. Wi-Fi diagnostics are printed to the serial monitor at `115200` baud.
