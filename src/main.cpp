#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <PN5180.h>
#include <PN5180ISO15693.h>
#include <SPI.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "esp_system.h"
#include "esp_task_wdt.h"

#include "Fonts/FreeSans18pt7b.h"
#include "Fonts/FreeSans12pt7b.h"
#include "Fonts/FreeSans9pt7b.h"

// Custom local headers
#include "settings.h"
#include "storage.h"
#include "webserver.h"
#include "eu_flag.h"
#include "clock.h"
#include "i_love_agres_anim.h"
#include "amicslogo.h"
#include "arbollogo.h"

// Hardware Defines
#define R1 13
#define G1 14
#define B1 12
#define R2 10
#define G2 11
#define B2 3
#define CH_A 18
#define CH_B 8
#define CH_C 16
#define CH_D 17
#define CH_E 9
#define LAT 15
#define OE 6
#define CLK 7

const char *ssid = "SiProject_AGRES";
const char *password = "1234agresAGRES@";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0);
MatrixPanel_I2S_DMA *dma_display = nullptr;
PN5180ISO15693 nfc(38, 1, 2);

// Image/Icon Buffer
static uint16_t current_icon_buffer[4096];

// --- SAFE RAM BUFFERING VARIABLES ---
uint8_t pending_settings_data[4096];
size_t pending_settings_length = 0;
bool pending_settings_save = false;

uint8_t pending_users_data[8192];
size_t pending_users_length = 0;
bool pending_users_save = false;

uint8_t pending_icon_data[8192];
char pending_icon_filename[64] = ""; 
bool pending_icon_save = false;
size_t pending_icon_length = 0;

// Reboot tracking variables
bool needs_reboot = false;
uint32_t reboot_timer = 0;

// === RESTORED EXACT ORIGINAL CLEAR_AREA ===
inline void clear_area(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end) {
    for (uint16_t x = x_start; x <= x_end; x++) {
        for (uint16_t y = y_start; y <= y_end; y++) {
            dma_display->drawPixel(x, y, 0);
        }
    }
}

// === RESTORED EXACT ORIGINAL DRAW_AMICS ===
void draw_amics() {
    clear_area(192, 0, 64 * 3, 64);
    dma_display->fillCircle(32 + 192, 32, 28, 0x001F);
    for (int x = 192; x < 192 + 64; x++) {
        for (int y = 0; y < 64; y++) {
            if (amics_map[((x - 192) + y * 64)])
                dma_display->drawPixel(x, y, 0xFFFF);
        }
    }
    dma_display->flipDMABuffer();
    dma_display->fillCircle(32 + 192, 32, 28, 0x001F);
    for (int x = 192; x < 192 + 64; x++) {
        for (int y = 0; y < 64; y++) {
            if (amics_map[((x - 192) + y * 64)])
                dma_display->drawPixel(x, y, 0xFFFF);
        }
    }
    dma_display->flipDMABuffer();
}

// Updated to support dynamic fonts
inline void drawText(int x, int y, const char *text, uint16_t col, const GFXfont *gfxFont) {
    if (!gfxFont) return;
    while (*text) {
        char c = *text++;
        uint8_t first = pgm_read_byte(&gfxFont->first);
        GFXglyph *glyph = gfxFont->glyph + c - first;
        uint8_t *bitmap = gfxFont->bitmap;
        uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
        uint8_t w = pgm_read_byte(&glyph->width), h = pgm_read_byte(&glyph->height);
        int8_t xo = pgm_read_byte(&glyph->xOffset), yo = pgm_read_byte(&glyph->yOffset);
        uint8_t xx, yy, bits = 0, bit = 0;
        
        for (yy = 0; yy < h; yy++) {
            for (xx = 0; xx < w; xx++) {
                if (!(bit++ & 7)) { bits = pgm_read_byte(&bitmap[bo++]); }
                int xxx = x + xo + xx;
                // Clip at x=191 so text doesn't overwrite the right-side logo
                if (bits & 0x80 && xxx < 192) {
                    dma_display->drawPixel(xxx, y + yo + yy, col);
                }
                bits <<= 1;
            }
        }
        x += glyph->xAdvance;
    }
}

