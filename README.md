# G8 Display Controller

这是一个给 **Samsung Odyssey OLED G8** 做的桌面触摸控制器。

我不太喜欢每次切换设备都去摸显示器上的按键，所以做了一个 4.3 寸的小屏幕放在桌面上。平时可以直接在屏幕上切换 PC、Mac、PS5、Switch，也可以控制显示器电源。

目前整套方案由两部分组成：

- **ESP32-S3 触摸屏**：负责显示界面和接收触摸操作
- **Raspberry Pi**：负责真正向显示器和 HDMI 切换器发送控制命令

## 现在可以做什么

- 在 PC / Mac / PS5 / Switch 之间切换
- 显示当前正在使用的设备
- 控制显示器开关
- 显示控制器是否在线
- 自动从树莓派同步当前状态

界面是按照这块 4.3 寸屏幕的 **800×480** 分辨率专门做的，不是网页直接缩放过去的。

## 用到的设备

- Waveshare ESP32-S3-Touch-LCD-4.3
- Samsung Odyssey OLED G8
- Raspberry Pi
- BroadLink RM4 Mini（如果需要红外控制 HDMI 切换器）
- HDMI 切换器（可选）

只使用 G8 本身的输入源时，不一定需要 BroadLink 和 HDMI 切换器。

## 使用方法

先准备好 Waveshare 官方 Demo 里的 Arduino 库，然后用 Arduino IDE 打开：

```text
G8_Display_Controller.ino
```

这套工程目前使用的是：

```text
ESP32 by Espressif Systems  3.0.7
Board                       ESP32S3 Dev Module
Flash Size                  16MB (128Mb)
PSRAM                       OPI PSRAM
Partition Scheme            16M Flash (3MB APP / 9.9MB FATFS)
```

如果你使用的是板子上的 USB-to-UART 接口，`USB CDC On Boot` 建议设为 `Disabled`。

## 配置 Wi-Fi

烧录之前打开 `secrets.h`，填入自己的 Wi-Fi 和树莓派地址：

```cpp
#define WIFI_SSID     "YOUR_WIFI_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define G8_API_BASE   "http://YOUR_PI_IP:5000"
```

ESP32 和 Raspberry Pi 需要在同一个局域网里。

为了避免把家里的 Wi-Fi 密码上传到 GitHub，建议一直保留仓库里的占位内容，只在本地修改。

## 关于树莓派

ESP32 本身不直接控制 Samsung 显示器，也不直接处理 BroadLink。

它只负责把“切到 PC”“切到 Switch”“关闭显示器”这类操作发给树莓派。这样以后想修改显示器控制逻辑、红外逻辑或者新增设备时，不需要重新改整套屏幕驱动。

## 当前状态

目前已经完成并实际测试过：

- 屏幕显示正常
- 触摸正常
- Wi-Fi 连接正常
- PC / Switch 切换正常
- 显示器电源控制正常
- 当前设备状态同步正常

Mac 和 PS5 的控制逻辑可以根据实际接线继续补充。

## 一点说明

这块 Waveshare 屏幕在刷新方式上比较挑，工程里已经保留了目前测试稳定的显示配置。如果界面能够正常工作，不建议随意升级 LVGL、ESP32_Display_Panel 或修改刷新模式。

后续如果有新的设备、界面调整或者控制方式，会继续在这个仓库里更新。
