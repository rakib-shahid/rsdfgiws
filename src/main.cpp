#include "TFTSetup.h"
#include "Secrets.h"
#include "TokenManager.h"
#include "SpotifyFunctions.h"
#include "WifiSetup.h"

const int ledPin = 27;
unsigned long lastSpotifyUpdate = 0;
const unsigned long spotifyInterval = 200;

void setup()
{
    Serial.begin(115200);
    initializeTFT();
    initializeTouch();

    setupWifi(tft);

    // token stuff
    initializeFileSystem();
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

    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
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
    unsigned long currentMillis = millis();
    if (currentMillis - lastSpotifyUpdate >= spotifyInterval)
    {
        lastSpotifyUpdate = currentMillis;
        getCurrentlyPlayingTrack(accessToken);
    }

    if (spotifyData.is_playing && hasSongChanged(spotifyData, lastSpotifyData))
    {
        lastSpotifyData = spotifyData;
        // tft.fillScreen(TFT_BLACK);
        tft.fillRect(0, 60, tft_width, 80, TFT_BLACK);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);
        tft.setTextSize(2);
        tft.setCursor(0, 60);
        // tft.println("Currently Playing:");
        tft.setTextSize(1);
        tft.println(spotifyData.name);
        tft.println(spotifyData.artist);
        tft.println("Volume: " + String(spotifyData.volume));
    }

    if (touch.touched())
    {
        TS_Point p = touch.getPoint();
        int x = p.x;
        int y = p.y;
        mapTouchCoordinates(x, y);
        // Serial.println("LED ON");
        // if (x > 0 && y > 0)
        // {
        //     digitalWrite(ledPin, HIGH);
        //     Serial.printf("Touch at: %d, %d\n", x, y);
        //     tft.fillCircle(x, y, 5, TFT_WHITE);
        // }
    }
    else
    {
        digitalWrite(ledPin, LOW);
        // Serial.println("LED OFF");
    }
}
