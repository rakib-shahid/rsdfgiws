#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

#include <WiFi.h>
#include <WiFiManager.h>
#include <TFT_eSPI.h>

bool setupWifi(TFT_eSPI &tft);
bool pingTest(bool connected);

#endif // WIFI_SETUP_H