void display_uid_owner(u_int8_t *uid) {
    if (uid[0] == 0) return;
    
    char uidStr[5];
    sprintf(uidStr, "%02X%02X", uid[0], uid[1]);
    
    DynamicUser user;
    if (!getUserFromFlash(String(uidStr), user)) {
        return; 
    }

    // Clear BOTH buffers to prevent ghosting from previous animations
    clear_area(0, 0, 191, 64);
    dma_display->flipDMABuffer();
    clear_area(0, 0, 191, 64);
    dma_display->flipDMABuffer();

    dma_display->setFont(&FreeSans12pt7b);
    dma_display->setTextWrap(false);
    dma_display->setTextColor(0xFFFF); 
    
    timeClient.update(); 
    int hour = timeClient.getHours();
    String text = (hour < 12 && hour > 6) ? "Bon dia," : (hour >= 12 && hour < 19) ? "Bona vespra," : "Bona nit,";
    
    int16_t x1, y1; uint16_t w, h;
    dma_display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    dma_display->setCursor((192 - w) / 2, (64 + h) / 2);
    dma_display->print(text);
    dma_display->flipDMABuffer();
    delay(1500);
    
    clear_area(0, 0, 191, 64);
    dma_display->getTextBounds(user.name, 0, 0, &x1, &y1, &w, &h);
    dma_display->setCursor((192 - w) / 2, (64 + h) / 2);
    dma_display->print(user.name + "!");
    dma_display->flipDMABuffer();
    delay(1500);

    // Clear BOTH buffers again before the text scrolling begins
    clear_area(0, 0, 191, 64);
    dma_display->flipDMABuffer();
    clear_area(0, 0, 191, 64);
    dma_display->flipDMABuffer();
    
    bool using_custom_icon = false;
    if (user.iconPath != "" && LittleFS.exists(user.iconPath)) {
        File file = LittleFS.open(user.iconPath, "r");
        if (file) {
            file.read((uint8_t*)current_icon_buffer, 8192); 
            file.close();
            using_custom_icon = true;
        }
    } 

    if (using_custom_icon) {
        clear_area(192, 0, 255, 64);
        for(int y = 0; y < 64; y++) {
            for(int x = 0; x < 64; x++) {
                dma_display->drawPixel(192 + x, y, current_icon_buffer[y * 64 + x]);
            }
        }
        dma_display->flipDMABuffer();
        clear_area(192, 0, 255, 64);
        for(int y = 0; y < 64; y++) {
            for(int x = 0; x < 64; x++) {
                dma_display->drawPixel(192 + x, y, current_icon_buffer[y * 64 + x]);
            }
        }
        dma_display->flipDMABuffer();
    } else {
        draw_amics(); 
    }
    
    uint16_t msgColor = 0xF800; // Default fallback color
    const GFXfont* fontToUse = &FreeSans18pt7b; 
    dma_display->setFont(fontToUse);
    dma_display->getTextBounds(user.message, 0, 0, &x1, &y1, &w, &h);
    
    // Start at 192 (right edge) and scroll EXACTLY until text is completely offscreen left
    int limit = -((int)w) - 10;
    for (int x = 192; x >= limit; x--) {
        dma_display->startWrite();
        drawText(x, 40, user.message.c_str(), msgColor, fontToUse);
        dma_display->endWrite();
        
        dma_display->flipDMABuffer(); 
        
        // Exact original black text erasure
        dma_display->startWrite();
        drawText(x + 1, 40, user.message.c_str(), 0, fontToUse);
        dma_display->endWrite();
        
        delay(current_settings.scroll_speed_ms);
        yield();
    }
}

void try_uid_read_delay(time_t delayms) {
    uint8_t uid[8];
    time_t start = millis();
    
    while (millis() - start < delayms) {
        if (nfc.getInventory(uid) == ISO15693_EC_OK) {
            display_uid_owner(uid);
        } else {
            delay(1); 
        }
        yield(); 
    }
}

void bones_festes_AGRES() {
    clear_area(0, 0, 192, 64);
    String text = "Bones festes AGRES!";
    dma_display->setFont(&FreeSans9pt7b);
    dma_display->setTextColor(0xFFFF);
    int16_t x1, y1; uint16_t w, h;
    dma_display->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
    dma_display->setCursor((192 - w) / 2, (64 + h) / 2);
    dma_display->print(text);
    dma_display->flipDMABuffer();
    try_uid_read_delay(10000);
}

void dma_display_init(){
    HUB75_I2S_CFG::i2s_pins _pins = {R1, G1, B1, R2, G2, B2, CH_A, CH_B, CH_C, CH_D, CH_E, LAT, OE, CLK};
    HUB75_I2S_CFG mxconfig(64, 64, 4, _pins, HUB75_I2S_CFG::FM6126A, true, HUB75_I2S_CFG::HZ_20M, 1, false);
    mxconfig.setPixelColorDepthBits(5);
    dma_display = new MatrixPanel_I2S_DMA(mxconfig);
    dma_display->setRotation(2); 
    if (not dma_display->begin()) {
        ESP.restart();
    }
    dma_display->setBrightness8(current_settings.brightness);
    dma_display->fillScreenRGB888(0, 0, 0);
    dma_display->flipDMABuffer();
}

IPAddress local_IP(192, 168, 0, 47); // Change 47 to whatever IP you want it to always be
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

void setup() {
    Serial.begin(115200);  
    nfc.begin();
    nfc.reset();
    nfc.setupRF();

    if (!WiFi.config(local_IP, gateway, subnet)) {
        Serial.println("Static IP configuration failed!");
    }

    WiFi.begin(ssid, password);
    delay(1000);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
    }
    
    initStorage();
    loadSettings();
    initWebServer();

    dma_display_init();
    draw_amics();
    
    timeClient.begin();
    setenv("TZ", current_settings.timezone.c_str(), 1);
    tzset();
}

