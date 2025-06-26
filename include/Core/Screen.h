#pragma once
#include "AppContext.h"
#include <TFT_eSPI.h>

class Screen {
public:
  virtual void begin(TFT_eSPI &tft, AppContext &ctx) = 0;
  virtual void update(TFT_eSPI &tft, AppContext &ctx) = 0;
  virtual void handleTouch(int x, int y, AppContext &ctx) = 0;
  virtual ~Screen() {}
};
