// TFTSetup.h
#pragma once

#include <TFT_eSPI.h>
#include <Wire.h>
#include <Adafruit_FT6206.h>

// Declare a global TFT_eSPI object
extern TFT_eSPI tft;
extern Adafruit_FT6206 touch;
extern const int tft_width;
extern const int tft_height;

// Function to initialize the TFT display
void initializeTFT();
void initializeTouch();
