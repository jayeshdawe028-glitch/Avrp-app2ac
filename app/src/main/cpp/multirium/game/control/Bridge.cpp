//
// Created by viski on 26.04.2025.
//

#include "Bridge.h"
#include "patch.h"

void CBridge::InjectHooks() {
   // CHook::Redirect("_ZN7CBridge4InitEv", CBridge::Init);
   // CHook::Redirect("_ZN7CBridge6UpdateEv", CBridge::Update);
   // CHook::Redirect("_ZN7CBridge22ShouldLightsBeFlashingEv", CBridge::FindBridgeEntities);
   // CHook::Redirect("_ZN7CBridge18FindBridgeEntitiesEv", CBridge::ShouldLightsBeFlashing);
}

void CBridge::Init(CBridge *thiz) {
    // NOP
}

void CBridge::Update() {
    // NOP
}

// used in CEntity::ProcessLightsForEntity
// always false
bool CBridge::ShouldLightsBeFlashing() {
    return false;
}

void CBridge::FindBridgeEntities() {
    // NOP
}

// used in CPlayerInfo::Process
bool CBridge::ThisIsABridgeObjectMovingUp(uint32) {
    return false;
}