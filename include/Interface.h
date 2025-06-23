#pragma once
#include "Spotify/SpotifyClient.h"
#include "Spotify/TokenManager.h"
#include "TFTSetup.h"

enum ButtonRegion { NONE, PREV, PLAY_PAUSE, SEEK_BAR, NEXT };
void drawPlaybackControls(TFT_eSPI &tft);
void drawInitialProgressBar(TFT_eSPI &tft);
void drawProgressBar(TFT_eSPI &tft, SpotifyData currentData,
                     SpotifyData lastData);
void handlePlaybackControls(TokenInfo &tokenInfo, ButtonRegion button, int x,
                            int y);
ButtonRegion getButtonRegion(int x, int y);
