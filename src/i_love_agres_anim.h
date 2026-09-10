#pragma once

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Fonts/FreeSans18pt7b.h>
#include <NTPClient.h>
#include "FreeSans8pt7b.h"

void i_love_agres_animation(MatrixPanel_I2S_DMA *dma_display, time_t runTime, void (*call_delay)(time_t))
{
    time_t start = millis();
    dma_display->setTextColor(0xFFFF);
    dma_display->setFont(&FreeSans18pt7b);
    for (uint16_t x = 0; x < 192; x++)
    {
        for (uint16_t y = 0; y < 64; y++)
        {
            dma_display->drawPixel(x, y, 0);
        }
    }
    dma_display->flipDMABuffer();
    for (uint16_t x = 0; x < 192; x++)
    {
        for (uint16_t y = 0; y < 64; y++)
        {
            dma_display->drawPixel(x, y, 0);
        }
    }
    dma_display->setCursor(3, 40);
    dma_display->print("I");
    // heart
    dma_display->setCursor(55, 40);
    dma_display->print("AGRES!");
    dma_display->flipDMABuffer();
    dma_display->setCursor(3, 40);
    dma_display->print("I");
    // heart
    dma_display->setCursor(55, 40);
    dma_display->print("AGRES!");

    while (millis() - start < runTime)
    {
        // Clear only the heart area before redrawing
        for (uint16_t x = 10; x < 55; x++)
        {
            for (uint16_t y = 10; y < 50; y++)
            {
                dma_display->drawPixel(x, y, 0);
            }
        }

        float t = (millis() - start) / 1000.0;
        float scaleFactor = 0.6 + 0.1 * sin(t * 2 * PI);
        
        for (int y = -20; y <= 20; y++)
        {
            for (int x = -20; x <= 20; x++)
            {
                float xs = (x / 18.5) / scaleFactor; // Ensures it fits within 40px width
                float ys = (y / 22.5) / scaleFactor; // Ensures it fits within 40px height
                float heartEq = (xs * xs + ys * ys - 1) * (xs * xs + ys * ys - 1) * (xs * xs + ys * ys - 1) - xs * xs * ys * ys * ys;
        
                if (heartEq <= 0)
                {
                    int px = 35 + x; // Keep it centered within (10,10) to (50,50)
                    int py = 30 - y;
                    dma_display->drawPixel(px, py, dma_display->color565(255, 0, 0));
                }
            }
        }
        dma_display->flipDMABuffer();
        call_delay(10);
    }
}
