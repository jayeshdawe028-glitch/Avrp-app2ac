//
// Created by viski on 01.05.2025.
//

#pragma once

#include "game/fakerw/RenderWare.h"
#include "main.h"

class CCustomBuildingDNPipeline {
public:
    static inline float m_fDNBalanceParam = 1.0f;
public:
    static void InjectHooks();
};
