# 代码库结构

**分析日期:** 2026-02-03

## 目录布局

```
esp32-ws-audio-uploader/
├── src/                    # C++ 源代码
│   └── main.cpp           # 唯一的应用代码文件（完整实现）
├── platformio.ini         # PlatformIO 构建配置
├── README.md              # 项目说明和快速开始指南
├── SPEC.md                # 协议规范文档
├── LICENSE                # 许可证文件
└── .gitignore             # Git 忽略规则
```

## 目录用途

**src/ 目录:**
- 用途: 包含所有 C++ 源代码
- 包含: Arduino 草图和库代码
- 关键文件: `main.cpp`

## 关键文件位置

**入口点:**
- `src/main.cpp`: Arduino 主文件，包含 `setup()` 和 `loop()` 函数

**配置:**
- `platformio.ini`: PlatformIO 构建配置，指定硬件平台、库依赖、编译标志

**文档:**
- `README.md`: 用户说明、配置步骤、协议概述
- `SPEC.md`: 详细的 WebSocket 消息协议规范

## 命名约定

**文件:**
- 单一主文件模式: `main.cpp`（Arduino 约定）
- 配置文件: `platformio.ini`、`.gitignore`

**目录:**
- `src/` - Arduino/PlatformIO 标准目录
- `.git/` - Git 版本控制（自动生成）
- `.claude/` - Claude 编辑器配置（自动生成）

**代码中的命名（C++）:**
- **常量:** 全大写带下划线（如 `WIFI_SSID`、`SAMPLE_RATE`、`MAX_RECORD_MS`）
- **全局变量:** 前缀 `static`，驼峰或下划线命名（如 `wsConnected`、`recording`、`currentReqId`）
- **函数:** 驼峰命名法（如 `setupAudio()`、`recordOneChunkAndSend()`、`playPendingBeeps()`）
- **结构体/Enum:** 驼峰命名（如 `BeepKind`、`BeepPattern`）
- **数组:** 使用 `s` 前缀表示字符串（如 `recentIds[]`）

## 添加新代码的位置

**新功能:**
- 位置: `src/main.cpp` 中的合适逻辑位置
- 示例：新的按钮处理 → 在 `loop()` 函数的按钮轮询部分添加
- 示例：新的消息类型处理 → 在 `handleHookEvent()` 或 `webSocketEvent()` 中添加

**新的 Beep 类型:**
- 添加 `BeepKind` 枚举值
- 在 `patternFor()` switch 语句中定义新的频率/持续时间
- 在 `queueBeep()` 中添加对应的 pending 计数器逻辑
- 在 `playPendingBeeps()` 的优先级顺序中定位

**新的 Hook 事件类型:**
- 在 `handleHookEvent()` 的事件名称检查中添加 `strcmp()` 分支
- 决定触发哪种 Beep 或其他动作
- 将动作传递给 `queueBeep()` 或其他处理函数

**WebSocket 协议扩展:**
- 修改 `sendStart()` 改变启动消息体
- 修改 `webSocketEvent()` 的 WStype_TEXT 处理新的响应类型
- 修改 `handleHookEvent()` 处理新的 hook 字段

**音频格式变更:**
- 修改 `SAMPLE_RATE`、`CHANNELS`、`BIT_DEPTH`、`FORMAT` 常量
- 重新计算 `CHUNK_SAMPLES` 和 `CHUNK_BYTES`（保持 20ms 窗口）
- 更新缓冲区大小（如果需要）

## 特殊目录

**src/:**
- 用途: Arduino/PlatformIO 标准源目录
- 生成: 否
- 提交: 是（仅 `main.cpp`）

**.git/:**
- 用途: Git 版本控制元数据
- 生成: 是（自动）
- 提交: 否（自动忽略）

**.claude/:**
- 用途: Claude 编辑器本地配置
- 生成: 是（自动）
- 提交: 否（在 .gitignore 中）

## 文件大小和复杂度

**src/main.cpp:** 308 行
- 编译指令和包含: 行 1-6
- 用户配置段: 行 8-23
- 音频参数常量: 行 25-33
- WebSocket 全局变量: 行 38
- Beep 定义和辅助函数: 行 40-103
- 录音状态变量: 行 65-72
- Hook 去重环形缓冲区: 行 74-84
- Beep 排队/播放函数: 行 86-132
- 请求 ID 生成: 行 134-137
- WebSocket 消息构建: 行 141-165
- Hook 事件处理: 行 167-185
- WebSocket 事件回调: 行 187-223
- 硬件初始化: 行 225-237
- 音频录制和发送: 行 239-250
- Arduino setup: 行 252-272
- Arduino loop: 行 274-307

## 代码组织模式

**全局作用域:**
- 配置常量（上部）
- WebSocket 客户端实例（行 38）
- 枚举和结构体定义（行 40-51）
- 状态变量（行 65-84）

**函数层次:**
- 辅助工具函数 (`seenId`, `queueBeep`, `playPendingBeeps`)
- 协议函数 (`sendStart`, `sendEnd`, `handleHookEvent`)
- 事件回调 (`webSocketEvent`)
- 硬件函数 (`setupAudio`, `recordOneChunkAndSend`)
- Arduino 生命周期 (`setup`, `loop`)

**设计考虑:**
- 所有函数标记为 `static`（文件作用域），避免命名冲突
- 使用 `static` 局部缓冲区（如 `buf[CHUNK_SAMPLES]` 在 `recordOneChunkAndSend()` 中）优化性能
- 使用 volatile 关键字修饰由中断修改的状态（如 `volatile bool wsConnected`）

---

*结构分析: 2026-02-03*
