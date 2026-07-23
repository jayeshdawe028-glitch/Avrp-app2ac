//
// Created by viski on 23.04.2025.
//

#include "Coronas.h"
#include "vendor/armhook/patch.h"
#include "game/RW/TxdStore.h"

//RwTexture* gpCoronaTexture[10];
RwTexture **gpCoronaTexture = nullptr;

// Creates corona by texture
void CCoronas::RegisterCorona(uint32 id, CEntity* attachTo, uint8 red, uint8 green, uint8 blue, uint8 alpha, const CVector& posn, float radius, float farClip, RwTexture* texture, eCoronaFlareType flareType, bool enableReflection, bool checkObstacles, int32 _param_not_used, float angle, bool longDistance, float nearClip, uint8 fadeState, float fadeSpeed, bool onlyFromBelow, bool reflectionDelay) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x005A3AAC + 1 : 0x6C71D0), id, attachTo, red, green, blue, alpha, posn, radius, farClip, texture, flareType, enableReflection, checkObstacles, _param_not_used, angle, longDistance, nearClip, fadeState, fadeSpeed, onlyFromBelow, reflectionDelay);
}

// Creates corona by type
void CCoronas::RegisterCorona(uint32 id, CEntity* attachTo, uint8 red, uint8 green, uint8 blue, uint8 alpha, const CVector* posn, float radius, float farClip, eCoronaType coronaType, eCoronaFlareType flareType, bool enableReflection, bool checkObstacles, int32 _param_not_used, float angle, bool longDistance, float nearClip, uint8 fadeState, float fadeSpeed, bool onlyFromBelow, bool reflectionDelay) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x005A3A20 + 1 : 0x6C7174), id, attachTo, red, green, blue, alpha, posn, radius, farClip, coronaType, flareType, enableReflection, checkObstacles, _param_not_used, angle, longDistance, nearClip, fadeState, fadeSpeed, onlyFromBelow, reflectionDelay);
}
void (*CCoronas__Init)();
void CCoronas::InjectHooks() {
    gpCoronaTexture = (RwTexture **)((uintptr_t)g_libGTASA + 0xCC6A18);

    CHook::Write(g_libGTASA + (VER_x32 ? 0x00679A84 : 0x851528), &CCoronas::SunScreenX);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00679784 : 0x850F20), &CCoronas::SunScreenY);

    CHook::Write(g_libGTASA + (VER_x32 ? 0x00679F40 : 0x851E90), &CCoronas::SunBlockedByClouds);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00675F6C : 0x849F60), &CCoronas::MoonSize);

   // CHook::InlineHook("_ZN8CCoronas4InitEv", CCoronas::Init, &CCoronas__Init);
}

// Initialises coronas
// 0x6FAA70

void CCoronas::Init() {
    {
        CCoronas__Init();
        Log("CCoronas::Init()");

        CTxdStore::ScopedTXDSlot txd{"particle"};
    }

    for (auto & corona : aCoronas) {
        corona = CRegisteredCorona{};
    }
}

void CCoronas::Render() {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x005A23B8 + 1 : 0x6C5CD4));
}
