#include <Arduino.h>
#include <unity.h>
#include "Config.h"
#include "NetworkManager.h"
#include "AudioManager.h"

// onHookEvent is defined in main.cpp and linked via test_build_src=yes
extern void onHookEvent(const char* eventName);

// Helper: reset AudioMgr beep state by starting then stopping recording
static void resetBeepState() {
    AudioMgr.startRecording();  // clears all pending beeps
    AudioMgr.stopRecording();
}

void setUp(void) {
    resetBeepState();
}

void tearDown(void) {
}

// ==================== Config 参数一致性 ====================

void test_config_audio_params(void) {
    TEST_ASSERT_EQUAL(16000, SAMPLE_RATE);
    TEST_ASSERT_EQUAL(1, CHANNELS);
    TEST_ASSERT_EQUAL(16, BIT_DEPTH);
}

void test_config_chunk_calculation(void) {
    // 20ms @16kHz => 320 samples, each 2 bytes, mono => 640 bytes
    TEST_ASSERT_EQUAL(320, CHUNK_SAMPLES);
    int expected = CHUNK_SAMPLES * (BIT_DEPTH / 8) * CHANNELS;
    TEST_ASSERT_EQUAL(expected, CHUNK_BYTES);
    TEST_ASSERT_EQUAL(640, CHUNK_BYTES);
}

// ==================== AudioManager 录音状态机 ====================

void test_audio_initial_state(void) {
    // After setUp resets, should not be recording
    TEST_ASSERT_FALSE(AudioMgr.isRecording());
}

void test_audio_start_stop_recording(void) {
    AudioMgr.startRecording();
    TEST_ASSERT_TRUE(AudioMgr.isRecording());
    AudioMgr.stopRecording();
    TEST_ASSERT_FALSE(AudioMgr.isRecording());
}

void test_audio_double_stop_safe(void) {
    AudioMgr.startRecording();
    AudioMgr.stopRecording();
    AudioMgr.stopRecording();  // second stop should not crash
    TEST_ASSERT_FALSE(AudioMgr.isRecording());
}

void test_audio_recording_state_after_start(void) {
    AudioMgr.startRecording();
    TEST_ASSERT_TRUE(AudioMgr.isRecording());
    AudioMgr.stopRecording();
}

// ==================== AudioManager 蜂鸣队列 ====================

void test_audio_beep_queue_increments(void) {
    AudioMgr.queueBeep(BEEP_START);
    TEST_ASSERT_EQUAL_UINT8(1, AudioMgr.pendingBeeps(BEEP_START));
    AudioMgr.queueBeep(BEEP_START);
    TEST_ASSERT_EQUAL_UINT8(2, AudioMgr.pendingBeeps(BEEP_START));
}

void test_audio_beep_queue_multiple_types(void) {
    AudioMgr.queueBeep(BEEP_START);
    AudioMgr.queueBeep(BEEP_PERMISSION);
    AudioMgr.queueBeep(BEEP_FAILURE);
    AudioMgr.queueBeep(BEEP_STOP);
    TEST_ASSERT_EQUAL_UINT8(1, AudioMgr.pendingBeeps(BEEP_START));
    TEST_ASSERT_EQUAL_UINT8(1, AudioMgr.pendingBeeps(BEEP_PERMISSION));
    TEST_ASSERT_EQUAL_UINT8(1, AudioMgr.pendingBeeps(BEEP_FAILURE));
    TEST_ASSERT_EQUAL_UINT8(1, AudioMgr.pendingBeeps(BEEP_STOP));
}

