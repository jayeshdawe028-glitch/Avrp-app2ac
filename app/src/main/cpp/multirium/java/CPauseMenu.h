//
// Created by viski on 31.03.2025.
//

#ifndef X64_CPAUSEMENU_H
#define X64_CPAUSEMENU_H

#include "game/mobilemenu/MobileMenu.h"

#pragma pack(push, 1)
struct Setting
{
    char *szName;
    uintptr_t values;
    uint32_t value;
    uint32_t defaultValue;
    uint32_t min;
    uint32_t max;
    bool visible;
    uint8_t pad[3];
    uint32_t type;
};
VALIDATE_SIZE(Setting, (VER_x32 ? 0x20 : 0x28));
#pragma pack(pop)

typedef struct Setting Setting;

class CPauseMenu {
public:
    static void InjectHooks();

    static bool bIsShow;

    static jobject thiz;
    static jclass clazz;
    static inline Setting* Settings;
};

#endif //X64_CPAUSEMENU_H
