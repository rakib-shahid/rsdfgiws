#pragma once

#include "Core/AppContext.h"
#include <WiFi.h>
// #include <WiFiManager.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <TFT_eSPI.h>

extern DNSServer dns;

bool setupWifi(TFT_eSPI &tft, AppContext appContext);
bool pingTest(bool connected);
