#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSerif9pt7b.h>
#include "FreeSans8pt7b.h"

static const int RADIUS = 4;
static const float ANGLE_STEP = TWO_PI / 5;
static const float INNER_RADIUS = RADIUS * 0.382;
static float cosValues[5], sinValues[5];
static float innerCosValues[5], innerSinValues[5];
static const uint16_t color = 0xFFE0;
static const uint16_t blue = 0x1F;

void precomputeTrigValues()
{
    float angle = -PI / 2;
    for (int i = 0; i < 5; i++)
    {
        cosValues[i] = cos(angle);
        sinValues[i] = sin(angle);
        innerCosValues[i] = cos(angle + ANGLE_STEP / 2);
        innerSinValues[i] = sin(angle + ANGLE_STEP / 2);
        angle += ANGLE_STEP;
    }
}

void draw_star(int x, int y, MatrixPanel_I2S_DMA *dma_display)
{
    static bool trigComputed = false;
    if (!trigComputed)
    {
        precomputeTrigValues();
        trigComputed = true;
    }

    int cx = x, cy = y;
    for (int i = 0; i < 5; i++)
    {
        int x1 = x + RADIUS * cosValues[i];
        int y1 = y + RADIUS * sinValues[i];

        int x2 = x + INNER_RADIUS * innerCosValues[i];
        int y2 = y + INNER_RADIUS * innerSinValues[i];

        int x3 = x + RADIUS * cosValues[(i + 1) % 5];
        int y3 = y + RADIUS * sinValues[(i + 1) % 5];

        dma_display->drawLine(cx, cy, x1, y1, color);
        dma_display->drawLine(cx, cy, x2, y2, color);
        dma_display->drawLine(cx, cy, x3, y3, color);

        dma_display->drawLine(x1, y1, x2, y2, color);
        dma_display->drawLine(x2, y2, x3, y3, color);
    }
}

void draw_eu_flag(MatrixPanel_I2S_DMA *dma_display)
{
    const int centerX = 48;  
    const int centerY = 32;
    const int radius = 21;
    const int numStars = 12;
    static int starX[numStars], starY[numStars];
    static bool precomputed = false;

    if (!precomputed)
    {
        for (int i = 0; i < numStars; i++)
        {
            float angle = TWO_PI * i / numStars - PI / 2;
            starX[i] = centerX + radius * cos(angle);
            starY[i] = centerY + radius * sin(angle);
        }
        precomputed = true;
    }

    dma_display->fillRect(0, 0, 96, 64, blue);

    for (int i = 0; i < numStars; i++)
    {
        draw_star(starX[i], starY[i], dma_display);
    }
}

void draw_eu_sol_corp(MatrixPanel_I2S_DMA *dma_display){
    draw_eu_flag(dma_display);
    dma_display->setFont(&FreeSans8pt7b);
    dma_display->setCursor(97, 20);
    dma_display->setTextSize(1);
    dma_display->setTextColor(blue);
    dma_display->print("EUROPEAN");

    dma_display->setCursor(97, 36);
    dma_display->setTextColor(dma_display->color565(255, 0, 255));
    dma_display->print("SOLIDARITY");

    dma_display->setCursor(97, 52);
    dma_display->setTextColor(dma_display->color565(30, 30, 255));
    dma_display->print("CORPS");
    dma_display->fillRect(190, 0, 2, 64, blue);
}

void draw_eu_erasmus(MatrixPanel_I2S_DMA *dma_display){
    draw_eu_flag(dma_display);
    dma_display->setFont(&FreeSans8pt7b);
    dma_display->setCursor(98, 60);
    dma_display->setTextSize(1);
    dma_display->setTextColor(blue);
    dma_display->print("ERASMUS+");
}

