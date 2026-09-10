#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "webserver.h"
#include "settings.h"

AsyncWebServer server(80);

extern uint8_t pending_settings_data[4096];
extern size_t pending_settings_length;
extern bool pending_settings_save;

extern uint8_t pending_users_data[8192];
extern size_t pending_users_length;
extern bool pending_users_save;

extern uint8_t pending_icon_data[8192];
extern char pending_icon_filename[64];
extern bool pending_icon_save;
extern size_t pending_icon_length;

void initWebServer() {
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, OPTIONS");
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    server.on("/api/users", HTTP_GET, [](AsyncWebServerRequest *request){
        if (LittleFS.exists("/users.json")) {
            request->send(LittleFS, "/users.json", "application/json");
        } else {
            request->send(200, "application/json", "[]");
        }
    });

    server.on("/api/settings", HTTP_GET, [](AsyncWebServerRequest *request){
        if (LittleFS.exists("/settings.json")) {
            request->send(LittleFS, "/settings.json", "application/json");
        } else {
            request->send(200, "application/json", "{}");
        }
    });

    server.on("/api/settings", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        if (index == 0) pending_settings_length = 0; 
        for (size_t i = 0; i < len; i++) {
            if (index + i < 4096) pending_settings_data[index + i] = data[i];
        }
        if (index + len == total) {
            pending_settings_length = total;
            pending_settings_save = true; 
        }
    });

    server.on("/api/users", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "application/json", "{\"status\":\"ok\"}");
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        if (index == 0) pending_users_length = 0; 
        for (size_t i = 0; i < len; i++) {
            if (index + i < 8192) pending_users_data[index + i] = data[i];
        }
        if (index + len == total) {
            pending_users_length = total;
            pending_users_save = true;
        }
    });

    server.on("/api/upload_icon", HTTP_POST, [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "OK");
    }, NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
        if (index == 0) {
            if (request->hasArg("file")) {
                String f = request->arg("file");
                strncpy(pending_icon_filename, f.c_str(), sizeof(pending_icon_filename) - 1);
                pending_icon_filename[sizeof(pending_icon_filename) - 1] = '\0';
            }
            pending_icon_length = 0;
        }
        for (size_t i = 0; i < len; i++) {
            if (index + i < 8192) pending_icon_data[index + i] = data[i];
        }
        if (index + len == total) {
            pending_icon_length = total;
            pending_icon_save = true;
        }
    });

    server.onNotFound([](AsyncWebServerRequest *request){
        if (request->method() == HTTP_OPTIONS) {
            request->send(200); 
        } else {
            request->send(404, "text/plain", "Not found");
        }
    });

    server.begin();
    Serial.println("Async Web Server started on port 80");
}