void test_audio_start_recording_clears_pending(void) {
    AudioMgr.queueBeep(BEEP_START);
    AudioMgr.queueBeep(BEEP_PERMISSION);
    AudioMgr.queueBeep(BEEP_FAILURE);
    AudioMgr.queueBeep(BEEP_STOP);
    AudioMgr.startRecording();
    TEST_ASSERT_EQUAL_UINT8(0, AudioMgr.pendingBeeps(BEEP_START));
    TEST_ASSERT_EQUAL_UINT8(0, AudioMgr.pendingBeeps(BEEP_PERMISSION));
    TEST_ASSERT_EQUAL_UINT8(0, AudioMgr.pendingBeeps(BEEP_FAILURE));
    TEST_ASSERT_EQUAL_UINT8(0, AudioMgr.pendingBeeps(BEEP_STOP));
    AudioMgr.stopRecording();
}

void test_audio_beep_queue_priority_order(void) {
    // Each type queues independently
    AudioMgr.queueBeep(BEEP_STOP);
    AudioMgr.queueBeep(BEEP_STOP);
    AudioMgr.queueBeep(BEEP_START);
    TEST_ASSERT_EQUAL_UINT8(2, AudioMgr.pendingBeeps(BEEP_STOP));
    TEST_ASSERT_EQUAL_UINT8(1, AudioMgr.pendingBeeps(BEEP_START));
    TEST_ASSERT_EQUAL_UINT8(0, AudioMgr.pendingBeeps(BEEP_PERMISSION));
    TEST_ASSERT_EQUAL_UINT8(0, AudioMgr.pendingBeeps(BEEP_FAILURE));
}

// ==================== NetworkManager 状态 ====================

void test_network_initial_state(void) {
    TEST_ASSERT_FALSE(NetworkMgr.isConnected());
}

// ==================== Hook 事件路由 ====================

void test_hook_connected_queues_start_beep(void) {
    onHookEvent("Connected");
    TEST_ASSERT_EQUAL_UINT8(1, AudioMgr.pendingBeeps(BEEP_START));
}

void test_hook_permission_queues_permission_beep(void) {
    onHookEvent("PermissionRequest");
    TEST_ASSERT_EQUAL_UINT8(1, AudioMgr.pendingBeeps(BEEP_PERMISSION));
}

void test_hook_notification_queues_permission_beep(void) {
    onHookEvent("Notification");
    TEST_ASSERT_EQUAL_UINT8(1, AudioMgr.pendingBeeps(BEEP_PERMISSION));
}

void test_hook_failure_queues_failure_beep(void) {
    onHookEvent("PostToolUseFailure");
    TEST_ASSERT_EQUAL_UINT8(1, AudioMgr.pendingBeeps(BEEP_FAILURE));
}

// ==================== Config 常量合理性 ====================

void test_config_timing_constants(void) {
    TEST_ASSERT_GREATER_THAN(0, MAX_RECORD_MS);
    TEST_ASSERT_GREATER_THAN(KEEPALIVE_PULSE_DURATION_MS, KEEPALIVE_PULSE_INTERVAL_MS);
}

// ==================== Test Runner ====================

void setup() {
    delay(2000);
    UNITY_BEGIN();

    // Config
    RUN_TEST(test_config_audio_params);
    RUN_TEST(test_config_chunk_calculation);

    // AudioManager 录音状态机
    RUN_TEST(test_audio_initial_state);
    RUN_TEST(test_audio_start_stop_recording);
    RUN_TEST(test_audio_double_stop_safe);
    RUN_TEST(test_audio_recording_state_after_start);

    // AudioManager 蜂鸣队列
    RUN_TEST(test_audio_beep_queue_increments);
    RUN_TEST(test_audio_beep_queue_multiple_types);
    RUN_TEST(test_audio_start_recording_clears_pending);
    RUN_TEST(test_audio_beep_queue_priority_order);

    // NetworkManager
    RUN_TEST(test_network_initial_state);

    // Hook 事件路由
    RUN_TEST(test_hook_connected_queues_start_beep);
    RUN_TEST(test_hook_permission_queues_permission_beep);
    RUN_TEST(test_hook_notification_queues_permission_beep);
    RUN_TEST(test_hook_failure_queues_failure_beep);

    // Config 常量合理性
    RUN_TEST(test_config_timing_constants);

    UNITY_END();
}

void loop() {
}
