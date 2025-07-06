#include "TFTSetup.h"
#include "pngle.h"
#include "support_functions.h"
#include <HTTPClient.h>
#include <lvgl.h>
#include "Core/AppContext.h"
#define IMG_BUF_SIZE (190 * 190 * 2)

uint16_t imageBuffer[0];

// unused afaik, old manual decode
void drawPNGFromURL(TFT_eSPI &tft, const char *url, int16_t x, int16_t y)
{
    setPngPosition(x, y);
    unsigned long start = millis();

    HTTPClient http;
    http.begin(url);

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK)
    {
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

    while (http.connected() && (total > 0 || total == -1))
    {
        size_t size = stream->available();
        if (timeout > 2000)
            break;
        if (!size)
        {
            delay(2);
            timeout++;
            continue;
        }

        size = std::min(size, sizeof(buf) - remain);
        if ((len = stream->readBytes(buf + remain, size)) > 0)
        {
            int fed = pngle_feed(pngle, buf, remain + len);
            if (fed < 0)
            {
                Serial.printf("[PNG] ERROR: %s\n", pngle_error(pngle));
                break;
            }
            remain = remain + len - fed;
            if (remain > 0)
                memmove(buf, buf + fed, remain);
            total -= len;
        }
    }

    if (pc)
    {
        tft.pushImage(png_dx + sx, png_dy + sy, pc, 1, lbuf);
        pc = 0;
    }

    pngle_destroy(pngle);
    http.end();

    Serial.printf("[PNG] Total time: %lu ms\n", millis() - start);
}

// offloaded decode, faster but shittier
bool drawRawImageFromURL(TFT_eSPI &tft, const char *url, int16_t x, int16_t y)
{
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK)
    {
        Serial.printf("[RAW] HTTP GET failed: %d\n", httpCode);
        http.end();
        return false;
    }

    WiFiClient *stream = http.getStreamPtr();
    int len = http.getSize();
    if (len <= 4)
    {
        Serial.println("[RAW] Invalid image size");
        http.end();
        return false;
    }

    uint8_t header[4];
    if (stream->readBytes(header, 4) != 4)
    {
        Serial.println("[RAW] Failed to read header");
        http.end();
        return false;
    }

    int width = (header[0] << 8) | header[1];
    int height = (header[2] << 8) | header[3];

    int pixels = width * height;
    int dataLen = pixels * 2;

    int bytesRead = stream->readBytes((uint8_t *)imageBuffer, dataLen);
    if (bytesRead != dataLen)
    {
        Serial.printf("[RAW] Incomplete read: got %d / %d\n", bytesRead, dataLen);
        http.end();
        return false;
    }

    tft.pushImage(x, y, width, height, imageBuffer);
    http.end();

    Serial.printf("[RAW] Drew image %dx%d at (%d, %d)\n", width, height, x, y);
    return true;
}

bool drawRawImageWithSprite(TFT_eSPI &tft, const char *url, int16_t x,
                            int16_t y)
{
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK)
    {
        Serial.printf("[RAW] HTTP GET failed: %d\n", httpCode);
        http.end();
        return false;
    }

    WiFiClient *stream = http.getStreamPtr();
    if (http.getSize() <= 4)
    {
        Serial.println("[RAW] Invalid image size");
        http.end();
        return false;
    }

    uint8_t header[4];
    if (stream->readBytes(header, 4) != 4)
    {
        Serial.println("[RAW] Failed to read header");
        http.end();
        return false;
    }

    int width = (header[0] << 8) | header[1];
    int height = (header[2] << 8) | header[3];
    int pixels = width * height;
    int dataLen = pixels * 2;

    TFT_eSprite sprite = TFT_eSprite(&tft);
    sprite.setColorDepth(16);
    sprite.setSwapBytes(true); // RGB565 format

    if (!sprite.createSprite(width, height))
    {
        Serial.println("[SPRITE] Failed to create sprite");
        http.end();
        return false;
    }

    // get raw buffer
    uint16_t *buf = (uint16_t *)sprite.getPointer();
    int bytesRead = stream->readBytes((uint8_t *)buf, dataLen);
    if (bytesRead != dataLen)
    {
        Serial.printf("[RAW] Incomplete read: got %d / %d\n", bytesRead, dataLen);
        sprite.deleteSprite();
        http.end();
        return false;
    }

    sprite.pushSprite(x, y);
    sprite.deleteSprite();
    http.end();

    Serial.printf("[SPRITE] Drew sprite %dx%d at (%d, %d)\n", width, height, x,
                  y);
    return true;
}

