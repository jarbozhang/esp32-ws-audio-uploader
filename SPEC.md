# 规范（esp32-ws-audio-uploader）

本项目是整体系统中 ESP32 客户端侧的实现。

规范源文档位于 Obsidian：
`/Volumes/100.86.103.28/obsidian/20 Areas/Hardware/Claude Code/ESP32 + WebSocket 语音上传 + whisper.cpp ASR（Mac Node）- Spec.md`

## 客户端架构（Phase 2: Refactor and Advanced Connectivity）

为了提高代码的可维护性和功能扩展性，客户端固件已从单一文件结构重构为模块化设计：

-   **`src/Config.h`**：集中管理所有配置参数，包括 WiFi 凭据列表、WebSocket 服务器地址、认证令牌以及音频常量等。
-   **`src/AudioManager.h/cpp`**：封装与 M5Unified 库相关的音频输入（麦克风）、输出（扬声器）以及蜂鸣音播放逻辑。负责音频数据的采集和蜂鸣音的排队/播放。
-   **`src/NetworkManager.h/cpp`**：处理所有网络相关的任务，包括多网络 WiFi 连接管理（使用 `WiFiMulti`）、mDNS 服务发现（解析 WebSocket 服务器主机名）、以及 WebSocket 客户端通信的生命周期管理。
-   **`src/main.cpp`**：作为主协调器，仅负责初始化 `AudioManager` 和 `NetworkManager`，并在主循环中调用它们的更新方法，实现模块间的协作。

## 测试与验证（Phase 3: Generate Testing Methods）

为确保系统稳定性，引入了分层测试策略：

1.  **手动验证清单 (`test/README.md`)**：
    -   详细列出了针对 Connectivity（连接性）、Audio（音频功能）、Latency（延迟）和 Hook Events（事件响应）的验收标准和操作步骤。

2.  **模拟服务器 (`scripts/mock_server.py`)**：
    -   轻量级 Python 脚本，基于 `websockets` 库。
    -   **功能**：
        -   接收并校验 `start`/`end` JSON 消息结构。
        -   接收并记录音频二进制数据长度。
        -   提供 CLI 交互界面，可手动触发 `PermissionRequest`、`PostToolUseFailure`、`Stop` 等 Hook 事件，用于验证 ESP32 的事件处理和蜂鸣音反馈逻辑。

3.  **设备端单元测试 (`test/test_main.cpp`)**：
    -   基于 `Unity` 框架，直接在 ESP32 硬件上运行。
    -   验证 `Config` 加载的正确性（如 WiFi 列表非空）。
    -   验证 `NetworkManager` 和 `AudioManager` 的初始状态。

## WebSocket

-   URL：`ws://<mac-host>:8765/ws`
    -   `<mac-host>` 现在通过 **mDNS 服务发现**动态解析，例如 `jiabos-macbook-pro-2.local`。客户端会自动尝试解析配置的 mDNS 主机名以获取服务器 IP。
-   认证：在 `start` 消息中发送 `token`，必须与服务器的 `AUTH_TOKEN` 一致

### Start（启动录音）
```json
{
  "type": "start",
  "token": "...",
  "reqId": "...",
  "mode": "paste",
  "format": "pcm_s16le",
  "sampleRate": 16000,
  "channels": 1,
  "bitDepth": 16
}
```

### Audio（音频数据）
二进制帧：原始 PCM 字节。

默认音频格式：
- 采样率 16kHz
- 单声道
- 16-bit 有符号小端序（`pcm_s16le`）

### End（停止录音）
```json
{ "type": "end", "reqId": "..." }
```

## Hook 事件广播（服务器 → ESP32）

Mac 服务器可能广播 hook 事件：
```json
{ "type": "hook", "id": "uuid", "ts": 1730000000000, "hook_event_name": "Stop" }
```

ESP32 使用这些事件触发蜂鸣音（录音期间排队，停止后播放）。
