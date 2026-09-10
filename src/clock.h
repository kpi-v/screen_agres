#pragma once

#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Fonts/FreeSans18pt7b.h>
#include <NTPClient.h>
#include "FreeSans8pt7b.h"

#include <time.h> // Required for localtime()

void clock(MatrixPanel_I2S_DMA *dma_display, NTPClient *timeClient, time_t run_time, void (*call_delay)(time_t))
{
    // --- INITIALIZATION ---
    dma_display->setTextColor(dma_display->color565(255, 255, 255));
    dma_display->setFont(&FreeSans18pt7b);

    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
    yield();
    timeClient->forceUpdate();
    yield();

    // Variable to track the last second drawn to the screen.
    // Initialize to -1 to guarantee the first frame is always drawn.
    int lastDisplayedSecond = -1;

    time_t start = millis();

    // --- MAIN LOOP ---
    // This loop now runs as fast as possible, with no delay.
    while (millis() - start < run_time)
    {
        // Get the most current time. The NTPClient library updates this in the background.
        time_t epochTime = timeClient->getEpochTime();
        struct tm *timeinfo = localtime(&epochTime);
        int currentSecond = timeinfo->tm_sec;
        yield();

        // FIX: The core logic. Only update the display if the second has changed.
        if (currentSecond != lastDisplayedSecond)
        {
            // A new second has begun, so update the tracking variable.
            lastDisplayedSecond = currentSecond;

            // Clear only the clock area. Adjust values for your font/position.
            for (uint16_t x = 0; x < 192; x++)
            {
                for (uint16_t y = 0; y < 64; y++)
                {
                    dma_display->drawPixel(x, y, 0);
                }
            }

            // Set cursor and print the new time.
            dma_display->setCursor(25, 40);

            if (timeinfo->tm_hour < 10)
                dma_display->print("0");
            dma_display->print(timeinfo->tm_hour);

            dma_display->print(":");
            if (timeinfo->tm_min < 10)
                dma_display->print("0");
            dma_display->print(timeinfo->tm_min);

            dma_display->print(":");
            if (timeinfo->tm_sec < 10)
                dma_display->print("0");
            dma_display->print(timeinfo->tm_sec);

            // Flip the buffer to show the newly drawn time.
            dma_display->flipDMABuffer();
        }

        // No delay here! We want the loop to check the time constantly.
        // We could add a tiny call_delay(10) to be nice to the processor,
        // but it's not required for this logic to work.
        yield();
        call_delay(10); // Optional: prevent watchdog resets and yield to other tasks.
    }
}