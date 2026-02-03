# 编码规范

**分析日期：** 2026-02-03

## 命名规范

**文件命名：**
- 单个主文件：`main.cpp` - 应用程序入口
- 约定：使用小写字母，单词之间用下划线分隔（如果需要多文件时）

**函数命名：**
- 驼峰命名法（camelCase）：`setupAudio()`、`recordOneChunkAndSend()`、`sendStart()`
- 静态函数前缀 `static`：`static void setupAudio()`、`static bool recordOneChunkAndSend()`
- 用途清晰的动词开头：`setup*`、`send*`、`record*`、`play*`、`queue*`、`handle*`

**变量命名：**
- 全局变量/静态变量：使用 camelCase，`wsConnected`、`recording`、`currentReqId`
- 局部变量：camelCase，`recordStartMs`、`micCfg`、`ok`
- 常量：全大写 + 下划线：`MAX_RECORD_MS`、`SAMPLE_RATE`、`CHUNK_SAMPLES`、`WS_HOSTNAME`
- 缓冲区/数组：描述性名称，`buf`（局部缓冲区）、`recentIds`（ID去重环）

**类型命名：**
- 枚举类型：PascalCase，`BeepKind`、`WStype_t`（来自库）
- 结构体：PascalCase，`BeepPattern`
- 枚举值：全大写 + 下划线，`BEEP_STOP`、`BEEP_PERMISSION`、`BEEP_FAILURE`、`WStype_DISCONNECTED`

## 代码风格

**格式化：**
- 缩进：2空格（Arduino/PlatformIO 默认）
- 大括号：K&R 风格（开括号在同一行）
- 行长度：无严格限制，但一般保持可读性

**注释：**
- 中文和英文混用，关键概念用英文
- 行注释用 `//`，块注释用 `/* */`
- 配置段落用分隔符：`// ========= User config =========`
- 实现细节注释简洁明了，例如：
  ```cpp
  // Chunking: 20ms @16kHz => 320 samples => 640 bytes (s16)
  // Optional: keep pending from earlier; currently reset on each recording.
  // Priority: permission > failure > stop
  ```

## 导入组织

**包含顺序：**
1. Arduino 标准库：`#include <Arduino.h>`
2. 通信库：`#include <WiFi.h>`、`#include <WebSocketsClient.h>`
3. 数据处理库：`#include <ArduinoJson.h>`
4. 硬件库：`#include <M5Unified.h>`

## 错误处理

**模式：**
- 布尔返回值表示成功/失败：`bool recordOneChunkAndSend()`、`bool M5.Mic.record()`
- 显式检查连接状态：`if (!wsConnected) return false;`
- 显式检查资源可用性：`if (!M5.Mic.isEnabled()) return false;`
- 简单错误上报：`Serial.printf()`
- 无异常处理（嵌入式 C++ 常见做法）

**常见检查：**
```cpp
// 路径 src/main.cpp - 行 243-246
if (!M5.Mic.isEnabled()) return false;
if (!wsConnected) return false;

bool ok = M5.Mic.record(buf, CHUNK_SAMPLES, SAMPLE_RATE);
if (!ok) return false;
```

## 日志记录

**框架：** `Serial` 类（Arduino 标准）

**日志模式：**
- 状态报告：`Serial.println("WiFi OK, IP=");`
- 格式化输出：`Serial.printf("WS json: %s\n", s.c_str());`
- 调试信息：`Serial.print(".");`（点号进度指示）
- 路径：`src/main.cpp` 行 260-267、206、217、260-267

**日志时机：**
- WiFi 连接状态变化
- WebSocket 连接/断开
- 录音开始/停止
- JSON 解析错误或成功接收

## 设计模式

**队列和延迟处理：**
- 蜂鸣音在录音时排队，停止后统一播放
- 目的：避免在使用麦克风时切换到扬声器
- 路径：`src/main.cpp` 行 69-73、86-103、105-132、299-300

**去重机制（Ring Buffer）：**
```cpp
// 路径 src/main.cpp - 行 74-84
static String recentIds[16];
static uint8_t recentIdx = 0;
static bool seenId(const String &id) {
  if (!id.length()) return false;
  for (auto &s : recentIds) {
    if (s == id) return true;
  }
  recentIds[recentIdx++ % 16] = id;
  return false;
}
```

**请求 ID 生成：**
```cpp
// 路径 src/main.cpp - 行 134-137
static String makeReqId() {
  return String("req-") + String((uint32_t)ESP.getEfuseMac(), HEX)
         + String("-") + String(millis());
}
```

## 配置管理

**用户配置段（顶部）：**
- 位置：`src/main.cpp` 行 8-23
- WiFi SSID/密码：`WIFI_SSID`、`WIFI_PASS`（需要用户编辑）
- 认证令牌：`AUTH_TOKEN`（必须与 Mac 服务器匹配）
- 主机名：`WS_HOSTNAME`（支持编译时标志覆盖）

**音频格式常量：**
- 采样率、通道数、位深度、格式（PCM S16LE）
- 块大小计算：20ms @ 16kHz 单通道 = 320 样本 = 640 字节
- 位置：`src/main.cpp` 行 25-33

**安全常量：**
- 最大录音时长：`MAX_RECORD_MS = 8000`（8秒保护）

## 函数组织

**静态函数与全局状态：**
- 大多数函数使用 `static` 限制作用域（模块内可见）
- 全局变量集中在顶部：`wsConnected`、`recording`、`recordStartMs` 等
- 函数按逻辑分组：配置→发送→处理→主循环

**职责分离：**
- `setupAudio()`：初始化硬件
- `sendStart()`、`sendEnd()`：协议消息
- `recordOneChunkAndSend()`：音频流处理
- `queueBeep()`、`playPendingBeeps()`：音频反馈
- `handleHookEvent()`：事件处理
- `webSocketEvent()`：底层 WebSocket 回调
- `setup()`、`loop()`：Arduino 标准入口

---

*编码规范分析日期：2026-02-03*
