#include "settings.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

volatile bool settings_need_reload = false;
DisplaySettings current_settings;

uint16_t hexToRGB565(String hex) {
    if (hex.startsWith("#")) hex.remove(0, 1);
    long rgb = strtol(hex.c_str(), NULL, 16);
    return (((rgb >> 16) & 0xF8) << 8) | ((((rgb >> 8) & 0xFF) & 0xFC) << 3) | ((rgb & 0xFF) >> 3);
}

bool loadSettings() {
    File file = LittleFS.open("/settings.json", "r");
    if (!file) return false;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) return false;

    current_settings.show_eu_sol_corp = doc["toggles"]["eu_sol_corp"] | true;
    current_settings.show_bones_festes = doc["toggles"]["bones_festes"] | true;
    current_settings.show_eu_erasmus = doc["toggles"]["eu_erasmus"] | true;
    current_settings.show_clock = doc["toggles"]["clock"] | true;
    current_settings.show_i_love_agres = doc["toggles"]["i_love_agres"] | true;

    current_settings.brightness = doc["display"]["brightness"] | 255;
    current_settings.timezone = doc["time"]["timezone"] | "CET-1CEST,M3.5.0,M10.5.0/3";
    current_settings.screen_on_hour = doc["time"]["screen_on_h"] | 7;
    current_settings.screen_on_minute = doc["time"]["screen_on_m"] | 30;
    current_settings.screen_off_hour = doc["time"]["screen_off_h"] | 0;
    current_settings.screen_off_minute = doc["time"]["screen_off_m"] | 0;

    current_settings.global_text_color_hex = doc["styling"]["global_text_color"] | "#FFFFFF";
    current_settings.global_text_color_565 = hexToRGB565(current_settings.global_text_color_hex);
    current_settings.scroll_speed_ms = doc["styling"]["scroll_speed_ms"] | 10;

    current_settings.display_order.clear();
    for (int id : doc["order"].as<JsonArray>()) {
        current_settings.display_order.push_back(id);
    }

    // Parse array of custom message objects
current_settings.custom_messages.clear();
    for (JsonObject msgObj : doc["custom_messages"].as<JsonArray>()) {
        CustomMessage msg;
        msg.text = msgObj["text"] | "";
        msg.color_hex = msgObj["color"] | "#FFFFFF";
        msg.color_565 = hexToRGB565(msg.color_hex);
        msg.font_id = msgObj["font_id"] | 1;
        msg.icon_file = msgObj["icon_file"] | ""; // Load icon path
        current_settings.custom_messages.push_back(msg);
    }

    current_settings.icons.clear();
    for (String icon : doc["icons"].as<JsonArray>()) {
        current_settings.icons.push_back(icon);
    }

    return true;
}

bool saveSettings() {
    JsonDocument doc;
    
    doc["toggles"]["eu_sol_corp"] = current_settings.show_eu_sol_corp;
    doc["toggles"]["bones_festes"] = current_settings.show_bones_festes;
    doc["toggles"]["eu_erasmus"] = current_settings.show_eu_erasmus;
    doc["toggles"]["clock"] = current_settings.show_clock;
    doc["toggles"]["i_love_agres"] = current_settings.show_i_love_agres;

    doc["display"]["brightness"] = current_settings.brightness;
    
    doc["time"]["timezone"] = current_settings.timezone;
    doc["time"]["screen_on_h"] = current_settings.screen_on_hour;
    doc["time"]["screen_on_m"] = current_settings.screen_on_minute;
    doc["time"]["screen_off_h"] = current_settings.screen_off_hour;
    doc["time"]["screen_off_m"] = current_settings.screen_off_minute;

    doc["styling"]["global_text_color"] = current_settings.global_text_color_hex;
    doc["styling"]["scroll_speed_ms"] = current_settings.scroll_speed_ms;

    JsonArray orderArray = doc["order"].to<JsonArray>();
    for (int id : current_settings.display_order) orderArray.add(id);

    // Save array of custom message objects
JsonArray msgArray = doc["custom_messages"].to<JsonArray>();
    for (const CustomMessage& msg : current_settings.custom_messages) {
        JsonObject msgObj = msgArray.add<JsonObject>();
        msgObj["text"] = msg.text;
        msgObj["color"] = msg.color_hex;
        msgObj["font_id"] = msg.font_id;
        msgObj["icon_file"] = msg.icon_file; // Save icon path
    }

    JsonArray iconArray = doc["icons"].to<JsonArray>();
    for (const String& icon : current_settings.icons) iconArray.add(icon);

    File file = LittleFS.open("/settings.json", "w");
    if (!file) return false;

    serializeJson(doc, file);
    file.close();
    
    setenv("TZ", current_settings.timezone.c_str(), 1);
    tzset();
    
    return true;
}