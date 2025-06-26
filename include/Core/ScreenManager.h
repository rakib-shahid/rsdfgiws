#pragma once
#include "Screen.h"

class ScreenManager {
private:
  Screen *currentScreen = nullptr;

public:
  void setScreen(Screen *screen, TFT_eSPI &tft, AppContext &ctx);
  void update(TFT_eSPI &tft, AppContext &ctx);
  void handleTouch(int x, int y, AppContext &ctx);
};
