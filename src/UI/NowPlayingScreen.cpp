#include "UI/NowPlayingScreen.h"
#include "ImageFunctions.h"
#include "Interface.h"
#include "Spotify/SpotifyClient.h"
#include "TFTSetup.h"

extern PlayerData playerData;
extern PlayerData lastPlayerData;
unsigned long lastTouch;
unsigned long timeoutValue;
bool isActive;
String imgWidth;

void NowPlayingScreen::begin(TFT_eSPI &tft, AppContext &ctx) {
  tft.fillScreen(TFT_BLACK);
  drawPlaybackControls(tft);
  drawInitialProgressBar(tft);
  timeoutValue = 10000;
  lastTouch = millis();
  isActive = true;
  imgWidth = "210";
}

void NowPlayingScreen::update(TFT_eSPI &tft, AppContext &ctx) {
  if (xSemaphoreTake(spotifyMutex, 10 / portTICK_PERIOD_MS)) {
    if (millis() - lastTouch >= timeoutValue && isActive) {
      Serial.println("More than 10s has passed! Going inactive!");
      isActive = false;
      // draw bigger
      imgWidth = "320";
      String rawUrl = "http://rsdfgiws.rakibshahid.com/raw?url=" +
                      playerData.album_art_url + "&width=" + imgWidth;
      wipePlaybackControls(tft);
      wipeInitialProgressBar(tft);
      drawRawImageWithSprite(tft, rawUrl.c_str(), 0, 0);
    }

    if (hasSongChanged(playerData, lastPlayerData)) {

      tft.setTextColor(TFT_WHITE, TFT_BLACK);
      tft.fillRect(330, 0, 150, 50, TFT_BLACK);
      tft.setCursor(330, 0);
      tft.setTextSize(1);
      tft.println(playerData.name);
      tft.setCursor(330, 10);
      tft.println(playerData.artist);
      tft.setCursor(330, 20);
      tft.println(playerData.progress_ms / 1000);
      // draw regular
      // int canvasW = atoi(imgWidth.c_str());
      // tft.fillRect(0, 0, canvasW, canvasW, TFT_BLACK);

      String rawUrl = "http://rsdfgiws.rakibshahid.com/raw?url=" +
                      playerData.album_art_url + "&width=" + imgWidth;
      drawRawImageWithSprite(tft, rawUrl.c_str(), 0, 0);
    }
    if (playerData.is_playing &&
        hasProgressChanged(playerData, lastPlayerData)) {
      drawProgressBar(tft, isActive, playerData, lastPlayerData);
    }
    lastPlayerData = playerData;
    xSemaphoreGive(spotifyMutex);
  }
}

void NowPlayingScreen::handleTouch(int x, int y, AppContext &ctx) {
  ButtonRegion region = getButtonRegion(x, y);
  lastTouch = millis();
  if (!isActive) {
    Serial.println("Touch detected! No longer inactive!");
    isActive = true;
    // redraw smaller
    int canvasW = atoi(imgWidth.c_str());
    tft.fillRect(0, 0, canvasW, canvasW, TFT_BLACK);
    imgWidth = "210";
    String rawUrl =
        "http://rsdfgiws.rakibshahid.com/raw?url=" + playerData.album_art_url +
        "&width=" + imgWidth;
    wipeProgressBar(tft, false, playerData);
    drawInitialProgressBar(tft);
    drawPlaybackControls(tft);
    drawRawImageWithSprite(tft, rawUrl.c_str(), 0, 0);
  }
  if (region != NONE && isActive) {
    handlePlaybackControls(ctx.tokens, region, x, y);
  }
}
