#include "Interface.h"
#include "Spotify/SpotifyTypes.h"
#include <HTTPClient.h>

HTTPClient playbackHttpClient;

void drawPlaybackControls(TFT_eSPI &tft) {
  // tft.drawRect(2, 298, 476, 18, TFT_WHITE);
  // tft.drawRect(2, 268, 476, 48, TFT_WHITE);
  tft.drawRect(3, 248, 158, 68, TFT_WHITE);
  tft.drawRect(3 + 158, 248, 158, 68, TFT_WHITE);
  tft.drawRect(3 + 158 * 2, 248, 158, 68, TFT_WHITE);
}

void wipePlaybackControls(TFT_eSPI &tft) {
  tft.fillRect(3, 248, 480, 68, TFT_BLACK);
}

void drawInitialProgressBar(TFT_eSPI &tft) {
  tft.drawRect(3, 220, 474, 20, TFT_WHITE);
}

void wipeInitialProgressBar(TFT_eSPI &tft) {
  tft.fillRect(3, 220, 474, 20, TFT_BLACK);
}

void drawProgressBar(TFT_eSPI &tft, bool isActive, PlayerData currentData,
                     PlayerData lastData) {
  if (currentData.progress_ms < lastData.progress_ms) {
    // clear the progress bar
    if (isActive) {
      tft.fillRect(5, 222, 470, 16, TFT_BLACK);
    } else {
      tft.fillRect(320, 304, 160, 16, TFT_BLACK);
    }
  }
  int progressTotalWidth;
  // calculate progress
  if (isActive) {
    progressTotalWidth = 470;
  } else {
    progressTotalWidth = 160;
  }
  int progress =
      (currentData.progress_ms * progressTotalWidth) / currentData.total_ms;
  // draw progress
  if (isActive) {
    tft.fillRect(5, 222, progress, 16, TFT_WHITE);
  } else {
    tft.fillRect(320, 304, progress, 16, TFT_WHITE);
  }
}

void wipeProgressBar(TFT_eSPI &tft, bool isActive, PlayerData playerData) {
  int progressTotalWidth;
  // calculate progress
  if (isActive) {
    progressTotalWidth = 470;
  } else {
    progressTotalWidth = 160;
  }
  int progress =
      (playerData.progress_ms * progressTotalWidth) / playerData.total_ms;
  // draw progress
  if (isActive) {
    tft.fillRect(5, 222, progress, 16, TFT_BLACK);
  } else {
    tft.fillRect(320, 304, progress, 16, TFT_BLACK);
  }
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
