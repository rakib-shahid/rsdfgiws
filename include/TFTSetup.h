// TFTSetup.h
#ifndef TFTSETUP_H
#define TFTSETUP_H

#include <TFT_eSPI.h>
#include <Wire.h>
#include <Adafruit_FT6206.h>

// Declare a global TFT_eSPI object
extern TFT_eSPI tft;
extern Adafruit_FT6206 touch;

// Function to initialize the TFT display
void initializeTFT();
void initializeTouch();

#endif // TFTSETUP_H
