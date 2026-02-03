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
  "mode": "return_only",
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