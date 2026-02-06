# ESP32 WebSocket Audio Uploader - Claude 项目指令

## 项目概述

ESP32-S3 固件（M5Stack Atom EchoS3R），通过 I2S 麦克风录音并以 WebSocket 流式传输到 Mac 上的 Whisper ASR 服务器。支持 Push-to-Talk、Hook 事件蜂鸣反馈、GPIO 按钮控制 Claude Code CLI。

配套服务器：https://github.com/jarbozhang/mac-whisper-ws-asr-server

## 技术栈

- **语言**：C++（Arduino/ESP32 方言）
- **硬件**：ESP32-S3-DevKitC-1（M5Stack Atom EchoS3R）
- **构建系统**：PlatformIO
- **框架**：Espressif Arduino
- **核心依赖**：WebSockets ^2.4.1、M5Unified ^0.2.8、ArduinoJson ^7.0.4
- **测试**：Unity（片上）、Python websockets（模拟服务器）

## 构建与部署命令

```bash
pio run                    # 编译
pio run -t upload          # 编译并刷写到 ESP32
pio device monitor         # 串口监控（115200 baud）
pio test -e esp32-s3       # 运行片上单元测试
python scripts/mock_server.py  # 启动模拟 WebSocket 服务器
```

## 代码结构

```
src/
├── main.cpp              # 应用入口，loop() 主循环，按钮状态机
├── AudioManager.h/cpp    # 录音、蜂鸣队列、Mic/Speaker 切换
├── NetworkManager.h/cpp  # WiFi Multi、mDNS、WebSocket、JSON 协议
├── Config.h              # 所有编译时常量集中在此
└── secrets.h             # WiFi 凭证、AUTH_TOKEN（gitignored）
```

## 编码规范（必须遵守）

- **缩进**：2 空格，K&R 大括号风格
- **常量**：全大写下划线 `SAMPLE_RATE`
- **成员变量**：下划线前缀 `_recording`、`_wsConnected`
- **函数**：camelCase，动词开头 `sendStart()`、`queueBeep()`
- **类/枚举**：PascalCase `AudioManager`、`BeepKind`
- **日志前缀**：`[Main]`、`[Audio]`、`[NM]`
- **错误处理**：布尔返回 + Serial.printf()，不用异常
- **注释**：中英混用，关键概念用英文

## 架构规则

- **Manager 模式**：`AudioMgr` 和 `NetworkMgr` 是全局单例，通过 `begin()` 初始化，`update()`/`loop()` 驱动
- **回调解耦**：网络层通过 `setHookCallback()` 通知应用层，不直接依赖 AudioManager
- **蜂鸣队列**：录音期间蜂鸣排队，停止后按优先级播放（START > PERMISSION > FAILURE > STOP），因为 Mic 和 Speaker 不能同时使用
- **Hook 去重**：16 元素环形缓冲防重复事件

## 决策指南（减少提问）

当遇到模糊决策时，按以下原则自行判断，无需询问：

1. **新常量** → 放入 `Config.h`，全大写命名
2. **新 Hook 事件类型** → 在 `BeepKind` 枚举添加，在 `patternFor()` 添加模式，在 `onHookEvent()` 添加分支
3. **新控制命令** → 在 `NetworkManager` 添加 `sendXxx()` 方法，在 `main.cpp` 按钮数组添加绑定
4. **新 WebSocket 消息** → 在 `webSocketEvent()` 的 WStype_TEXT 分支处理
5. **GPIO 引脚** → 使用 INPUT_PULLUP，活跃低电平，15ms 防抖
6. **音频参数** → 16kHz 单声道 16-bit PCM，20ms 帧（320 样本 = 640 字节）
7. **两种方案都行时** → 选择更简单、改动更小的方案
8. **不确定是否需要抽象** → 不抽象，直接写，三行重复代码好过过早抽象
9. **依赖选择** → 优先使用已有依赖（M5Unified、ArduinoJson、WebSockets），避免引入新库
10. **敏感信息** → 放入 `secrets.h`（gitignored），提供 template 文件

## 硬件约束（重要）

- ESP32-S3 只有 320KB RAM 和 16MB Flash — 避免大内存分配
- Atom EchoS3R 的 **Mic 和 Speaker 互斥** — 不能同时录音和播放
- Mic→Speaker 切换需要 100ms 延迟（I2S 稳定）
- 充电宝供电需要保活脉冲（GPIO48 每 30 秒拉高 100ms）

## 文件修改指南

| 要做什么 | 改哪里 |
|---|---|
| 新录音功能 | `AudioManager.cpp` + `Config.h` |
| 新 Hook 事件 | `AudioManager.h`(枚举) + `AudioManager.cpp`(模式) + `main.cpp`(回调) |
| 新控制按钮 | `NetworkManager.h/cpp`(命令) + `main.cpp`(按钮绑定) |
| 新 WebSocket 消息 | `NetworkManager.cpp`(解析) + `NetworkManager.h`(方法) |
| 调整参数/阈值 | `Config.h` |
| WiFi/认证 | `secrets.h` |

## 项目状态

当前位于 v0.1 里程碑，Phase 3 已完成，已完成 12 个快速迭代任务。详细状态见 `.planning/STATE.md`，路线图见 `.planning/ROADMAP.md`。
