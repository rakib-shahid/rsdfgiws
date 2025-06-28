#include "UI/NowPlayingScreen.h"
#include "ImageFunctions.h"
#include "Interface.h"
#include "Spotify/SpotifyClient.h"

extern PlayerData playerData;
extern PlayerData lastPlayerData;

void NowPlayingScreen::begin(TFT_eSPI &tft, AppContext &ctx) {
  tft.fillScreen(TFT_BLACK);
  drawPlaybackControls(tft);
  drawInitialProgressBar(tft);
}

void NowPlayingScreen::update(TFT_eSPI &tft, AppContext &ctx) {
  if (xSemaphoreTake(spotifyMutex, 10 / portTICK_PERIOD_MS)) {
    if (hasSongChanged(playerData, lastPlayerData)) {
      tft.fillRect(0, 0, tft.width(), 210, TFT_BLACK);

      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.setCursor(330, 0);
      tft.setTextSize(1);
      tft.println(playerData.name);
      tft.setCursor(330, 10);
      tft.println(playerData.artist);
      tft.setCursor(330, 20);
      tft.println(playerData.progress_ms / 1000);

      String rawUrl =
          "http://rsdfgiws.rakibshahid.com/raw?url=" + playerData.album_art_url;
      // drawRawImageFromURL(tft, rawUrl.c_str(), 0, 0);
      drawRawImageWithSprite(tft, rawUrl.c_str(), 0, 0);
    }
    if (playerData.is_playing &&
        hasProgressChanged(playerData, lastPlayerData)) {
      drawProgressBar(tft, playerData, lastPlayerData);
    }
    lastPlayerData = playerData;
    xSemaphoreGive(spotifyMutex);
  }
}

void NowPlayingScreen::handleTouch(int x, int y, AppContext &ctx) {
  ButtonRegion region = getButtonRegion(x, y);
  if (region != NONE) {
    handlePlaybackControls(ctx.tokens, region, x, y);
  }
}
