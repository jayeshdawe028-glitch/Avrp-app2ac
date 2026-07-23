//
// Created by viski on 12.05.2025.
//

#ifndef MULTIRIUM_RWHELPER_H
#define MULTIRIUM_RWHELPER_H

#include "game/fakerw/RenderWare.h"

//void GetNameAndDamage(const char* name, char* objName, bool& bIsDamageModel);
//RpAtomic* GetFirstAtomicCallback(RpAtomic* atomic, void* data);
//RpAtomic* GetFirstAtomic(RpClump* clump);
//RpAtomic* Get2DEffectAtomicCallback(RpAtomic* atomic, void* data);
RpAtomic* Get2DEffectAtomic(RpClump* clump);

#endif //MULTIRIUM_RWHELPER_H
