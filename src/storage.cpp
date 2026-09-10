#include "storage.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

void initStorage() {
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Mount Failed");
        return;
    }
    Serial.println("LittleFS Mounted Successfully");
}

bool getUserFromFlash(const String& uid_hex, DynamicUser& userOut) {
    File file = LittleFS.open("/users.json", "r");
    if (!file) {
        Serial.println("users.json not found");
        return false;
    }

    // Allocate JSON document in RAM temporarily.
    // It is destroyed and memory is freed immediately when this function exits.
    JsonDocument doc; 
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.println("JSON Parse Error");
        return false;
    }

    // Search the array for the matching UID
    for (JsonObject elem : doc.as<JsonArray>()) {
        if (elem["uid"].as<String>().equalsIgnoreCase(uid_hex)) {
            userOut.uid_hex = uid_hex;
            userOut.name = elem["name"].as<String>();
            userOut.message = elem["message"].as<String>();
            userOut.group = elem["group"].as<int>();
            userOut.iconPath = elem["icon"].as<String>();
            return true; // Match found
        }
    }
    
    return false; // UID not found in file
}