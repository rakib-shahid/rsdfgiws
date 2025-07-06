#pragma once

#include "../Spotify/SpotifyTypes.h"
#include "../Spotify/TokenManager.h"
#include <TFT_eSPI.h>
#include <lvgl.h>

struct AppContext
{
    TFT_eSPI *tft;
    TokenInfo tokens;
    PlayerData playerData;

    lv_display_t *lvDisplay = nullptr;
    lv_indev_t *lvInputDevice = nullptr;
    lv_obj_t *lvScreen = nullptr;

    uint16_t *lvBuffer = nullptr;

    uint16_t *imageBuffer = nullptr;
    size_t imageBufferSize = 0;
    lv_image_dsc_t imageDesc{};
    lv_obj_t *imageObj = nullptr;

    lv_obj_t *canvas = nullptr;
    lv_draw_buf_t *canvasBuf = nullptr;
};