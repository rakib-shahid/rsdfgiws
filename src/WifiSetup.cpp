#include <WifiSetup.h>
#include <ESP32Ping.h>
#include <TFT_eSPI.h>

bool setupWifi(TFT_eSPI &tft)
{
    // Create an instance of the WiFiManager library
    WiFiManager wifiManager;

    // Set the name of the access point
    wifiManager.autoConnect("rsdfgiws");

    // Print the IP address of the ESP32
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