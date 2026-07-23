//
// Created by viski on 31.03.2025.
//

#pragma once

#include "game/renderer/sprite2d.h"
#include "WidgetGta.h"

class CWidgetRadar : public CWidgetGta {
    CSprite2d mSpriteCrossHairs;

public:
    static void InjectHooks();

    static void LoadRadarTexture();

    static void RenderRadarTexture();
};
VALIDATE_SIZE(CWidgetRadar, (VER_x32 ? 0x94 : 0xB0));

