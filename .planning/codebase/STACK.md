# 技术栈

**分析日期:** 2026-02-03

## 编程语言

**主要:**
- C++ (C++11/17) - ESP32 固件主要代码，位置: `src/main.cpp`
- Arduino 方言 (Arduino Framework for ESP32)

## 运行时

**环境:**
- ESP32-S3 开发板 (ESP32-S3-DevKitC-1)
- Espressif Arduino Framework (基于 Arduino Core for ESP32)

**处理器:**
- Xtensa 32-bit Dual-core (ESP32-S3)
- RAM: 320 KB (内部 SRAM)
- Flash: 16 MB (PSRAM)

## 构建系统

**项目管理:**
- PlatformIO 1.x - 构建、编译和上传工具

**构建配置:**
- 配置文件: `platformio.ini`
- 编译目标: `esp32-s3-devkitc-1`
- 平台: `espressif32`
- 框架: `arduino`

## 关键依赖

**必需库:**

- **WebSockets** (links2004/WebSockets@^2.4.1)
  - 用途: WebSocket 客户端通信，连接到 Mac 服务器 (`ws://host:8765/ws`)
  - 核心功能: 发送音频数据、接收 JSON 消息和 hook 事件

- **M5Unified** (m5stack/M5Unified@^0.2.8)
  - 用途: M5Stack Atom EchoS3R 硬件抽象层
  - 功能模块:
    - `M5.Mic` - I2S 麦克风录音 (16kHz, 单声道, 16-bit PCM)
    - `M5.Speaker` - 扬声器控制 (beep 声音)
    - `M5.BtnA` - 按钮输入 (推说话: 按住录音)

- **ArduinoJson** (bblanchon/ArduinoJson@^7.0.4)
  - 用途: JSON 序列化/反序列化
  - 使用场景:
    - 序列化: `start` 消息，包含令牌、请求ID、音频格式参数
    - 反序列化: 接收 `hook` 事件 JSON 并提取 event 类型

## 配置

**构建标志:**
```ini
-DARDUINO_USB_CDC_ON_BOOT=1
```
- 启用 USB CDC (虚拟串口)，用于调试和串行监控

**可选编译时标志:**
- `-DWS_HOSTNAME=\"your-mac.local\"` - 覆盖 Mac 服务器 mDNS 主机名

**运行时配置 (位置: `src/main.cpp` 第 8-22 行):**
- `WIFI_SSID` - WiFi 网络名称
- `WIFI_PASS` - WiFi 密码
- `WS_HOSTNAME` - Mac 服务器 mDNS 主机名 (默认: `jiabos-macbook-pro-2.local`)
- `WS_PORT` - WebSocket 端口 (默认: 8765)
- `AUTH_TOKEN` - 身份验证令牌 (必须与 Mac 服务器匹配)

## 音频格式常数

**录音参数 (位置: `src/main.cpp` 第 25-36 行):**
- 采样率: 16000 Hz
- 声道: 单声道 (1 channel)
- 位深: 16-bit 有符号小端字节序 (`pcm_s16le`)
- 分块大小: 320 样本 = 640 字节 (对应 20ms @ 16kHz)
- 最大录音时长: 8 秒 (安全上限)

## 平台要求

**开发:**
- PlatformIO CLI 或 IDE 扩展
- USB 串口驱动 (CP210x 或 CH340，取决于开发板)
- Python 3.6+ (PlatformIO 运行时)

**生产/部署:**
- ESP32-S3-DevKitC-1 开发板或兼容的 ESP32-S3 设备
- M5Stack Atom EchoS3R (或兼容的 I2S 麦克风和扬声器)
- WiFi 2.4GHz 连接
- 同一网络上运行的 Mac Whisper WebSocket ASR 服务器

---

*技术栈分析: 2026-02-03*
