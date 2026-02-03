# 外部集成

**分析日期:** 2026-02-03

## APIs 和外部服务

**WebSocket 音频流服务:**
- **Mac Whisper WebSocket ASR 服务器**
  - 地址: `ws://<mac-hostname>:8765/ws`
  - 默认主机: `jiabos-macbook-pro-2.local` (可通过 `-DWS_HOSTNAME` 构建标志覆盖)
  - 端口: 8765
  - 路径: `/ws`
  - 用途: 发送实时音频（PCM 二进制帧）进行自动语音识别 (ASR)，接收文字转录结果
  - SDK: `WebSocketsClient` (links2004/WebSockets@^2.4.1)
  - 认证: 令牌方式，在 `start` JSON 消息中发送 `token` 字段

**Hook 事件广播 (来自 Mac 服务器):**
- 用途: Mac 服务器转发 Claude Code hook 事件
- 格式: JSON 文本帧
- 示例:
  ```json
  { "type": "hook", "id": "uuid", "ts": 1730000000000, "hook_event_name": "Stop", "session_id": "..." }
  ```
- 实现: `webSocketEvent()` 函数处理类型为 `"hook"` 的消息，`handleHookEvent()` 解析事件名称

## 数据存储

**数据库:**
- 不使用数据库 - 这是嵌入式设备固件，不保存持久数据

**文件存储:**
- 不使用文件存储 - 音频数据直接流式传输到服务器

**缓存:**
- 无专用缓存服务
- 本地缓存: 待处理 beep 队列 (`pendingStop`, `pendingPermission`, `pendingFailure`)
- 重复数据删除环: 最近 16 个 hook ID (`recentIds[16]`)，防止重复处理相同事件

## 身份验证和身份识别

**认证提供者:**
- 自定义令牌认证
  - 实现位置: `src/main.cpp` 第 141-156 行 (`sendStart()` 函数)
  - 令牌值: `AUTH_TOKEN` 常数 (第 22 行)
  - 传输方式: 在 WebSocket `start` JSON 消息中作为 `"token"` 字段发送
  - 要求: 令牌必须与 Mac 服务器的 `AUTH_TOKEN` 完全匹配

**设备标识:**
- 请求 ID: 由 `makeReqId()` 函数生成，格式 `"req-{efuse_mac_hex}-{millis}"`
  - 用途: 关联请求和响应，支持多个并发请求
  - 实现位置: `src/main.cpp` 第 134-137 行

## 监控和可观察性

**错误追踪:**
- 无专门服务
- 本地日志: 通过 `Serial.println()` 和 `Serial.printf()` 输出到串口监视器

**日志:**
- 目标: 串口 UART (115200 baud)
- 日志点:
  - WiFi 连接状态和 IP 地址
  - WebSocket 连接/断开事件
  - 录音开始/停止事件
  - 接收的 JSON 消息 (hook 事件和 ASR 响应)
  - 按钮按压事件和状态警告

**调试:**
- 命令: `pio device monitor` - 实时查看串口日志

## CI/CD 和部署

**托管:**
- 目标设备: ESP32-S3 开发板 (物理设备)
- 不涉及云托管

**部署流程:**
- 使用 PlatformIO 构建和上传
- 命令:
  - `pio run -t upload` - 编译并上传固件到设备
  - `pio device monitor` - 监控执行日志

**CI 管道:**
- 无自动化 CI/CD (单个嵌入式设备项目)

## 环境配置

**必需的环境变量:**
- 无通过环境变量配置
- 所有配置硬编码在 `src/main.cpp` (第 8-23 行)

**编译时配置:**
- `WS_HOSTNAME` - 可通过 `-DWS_HOSTNAME="hostname"` 构建标志设置
- 示例: `platformio.ini` 中添加 `build_flags = -DWS_HOSTNAME=\"custom-mac.local\"`

