#include <Arduino.h>
#include <unity.h>
#include "Config.h"
#include "NetworkManager.h"
#include "AudioManager.h"

void setUp(void) {
    // set stuff up here
}

void tearDown(void) {
    // clean stuff up here
}

void test_config_loading(void) {
    // Check that we have at least one WiFi credential configured
    TEST_ASSERT_GREATER_THAN(0, WIFI_NETWORKS.size());
    
    // Check default constants
    TEST_ASSERT_EQUAL(16000, SAMPLE_RATE);
    TEST_ASSERT_EQUAL_STRING("/ws", WS_PATH);
}

void test_network_manager_state(void) {
    // Initially should not be connected (unless begin() was called in main setup, 
    // but tests have their own setup usually if defined properly, 
    // or we just check the variable state)
    
    // Note: Since NetworkMgr is global, its state depends on whether the test runner 
    // runs setup() from main.cpp. PlatformIO tests usually define their own setup/loop 
    // or run independently. 
    
    TEST_ASSERT_FALSE(NetworkMgr.isConnected());
}

void test_audio_manager_state(void) {
    // Initially not recording
    TEST_ASSERT_FALSE(AudioMgr.isRecording());
}

void setup() {
    delay(2000); // Wait for board to init
    UNITY_BEGIN();
    RUN_TEST(test_config_loading);
    RUN_TEST(test_network_manager_state);
    RUN_TEST(test_audio_manager_state);
    UNITY_END();
}

void loop() {
}
