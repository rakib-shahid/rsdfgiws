#include "WifiSetup.h"
#include <ESP32Ping.h>
#include <WifiSetup.h>

// AsyncWebServer server(80);
DNSServer dns;

bool setupWifi(TFT_eSPI &tft, AppContext appContext) {
  AsyncWiFiManager wifiManager(appContext.tokens.webserver, &dns);

  wifiManager.setConnectTimeout(20);
  wifiManager.setConfigPortalTimeout(30);

  tft.println("Starting WiFi...");

  bool connected = wifiManager.autoConnect("rsdfgiws");

  if (!connected) {
    tft.println("WiFi failed. Restarting...");
    Serial.println("WiFi failed. Restarting...");
    delay(3000);
    ESP.restart();
  }

  tft.println(WiFi.localIP());
  Serial.println(WiFi.localIP());

  tft.println("WiFi connected");
  Serial.println("WiFi connected");
  return true;
}

bool pingTest(bool connected) {
  if (connected) {
    if (Ping.ping("www.google.com")) {
      Serial.println("Ping successful");
      return true;
    } else {
      Serial.println("Ping failed");
      return false;
    }
  } else {
    Serial.println("Not connected to WiFi");
    return false;
  }
}
