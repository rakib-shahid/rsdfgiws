#pragma once
#include "TFTSetup.h"
#include "SpotifyFunctions.h"

enum ButtonRegion
{
    NONE,
    PREV,
    PLAY_PAUSE,
    NEXT
};
void drawPlaybackControls(TFT_eSPI &tft);
void drawInitialProgressBar(TFT_eSPI &tft);
void drawProgressBar(TFT_eSPI &tft, SpotifyData currentData, SpotifyData lastData);
void handlePlaybackControls(int x, int y);
ButtonRegion getButtonRegion(int x, int y);
