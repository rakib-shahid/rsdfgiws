#pragma once
#include "Core/Screen.h"

class NowPlayingScreen : public Screen {
public:
  void begin(TFT_eSPI &tft, AppContext &ctx) override;
  void update(TFT_eSPI &tft, AppContext &ctx) override;
  void handleTouch(int x, int y, AppContext &ctx) override;
};