bool downloadRawImageToBuffer(const char *url, uint16_t *buffer, int expectedW, int expectedH)
{
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK)
    {
        Serial.printf("[RAW] HTTP GET failed: %d\n", httpCode);
        http.end();
        return false;
    }

    WiFiClient *stream = http.getStreamPtr();
    if (http.getSize() <= 4)
    {
        Serial.println("[RAW] Invalid image size");
        http.end();
        return false;
    }

    uint8_t header[4];
    if (stream->readBytes(header, 4) != 4)
    {
        Serial.println("[RAW] Failed to read header");
        http.end();
        return false;
    }

    int width = (header[0] << 8) | header[1];
    int height = (header[2] << 8) | header[3];

    if (width != expectedW || height != expectedH)
    {
        Serial.printf("[RAW] Unexpected image size: %dx%d (expected %dx%d)\n",
                      width, height, expectedW, expectedH);
        http.end();
        return false;
    }

    int dataLen = width * height * 2;
    int bytesRead = stream->readBytes((uint8_t *)buffer, dataLen);
    if (bytesRead != dataLen)
    {
        Serial.printf("[RAW] Incomplete read: got %d / %d\n", bytesRead, dataLen);
        http.end();
        return false;
    }

    for (int i = 0; i < width * height; i++)
    {
        uint16_t px = buffer[i];
        buffer[i] = (px << 8) | (px >> 8); // swap endian
    }

    http.end();
    Serial.printf("[RAW] Downloaded %dx%d image (%d bytes)\n", width, height, dataLen);
    return true;
}

bool drawRawImageWithLVGL(const char *url, int width, int height, AppContext &ctx)
{
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode != HTTP_CODE_OK)
    {
        Serial.printf("[LVGL] HTTP GET failed: %d\n", httpCode);
        http.end();
        return false;
    }

    WiFiClient *stream = http.getStreamPtr();
    if (http.getSize() <= 4)
    {
        Serial.println("[LVGL] Invalid image size");
        http.end();
        return false;
    }

    uint8_t header[4];
    if (stream->readBytes(header, 4) != 4)
    {
        Serial.println("[LVGL] Failed to read header");
        http.end();
        return false;
    }

    int imgW = (header[0] << 8) | header[1];
    int imgH = (header[2] << 8) | header[3];

    if (imgW != width || imgH != height)
    {
        Serial.printf("[LVGL] Image size mismatch: got %dx%d, expected %dx%d\n", imgW, imgH, width, height);
        http.end();
        return false;
    }

    size_t dataLen = width * height * 2;
    if (!ctx.imageBuffer || ctx.imageBufferSize < dataLen)
    {
        if (ctx.imageBuffer)
        {
            free(ctx.imageBuffer);
            ctx.imageBuffer = nullptr;
            ctx.imageBufferSize = 0;
        }

        ctx.imageBuffer = (uint16_t *)heap_caps_malloc(dataLen, MALLOC_CAP_SPIRAM);
        if (!ctx.imageBuffer)
        {
            Serial.println("[LVGL] Failed to allocate image buffer in PSRAM");
            http.end();
            return false;
        }

        ctx.imageBufferSize = dataLen;
    }

    int bytesRead = stream->readBytes((uint8_t *)ctx.imageBuffer, dataLen);
    http.end();

    if (bytesRead != dataLen)
    {
        Serial.printf("[LVGL] Incomplete read: %d / %d\n", bytesRead, dataLen);
        return false;
    }

    // swap endian (big-endian to little-endian for RGB565)
    for (int i = 0; i < width * height; ++i)
    {
        uint16_t px = ctx.imageBuffer[i];
        ctx.imageBuffer[i] = (px << 8) | (px >> 8);
    }

    // update LVGL image descriptor
    ctx.imageDesc.header.w = width;
    ctx.imageDesc.header.h = height;
    ctx.imageDesc.header.cf = LV_COLOR_FORMAT_RGB565;
    ctx.imageDesc.data_size = dataLen;
    ctx.imageDesc.data = (const uint8_t *)ctx.imageBuffer;

    // delete old image object if exists
    if (ctx.imageObj)
    {
        lv_obj_del(ctx.imageObj);
        ctx.imageObj = nullptr;
    }

    if (!ctx.lvScreen)
    {
        Serial.println("[LVGL] Error: ctx.lvScreen is null!");
        return false;
    }

    // create and draw image
    ctx.imageObj = lv_image_create(ctx.lvScreen);
    lv_image_set_src(ctx.imageObj, &ctx.imageDesc);
    lv_obj_set_pos(ctx.imageObj, 0, 0); // top-left corner

    Serial.printf("[DEBUG] img_dsc.w=%d h=%d data=%p\n", width, height, ctx.imageDesc.data);
    Serial.println("✅ Image loaded and displayed");
    return true;
}