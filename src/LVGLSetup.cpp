#include "LVGLSetup.h"
#include "esp_heap_caps.h"
#include "Core/AppContext.h"
#include <Adafruit_FT6206.h>

extern TFT_eSPI tft;
extern Adafruit_FT6206 touch;

static void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, lv_area_get_width(area), lv_area_get_height(area));
    tft.pushColors((uint16_t *)px_map, lv_area_get_width(area) * lv_area_get_height(area), true);
    tft.endWrite();
    lv_display_flush_ready(disp);
}

static void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    if (touch.touched())
    {
        TS_Point p = touch.getPoint();
        data->point.x = p.y;
        data->point.y = 320 - p.x;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

void initializeLVGL(AppContext &appContext)
{
    Serial.println("[LVGL] Initializing LVGL...");
    lv_init();
    Serial.println("[LVGL] Creating LVGL display buffers...");
    appContext.lvBuffer = (uint16_t *)heap_caps_malloc(480 * 40 * sizeof(uint16_t), MALLOC_CAP_SPIRAM);
    if (!appContext.lvBuffer)
    {
        Serial.println("[LVGL] Failed to allocate LVGL display buffer!");
        while (true)
            delay(1000);
    }
    Serial.println("[LVGL] Creating LVGL Display");
    appContext.lvDisplay = lv_display_create(480, 320);
    lv_display_set_buffers(appContext.lvDisplay, appContext.lvBuffer, NULL, 480 * 40 * sizeof(uint16_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(appContext.lvDisplay, my_disp_flush);
    Serial.println("[LVGL] LVGL display created");
    Serial.println("[LVGL] Creating LVGL input device...");
    appContext.lvInputDevice = lv_indev_create();
    lv_indev_set_type(appContext.lvInputDevice, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(appContext.lvInputDevice, my_touchpad_read);
    appContext.lvScreen = lv_screen_active();
    Serial.println("[LVGL] LVGL initialized");
}