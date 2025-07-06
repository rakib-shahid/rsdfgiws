#include "UI/NowPlayingScreen.h"
#include "ImageFunctions.h"
#include "Interface.h"
#include "Spotify/SpotifyClient.h"
#include "TFTSetup.h"

extern PlayerData playerData;
extern PlayerData lastPlayerData;
unsigned long lastTouch;
unsigned long timeoutValue;
bool isActive;
String imgWidth;

static void animate_image_scale(lv_obj_t *img, int32_t scale, uint32_t duration_ms)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, img);
    lv_anim_set_exec_cb(&a, [](void *obj, int32_t v)
                        { lv_image_set_scale(static_cast<lv_obj_t *>(obj), v); });
    lv_anim_set_values(&a, lv_image_get_scale(img), scale);
    lv_anim_set_duration(&a, duration_ms);
    lv_anim_start(&a);
}

void NowPlayingScreen::begin(TFT_eSPI &tft, AppContext &ctx)
{
    tft.fillScreen(TFT_BLACK);
    drawPlaybackControls(tft);
    drawInitialProgressBar(tft);
    timeoutValue = 10000;
    lastTouch = millis();
    isActive = true;

    // Download large 320x320 image once
    String url = "http://rsdfgiws.rakibshahid.com/raw?url=" + playerData.album_art_url + "&width=320";
    if (drawRawImageWithLVGL(url.c_str(), 320, 320, ctx))
    {
        lv_image_set_scale(ctx.imageObj, 168);  // 210px (256 * 210 / 320)
        lv_image_set_pivot(ctx.imageObj, 0, 0); // top-left corner
        lv_obj_set_pos(ctx.imageObj, 0, 0);
    }
}

void NowPlayingScreen::update(TFT_eSPI &tft, AppContext &ctx)
{
    if (xSemaphoreTake(spotifyMutex, 10 / portTICK_PERIOD_MS))
    {
        if (millis() - lastTouch >= timeoutValue && isActive)
        {
            Serial.println("More than 10s has passed! Going inactive!");
            isActive = false;
            animate_image_scale(ctx.imageObj, 256, 1000); // full 320px over 1s
        }

        if (hasSongChanged(playerData, lastPlayerData))
        {
            // Reload image
            String url = "http://rsdfgiws.rakibshahid.com/raw?url=" + playerData.album_art_url + "&width=320";
            if (drawRawImageWithLVGL(url.c_str(), 320, 320, ctx))
            {
                uint16_t scale = isActive ? 168 : 256;
                lv_image_set_scale(ctx.imageObj, scale);
                lv_image_set_pivot(ctx.imageObj, 0, 0);
                lv_obj_set_pos(ctx.imageObj, 0, 0);
            }
        }

        // if (playerData.is_playing && hasProgressChanged(playerData, lastPlayerData)) {
        //     drawProgressBar(tft, isActive, playerData, lastPlayerData);
        // }

        lastPlayerData = playerData;
        xSemaphoreGive(spotifyMutex);
    }
}

void NowPlayingScreen::handleTouch(int x, int y, AppContext &ctx)
{
    ButtonRegion region = getButtonRegion(x, y);
    lastTouch = millis();

    if (!isActive)
    {
        Serial.println("Touch detected! No longer inactive!");
        isActive = true;
        animate_image_scale(ctx.imageObj, 168, 500); // scale down to 210px over 0.5s
    }

    if (region != NONE && isActive)
    {
        handlePlaybackControls(ctx.tokens, region, x, y);
    }
}
