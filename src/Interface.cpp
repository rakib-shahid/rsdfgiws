#include "Interface.h"
#include <HTTPClient.h>

HTTPClient playbackHttpClient;

void drawPlaybackControls(TFT_eSPI &tft) {
  // tft.drawRect(2, 298, 476, 18, TFT_WHITE);
  // tft.drawRect(2, 268, 476, 48, TFT_WHITE);
  tft.drawRect(3, 248, 158, 68, TFT_WHITE);
  tft.drawRect(3 + 158, 248, 158, 68, TFT_WHITE);
  tft.drawRect(3 + 158 * 2, 248, 158, 68, TFT_WHITE);
}

void drawInitialProgressBar(TFT_eSPI &tft) {
  tft.drawRect(3, 220, 474, 20, TFT_WHITE);
}

void drawProgressBar(TFT_eSPI &tft, PlayerData currentData,
                     PlayerData lastData) {
  if (playerData.progress_ms < lastPlayerData.progress_ms) {
    // clear the progress bar
    tft.fillRect(5, 222, 470, 16, TFT_BLACK);
  }

  // calculate the progress
  int progress = (currentData.progress_ms * 470) / currentData.total_ms;
  tft.fillRect(5, 222, progress, 16, TFT_WHITE);
}

void handlePlaybackControls(TokenInfo &tokenInfo, ButtonRegion region, int x,
                            int y) {
  switch (region) {
  // previous button
  case PREV:
    Serial.println("Previous button pressed");
    skipToPreviousTrack(tokenInfo.accessToken, playbackHttpClient);
    break;
  // play/pause button
  case PLAY_PAUSE:
    Serial.println("Play/Pause button pressed");
    if (playerData.is_playing) {
      togglePause(tokenInfo.accessToken, playbackHttpClient);
    } else {
      togglePlay(tokenInfo.accessToken, playbackHttpClient);
    }
    break;
  // next button
  case NEXT:
    Serial.println("Next button pressed");
    skipToNextTrack(tokenInfo.accessToken, playbackHttpClient);
    break;
  // seek bar
  case SEEK_BAR:
    Serial.println("Seek bar touched");
    seekTo(tokenInfo, x, playbackHttpClient);

  default:
    break;
  }
}

ButtonRegion getButtonRegion(int x, int y) {
  if (x >= 3 && x <= 161 && y >= 248 && y <= 316)
    return PREV;
  else if (x >= 161 && x <= 319 && y >= 248 && y <= 316)
    return PLAY_PAUSE;
  else if (x >= 319 && x <= 477 && y >= 248 && y <= 316)
    return NEXT;
  else if (x >= 0 && x <= 477 && y >= 220 && y <= 236)
    return SEEK_BAR;
  else
    return NONE;
}
