#include "TFTSetup.h"

// Initialize touch controller

// Define the LED pin
const int ledPin = 27;

void setup()
{
    Serial.begin(115200); // Start serial communication
    initializeTFT();
    initializeTouch();

    // // Reset the touch controller
    // pinMode(16, OUTPUT);
    // digitalWrite(16, LOW); // Hold reset
    // delay(100);
    // digitalWrite(16, HIGH); // Release reset

    // Initialize the LED pin as OUTPUT
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW); // Start with LED off
}

void loop()
{

    // Check if the screen is being touched
    if (touch.touched())
    {
        TS_Point p = touch.getPoint(); // Get the touch coordinates

        // Map the touch coordinates to the screen size
        int x = map(p.x, 0, tft_width, 0, tft.width());
        int y = map(p.y, 0, tft_height, 0, tft.height());

        // Turn on the LED while touching the screen
        Serial.println("LED ON");
        digitalWrite(ledPin, HIGH);

        // Print the touch coordinates to Serial Monitor
        Serial.printf("Touch at: %d, %d\n", x, y);

        // Draw a small white circle at the touch point
        tft.fillCircle(x, y, 5, TFT_WHITE);
    }
    else
    {
        // Turn off the LED when no touch is detected
        digitalWrite(ledPin, LOW);
        Serial.println("LED OFF");
    }
}
