//
// Created by viski on 26.04.2025.
//

#ifndef MULTIRIUM_BRIDGE_H
#define MULTIRIUM_BRIDGE_H

#include "../main.h"

class CBridge {
public:
    static void InjectHooks();

    static void Init(CBridge *thiz);
    static void Update();
    static void FindBridgeEntities();
    static bool ShouldLightsBeFlashing();
    static bool ThisIsABridgeObjectMovingUp(uint32);
};


#endif //MULTIRIUM_BRIDGE_H
