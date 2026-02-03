# 规范（esp32-ws-audio-uploader）

本项目是整体系统中 ESP32 客户端侧的实现。

规范源文档位于 Obsidian：
`/Volumes/100.86.103.28/obsidian/20 Areas/Hardware/Claude Code/ESP32 + WebSocket 语音上传 + whisper.cpp ASR（Mac Node）- Spec.md`

## WebSocket

- URL：`ws://<mac-host>:8765/ws`
- 认证：在 `start` 消息中发送 `token`，必须与服务器的 `AUTH_TOKEN` 一致

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
