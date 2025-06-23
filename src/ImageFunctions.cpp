#include "TFTSetup.h"
#include "pngle.h"
#include "support_functions.h"
#include <HTTPClient.h>
#define IMG_BUF_SIZE (190 * 190 * 2)

uint16_t imageBuffer[IMG_BUF_SIZE / 2];

// unused afaik, old manual decode
void drawPNGFromURL(TFT_eSPI &tft, const char *url, int16_t x, int16_t y) {
  setPngPosition(x, y);
  unsigned long start = millis();

  HTTPClient http;
  http.begin(url);

  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    http.end();
    return;
  }

  int total = http.getSize();
  WiFiClient *stream = http.getStreamPtr();
  pngle_t *pngle = pngle_new();
  pngle_set_draw_callback(pngle, pngle_on_draw);

  uint8_t buf[1024];
  int remain = 0;
  int len;
  uint32_t timeout = 0;

  while (http.connected() && (total > 0 || total == -1)) {
    size_t size = stream->available();
    if (timeout > 2000)
      break;
    if (!size) {
      delay(2);
      timeout++;
      continue;
    }

    size = std::min(size, sizeof(buf) - remain);
    if ((len = stream->readBytes(buf + remain, size)) > 0) {
      int fed = pngle_feed(pngle, buf, remain + len);
      if (fed < 0) {
        Serial.printf("[PNG] ERROR: %s\n", pngle_error(pngle));
        break;
      }
      remain = remain + len - fed;
      if (remain > 0)
        memmove(buf, buf + fed, remain);
      total -= len;
    }
  }

  if (pc) {
    tft.pushImage(png_dx + sx, png_dy + sy, pc, 1, lbuf);
    pc = 0;
  }

  pngle_destroy(pngle);
  http.end();

  Serial.printf("[PNG] Total time: %lu ms\n", millis() - start);
}

// offloaded decode, faster but shittier
bool drawRawImageFromURL(TFT_eSPI &tft, const char *url, int16_t x, int16_t y) {
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[RAW] HTTP GET failed: %d\n", httpCode);
    http.end();
    return false;
  }

  WiFiClient *stream = http.getStreamPtr();
  int len = http.getSize();
  if (len <= 4) {
    Serial.println("[RAW] Invalid image size");
    http.end();
    return false;
  }

  uint8_t header[4];
  if (stream->readBytes(header, 4) != 4) {
    Serial.println("[RAW] Failed to read header");
    http.end();
    return false;
  }

  int width = (header[0] << 8) | header[1];
  int height = (header[2] << 8) | header[3];

  int pixels = width * height;
  int dataLen = pixels * 2;

  int bytesRead = stream->readBytes((uint8_t *)imageBuffer, dataLen);
  if (bytesRead != dataLen) {
    Serial.printf("[RAW] Incomplete read: got %d / %d\n", bytesRead, dataLen);
    http.end();
    return false;
  }

  tft.pushImage(x, y, width, height, imageBuffer);
  http.end();

  Serial.printf("[RAW] Drew image %dx%d at (%d, %d)\n", width, height, x, y);
  return true;
}
