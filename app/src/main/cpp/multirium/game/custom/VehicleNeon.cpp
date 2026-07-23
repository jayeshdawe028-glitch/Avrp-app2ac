//
// Created by viski on 11.04.2025.
//

#include "VehicleNeon.h"
#include "game/vehicles/vehicle.h"
#include "util/CUtil.h"
#include "game/renderer/Shadows.h"

RwTexture *CVehicleNeon::pNeonTex;

void CVehicleNeon::Initalise() {
    Log(__FUNCTION__);

    CShadows::gpShadowHeadLightsTex2 = CUtil::LoadTextureFromDB("txd", "headlight1");
    CVehicleNeon::pNeonTex = CUtil::LoadTextureFromDB(OBFUSCATE("samp"), OBFUSCATE("neonaper3"));
}