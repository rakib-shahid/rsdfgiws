#pragma once

#include "../Spotify/SpotifyTypes.h"
#include "../Spotify/TokenManager.h"
#include <TFT_eSPI.h>

struct AppContext {
  TFT_eSPI *tft;
  TokenInfo tokenInfo;
  PlayerData playerData;
};
