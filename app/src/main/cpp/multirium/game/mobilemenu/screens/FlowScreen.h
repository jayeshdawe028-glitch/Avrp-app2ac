//
// Created by viski on 03.06.2025.
//

#ifndef MULTIRIUM_FLOWSCREEN_H
#define MULTIRIUM_FLOWSCREEN_H

#include "MenuScreen.h"
#include "../main.h"
#include "game/mobilemenu/OsArray.h"

class FlowScreen : MenuScreen_168 {
public:
    struct MenuItem {
        RwTexture *itemTex;
        const uint8_t* text;
        void (*selected)(void);
    };
    static_assert(sizeof(MenuItem) == (VER_x32 ? 0xC : 0x18));

    bool isPauseScreen;
    char pad16[2];
    RwTexture* presentsTex;
    bool hasBriefs;
    char pad21[7];
    RwTexture* curCharTex;
    RwTexture* newCharTex;
    float charCountdown;
    int panelFlip;
    float statsStart;
    float briefsEnd;
    int selectedItem;
    float curItemOffset;
    bool selectOnScroll;
    char pad51[7];
    OSArray<FlowScreen::MenuItem> items;
    CVector2D lastMouseCoords;
    float totalOffset;
    int timeClicked;
public:
    static void OnStats();
    static void OnBriefs();
};
VALIDATE_SIZE(FlowScreen, (VER_x32 ? 0x60 : 0x78));

#endif //MULTIRIUM_FLOWSCREEN_H
