//
// Created by viski on 01.05.2025.
//

#include "CustomBuildingDNPipeline.h"
#include "../CustomCar/CustomCarEnvMapPipeline.h"
#include "../vendor/armhook/patch.h"
#include "MemoryMgr.h"

void CCustomBuildingDNPipeline::InjectHooks() {
    CHook::Write(g_libGTASA + (VER_x32 ? 0x676E88 : 0x84BD68), &m_fDNBalanceParam);
}
