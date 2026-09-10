#pragma once
#include <Arduino.h>

struct DynamicUser {
    String uid_hex;
    String name;
    String message;
    int group;
    String iconPath;
};

void initStorage();
bool getUserFromFlash(const String& uid_hex, DynamicUser& userOut);