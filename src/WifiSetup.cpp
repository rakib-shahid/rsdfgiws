#include <WifiSetup.h>
#include <ESP32Ping.h>
#include <TFT_eSPI.h>

AsyncWebServer server(80);
DNSServer dns;

bool setupWifi(TFT_eSPI &tft)
{
    AsyncWiFiManager wifiManager(&server, &dns);

    wifiManager.autoConnect("rsdfgiws");

    tft.println(WiFi.localIP());
    Serial.println(WiFi.localIP());

    if (WiFi.status() == WL_CONNECTED)
    {
        tft.println("WiFi connected");
        Serial.println("WiFi connected");
        return true;
    }
    else
    {
        tft.println("WiFi not connected");
        Serial.println("WiFi not connected");
        return false;
    }
}

bool pingTest(bool connected)
{
    if (connected)
    {
        if (Ping.ping("www.google.com"))
        {
            Serial.println("Ping successful");
            return true;
        }
        else
        {
            Serial.println("Ping failed");
            return false;
        }
    }
    else
    {
        Serial.println("Not connected to WiFi");
        return false;
    }
}