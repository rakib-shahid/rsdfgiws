#pragma once
#include "Core/AppContext.h"
#include <TFT_eSPI.h>

bool drawPNGFromURL(TFT_eSPI &tft, const char *url, int16_t x, int16_t y);
bool drawRawImageFromURL(TFT_eSPI &tft, const char *url, int16_t x, int16_t y);
bool drawRawImageWithSprite(TFT_eSPI &tft, const char *url, int16_t x,
                            int16_t y);
