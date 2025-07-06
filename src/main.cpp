#include "Core/AppContext.h"
#include "Core/ScreenManager.h"
#include "ImageFunctions.h"
#include "Interface.h"
#include "LVGLSetup.h"
#include "Secrets.h"
#include "Spotify/SpotifyClient.h"
#include "Spotify/TokenManager.h"
#include "TFTSetup.h"
#include "UI/NowPlayingScreen.h"
#include "WifiSetup.h"

const int ledPin = 27;
unsigned long lastSpotifyUpdate = 0;
const unsigned long spotifyInterval = 200;

ButtonRegion activeButton = NONE;
bool wasTouched = false;
TokenInfo tokenInfo;
AppContext appContext;

ScreenManager screenManager;
NowPlayingScreen nowPlayingScreen;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    initializeTFT();
    initializeTouch();

    Serial.printf("PSRAM initialized: %s\n", psramInit() ? "yes" : "no");
    Serial.printf("Total PSRAM: %u bytes\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %u bytes\n", ESP.getFreePsram());
    Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
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
    if (!isAccessTokenValid(appContext.tokens))
    {
        Serial.println("Access token is invalid. Refreshing...");
        if (!isRefreshTokenValid(appContext.tokens))
        {
            Serial.println("Refresh token is invalid. Checking auth code...");
            getAuthorizationCode(appContext.tokens);
            authCode = getToken(appContext.tokens.authTokenFileName);
            exchangeCodeForToken(appContext.tokens, authCode);
            readAccessToken(appContext.tokens);
        }
        else
        {
            Serial.println(
                "Refresh token is valid. Exchanging for new access token...");
            refreshSpotifyTokens(appContext.tokens);
        }
    }
    else
    {
        Serial.println("Access token is valid.");
    }
    tft.println("Spotify tokens initialized");
    SPIFFS.end();
    Serial.println("[SPIFFS] SPIFFS ended");
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    startSpotifyTask(appContext);

    initializeLVGL(appContext);

    screenManager.setScreen(&nowPlayingScreen, tft, appContext);
}

// map coordinates since touches register weird
void mapTouchCoordinates(int &x, int &y)
{
    int temp;
    switch (tft.getRotation())
    {
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

void loop()
{
    static uint32_t last_ms = millis();
    uint32_t now = millis();
    uint32_t diff = now - last_ms;
    lv_timer_handler();
    lv_tick_inc(diff);
    last_ms = now;
    screenManager.update(tft, appContext);

    if (touch.touched())
    {
        TS_Point p = touch.getPoint();
        int x = p.x;
        int y = p.y;
        mapTouchCoordinates(x, y);

        if (x > 3 && x < 477 && y > 2 && y < 318)
        {
            screenManager.handleTouch(x, y, appContext);
        }
    }
}