void scroll_custom_message(const CustomMessage& msg) {
    // Clear BOTH buffers to prevent ghosting from previous animations
    clear_area(0, 0, 191, 64);
    dma_display->flipDMABuffer();
    clear_area(0, 0, 191, 64);
    dma_display->flipDMABuffer();
    
    bool using_custom_icon = false;

    if (msg.icon_file != "" && LittleFS.exists(msg.icon_file)) {
        File file = LittleFS.open(msg.icon_file, "r");
        if (file) {
            file.read((uint8_t*)current_icon_buffer, 8192); 
            file.close();
            using_custom_icon = true;
            
            // Replicating original draw_amics double-buffer logic
            clear_area(192, 0, 255, 64);
            for(int y = 0; y < 64; y++) {
                for(int x = 0; x < 64; x++) {
                    dma_display->drawPixel(192 + x, y, current_icon_buffer[y * 64 + x]);
                }
            }
            dma_display->flipDMABuffer();
            
            clear_area(192, 0, 255, 64);
            for(int y = 0; y < 64; y++) {
                for(int x = 0; x < 64; x++) {
                    dma_display->drawPixel(192 + x, y, current_icon_buffer[y * 64 + x]);
                }
            }
            dma_display->flipDMABuffer();
        }
    } 
    
    if (!using_custom_icon) {
        draw_amics(); 
    }
    
    const GFXfont* fontToUse;
    switch (msg.font_id) {
        case 0: fontToUse = &FreeSans9pt7b; break;
        case 1: fontToUse = &FreeSans12pt7b; break;
        case 2: fontToUse = &FreeSans18pt7b; break;
        default: fontToUse = &FreeSans18pt7b; break;
    }
    dma_display->setFont(fontToUse);
    
    int16_t x1, y1; uint16_t w, h;
    dma_display->getTextBounds(msg.text, 0, 0, &x1, &y1, &w, &h);
    
    // Start at 192 (right edge) and scroll EXACTLY until text is completely offscreen left
    int limit = -((int)w) - 10;
    for (int x = 192; x >= limit; x--) {
        dma_display->startWrite();
        drawText(x, 40, msg.text.c_str(), msg.color_565, fontToUse);
        dma_display->endWrite();
        
        dma_display->flipDMABuffer(); 
        
        // Exact original black text erasure
        dma_display->startWrite();
        drawText(x + 1, 40, msg.text.c_str(), 0, fontToUse);
        dma_display->endWrite();
        
        delay(current_settings.scroll_speed_ms);
        yield();
    }
}

void loop() {
if (pending_settings_save || pending_users_save || pending_icon_save) {
    dma_display->stopDMAoutput();
        if (pending_settings_save) {
            File file = LittleFS.open("/settings.json", "w");
            if (file) { file.write(pending_settings_data, pending_settings_length); file.close(); }
            loadSettings(); // Update RAM variables live
            dma_display->setBrightness8(current_settings.brightness); // Apply brightness instantly
            pending_settings_save = false;
        }
        if (pending_users_save) {
            File file = LittleFS.open("/users.json", "w");
            if (file) { file.write(pending_users_data, pending_users_length); file.close(); }
            pending_users_save = false;
        }
        if (pending_icon_save) {
            if (!LittleFS.exists("/icons")) { LittleFS.mkdir("/icons"); }
            File file = LittleFS.open(pending_icon_filename, "w");
            if (file) { file.write(pending_icon_data, pending_icon_length); file.close(); }
            pending_icon_save = false;
        }
        ESP.restart(); 
    }

    if (WiFi.status() != WL_CONNECTED) { WiFi.reconnect(); delay(1000); }
       timeClient.update(); 
    yield();
    int hour = timeClient.getHours();
    
   /* if (hour >= current_settings.screen_off_hour && hour < current_settings.screen_on_hour) {
        dma_display->fillScreenRGB888(0, 0, 0);
        dma_display->flipDMABuffer();
        while (hour < current_settings.screen_on_hour) {
            try_uid_read_delay(10000); 
            timeClient.update();       
            hour = timeClient.getHours();
            yield();                   
        }
        draw_amics();
    }*/

    if (current_settings.show_eu_sol_corp) {
        clear_area(0, 0, 192, 64);
        draw_eu_sol_corp(dma_display);
        dma_display->flipDMABuffer();
        try_uid_read_delay(3000);
    }
    
    if (current_settings.show_bones_festes) { bones_festes_AGRES(); }
    
    if (current_settings.show_eu_erasmus) {
        clear_area(0, 0, 192, 64);
        draw_eu_erasmus(dma_display);
        dma_display->flipDMABuffer();
        try_uid_read_delay(3000);
    }
    
    if (current_settings.show_clock) { clock(dma_display, &timeClient, 30000, try_uid_read_delay); }
    if (current_settings.show_i_love_agres) { i_love_agres_animation(dma_display, 30000, try_uid_read_delay); }

    for (const CustomMessage& msg : current_settings.custom_messages) {
        scroll_custom_message(msg);
        try_uid_read_delay(1000);
    }
}