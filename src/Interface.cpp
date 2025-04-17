#include "Interface.h"

void drawPlaybackControls(TFT_eSPI &tft)
{
    // tft.drawRect(2, 298, 476, 18, TFT_WHITE);
    // tft.drawRect(2, 268, 476, 48, TFT_WHITE);
    tft.drawRect(3, 248, 158, 68, TFT_WHITE);
    tft.drawRect(3 + 158, 248, 158, 68, TFT_WHITE);
    tft.drawRect(3 + 158 * 2, 248, 158, 68, TFT_WHITE);
}

void drawInitialProgressBar(TFT_eSPI &tft)
{
    tft.drawRect(3, 220, 474, 20, TFT_WHITE);
}

void drawProgressBar(TFT_eSPI &tft, SpotifyData currentData, SpotifyData lastData)
{
    if (spotifyData.progress_ms < lastSpotifyData.progress_ms)
    {
        // clear the progress bar
        tft.fillRect(5, 222, 470, 16, TFT_BLACK);
    }

    // calculate the progress
    int progress = (currentData.progress_ms * 470) / currentData.total_ms;
    tft.fillRect(5, 222, progress, 16, TFT_WHITE);
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
        if (spotifyData.is_playing)
        {
            togglePause(accessToken);
        }
        else
        {
            togglePlay(accessToken);
        }
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
