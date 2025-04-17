#include "Interface.h"
#include "SpotifyFunctions.h"

void drawPlaybackControls(TFT_eSPI &tft)
{
    // tft.drawRect(2, 298, 476, 18, TFT_WHITE);
    // tft.drawRect(2, 268, 476, 48, TFT_WHITE);
    tft.drawRect(3, 248, 158, 68, TFT_WHITE);
    tft.drawRect(3 + 158, 248, 158, 68, TFT_WHITE);
    tft.drawRect(3 + 158 * 2, 248, 158, 68, TFT_WHITE);
}

void handlePlaybackControls(int x, int y)
{
    if (x >= 3 && x <= 161 && y >= 248 && y <= 316)
    {
        // previous button
        Serial.println("Previous button pressed");
        skipToPreviousTrack(accessToken);
    }
    else if (x >= 161 && x <= 319 && y >= 248 && y <= 316)
    {
        // play/pause button
        Serial.println("Play/Pause button pressed");
        togglePlay(accessToken);
    }
    else if (x >= 319 && x <= 477 && y >= 248 && y <= 316)
    {
        // next button
        Serial.println("Next button pressed");
        skipToNextTrack(accessToken);
    }
}

ButtonRegion getButtonRegion(int x, int y)
{
    if (x >= 3 && x <= 161 && y >= 248 && y <= 316)
        return PREV;
    else if (x >= 161 && x <= 319 && y >= 248 && y <= 316)
        return PLAY_PAUSE;
    else if (x >= 319 && x <= 477 && y >= 248 && y <= 316)
        return NEXT;
    else
        return NONE;
}
