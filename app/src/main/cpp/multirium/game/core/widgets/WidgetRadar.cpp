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
    delete radar;
}

void CWidgetRadar::RenderRadarTexture()
{
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