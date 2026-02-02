# 代码库结构

**分析日期:** 2026-02-02

## 目录布局

```
esp32-ws-audio-uploader/
├── src/                    # 固件源代码
│   └── main.cpp           # 单一入口点，包含所有应用逻辑
├── platformio.ini         # PlatformIO 构建配置
├── README.md              # 项目概述和协议文档
├── SPEC.md                # Obsidian 规格文档的引用
├── LICENSE                # MIT 许可证
└── .gitignore             # 排除 .pio 和 .vscode 目录
```

## 目录用途

**src/:**
- 用途: Arduino/C++ 固件源代码
- 包含: 主应用入口点和协议实现
- 关键文件: `main.cpp` - 完整应用 (93 行)

## 关键文件位置

**入口点:**
- `src/main.cpp`: 单一 C++ 文件，包含 `setup()` 和 `loop()` Arduino 入口点

**配置:**
- `platformio.ini`: ESP32-S3 开发板选择、构建标志、库依赖

**文档:**
- `README.md`: 协议规范和项目目的
- `SPEC.md`: 外部 Obsidian 规格的链接

**构建产物:**
- `.pio/`: 生成的构建输出 (被 gitignore)
- `.vscode/`: IDE 设置 (被 gitignore)

## 命名约定

**文件:**
- `main.cpp`: 单一源文件 (Arduino 标准)
- `platformio.ini`: 标准 PlatformIO 配置名称

**函数:**
- 小驼峰命名: `sendStart()`, `sendEnd()`, `webSocketEvent()`, `setup()`, `loop()`

**变量:**
- 常量: 大写蛇形命名: `WIFI_SSID`, `WS_HOST`, `WS_PORT`, `SAMPLE_RATE`, `CHANNELS`, `BIT_DEPTH`, `AUTH_TOKEN`
- 静态模块变量: 小驼峰命名: `reqId`, `ws`

**类型:**
- 来自库的 Arduino/WebSocket 类型: `WStype_t`, `WebSocketsClient`

## 在何处添加新代码

**新功能 (例如按钮输入、I2S 录制):**
- 主要代码: 向 `src/main.cpp` 添加函数 (或如果超过 200 行则创建单独的 `.cpp/.h` 文件对)
- 测试: 创建 `test/` 目录，包含相应的测试文件 (尚不存在)

**新模块/组件:**
- 在 `src/` 中创建单独的 `.h` 头文件，包含声明
- 在 `src/` 中创建相应的 `.cpp` 实现文件
- 从 `main.cpp` 中包含

**工具/辅助函数:**
- 共享消息格式化: 在 `src/main.cpp` 的 `sendStart()`/`sendEnd()` 下方添加函数
- WiFi 工具: 如果 WiFi 逻辑增长超出设置范围，创建 `src/wifi_utils.h/cpp`
- 音频工具: 为 I2S 和采样率管理创建 `src/audio_utils.h/cpp`

**配置:**
- 硬编码参数: 在 `src/main.cpp` 顶部添加静态常量全局变量 (lines 6-20)
- 构建时设置: 添加到 `platformio.ini` 的 build_flags 或新的 ini 环境部分

## 特殊目录

**构建输出 (.pio):**
- 用途: PlatformIO 构建产物、编译的二进制文件、依赖项
- 生成: 是 (由 PlatformIO 自动生成)
- 提交: 否 (被 gitignore)

**IDE 配置 (.vscode):**
- 用途: Visual Studio Code 工作区设置和扩展推荐
- 生成: 手动 (用户配置)
- 提交: 否 (被 gitignore)

## 扩展代码库

**添加按钮输入 (PTT):**
1. 在 `main.cpp` 顶部添加 GPIO 引脚常量
2. 在 WiFi 之后的 `setup()` 中添加 `void initButton()`
3. 添加 `bool readButton()` 辅助函数
4. 在 `loop()` 中添加 PTT 状态跟踪逻辑

**添加 I2S 音频录制:**
1. 创建 `src/audio_capture.h/cpp` 用于 I2S 驱动抽象
2. 在 `main.cpp` 中包含并在 `setup()` 中调用初始化
3. 添加 `uint8_t* readAudioFrame()` 函数
4. 用 `if (pttPressed) { streamAudioFrames(); }` 替换 `loop()` 中的延迟

**添加消息解析:**
1. 创建 `src/ws_protocol.h/cpp` 用于 JSON 消息处理
2. 添加 `void parseServerResult(const char* json)` 函数
3. 在 TEXT 情况下从 `webSocketEvent()` 调用

---

*结构分析: 2026-02-02*
