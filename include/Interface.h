#pragma once
#include "TFTSetup.h"
enum ButtonRegion
{
    NONE,
    PREV,
    PLAY_PAUSE,
    NEXT
};
void drawPlaybackControls(TFT_eSPI &tft);
void handlePlaybackControls(int x, int y);
ButtonRegion getButtonRegion(int x, int y);
