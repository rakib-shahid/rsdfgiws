#include "Core/ScreenManager.h"

void ScreenManager::setScreen(Screen *screen, TFT_eSPI &tft, AppContext &ctx) {
  currentScreen = screen;
  currentScreen->begin(tft, ctx);
}

void ScreenManager::update(TFT_eSPI &tft, AppContext &ctx) {
  if (currentScreen) {
    currentScreen->update(tft, ctx);
  }
}

void ScreenManager::handleTouch(int x, int y, AppContext &ctx) {
  if (currentScreen) {
    currentScreen->handleTouch(x, y, ctx);
  }
}
