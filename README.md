# esp32-ws-audio-uploader

ESP32-S3 固件，通过 I2S 麦克风（M5Stack Atom EchoS3R / M5Unified）录制音频，并上传到 Mac WebSocket ASR 服务器。

配套服务器：
- https://github.com/jarbozhang/mac-whisper-ws-asr-server

## 功能说明

- **模块化代码结构**：将 `main.cpp` 拆分为 `Config.h`, `AudioManager`, `NetworkManager` 等模块，提升代码可读性和可维护性。
- **多网络 WiFi 连接**：支持配置多个 WiFi 网络，设备将自动扫描并连接到可用的网络。
- **mDNS 服务发现**：通过 mDNS 自动发现 Mac 服务器，无需硬编码 IP 地址。
- 通过 WebSocket 连接到 Mac 服务器：`ws://<mac-host>:8765/ws`
- 推送录音（Push-to-Talk）：
  - 按住 BtnA 开始录音
  - 以二进制 PCM 帧流式发送音频
  - 松开 BtnA 停止录音并请求 ASR 识别
- 监听 Mac 服务器转发的 Claude Code hook 事件广播，触发通知**蜂鸣音**。

## 配置

主要配置现在集中在 `src/Config.h` 文件中：
- **WiFi 网络**：在 `Config.h` 中定义 `WIFI_NETWORKS` 列表，包含 SSID 和密码。
  ```cpp
  static const std::vector<WiFiCredential> WIFI_NETWORKS = {
      {"YOUR_WIFI_SSID_1", "YOUR_WIFI_PASS_1"},
      {"YOUR_WIFI_SSID_2", "YOUR_WIFI_PASS_2"}
  };
  ```
- **认证令牌**：`AUTH_TOKEN`（必须与 Mac 服务器的 `AUTH_TOKEN` 一致）
- **Mac 主机地址**：
  - 默认使用 mDNS 主机名：`jiabos-macbook-pro-2.local`
  - 可在 `Config.h` 中修改 `WS_HOSTNAME`，或在编译时覆盖：
    - PlatformIO 编译标志：`-DWS_HOSTNAME="your-mac.local"`

## 协议（ASR）

ESP32 → Mac
- 连接 `ws://<mac-host>:8765/ws`
- 发送 JSON `start`（包含 token、reqId、音频格式参数）
- 以二进制帧流式发送音频块（原始 PCM）
- 发送 JSON `end`

Mac → ESP32
- 接收 JSON `ack`
- 接收 JSON `result`，其中包含 `text` 识别文本

音频格式（当前默认）：
- `format`: `pcm_s16le`
- `sampleRate`: 16000
- `channels`: 1
- `bitDepth`: 16

分块规格（当前配置）：
- 20ms @ 16kHz 单声道 s16 => 320 样本 => 每帧 640 字节

## 协议（Hook 事件广播）

Mac 服务器可广播 Claude Code hook 事件，以 JSON 文本帧形式发送：

```json
{ "type": "hook", "id": "uuid", "ts": 1730000000000, "hook_event_name": "Stop", "session_id": "..." }
```

ESP32 蜂鸣音规则（当前）：
- `PermissionRequest` → 蜂鸣音（需要授权）
- `PostToolUseFailure` → 蜂鸣音（工具执行失败）
- `Stop` → 蜂鸣音（Claude 输出结束）

注意：Atom EchoS3R 的麦克风和扬声器**不能同时使用**。
本固件**不会中断录音**；蜂鸣音会排队，在录音停止后统一播放。

## 测试与验证

项目包含完善的测试工具，位于 `test/` 和 `scripts/` 目录。

### 1. 手动验证 (Manual Check)
参考 `test/README.md` 中的检查清单，手动验证 WiFi 连接、mDNS 解析、WebSocket 连接、录音功能和音频质量。

### 2. 模拟服务器 (Mock Server)
使用轻量级 Python 模拟服务器验证 ESP32 协议，无需启动完整的 ASR 后端。

```bash
pip install websockets
python scripts/mock_server.py
```
**控制命令**：
- `p`: 模拟发送 `PermissionRequest` Hook 事件
- `f`: 模拟发送 `PostToolUseFailure` Hook 事件
- `s`: 模拟发送 `Stop` Hook 事件

### 3. 设备端单元测试 (On-Device Unit Tests)
使用 Unity 框架在 ESP32 硬件上运行单元测试。

```bash
pio test -e esp32-s3
```

## 构建与刷写（PlatformIO）

本项目使用 PlatformIO + Arduino 框架。

### 前置要求

PlatformIO 需要 **Python 3.10–3.13**。如果系统 Python 不满足要求（例如 Homebrew 的受管理环境或 Anaconda base < 3.10），需要先创建独立环境：

```bash
# Anaconda / Miniconda
conda create -n pio python=3.12 -y
conda activate pio
pip install platformio pyyaml          # pyyaml 是 espressif32 平台构建器所需的隐式依赖
```

> `pyyaml` 被 ESP32 Arduino 框架构建器隐式引用，但未声明为 PlatformIO 依赖。如果出现 `ModuleNotFoundError: No module named 'yaml'`，需要手动安装。

### 首次构建

第一次运行 `pio run` 会自动下载：
- **espressif32** 平台 + Xtensa 工具链
- **Arduino ESP32** 框架（IDF 5.5）
- 库：WebSockets 2.7.3、M5Unified 0.2.13（+M5GFX）、ArduinoJson 7.4.2

```bash
pio run                  # 仅编译（首次运行时下载所有依赖）
pio run -t upload        # 编译并刷写固件到设备
pio device monitor       # 打开串口监控（11200 baud）
```

### 已验证的构建输出（2026-02-03）

| 资源 | 已用 | 总量 |
|------|------|------|
| RAM | 50 KB (15.4%) | 328 KB |
| Flash | 1.3 MB (40.2%) | 3.3 MB |

### 已知编译警告（无影响）

- `WebSocketsClient.cpp`：`flush()` 弃用提示 — 上游库问题，无功能影响。
- `main.cpp`：`StaticJsonDocument` 在 ArduinoJson v7 中弃用 — 可替换为 `JsonDocument`，当前运行无影响。

## 备注

规范源文档（Obsidian）：
`/Volumes/100.86.103.28/obsidian/20 Areas/Hardware/Claude Code/ESP32 + WebSocket 语音上传 + whisper.cpp ASR（Mac Node）- Spec.md`
