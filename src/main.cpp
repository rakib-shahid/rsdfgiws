#include <TFT_eSPI.h>        // TFT display library
#include <Wire.h>            // I²C library
#include <Adafruit_FT6206.h> // FT6336U/FT6206 touch controller library

// Initialize TFT display
TFT_eSPI tft = TFT_eSPI(320, 480);

// Initialize touch controller
Adafruit_FT6206 touch = Adafruit_FT6206();

// Define the LED pin
const int ledPin = 27;

void setup()
{
    Serial.begin(115200); // Start serial communication

    // Initialize TFT display
    tft.init();
    tft.setRotation(0);        // Set the rotation (adjust if needed)
    tft.fillScreen(TFT_BLACK); // Clear the screen

    // Initialize I²C with GPIO 19 for SCL and GPIO 21 for SDA
    Wire.begin(21, 19); // SDA on GPIO 21, SCL on GPIO 19

    // Reset the touch controller
    pinMode(16, OUTPUT);
    digitalWrite(16, LOW); // Hold reset
    delay(100);
    digitalWrite(16, HIGH); // Release reset

    // Initialize the LED pin as OUTPUT
    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW); // Start with LED off

    // Initialize the touch controller with sensitivity 40
    if (!touch.begin(40))
    {
        Serial.println("Touch controller not found!");
        while (1)
            ; // Stop if touch controller fails
    }
    Serial.println("Touch controller initialized.");
}

void loop()
{
    // Check if the screen is being touched
    if (touch.touched())
    {
        TS_Point p = touch.getPoint(); // Get the touch coordinates

        // Map the touch coordinates to the screen size
        int x = map(p.x, 0, 320, 0, tft.width());
        int y = map(p.y, 0, 480, 0, tft.height());

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
