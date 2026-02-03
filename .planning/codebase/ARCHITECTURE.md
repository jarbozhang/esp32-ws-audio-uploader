# 架构

**分析日期:** 2026-02-03

## 模式概述

**整体模式:** 事件驱动的嵌入式客户端

**关键特征:**
- 单一主程序文件（`src/main.cpp`）包含所有业务逻辑
- 基于硬件中断和事件轮询的事件处理
- WebSocket 长连接用于实时音频传输
- 推送通话（Push-to-Talk）UI 模式，由硬件按钮触发

## 层级

**应用层（Application）:**
- 位置: `src/main.cpp`
- 用途: 主业务逻辑、用户交互处理、协议状态管理
- 包含: 按钮处理、录音控制、WebSocket 连接管理、音频 Beep 播放
- 依赖: M5Unified（硬件）、WebSocket 客户端、ArduinoJson
- 使用: Arduino 框架直接调用

**通信层（Communication）:**
- 位置: `src/main.cpp` 中的 WebSocket 事件处理函数
- 用途: WebSocket 连接、JSON 消息序列化/反序列化、协议消息处理
- 包含: `webSocketEvent()`、`sendStart()`、`sendEnd()`、Hook 事件解析
- 依赖: WebSockets 库、ArduinoJson
- 使用: 应用层调用，Arduino 事件系统驱动

**硬件抽象层（Hardware）:**
- 位置: `src/main.cpp` 中的 M5Unified 接口
- 用途: 音频录制、播放、按钮输入、WiFi 连接
- 包含: `setupAudio()`、`recordOneChunkAndSend()`、`playPendingBeeps()`
- 依赖: M5Unified、WiFi 库
- 使用: 应用层通过 M5Unified API 使用

## 数据流

**ASR 录音流程（用户启动到完成）:**

1. 用户按住 Button A（通过轮询检测）
2. 应用层调用 `sendStart()` 发送 JSON 启动消息（包含 token、reqId、音频格式）
3. 录音循环：
   - `recordOneChunkAndSend()` 从麦克风缓冲区读取 320 个样本（20ms @ 16kHz）
   - 转换为 640 字节的二进制 PCM 数据
   - 通过 WebSocket 发送二进制帧到 Mac 服务器
4. 用户释放 Button A 或达到 8 秒超时
5. 应用层调用 `sendEnd()` 发送停止信号
6. Mac 服务器处理音频并返回 `result` JSON（包含识别文本）
7. 在此期间收集的任何 Hook 事件触发的 Beep 在录音停止后播放

**Hook 事件处理流程（服务器发起）:**

1. Mac 服务器广播 Hook 事件 JSON 消息（`type: "hook"`）
2. WebSocket 接收器调用 `webSocketEvent()` 的 WStype_TEXT 分支
3. `handleHookEvent()` 解析事件类型（PermissionRequest、PostToolUseFailure、Stop）
4. `queueBeep()` 根据录音状态决定立即播放或排队：
   - 如果正在录音：增加对应的待处理 Beep 计数
   - 如果未录音：立即切换到扬声器并播放
5. 录音停止时 `playPendingBeeps()` 按优先级播放所有排队的 Beep

**状态管理:**

- `wsConnected`: WebSocket 连接状态，由事件回调更新
- `recording`: 录音进行中标志，由按钮事件和超时控制
- `currentReqId`: 当前请求 ID，用于关联 start/end 消息
- `pendingStop/Permission/Failure`: 待播放 Beep 计数器，在非录音状态时消费
- `recentIds[]`: 16 元素环形缓冲区用于 Hook ID 去重（防止重复播放 Beep）

## 关键抽象

**WebSocket 协议处理:**
- 位置: `src/main.cpp` 中的 `sendStart()`、`sendEnd()`、`handleHookEvent()`、`webSocketEvent()`
- 用途: 客户端与 Mac 服务器的通信协议实现
- 模式:
  - 启动消息包含完整的音频格式元数据（采样率、通道数、位深度）
  - 音频数据作为原始二进制帧流传输（不是 JSON）
  - 接收端通过事件处理回调处理不同消息类型
  - 使用 `currentReqId` 关联请求-响应对

**音频 Beep 系统:**
- 位置: `src/main.cpp` 中的 `BeepKind`、`BeepPattern`、`queueBeep()`、`playPendingBeeps()`
- 用途: 异步播放音频提示，不阻塞录音
- 模式:
  - 三种 Beep 类型，每种有唯一的频率、持续时间、重复次数
  - 使用排队系统：录音期间收集所有事件，停止后批量播放
  - 播放优先级：PermissionRequest > PostToolUseFailure > Stop
  - 通过 M5Unified 的扬声器/麦克风独占机制实现（硬件约束）

**Hook 事件去重:**
- 位置: `src/main.cpp` 中的 `recentIds[]`、`seenId()`
- 用途: 防止同一事件被多次播放 Beep
- 模式: 简单的 16 元素环形缓冲区，存储最近看到的事件 ID，按发送顺序轮换

**音频分块系统:**
- 位置: `src/main.cpp` 中的 `CHUNK_SAMPLES`、`CHUNK_BYTES`、`recordOneChunkAndSend()`
- 用途: 将连续音频分割成可管理的网络包
- 规格: 20ms @ 16kHz = 320 样本 = 640 字节（16-bit PCM）
- 模式: 每次轮询循环调用一次，形成低延迟的流传输

## 入口点

**setup() 函数:**
- 位置: `src/main.cpp` 第 252-272 行
- 触发: Arduino 启动时自动调用一次
- 职责:
  - 初始化串行通信（调试）
  - 调用 `setupAudio()` 初始化 M5Unified 和音频设备
  - 连接 WiFi
  - 启动 WebSocket 客户端并注册事件回调

**loop() 函数:**
- 位置: `src/main.cpp` 第 274-307 行
- 触发: 每 1ms 重复执行一次（加 1ms 延迟）
- 职责:
  - 处理 WebSocket 事件（`ws.loop()`）
  - 更新 M5 硬件状态（`M5.update()`）
  - 轮询按钮状态，启动/停止录音
  - 如果录音中，尝试读取并发送一个音频块
  - 录音停止时播放待处理的 Beep

## 错误处理

**策略:** 故障安全、日志记录、用户通知

**模式:**

- **WiFi 连接失败:** 阻塞式等待（带 Serial 输出），直到连接成功；用户看到"连接中..."
- **WebSocket 断开:** 应用层知道 `wsConnected` 标志，按钮按下时警告用户；自动重新连接（2 秒间隔）
- **无效 JSON 响应:** 反序列化失败时记录原始文本到 Serial，继续执行（不中断）
- **音频读取失败:** `recordOneChunkAndSend()` 返回 false，该轮询循环跳过发送
- **超时保护:** 强制 8 秒录音上限（`MAX_RECORD_MS`）防止无限录音

## 跨越关注点

**日志记录:**
- 使用 Serial.println/printf 输出调试信息到串行监视器
- 关键事件：WiFi 连接、WebSocket 连接/断开、录音开始/停止、接收 JSON 消息

**验证:**
- WiFi 连接状态检查
- WebSocket 连接状态检查（`wsConnected` 标志）
- JSON 反序列化错误检查（`deserializeJson()` 返回值）
- Hook 事件 ID 存在性检查（防止处理空 ID）

**认证:**
- 在 `start` 消息中包含 `AUTH_TOKEN`（硬编码或通过编译标志注入）
- 服务器端验证 token；客户端无额外验证逻辑

---

*架构分析: 2026-02-03*
