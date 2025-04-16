#include "ImageFunctions.h"
#include <TJpg_Decoder.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <TFT_eSPI.h>
#include "SpotifyFunctions.h"

static TFT_eSPI *tftPtr = nullptr;

static bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    if (!tftPtr)
        return false;
    if (y >= tftPtr->height())
        return false;
    tftPtr->pushImage(x, y, w, h, bitmap);
    return true;
}

bool drawJPGFromURL(TFT_eSPI &tft, const char *url, int16_t x, int16_t y)
{
    Serial.printf("[JPEG] Starting download: %s\n", url);
    unsigned long start = millis();

    // http.setReuse(true);
    // WiFiClientSecure *client = new WiFiClientSecure;
    // client->setInsecure();

    // HTTPClient http;
    // http.begin(*client, url);
    http.begin(url);

    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK)
    {
        Serial.printf("[JPEG] HTTP GET failed: %d\n", httpCode);
        return false;
    }

    unsigned long afterGet = millis();
    Serial.printf("[JPEG] HTTP GET success. Time: %lu ms\n", afterGet - start);

    WiFiClient *stream = http.getStreamPtr();
    int contentLength = http.getSize();

    Serial.printf("[JPEG] Content length: %d bytes\n", contentLength);

    uint8_t *jpegData = (uint8_t *)malloc(contentLength);
    if (!jpegData)
    {
        Serial.println("[JPEG] Failed to allocate memory for JPEG data");
        return false;
    }
    Serial.printf("[JPEG] Memory allocation done. Time: %lu ms\n", millis() - afterGet);

    unsigned long readStart = millis();
    int bytesRead = stream->readBytes(jpegData, contentLength);
    unsigned long readEnd = millis();

    if (bytesRead != contentLength)
    {
        Serial.printf("[JPEG] Incomplete read: got %d of %d bytes\n", bytesRead, contentLength);
        free(jpegData);
        return false;
    }
    Serial.printf("[JPEG] Data read done. Time: %lu ms\n", readEnd - readStart);

    tftPtr = &tft;
    TJpgDec.setCallback(tft_output);
    TJpgDec.setJpgScale(1);
    TJpgDec.setSwapBytes(true);

    Serial.println("[JPEG] Starting decode/draw...");
    unsigned long decodeStart = millis();
    JRESULT result = TJpgDec.drawJpg(x, y, jpegData, contentLength);
    unsigned long decodeEnd = millis();

    Serial.printf("[JPEG] Decode result: %d, Time: %lu ms\n", result, decodeEnd - decodeStart);

    free(jpegData);
    Serial.printf("[JPEG] Total time: %lu ms\n", millis() - start);

    tftPtr = nullptr;

    return (result == JDR_OK);
}
