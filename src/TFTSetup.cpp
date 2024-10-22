#include <TFTSetup.h>

// TFT_eSPI tft = TFT_eSPI();

void initializeTFT()
{
    // Initialize the TFT display
    tft.init();
    tft.setRotation(1);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, 0);
    pinMode(22, OUTPUT);
    digitalWrite(22, HIGH);
    tft.fillScreen(TFT_BLACK);
    // tft.println("Hello, World!");
}