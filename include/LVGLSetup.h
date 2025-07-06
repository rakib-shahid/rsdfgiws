#pragma once
#include <lvgl.h>
#include "Core/AppContext.h"

void initializeLVGL(AppContext &appContext);

extern lv_display_t *lv_disp;
