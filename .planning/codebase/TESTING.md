# 测试模式

**分析日期:** 2026-02-02

## 测试框架

**运行器:**
- 未检测到测试框架
- Arduino/PlatformIO 项目通常使用平台特定的测试 (例如 PlatformIO 单元测试)
- 当前项目: MVP 阶段，未配置测试基础设施

**断言库:**
- 不适用

**运行命令:**
```bash
# 当前测试方法: 通过串口输出的手动验证
# 使用 PlatformIO 添加单元测试:
pio test                # 运行所有测试
pio test --environment esp32-s3  # 为特定环境运行测试
```

**配置文件:**
- 未找到 `platformio.ini` 测试配置
- 添加测试: 创建 `test/` 目录并配置 `platformio.ini`

## 测试文件组织

**当前状态:**
- 不存在测试文件
- 单文件架构: `src/main.cpp`
- 适合在专用 `test/` 目录中添加测试

**推荐位置:**
- `test/test_*.cpp` - 遵循 PlatformIO 约定的单元测试文件
- 项目根级别的 `test/` 目录

**命名约定:**
```
test/test_websocket.cpp
test/test_wifi_connection.cpp
test/test_message_format.cpp
```

## 测试结构

**Arduino 单元测试的建议模式:**

```cpp
#include <Arduino.h>
#include <unity.h>

// 测试设置
void setUp(void) {
  // 每个测试之前
}

// 测试清理
void tearDown(void) {
  // 每个测试之后
}

// 测试用例
void test_message_format(void) {
  TEST_ASSERT_EQUAL(expected, actual);
}

// 主测试运行器 (由 PlatformIO 自动生成)
void runUnityTests(void) {
  UNITY_BEGIN();
  RUN_TEST(test_message_format);
  UNITY_END();
}

void setup() {
  runUnityTests();
}

void loop() {}
```

**当前代码中观察到的模式:**
- 事件驱动架构: 没有模拟 WebSocket 库很难进行单元测试
- 全局状态 (`WebSocketsClient ws`, `String reqId`): 需要测试隔离
- 基于回调的设计: 测试需要注入模拟回调

## 模拟

**框架:**
- Arduino 测试通常使用直接硬件模拟或依赖注入
- 当前配置中未检测到模拟库

**推荐方法:**
```cpp
// 用于测试的模拟 WebSocket 客户端
class MockWebSocketsClient {
  String lastMessage;
  void sendTXT(String msg) {
    lastMessage = msg;
  }
  String getLastMessage() {
    return lastMessage;
  }
};

// 通过接口或模板注入到生产代码中
```

**需要模拟的内容:**
- `WebSocketsClient` - 第三方库依赖
- WiFi 连接 - 硬件依赖
- 串口输出 - 用于在测试中捕获日志

**不需要模拟的内容:**
- JSON 消息格式生成 - 要验证的核心逻辑
- 状态机逻辑 (连接 → 开始 → 流式传输 → 结束 → 断开)
- 音频参数配置

## 测试数据和固定装置

**测试数据模式:**
```cpp
const char* TEST_WIFI_SSID = "TEST_WIFI";
const char* TEST_WIFI_PASS = "test_password";
const char* TEST_AUTH_TOKEN = "test_token_123";
const char* TEST_REQ_ID = "test-req-1";

String getExpectedStartMessage() {
  return String("{\"type\":\"start\",\"token\":\"") + TEST_AUTH_TOKEN +
         "\",\"reqId\":\"" + TEST_REQ_ID + "\",...}";
}
```

**位置:**
- `test/fixtures/` 用于共享测试数据
- 测试文件顶部的常量用于隔离的测试套件

## 覆盖率

**要求:**
- 未配置覆盖率强制执行
- 推荐最低覆盖率: 核心 WebSocket 消息生成 (100%)
- 状态机转换 (>80%)

**查看覆盖率:**
```bash
pio test --verbose  # 显示测试输出但不显示覆盖率指标
# 对于覆盖率报告，在 platformio.ini 中配置覆盖率工具
```

## 测试类型

**单元测试:**
- 范围: 单独的函数，如 `sendStart()`, `sendEnd()`
- 方法: 模拟 WebSocket 客户端，验证消息格式和结构
- 示例: 测试 `sendStart()` 生成包含所有必需字段的正确 JSON

**集成测试:**
- 范围: WebSocket 通信流程 (连接 → 开始 → 结束)
- 方法: 模拟 WebSocket 库回调，验证状态转换
- 示例: 测试接收 `WStype_CONNECTED` 触发 `sendStart()`

**硬件集成测试:**
- 范围: WiFi 连接、I2S 音频录制 (实现时)
- 方法: 在实际 ESP32-S3 硬件上运行
- 测试命令: `pio run -t upload && pio device monitor`

**端到端测试:**
- 框架: 当前未使用
- 推荐: 使用实际 WebSocket 服务器进行手动测试 (在 README.md 中指定)
- 可以在核心逻辑完成后通过设备模拟/仿真实现

## 无测试的验证 (当前 MVP 模式)

**手动验证:**
- 通过 `pio device monitor` 以 115200 波特率监控串口输出
- WebSocket 服务器响应验证 (手动检查服务器日志)
- 通过服务器端解析进行消息格式验证 (隐含在服务器接受中)

**观察到的测试方法:**
- 通过 `Serial.println()` 和 `Serial.printf()` 进行调试输出，显示:
  - WiFi 连接状态
  - WebSocket 连接/断开事件
  - 接收到的消息内容和二进制数据长度

## 推荐的测试策略 (下一阶段)

**阶段 1 - 单元测试:**
1. 创建 `test/test_messages.cpp` 进行消息格式验证
2. 模拟 WebSocket 客户端以捕获发送的消息
3. 验证 JSON 结构、字段类型、必需参数

**阶段 2 - 集成测试:**
1. 创建 `test/test_protocol.cpp` 进行状态机测试
2. 按顺序模拟 WebSocket 事件
3. 验证正确的消息发送顺序 (开始 → 块 → 结束)

**阶段 3 - 硬件测试:**
1. 添加 I2S 音频录制实现
2. 测试音频缓冲区填充和帧传输
3. 验证音频数据格式 (16kHz, 16 位单声道)

---

*测试分析: 2026-02-02*
