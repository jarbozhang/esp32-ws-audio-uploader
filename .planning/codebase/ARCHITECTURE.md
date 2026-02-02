# 系统架构

**分析日期:** 2026-02-02

## 架构模式概述

**总体架构:** 嵌入式客户端-服务器 WebSocket 协议实现

**关键特征:**
- ESP32-S3 微控制器的单文件固件脚手架
- 客户端主动连接到远程 ASR 服务器
- 由 WebSocket 生命周期事件驱动的状态机
- 同步阻塞 I/O 与周期性轮询循环

## 架构分层

**硬件抽象层:**
- 目的: 在 Arduino 库级别管理 WiFi 和 WebSocket 通信
- 位置: Arduino SDK + links2004/WebSockets 库
- 包含: WiFi 连接、WebSocket 客户端生命周期
- 依赖: ESP32 硬件外设
- 被使用: `src/main.cpp` 中的应用逻辑

**应用层:**
- 目的: 控制音频录制工作流和协议消息
- 位置: `src/main.cpp`
- 包含: WebSocket 事件处理、消息序列化、设备设置
- 依赖: Arduino WiFi 和 WebSocket 库
- 被使用: 无依赖 (入口点)

## 数据流

**连接建立流程:**

1. `setup()` 初始化串口调试输出
2. WiFi 开始连接到配置的 SSID，使用静态凭证
3. 在阻塞循环中等待 WiFi 连接成功
4. WebSocket 客户端开始连接到 `ws://{WS_HOST}:{WS_PORT}{WS_PATH}`
5. 注册事件回调和重连间隔 (2000ms)

**音频协议序列 (已规划):**

1. WebSocket 连接 → 调用 `webSocketEvent()`，参数为 `WStype_CONNECTED`
2. `sendStart()` 序列化 JSON，包含音频格式参数 (16kHz, 16-bit, 单声道 PCM)
3. 通过 `ws.sendBIN()` 以二进制帧流式传输音频数据
4. `sendEnd()` 发送完成信号，包含音频块计数
5. 服务器返回包含转录文本的 JSON 结果
6. 在 `webSocketEvent()` 处理器中处理结果

**状态管理:**
- 请求 ID (`reqId`) 作为全局静态字符串维护，用于请求跟踪
- WebSocket 连接状态由库管理 (自动重连)
- 无显式状态机；通过回调的事件驱动

## 关键抽象

**WebSocket 协议:**
- 目的: 抽象网络通信和连接生命周期
- 示例: `links2004/WebSockets@^2.4.1` 库
- 模式: 事件回调模式 (`webSocketEvent()`)

**消息序列化:**
- 目的: 为 start、end 和音频数据构造协议消息
- 示例: `src/main.cpp` (lines 27-41) - `sendStart()`, `sendEnd()`
- 模式: 使用 JSON 结构的字符串拼接

**音频配置:**
- 目的: 为服务器定义音频捕获参数
- 示例: `SAMPLE_RATE=16000`, `CHANNELS=1`, `BIT_DEPTH=16`
- 模式: 模块级别的静态常量全局变量 (lines 17-20)

## 入口点

**setup():**
- 位置: `src/main.cpp` (line 63)
- 触发: ESP32 启动/重置
- 职责: 初始化串口、连接 WiFi、建立 WebSocket 连接

**loop():**
- 位置: `src/main.cpp` (line 83)
- 触发: setup 完成后持续轮询
- 职责: 轮询 WebSocket 消息、处理按钮输入 (TODO)、流式传输音频 (TODO)

**webSocketEvent():**
- 位置: `src/main.cpp` (line 43)
- 触发: WebSocket 连接/断开、收到消息
- 职责: 记录连接状态、连接时发送 start 消息、记录收到的消息

## 错误处理

**策略:** 脚手架阶段的最小错误处理

**模式:**
- WiFi 连接: 带延迟的阻塞循环，无超时
- WebSocket: 通过库自动重连 (2000ms 间隔)
- 协议: 无验证或错误恢复 (MVP 脚手架)
- 串口调试: 通过 `Serial.println()` 的有限日志记录

## 横切关注点

**日志记录:** 115200 波特率的串口 UART 输出 (Arduino 默认)。有限的调试输出显示连接状态和收到的消息。

**验证:** 未实现。静态凭证和硬编码的音频参数。TODO 注释表明缺少按钮输入和 I2S 录制的验证。

**认证:** 通过在 start 消息中发送的 `AUTH_TOKEN` 字符串进行基本的基于令牌的认证。静态常量，在当前脚手架中未被服务器验证。

---

*架构分析: 2026-02-02*