**运行时配置需求:**
- WiFi SSID 和密码 (编辑 `WIFI_SSID` 和 `WIFI_PASS`)
- 认证令牌 (编辑 `AUTH_TOKEN`)

**密钥位置:**
- 硬编码在源代码中 (`src/main.cpp`)
- 无密钥管理系统
- 警告: 生产环境应考虑使用 ESP32 NVS (Non-Volatile Storage) 安全存储敏感凭证

## Webhooks 和回调

**传入 (ESP32 接收):**

1. **WebSocket 连接事件**
   - 事件: `WStype_CONNECTED` 和 `WStype_DISCONNECTED`
   - 处理: `webSocketEvent()` 回调函数 (第 187-223 行)
   - 动作: 更新 `wsConnected` 标志

2. **ASR 响应消息**
   - JSON 消息类型: `"ack"` (确认接收) 和 `"result"` (转录结果文本)
   - 处理: 解析 JSON，日志输出
   - 位置: `webSocketEvent()` 第 197-218 行

3. **Hook 事件广播**
   - JSON 消息类型: `"hook"`
   - 支持的事件:
     - `"PermissionRequest"` → 触发权限 beep (2000Hz, 80ms)
     - `"PostToolUseFailure"` → 触发失败 beep (800Hz, 200ms)
     - `"Stop"` → 触发停止 beep (1800Hz, 60ms)
   - 处理: `handleHookEvent()` 函数 (第 167-185 行)
   - 去重: 基于 `id` 字段，使用 16 元素环形缓冲区 `recentIds[]`

**传出 (ESP32 发送):**

1. **start 消息 (记录初始化)**
   - 触发: 用户按住 BtnA 且 WebSocket 已连接
   - 数据:
     ```json
     {
       "type": "start",
       "token": "...",
       "reqId": "...",
       "mode": "return_only",
       "format": "pcm_s16le",
       "sampleRate": 16000,
       "channels": 1,
       "bitDepth": 16
     }
     ```
   - 实现: `sendStart()` 函数 (第 141-156 行)

2. **Audio 数据帧 (二进制)**
   - 触发: 录音时持续发送
   - 数据: 原始 PCM 二进制，320 样本 = 640 字节
   - 频率: ~20ms 间隔（由 M5.Mic.record() 确定）
   - 实现: `recordOneChunkAndSend()` 函数 (第 239-250 行)

3. **end 消息 (记录完成)**
   - 触发: 用户释放 BtnA 或达到 8 秒最大录音时长
   - 数据:
     ```json
     {
       "type": "end",
       "reqId": "..."
     }
     ```
   - 实现: `sendEnd()` 函数 (第 158-165 行)

## 硬件集成

**I2S 麦克风 (输入):**
- 设备: M5Stack Atom EchoS3R 内置麦克风 (通过 M5Unified 抽象)
- 连接: I2S 接口 (ESP32-S3 内置)
- 参数: 16kHz, 单声道, 16-bit PCM
- 驱动: M5Unified `M5.Mic` 模块

**扬声器 (输出):**
- 设备: M5Stack Atom EchoS3R 内置扬声器
- 用途: 播放 beep 通知声音
- 限制: **不能同时使用麦克风和扬声器** → beeps 被队列化并在录音停止后播放

**WiFi (网络):**
- 驱动: ESP32 内置 WiFi (802.11 b/g/n 2.4GHz)
- 配置: Arduino WiFi API (`WiFi.begin()`, `WiFi.status()`)

## 通信协议

**WebSocket:**
- 协议版本: WebSocket RFC 6455 (通过 links2004/WebSockets 库实现)
- 消息类型: 文本 (JSON) 和二进制 (PCM 音频)
- 重连: 自动重连间隔 2000ms

**WiFi:**
- 模式: STA (Station / WiFi 客户端)
- 连接: `WiFi.begin(SSID, PASS)` 阻塞连接

---

*集成审计: 2026-02-03*
