//
// Created by User on 12.05.2025.
//

#include "RwHelper.h"
#include "patch.h"

RpAtomic* Get2DEffectAtomicCallback(RpAtomic* atomic, void* data) {
    return reinterpret_cast<RpAtomic *>(CHook::CallFunction<int>(
            "_Z25Get2DEffectAtomicCallbackP8RpAtomicPv", atomic, data));
}
