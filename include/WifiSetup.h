#pragma once

#include <WiFi.h>
// #include <WiFiManager.h>
#include <TFT_eSPI.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>

extern AsyncWebServer server;
extern DNSServer dns;

bool setupWifi(TFT_eSPI &tft);
bool pingTest(bool connected);