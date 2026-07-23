//
// Created by x1y2z on 20.09.2023.
//

#include "TimeCycle.h"
#include "vendor/armhook/patch.h"
#include "Shadows.h"
#include "SimpleTransform.h"
#include "Camera.h"

static uint8 gTimecycleHours[]      = { 5, 6, 7, 12, 19, 20, 22, 24 };
static uint8 gTimecycleStartHours[] = { 30, 30, 30, 50, 60, 60, 50, 35 };
//int &TunnelWeather = *(int*)0x8CDEE0; // 9 = WEATHER_FOGGY_SF, unchanged

void CTimeCycle::InjectHooks() {

    CHook::Redirect("_ZN10CTimeCycle6UpdateEv", &CTimeCycle::Update);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00676BB8 : 0x84B7D0), &CTimeCycle::m_CurrentColours);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x006770C0 : 0x84C1D0), &CTimeCycle::m_BelowHorizonGrey);
}

void CTimeCycle::Update()
{
    CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

    CSimpleTransform * p_m_transform = &TheCamera.m_placement;
    if (TheCamera.m_matrix)
        p_m_transform = (CSimpleTransform *) &TheCamera.m_matrix->m_pos.x; // SA: tx
    CTimeCycle::CalcColoursForPoint(p_m_transform->m_vPosn, &CTimeCycle::m_CurrentColours);
}

void CTimeCycle::CalcColoursForPoint(CVector point, CColourSet* set) {
    return CHook::CallFunction<void>("_ZN10CTimeCycle19CalcColoursForPointE7CVectorP10CColourSet", point, set);
}