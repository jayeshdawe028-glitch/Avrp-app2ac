//
// Created by viski on 30.04.2025.
//

#pragma once


#include "WidgetGta.h"
#include "game/renderer/sprite2d.h"

struct CWidgetButton : CWidgetGta
{
    uint32      m_nButtonFlags;
    float       m_fPulssateTime;
    CSprite2d   m_SpriteFill;
    float       m_fFillLevel;
    float       m_fTVTop;
    float       m_fTVBottom;
};
