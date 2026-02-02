# 技术栈

**分析日期:** 2026-02-02

## 编程语言

**主要语言:**
- C++ - ESP32-S3 微控制器的固件代码

**次要语言:**
- Arduino (C++ 方言) - Arduino 框架和平台特定 API

## 运行时环境

**环境:**
- ESP32-S3 微控制器 (Xtensa LX7 双核处理器)
- Arduino 框架

**包管理器:**
- PlatformIO - 嵌入式系统开发平台和依赖管理器

## 框架

**核心框架:**
- Arduino Framework - 为 ESP32 提供硬件抽象层

**网络连接:**
- WiFi (ESP32 内置)
- WebSockets 客户端库 - `links2004/WebSockets@^2.4.1`

**音频:**
- I2S (Inter-IC Sound) - 麦克风通信的硬件协议 (已规划，尚未实现)

## 关键依赖

**核心依赖:**
- `links2004/WebSockets@^2.4.1` - ESP32 的 WebSocket 客户端库，支持与 WebSocket ASR 服务器的双向通信

**硬件库:**
- WiFi - ESP32 内置 WiFi 功能
- Serial - 内置 UART 用于调试

## 配置

**环境配置:**
- 通过 `platformio.ini` 配置
- WiFi 凭证: SSID 和密码 (当前为源码中的占位符)
- WebSocket 服务器地址和端口硬编码在源码中
- 认证令牌硬编码在源码中

**构建配置:**
- `platformio.ini` - PlatformIO 项目配置
  - 平台: espressif32
  - 开发板: esp32-s3-devkitc-1
  - 监视器波特率: 115200
  - 构建标志: `-DARDUINO_USB_CDC_ON_BOOT=1` (启用通过 CDC 的 USB 串口)

## 平台要求

**开发环境:**
- PlatformIO CLI 或 VS Code PlatformIO 扩展
- USB 连接到 ESP32-S3 开发板

**硬件:**
- ESP32-S3-DevKitC-1 开发板
- I2S 麦克风 (计划在未来实现)
- WiFi 网络接入

**生产环境:**
- ESP32-S3 模块或开发板
- WiFi 网络连接
- 访问已知 IP 和端口 8765 的 Mac WebSocket ASR 服务器

## 音频配置

**规格参数:**
- 采样率: 16 kHz
- 声道: 1 (单声道)
- 位深度: 16-bit
- 格式: PCM 有符号 16 位小端序 (pcm_s16le)

---

*技术栈分析: 2026-02-02*
