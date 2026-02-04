# Plan: Check WiFi Power Settings

The user needs to ensure high power consumption to prevent the external battery bank from sleeping. We will maximize WiFi TX power and disable WiFi sleep mode.

## Tasks

### 1. Update NetworkManager
- **File:** `src/NetworkManager.cpp`
- **Action:**
    - In `connectWiFi()`, after confirming connection (or before), set TX power to maximum.
    - Disable WiFi sleep mode (`WiFi.setSleep(false)`).
    - Add Serial logs to confirm these settings are applied.

### 2. Verify
- **Action:** Compile the project to ensure no API errors.
