#include "TFTSetup.h"
#include "Secrets.h"
#include "TokenManager.h"
#include "SpotifyFunctions.h"
#include "WifiSetup.h"

const int ledPin = 27;

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
    if (touch.touched())
    {
        TS_Point p = touch.getPoint();
        int x = p.x;
        int y = p.y;
        mapTouchCoordinates(x, y);
        // Serial.println("LED ON");
        digitalWrite(ledPin, HIGH);
        Serial.printf("Touch at: %d, %d\n", x, y);
        tft.fillCircle(x, y, 5, TFT_WHITE);
    }
    else
    {
        digitalWrite(ledPin, LOW);
        // Serial.println("LED OFF");
    }
}
