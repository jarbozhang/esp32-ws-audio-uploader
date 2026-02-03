# 测试模式

**分析日期：** 2026-02-03

## 测试框架

**运行器：**
- 未使用标准单元测试框架
- 目标平台为嵌入式设备（ESP32-S3），不适合传统 xUnit 框架
- 测试方式：手动集成测试 + 串口监控

**断言库：**
- 无（嵌入式应用直接使用硬件）

**运行命令：**
```bash
# PlatformIO 编译和上传
pio run -t upload                    # 编译并刷写固件

# 监控串口输出
pio device monitor                   # 波特率 115200（platformio.ini 定义）

# 清理构建
pio run -t clean                     # 清除构建文件
```

## 测试文件组织

**位置：**
- 当前无专门测试文件
- 位置：`src/main.cpp` - 单个应用文件，包含全部逻辑

**命名：**
- 不适用（未分离测试）

**结构：**
```
src/
└── main.cpp           # 唯一源文件，包含所有功能和测试点
```

## 测试结构

**手动集成测试点：**

```cpp
// 路径 src/main.cpp - WiFi 连接测试 (行 258-267)
Serial.begin(115200);
delay(200);
setupAudio();

WiFi.mode(WIFI_STA);
WiFi.begin(WIFI_SSID, WIFI_PASS);
Serial.print("WiFi connecting");
while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
}
Serial.println();
Serial.print("WiFi OK, IP=");
Serial.println(WiFi.localIP());

ws.begin(WS_HOST, WS_PORT, WS_PATH);
ws.onEvent(webSocketEvent);
```

**测试模式：**
- 设置阶段：`setup()` 函数初始化硬件和网络
- 运行循环阶段：`loop()` 函数持续处理输入和事件
- 断言方式：串口输出验证状态（`Serial.println()`）

## 模拟（Mocking）

**框架：** 不适用

**模拟策略：**
- 无代码级模拟，使用真实硬件测试
- 依赖库函数：
  - `M5Unified`：完整初始化和运行
  - `WebSocketsClient`：真实 WebSocket 连接
  - `ArduinoJson`：真实 JSON 序列化/反序列化

**与真实资源互动：**
```cpp
// 路径 src/main.cpp - 行 225-237
// 实际初始化 M5Stack 硬件
static void setupAudio() {
  auto cfg = M5.config();
  M5.begin(cfg);

  auto micCfg = M5.Mic.config();
  micCfg.noise_filter_level = (micCfg.noise_filter_level + 8) & 255;
  M5.Mic.config(micCfg);

  M5.Speaker.end();
  M5.Mic.begin();
}
```

**模拟的内容：**
- 无：所有测试依赖硬件正确配置

## 测试数据与工具

**测试数据：**
- 音频缓冲区：`static int16_t buf[CHUNK_SAMPLES];` (行 240)
  - 320 个样本，16-bit PCM，16kHz 单声道
  - 20ms 块大小

**测试工具：**
```bash
# 1. PlatformIO 编译验证
pio run

# 2. 串口监控（验证运行时输出）
pio device monitor

# 3. 手动硬件测试
# - 连接 WiFi，检查 "WiFi OK" 消息
# - 按 BtnA 开始录音，检查 "Recording start" 消息
# - 释放 BtnA 停止录音，检查 "Recording stop" 消息
# - 验证蜂鸣音播放（停止后）
# - 检查 WebSocket 消息收发
```

## 覆盖范围

**要求：** 无强制覆盖率要求

**可验证功能：**

**WiFi 连接流程：**
```cpp
// 路径 src/main.cpp - 行 258-267
// 串口输出验证：
// "WiFi connecting..."
// "WiFi OK, IP=192.168.x.x"
```

**WebSocket 连接流程：**
```cpp
// 路径 src/main.cpp - 行 269-271
ws.begin(WS_HOST, WS_PORT, WS_PATH);
ws.onEvent(webSocketEvent);
ws.setReconnectInterval(2000);

// 预期串口输出：
// "WS connected"
// "WS disconnected"
```

**按钮和录音逻辑：**
```cpp
// 路径 src/main.cpp - 行 279-303
// 手动测试：
// 1. 按下 BtnA → "Recording start" 输出
// 2. 保持按下 → 音频块持续发送
// 3. 释放 BtnA → "Recording stop" 输出
// 4. 蜂鸣音播放（如有待处理）
```

**JSON 解析与事件处理：**
```cpp
// 路径 src/main.cpp - 行 203-218
// 测试接收：
// - 有效 hook 事件 → 调用 handleHookEvent()
// - 其他 JSON → 串口打印
// - 非 JSON 文本 → "WS text (non-json): ..." 输出
```

## 常见测试场景

**正常流程：**
1. 启动设备 → WiFi 连接
2. 等待 WebSocket 连接到 Mac 服务器
3. 按 BtnA 开始录音，流式发送 PCM 数据
4. 释放 BtnA 停止录音，发送 `end` 消息
5. 接收 ASR 结果
6. 接收 hook 事件，蜂鸣音反馈

**异常情况：**

**WiFi 未连接：**
```cpp
// 路径 src/main.cpp - 行 280-281
if (!wsConnected) {
  Serial.println("BtnA pressed but WS not connected");
}
```
- 预期：按下 BtnA 时，不启动录音

**WebSocket 断开：**
```cpp
// 路径 src/main.cpp - 行 189-191
case WStype_DISCONNECTED:
  wsConnected = false;
  Serial.println("WS disconnected");
```
- 预期：自动重连（2 秒间隔）

**录音超时：**
```cpp
// 路径 src/main.cpp - 行 294
if (M5.BtnA.wasReleased() || (millis() - recordStartMs) > MAX_RECORD_MS) {
```
- 预期：8 秒后自动停止录音

**Hook 事件去重：**
```cpp
// 路径 src/main.cpp - 行 169-170
if (id.length() && seenId(id)) return;
```
- 预期：重复的 hook ID 被忽略

## 手动测试清单

**硬件准备：**
- [ ] ESP32-S3 设备（如 M5Stack Atom EchoS3R）
- [ ] 配置 WiFi SSID 和密码（src/main.cpp 第 9-10 行）
- [ ] 配置 AUTH_TOKEN 与 Mac 服务器匹配（src/main.cpp 第 22 行）
- [ ] 配置 WS_HOSTNAME 为 Mac 主机名（src/main.cpp 第 14-16 行）
- [ ] Mac 服务器运行：https://github.com/jarbozhang/mac-whisper-ws-asr-server

**编译和刷写：**
- [ ] `pio run` - 编译不出错
- [ ] `pio device monitor` - 打开串口，波特率 115200
- [ ] `pio run -t upload` - 刷写固件，重启设备

**功能验证：**
- [ ] 串口输出 "WiFi connecting..." 然后 "WiFi OK"
- [ ] 串口输出 "WS connected"
- [ ] 按下 BtnA：输出 "Recording start"，音频发送
- [ ] 释放 BtnA：输出 "Recording stop"
- [ ] 设置 MAX_RECORD_MS 为较小值，验证超时停止
- [ ] Mac 服务器发送 hook 事件，设备蜂鸣
- [ ] 验证蜂鸣音优先级（权限 > 失败 > 停止）
- [ ] 验证去重（同 ID hook 事件只处理一次）

**网络测试：**
- [ ] 断开 WiFi，验证重连行为
- [ ] 杀死 Mac 服务器进程，验证 WebSocket 断开和自动重连
- [ ] 在网络中移动设备，验证稳定性

---

*测试模式分析日期：2026-02-03*
