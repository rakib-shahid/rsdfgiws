#include <TFTSetup.h>

// TFT_eSPI tft = TFT_eSPI();
const int sda_pin = 21;
const int scl_pin = 19;

const int tft_width = 480;
const int tft_height = 320;

const int backlight_pin = 22;

TFT_eSPI tft = TFT_eSPI(tft_height, tft_width);
Adafruit_FT6206 touch = Adafruit_FT6206();

void initializeTFT()
{
    // initialize tft
    tft.init();
    tft.setRotation(0);
    tft.fillScreen(TFT_BLACK);

    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(0, 0);
    tft.println("Hello, World!");

    // backlight high
    pinMode(backlight_pin, OUTPUT);
    digitalWrite(backlight_pin, HIGH);
}

void initializeTouch()
{

    // wire for touch
    Wire.begin(sda_pin, scl_pin);
    if (!touch.begin(40))
    {
        Serial.println("Touch controller not found!");
        while (1)
            ; // Stop if touch controller fails
    }
    Serial.println("Touch controller initialized.");
}