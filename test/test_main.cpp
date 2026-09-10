
#include <Arduino.h>
#include <unity.h>
#include "settings.h"
#include <LittleFS.h>

void setUp(void) {
    // Runs before every single test
    LittleFS.begin(true);
}

void tearDown(void) {
    // Runs after every single test
    LittleFS.end();
}

void test_hex_to_rgb565(void) {
    // Evaluate standard colors
    TEST_ASSERT_EQUAL_UINT16(0xFFFF, hexToRGB565("#FFFFFF")); // White
    TEST_ASSERT_EQUAL_UINT16(0x0000, hexToRGB565("000000"));  // Black (no hash)
    TEST_ASSERT_EQUAL_UINT16(0xF800, hexToRGB565("#FF0000")); // Red
}

void test_settings_fallback(void) {
    // Temporarily move the file to force the fallback logic
    LittleFS.rename("/settings.json", "/settings.backup");
    
    bool result = loadSettings();
    
    // Assert that the function returns false and applies safe defaults
    TEST_ASSERT_FALSE(result); 
    TEST_ASSERT_EQUAL_UINT8(255, current_settings.brightness);
    TEST_ASSERT_EQUAL_STRING("CET-1CEST,M3.5.0,M10.5.0/3", current_settings.timezone.c_str());
    
    // Restore the file
    LittleFS.rename("/settings.backup", "/settings.json");
}

void setup() {
    delay(2000); // Allow hardware serial to initialize
    UNITY_BEGIN();
    
    RUN_TEST(test_hex_to_rgb565);
    RUN_TEST(test_settings_fallback);
    
    UNITY_END();
}

void loop() {
    // Empty loop required for Arduino framework tests
}