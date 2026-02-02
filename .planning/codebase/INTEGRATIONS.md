# 外部集成

**分析日期:** 2026-02-02

## API 和外部服务

**WebSocket ASR 服务器:**
- 服务: 基于 Mac 的 Whisper.cpp ASR (自动语音识别) 服务器
- 用途: 接收来自 ESP32 的 PCM 音频并返回语音转文本结果
- 代码仓库: `https://github.com/jarbozhang/mac-whisper-ws-asr-server`
- 协议: 基于 TCP/IP 的 WebSocket
- 连接详情:
  - 主机: 可配置 (当前硬编码为 `192.168.1.10`)
  - 端口: `8765`
  - 路径: `/ws`

## 网络配置

**WiFi:**
- 连接类型: WiFi STA (Station 模式)
- SSID: 可配置 (占位符: `YOUR_WIFI`)
- 密码: 可配置 (占位符: `YOUR_PASS`)
- IP 地址: 连接后通过 DHCP 获取

## 认证与身份

**认证提供者:**
- 自定义基于令牌的认证
- 实现: 在 WebSocket `start` 消息中发送 Bearer token
- 环境: 认证令牌在源码中硬编码为 `AUTH_TOKEN` (当前设置为 `change_me`)
- 令牌使用: 在 WebSocket 握手期间包含在 JSON 负载中

## WebSocket 协议

**接收 (来自服务器):**
- 包含识别文本的类型为 `result` 的 JSON 消息
- 连接状态消息 (已连接/已断开事件)

**发送 (到服务器):**
- JSON `start` 消息包含:
  - `type`: "start"
  - `token`: 认证令牌
  - `reqId`: 请求 ID 标识符
  - `format`: 音频格式 ("pcm_s16le")
  - `sampleRate`: 16000
  - `channels`: 1
  - `bitDepth`: 16
  - `mode`: "return_only" (仅返回文本，无中间结果)
- 二进制帧: 原始 PCM 音频数据 (s16le 格式)
- JSON `end` 消息包含:
  - `type`: "end"
  - `reqId`: 请求 ID
  - `chunks`: 发送的音频块数量

## 数据存储

**数据库:**
- 未使用 - 无状态实时处理

**文件存储:**
- 本地文件系统: 未使用
- 所有数据通过 WebSocket 实时传输

**缓存:**
- 未使用

## 监控与可观测性

**错误跟踪:**
- 未集成 - 仅本地错误处理

**日志:**
- 通过 USB CDC 的串口控制台输出 (115200 波特率)
- 调试消息打印到 Serial，包括 WiFi 状态、WebSocket 事件和音频指标

## 硬件集成

**I2S 麦克风:**
- 当前已规划但未实现 (MVP 脚手架)
- 将通过 ESP32-S3 上的 I2S 引脚连接
- 音频捕获规格: 16 kHz 采样率、16 位深度、单声道

**串口通信:**
- USB CDC (USB 转串口) 用于调试和监控
- 波特率: 115200

## 环境配置

**所需环境变量 (当前硬编码):**
- WiFi SSID 和密码
- WebSocket 服务器 IP 地址
- WebSocket 服务器端口 (8765)
- 认证令牌

**配置位置:**
- `src/main.cpp` - Lines 6-13 包含连接参数和凭证
- 计划: 移动到外部配置文件或 EEPROM 存储

## Webhooks 和回调

**传入 Webhooks:**
- 包含 ASR 结果的 WebSocket 文本帧
- WebSocket 事件 (CONNECTED, DISCONNECTED, TEXT, BIN)

**传出 Webhooks:**
- 未使用 - 仅单向推送到 WebSocket 服务器

## 配套基础设施

**Mac 端服务器:**
- URL: `https://github.com/jarbozhang/mac-whisper-ws-asr-server`
- 技术: 集成 Whisper.cpp 的 Node.js WebSocket 服务器
- 处理: 使用 OpenAI 的 Whisper 模型进行语音转文本转录

---

*集成审计: 2026-02-02*
