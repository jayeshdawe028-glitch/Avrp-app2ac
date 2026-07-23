//
// Created by viski on 04.05.2025.
//

#ifndef X64_MOBILEMENU_H
#define X64_MOBILEMENU_H

#include "main.h"
#include "OsArray.h"
#include "screens/MenuScreen.h"

class MobileMenu {
public:
    CVector2D bgUVSize;
    CVector2D bgTargetCoords;
    CVector2D bgCurCoords;
    CVector2D bgStartCoords;
    OSArray<MenuScreen *> screenStack;

    MenuScreen *pendingScreen;

    uintptr_t bgTex;
    uintptr_t sliderEmpty;

    uintptr_t sliderFull;

    uintptr_t sliderNub;
    uintptr_t controlsBack;
    uintptr_t controlsBack2;
    uint32_t waypoint_blip;

    bool m_WantsToRestartGame;

    bool WantsToLoad;

    uint8_t pad1[2];
    uint32_t SelectedSlot;

    uint8_t CurrentGameNotResumable;

    uint8_t InitializedForSignOut;

    uint8_t pad2[2];
    float NEW_MAP_SCALE;

    float MAP_OFFSET_X;

    float MAP_OFFSET_Y;

    float MAP_AREA_X;

    float MAP_AREA_Y;

    uint8_t DisplayingMap;

    uint8_t isMapMode;

    uint8_t pointerMode;

    uint8_t isMouse;

    CVector2D pointerCoords[4];
    OSPointerState pointerState[4];
    unsigned int pointerPress[4];
public:
    MobileMenu();
    ~MobileMenu();

   // static void InitForPause_orig();
    static void InitForPause();
    static void InjectHooks();

    static void (*InitForPause_orig)(MobileMenu *);

    static void Load();

    static CVector2D CalcBGUVSize();

    static CVector2D GetRandomBGCoords();

    static void InitGameVars();

    static void ClampBGCoords(CVector2D* toClamp);

    static void Initialize(MobileMenu* menu);

    static bool IsMainMenuScreen(MobileMenu *menu);

    static void Render(MobileMenu *menu);
    static void PrepareToRender();
};
VALIDATE_SIZE(MobileMenu, (VER_x32 ? 0xB0 : 0xD0));

extern MobileMenu gMobileMenu;

#endif //X64_MOBILEMENU_H
