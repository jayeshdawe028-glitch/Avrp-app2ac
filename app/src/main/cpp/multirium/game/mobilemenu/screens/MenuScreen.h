//
// Created by viski on 03.06.2025.
//

#ifndef MULTIRIUM_MENUSCREEN_H
#define MULTIRIUM_MENUSCREEN_H

#include "../main.h"
#include "../OsArray.h"

class __attribute__((aligned(4))) MenuScreen {
public:
    int (**_vptr$MenuScreen)(void);
    RwTexture *arrowTex;
    float opacity;
    bool hasBack;
    uint8_t pad[3];
};
VALIDATE_SIZE(MenuScreen, (VER_x32 ? 0x10 : 0x18));

struct __attribute__((packed)) __attribute__((aligned(1))) MenuScreen_168 {
    int (**_vptr$MenuScreen)(void);
    RwTexture *arrowTex;
    float opacity;
    bool hasBack;
};
VALIDATE_SIZE(MenuScreen_168, (VER_x32 ? 0xD : 0x15));

#endif //MULTIRIUM_MENUSCREEN_H
