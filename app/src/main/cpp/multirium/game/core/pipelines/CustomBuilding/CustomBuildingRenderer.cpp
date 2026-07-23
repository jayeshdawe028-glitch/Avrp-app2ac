//
// Created by viski on 01.05.2025.
//

#include "CustomBuildingRenderer.h"
#include "../vendor/armhook/patch.h"

bool CCustomBuildingRenderer::Initialise() { // RwBool
    //RwBool result; // w0

   // result = CCustomBuildingPipeline::CreatePipe();
   // if ( result )
   //     return CCustomBuildingDNPipeline::CreatePipe() != 0;
   // return result;
    return CHook::CallFunction<bool>("_ZN23CCustomBuildingRenderer10InitialiseEv");
}

void CCustomBuildingRenderer::Update() {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x2CA3A4 + 1 : 0x38B6DC));
}
