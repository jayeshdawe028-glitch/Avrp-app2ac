//
// Created by viski on 15.02.2025.
//

#pragma once
#include <dlfcn.h>
#include "game/core/textures/TextureDatabaseRuntime.h"
#include "../main.h"
#include <arm_neon.h>
#include "game/core/SimpleTransform.h"
#include "game/Entity/Entity.h"
// что то новенькое
#include "imgui.h"


class CUtil {
public:
    static uintptr_t FindLib(const char *libname);

    // Converts degrees to radians
    // keywords: 0.017453292 flt_8595EC
    constexpr static float DegreesToRadians(float angleInDegrees) {
        return angleInDegrees * PI / 180.0F;
    }

    // Converts radians to degrees
    // 57.295826
    constexpr static float RadiansToDegrees(float angleInRadians) {
        return angleInRadians * 180.0F / PI;
    }

    static RwTexture *LoadTextureFromDB(const char *dbname, const char *texture);

    static RwTexture* GetTexture(const char* name);

    static bool ProcessInlineHexColor(const char *start, const char *end, ImVec4 &color);

    static void drawText(const char *fmt, ...);

    static void drawText(ImVec2 pos, ImColor col, const char *szStr, float font_size);

    static ImVec2 CalcTextSizeWithoutTags(char *szStr, float font_size);

    static CVector TransformPoint(const CSimpleTransform &t, const RwV3d &v);
};
void cp1251_to_utf8(char *out, const char *in, unsigned int len = 0);
char* ConvertColorToHtml(std::string format);
