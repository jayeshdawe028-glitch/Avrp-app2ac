//
// Created by viski on 01.05.2025.
//

#pragma once

#include "common.h"

class CCustomBuildingRenderer {
public:
    static void InjectHooks();

    CCustomBuildingRenderer();
    ~CCustomBuildingRenderer();

    static bool Initialise();
    static void Shutdown();
    static bool PluginAttach();
    static void AtomicSetup(RpAtomic* atomic);
    static bool IsCBPCPipelineAttached(RpAtomic* atomic);
    static void UpdateDayNightBalanceParam();
    static void Update();
};
