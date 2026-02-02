# 编码规范

**分析日期:** 2026-02-02

## 命名模式

**文件:**
- C++ 源文件: `main.cpp` (蛇形命名)
- 配置文件: `platformio.ini` (小写加点)

**函数:**
- 函数名使用小驼峰命名: `sendStart()`, `sendEnd()`, `webSocketEvent()`, `setup()`, `loop()`
- 回调函数遵循 WebSocket 库约定: `webSocketEvent()`
- Arduino 生命周期函数: `setup()`, `loop()` (Arduino 框架要求)

**变量:**
- 局部变量和函数参数使用小驼峰命名: `reqId`, `chunks`, `type`, `payload`, `length`
- 常量使用大写蛇形命名: `WIFI_SSID`, `WIFI_PASS`, `WS_HOST`, `WS_PORT`, `WS_PATH`, `AUTH_TOKEN`, `SAMPLE_RATE`, `CHANNELS`, `BIT_DEPTH`
- 全局变量: 小驼峰命名: `ws`, `reqId`

**类型:**
- 直接使用 C++ 标准类型: `String`, `uint8_t`, `size_t`, `uint16_t`, `uint32_t`, `WStype_t`

## 代码风格

**格式化:**
- 未配置显式格式化工具 (无 .prettierrc、eslint 或 clang-format 文件)
- 观察到的风格:
  - 2 空格缩进 (在 switch 语句、函数体中观察到)
  - 控制结构的闭合大括号后无尾随分号
  - 同行大括号 (1TBS 风格): `if (...) { ... }`
  - 使用 `+` 运算符进行字符串拼接 (Arduino String 类)

**代码检查:**
- 未找到代码检查配置
- 代码风格由 Arduino/PlatformIO 框架约定隐式强制执行

## 导入组织

**顺序:**
1. Arduino 框架头文件: `#include <Arduino.h>`
2. 标准库头文件: `#include <WiFi.h>`
3. 第三方库头文件: `#include <WebSocketsClient.h>`

**路径别名:**
- 不适用 (C++ 包含，而非模块导入)

## 错误处理

**模式:**
- 阻塞同步检查: `while (WiFi.status() != WL_CONNECTED)` 配合轮询
- 错误状态的串口日志: `Serial.println("WS disconnected")`
- 未观察到异常处理 (Arduino/嵌入式环境)
- 回调通过事件类型检查处理错误状态: `webSocketEvent()` 中的 `WStype_DISCONNECTED` 情况
- 无 try-catch 或错误传播机制 (嵌入式系统典型做法)

## 日志记录

**框架:** Arduino Serial 库 (通过 USB CDC 的控制台输出)

**模式:**
- 连接状态: `Serial.println()` 用于主要状态变化 (WiFi 已连接、WS 已连接/已断开)
- 调试信息: `Serial.printf()` 用于格式化输出 (WS 消息、二进制数据长度)
- 输出格式: 带数值的人类可读状态消息
- 无结构化日志记录或日志级别

## 注释

**何时添加注释:**
- TODO 注释标记未完成的功能: `// TODO: replace with your WiFi + server`, `// TODO: implement PTT + I2S recording loop`
- 内联注释解释非显而易见的部分: `// In MVP we don't actually record from I2S yet.`
- 节注释划分主要配置区域: `// Audio settings (MVP constants)`

**JSDoc/TSDoc:**
- 不适用 (C++, 无 JSDoc/TSDoc 约定)
- 需要时通过内联注释进行函数文档

## 函数设计

**大小:**
- 小而专注的函数: `sendStart()` (9 行), `sendEnd()` (5 行), `webSocketEvent()` (16 行)
- Arduino 生命周期函数: `setup()` (19 行), `loop()` (7 行)

**参数:**
- 回调函数由库签名决定: `void webSocketEvent(WStype_t type, uint8_t * payload, size_t length)`
- 辅助函数采用特定参数: `sendEnd(uint32_t chunks)` 用于块计数

**返回值:**
- 事件处理器和生命周期方法的 `void` 返回类型
- 不使用返回值进行错误处理 (事件驱动模型)

## 模块设计

**导出:**
- 单文件架构: `src/main.cpp` 包含所有应用代码
- Arduino 框架入口点: `setup()` 和 `loop()` 被隐式导出/要求
- 全局 WebSocket 客户端实例: `WebSocketsClient ws` 可被事件处理器和生命周期函数访问

**Barrel 文件:**
- 不适用 (单文件 Arduino 草图架构)

## 配置管理

**常量:**
- 文件顶部的硬编码凭证 (MVP 模式): `WIFI_SSID`, `WIFI_PASS`, `AUTH_TOKEN`
- 分组的音频格式常量: `SAMPLE_RATE`, `CHANNELS`, `BIT_DEPTH`
- 服务器连接常量: `WS_HOST`, `WS_PORT`, `WS_PATH`

**说明:**
- 注释指出需要用配置替换硬编码值的 TODO
- 未检测到环境变量支持或外部配置文件

---

*规范分析: 2026-02-02*
