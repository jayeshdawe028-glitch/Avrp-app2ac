//
// Created by viski on 31.03.2025.
//
#include "WidgetRadar.h"
#include "vendor/armhook/patch.h"
#include "CUtil.h"
#include "java/CHud.h"
#include "gui/gui.h"
#include "imgui.h"
#include "java/CPauseMenu.h"
#include "game.h"

void (*CWidgetRadar__Update)(CWidgetRadar* thiz);
void CWidgetRadar__Update_hook(CWidgetRadar* thiz)
{
    if (!thiz || !CWidgetRadar__Update)
        return;

    thiz->m_fTapHoldTime = 999.f;
    CWidgetRadar__Update(thiz);
}

RwTexture* hud_bg = nullptr;

void CWidgetRadar::LoadRadarTexture() {
    // StartGame() can run before the GTA texture database/streaming state is ready.
    // Do not enter the native TextureDatabaseRuntime::GetTexture() path until the
    // game reports itself loaded. The function is retried from RenderRadarTexture().
    if (hud_bg || !CGame::IsGameLoaded())
        return;

    hud_bg = CUtil::LoadTextureFromDB("samp", "hud_back");
    if (!hud_bg)
        Log("Radar texture disabled: SAMP texture database is unavailable");
}

extern CGUI *pGUI;
void DrawRadarTexture(float x, float y, float size1, float size2)
{
    // A missing SAMP texture database must never take down the native renderer.
    if (!hud_bg)
        return;

    CSprite2d* radar = new CSprite2d();
    if (!radar)
        return;

    radar->m_pTexture = hud_bg;

    CRGBA color;
    color.a = 255;
    color.r = 255;
    color.g = 255;
    color.b = 255;

    radar->Draw(x, y, size1, size2, &color);

    // hud_bg is owned by the database; don't let CSprite2d::~CSprite2d destroy it.
    radar->m_pTexture = nullptr;
    delete radar;
}

void CWidgetRadar::RenderRadarTexture()
{
    if (!hud_bg)
        LoadRadarTexture();

    if (CHud::bIsShow && hud_bg)
    {
        DrawRadarTexture(CHud::radarBgPos1.x, CHud::radarBgPos1.y, CHud::radarBgPos2.x, CHud::radarBgPos2.y);
    }
}

void(*CHud__Draw)();
void CHud__Draw_hook()
{
    if (!CHud__Draw)
        return;

    CHud__Draw();
}

void CWidgetRadar::InjectHooks() {
    //CHook::InlineHook("_ZN12CWidgetRadar10IsHeldDownEv", CWidgetRadar__Update_hook, &CWidgetRadar__Update);
    //CHook::InlineHook("_ZN4CHud4DrawEv", (void*)CHud__Draw_hook, (void**)&CHud__Draw);
}