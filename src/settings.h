#pragma once
#include <Arduino.h>
#include <vector>

struct CustomMessage {
    String text;
    String color_hex;
    uint16_t color_565; 
    int font_id;        
    String icon_file; // Path to the uploaded .bin file, e.g., "/icons/msg_0.bin"
};
extern volatile bool settings_need_reload;
struct DisplaySettings {
    // Toggles
    bool show_eu_sol_corp;
    bool show_bones_festes;
    bool show_eu_erasmus;
    bool show_clock;
    bool show_i_love_agres;
    
    // Sequence
    std::vector<int> display_order; 
    
    // Hardware & Time
    uint8_t brightness;             
    String timezone;                
    int screen_on_hour;
    int screen_on_minute;
    int screen_off_hour;
    int screen_off_minute;

    // Global Styling (for names and system text)
    String global_text_color_hex;          
    uint16_t global_text_color_565;        
    int scroll_speed_ms;            
    
    // Arrays for dynamic content
    std::vector<CustomMessage> custom_messages;
    std::vector<String> icons;
};

extern DisplaySettings current_settings;

bool loadSettings();
bool saveSettings();
uint16_t hexToRGB565(String hex);