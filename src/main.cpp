#include "Core/AppContext.h"
#include "ImageFunctions.h"
#include "Interface.h"
#include "Secrets.h"
#include "Spotify/SpotifyClient.h"
#include "Spotify/TokenManager.h"
#include "TFTSetup.h"
#include "WifiSetup.h"

const int ledPin = 27;
unsigned long lastSpotifyUpdate = 0;
const unsigned long spotifyInterval = 200;

ButtonRegion activeButton = NONE;
bool wasTouched = false;
TokenInfo tokenInfo;
AppContext appContext;

void setup() {
  Serial.begin(115200);
  delay(1000);
  initializeTFT();
  initializeTouch();

  tokenInfo.tokenFileName = "/token.txt";
  tokenInfo.authTokenFileName = "/authtoken.txt";
  tokenInfo.accessTokenFileName = "/accesstoken.txt";
  tokenInfo.refreshTokenFileName = "/refreshtoken.txt";
  tokenInfo.loggedIn = false;
  appContext.tokens = tokenInfo;

  tft.println("Initializing wifi");
  setupWifi(tft, appContext);

  tft.println("Wifi initialized");

  tft.println("Initializing SPIFFS");
  // token stuff
  initializeFileSystem();
  tft.println("SPIFFS initialized");
  tft.println("Initializing Spotify tokens");
  readAccessToken(appContext.tokens);
  String authCode = "";
  // check token validity:
  if (!isAccessTokenValid(appContext.tokens)) {
    Serial.println("Access token is invalid. Refreshing...");
    if (!isRefreshTokenValid(appContext.tokens)) {
      Serial.println("Refresh token is invalid. Checking auth code...");
      // if (!validateAuthCode(appContext.tokens)) {
      // Serial.println("Auth code invalid, getting a new auth code...");
      getAuthorizationCode(appContext.tokens);
      // }
      authCode = getToken(appContext.tokens.authTokenFileName);
      exchangeCodeForToken(appContext.tokens, authCode);
      readAccessToken(appContext.tokens);
    } else {
      Serial.println(
          "Refresh token is valid. Exchanging for new access token...");
      refreshSpotifyTokens(appContext.tokens);
    }
  } else {
    Serial.println("Access token is valid.");
  }
  tft.println("Spotify tokens initialized");
  SPIFFS.end();
  // String reft = getToken(appContext.tokens.refreshTokenFileName);
  // String accesst = getToken(appContext.tokens.accessTokenFileName);
  // Serial.println("[TOKENS] ");
  // Serial.println(reft);
  // Serial.println(accesst);
  // Serial.println("[END OF TOKENS]");
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  startSpotifyTask(appContext.tokens);

  drawPlaybackControls(tft);
  drawInitialProgressBar(tft);
}

// map coordinates since touches register weird
void mapTouchCoordinates(int &x, int &y) {
  int temp;
  switch (tft.getRotation()) {
  case 1:
    temp = x;
    x = y;
    y = tft.height() - temp;
    break;
  case 2:
    x = tft.height() - x;
    y = tft.width() - y;
    break;
  case 3:
    temp = x;
    x = tft.width() - y;
    y = temp;
    break;
  default:
    break;
  }
}

void loop() {
  if (xSemaphoreTake(spotifyMutex, 10 / portTICK_PERIOD_MS)) {
    if (hasSongChanged(playerData, lastPlayerData)) {
      if (playerData.is_playing) {
        // tft.fillScreen(TFT_BLACK);
        // clear image and text
        tft.fillRect(0, 0, tft.width(), 210, TFT_BLACK);

        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(2);
        tft.setCursor(330, 0);
        tft.setTextSize(1);
        tft.println(playerData.name);
        tft.setCursor(330, 10);
        tft.println(playerData.artist);
        tft.setCursor(330, 20);
        tft.println(playerData.progress_ms / 1000);

        String rawUrl = "http://rsdfgiws.rakibshahid.com/raw?url=" +
                        playerData.album_art_url;
        drawRawImageFromURL(tft, rawUrl.c_str(), 0, 0);
      } else {
        tft.fillRect(0, 0, tft.width(), 210, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(2);
        tft.setCursor(330, 0);
        tft.setTextSize(1);
        tft.println("Paused");
      }
    }
    if (playerData.is_playing &&
        hasProgressChanged(playerData, lastPlayerData)) {
      drawProgressBar(tft, playerData, lastPlayerData);
    }
    lastPlayerData = playerData;
    xSemaphoreGive(spotifyMutex);
  }

  if (touch.touched()) {
    TS_Point p = touch.getPoint();
    int x = p.x;
    int y = p.y;
    mapTouchCoordinates(x, y);
    // ignore noise
    if (x > 3 && x < 477 && y > 2 && y < 318) {
      ButtonRegion region = getButtonRegion(x, y);

      if (!wasTouched) {
        wasTouched = true;

        if (region != NONE) {
          activeButton = region;
          digitalWrite(ledPin, HIGH);
          handlePlaybackControls(appContext.tokens, region, x, y);
        }
      } else {
        if (region == NONE) {
          // Serial.printf("Touch at: %d, %d\n", x, y);
          // tft.fillCircle(x, y, 2, TFT_WHITE);
          digitalWrite(ledPin, HIGH);
        }
      }
    }
  } else {
    wasTouched = false;
    activeButton = NONE;
    digitalWrite(ledPin, LOW);
  }
}
