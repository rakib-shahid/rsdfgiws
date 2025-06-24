#pragma once

#include "../Spotify/TokenManager.h"
#include <TFT_eSPI.h>
#include <WiFiClientSecure.h>

struct AppContext {
  TFT_eSPI *tft;
  WiFiClientSecure *wifi;
};
