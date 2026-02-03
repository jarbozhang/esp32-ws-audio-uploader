---
phase: quick-001
plan: 01
type: execute
wave: 1
depends_on: []
files_modified:
  - src/main.cpp
  - src/NetworkManager.h
  - src/NetworkManager.cpp
  - src/Config.h
autonomous: true

must_haves:
  truths:
    - "GPIO 5 按下时发送 approve 命令到 WebSocket"
    - "GPIO 6 按下时发送 reject 命令到 WebSocket"
    - "GPIO 7 按下时发送 switch_model 命令到 WebSocket"
    - "GPIO 8 按下时发送 toggle_auto_approve 命令到 WebSocket"
    - "按钮使用下拉有效逻辑 (LOW = pressed)"
  artifacts:
    - path: "src/main.cpp"
      provides: "4 个外部按钮的初始化和事件处理"
      contains: "pinMode.*INPUT_PULLUP"
    - path: "src/NetworkManager.cpp"
      provides: "4 个控制命令的 WebSocket 发送方法"
      exports: ["sendApprove", "sendReject", "sendSwitchModel", "sendToggleAutoApprove"]
  key_links:
    - from: "src/main.cpp"
      to: "NetworkMgr.sendApprove"
      via: "button press event"
      pattern: "NetworkMgr\\.send(Approve|Reject|SwitchModel|ToggleAutoApprove)"
---

<objective>
为 M5Stack Atom EchoS3R 添加 4 个外部机械键盘按钮，用于控制 Claude Code CLI。

Purpose: 让用户通过物理按钮快速操作 Claude Code，无需切换到终端界面
Output: 修改后的固件，支持 GPIO 5/6/7/8 四个按钮发送控制命令
</objective>

<execution_context>
@/Users/jiabozhang/.claude/get-shit-done/workflows/execute-plan.md
@/Users/jiabozhang/.claude/get-shit-done/templates/summary.md
</execution_context>

<context>
@src/main.cpp
@src/NetworkManager.h
@src/NetworkManager.cpp
@src/Config.h
</context>

<tasks>

<task type="auto">
  <name>Task 1: 添加 GPIO 按钮定义和 NetworkManager 方法</name>
  <files>src/Config.h, src/NetworkManager.h, src/NetworkManager.cpp</files>
  <action>
  1. 在 Config.h 中添加 GPIO 引脚定义:
     ```cpp
     // Control buttons (active LOW with external pull-down)
     #define BTN_APPROVE_PIN      5
     #define BTN_REJECT_PIN       6
     #define BTN_SWITCH_MODEL_PIN 7
     #define BTN_AUTO_APPROVE_PIN 8
     ```

  2. 在 NetworkManager.h 中添加 4 个新方法声明:
     ```cpp
     void sendApprove();
     void sendReject();
     void sendSwitchModel();
     void sendToggleAutoApprove();
     ```

  3. 在 NetworkManager.cpp 中实现这 4 个方法，发送 JSON 命令到 WebSocket:
     - sendApprove: {"type": "command", "action": "approve"}
     - sendReject: {"type": "command", "action": "reject"}
     - sendSwitchModel: {"type": "command", "action": "switch_model"}
     - sendToggleAutoApprove: {"type": "command", "action": "toggle_auto_approve"}

  注意: 使用与 sendStart/sendEnd 相同的模式，使用 StaticJsonDocument 序列化 JSON
  </action>
  <verify>代码编译通过: `pio run`</verify>
  <done>Config.h 有 4 个 GPIO 定义，NetworkManager 有 4 个 send 方法实现</done>
</task>

<task type="auto">
  <name>Task 2: 在 main.cpp 中初始化按钮并处理事件</name>
  <files>src/main.cpp</files>
  <action>
  1. 在 setup() 中初始化 4 个 GPIO 为输入模式:
     ```cpp
     // 外部按钮使用下拉电阻，按下时为 LOW
     pinMode(BTN_APPROVE_PIN, INPUT_PULLUP);
     pinMode(BTN_REJECT_PIN, INPUT_PULLUP);
     pinMode(BTN_SWITCH_MODEL_PIN, INPUT_PULLUP);
     pinMode(BTN_AUTO_APPROVE_PIN, INPUT_PULLUP);
     ```
     注意: 使用 INPUT_PULLUP 并检测 LOW 状态，或者如果外部有下拉电阻则用 INPUT 模式

  2. 在 loop() 中添加按钮状态检测和去抖动逻辑:
     - 使用简单的状态变量记录上一次按钮状态
     - 检测按钮按下边沿 (从 HIGH 变为 LOW)
     - 按下时调用对应的 NetworkMgr.sendXxx() 方法
     - 添加 Serial.println 日志输出便于调试

  3. 按钮处理代码结构示例:
     ```cpp
     static bool lastApproveState = HIGH;
     bool approveState = digitalRead(BTN_APPROVE_PIN);
     if (approveState == LOW && lastApproveState == HIGH) {
         Serial.println("Approve button pressed");
         if (NetworkMgr.isConnected()) {
             NetworkMgr.sendApprove();
         }
     }
     lastApproveState = approveState;
     ```

  注意:
  - 下拉有效意味着按下时 GPIO 读到 LOW
  - 需要为 4 个按钮分别实现类似逻辑
  - 考虑添加简单的去抖动延时 (10-20ms)
  </action>
  <verify>
  1. 代码编译通过: `pio run`
  2. 上传到设备后，按下按钮在串口看到对应日志
  </verify>
  <done>
  - setup() 中有 4 个 pinMode 调用
  - loop() 中有 4 个按钮状态检测和处理
  - 按钮按下时调用对应的 NetworkMgr 方法
  </done>
</task>

</tasks>

<verification>
1. `pio run` 编译成功，无错误
2. `pio run -t upload` 上传成功
3. 串口监视器 (`pio device monitor`) 显示按钮按下日志
4. WebSocket 服务器收到对应的 JSON 命令
</verification>

<success_criteria>
- 4 个 GPIO 按钮正确初始化
- 按下每个按钮发送对应的 WebSocket 命令
- 无误触发，去抖动正常工作
- 不影响现有的 M5.BtnA 录音功能
</success_criteria>

<output>
完成后创建 `.planning/quick/001-add-4-buttons-claude-code-control/001-SUMMARY.md`
</output>
