#include "TFTSetup.h"
#include "Secrets.h"
#include "TokenManager.h"
#include "SpotifyFunctions.h"
#include "WifiSetup.h"
#include "ImageFunctions.h"
#include "Interface.h"

const int ledPin = 27;
unsigned long lastSpotifyUpdate = 0;
const unsigned long spotifyInterval = 200;

ButtonRegion activeButton = NONE;
bool wasTouched = false;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    initializeTFT();
    initializeTouch();
    tft.println("Initializing wifi");
    setupWifi(tft);

    tft.println("Wifi initialized");

    tft.println("Initializing SPIFFS");
    // token stuff
    initializeFileSystem();
    tft.println("SPIFFS initialized");
    tft.println("Initializing Spotify tokens");
    readAccessToken(tokenFileName);

    // check token validity:
    if (!isAccessTokenValid(accessToken))
    {
        Serial.println("Access token is invalid. Refreshing...");
        if (!isRefreshTokenValid())
        {
            Serial.println("Refresh token is invalid. Getting new auth code...");
            getAuthorizationCode();
        }
        else
        {
            Serial.println("Refresh token is valid. Exchanging for new access token...");
            refreshSpotifyTokens(refreshToken);
        }
    }
    else
    {
        Serial.println("Access token is valid.");
    }
    tft.println("Spotify tokens initialized");

    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);

    startSpotifyTask();

    drawPlaybackControls(tft);
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
    if (xSemaphoreTake(spotifyMutex, 10 / portTICK_PERIOD_MS))
    {
        if (spotifyData.is_playing && hasSongChanged(spotifyData, lastSpotifyData))
        {
            lastSpotifyData = spotifyData;
            // tft.fillScreen(TFT_BLACK);
            // clear image and text
            tft.fillRect(0, 0, tft.width(), 235, TFT_BLACK);

            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setTextSize(2);
            tft.setCursor(330, 0);
            tft.setTextSize(1);
            tft.println(spotifyData.name);
            tft.setCursor(330, 10);
            tft.println(spotifyData.artist);
            tft.setCursor(330, 20);
            tft.println(spotifyData.progress_ms / 1000);

            String rawUrl = "http://192.168.1.253:3000/raw?url=" + spotifyData.album_art_url;
            drawRawImageFromURL(tft, rawUrl.c_str(), 0, 0);
        }

        xSemaphoreGive(spotifyMutex);
    }

    if (touch.touched())
    {
        TS_Point p = touch.getPoint();
        int x = p.x;
        int y = p.y;
        mapTouchCoordinates(x, y);
        // ignore noise
        if (x > 3 && x < 477 && y > 2 && y < 318)
        {
            ButtonRegion region = getButtonRegion(x, y);

            if (!wasTouched)
            {
                wasTouched = true;

                if (region != NONE)
                {
                    activeButton = region;
                    digitalWrite(ledPin, HIGH);
                    handlePlaybackControls(x, y);
                }
            }
            else
            {
                if (region == NONE)
                {
                    Serial.printf("Touch at: %d, %d\n", x, y);
                    // tft.fillCircle(x, y, 2, TFT_WHITE);
                    digitalWrite(ledPin, HIGH);
                }
            }
        }
    }
    else
    {
        wasTouched = false;
        activeButton = NONE;
        digitalWrite(ledPin, LOW);
    }
}
