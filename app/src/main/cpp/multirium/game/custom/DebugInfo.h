//
// Created by viski on 06.05.2025.
//

#pragma once

#include <cstdint>
#include <malloc.h>

class CDebugInfo {
public:
    static bool m_uiDrawDebug;
    static void Draw();

    static void ToggleDebugDraw(bool toggle);
};