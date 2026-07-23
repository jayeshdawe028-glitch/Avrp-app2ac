//
// Created by viski on 15.03.2025.
//

#ifndef SA_MP_LAUNCHER_CHUD_H
#define SA_MP_LAUNCHER_CHUD_H

#include <jni.h>
#include "game/core/core/Vector2D.h"

// VOICE BRIDGE (HUD -> VOICE PLUGIN)
extern "C" void Voice_HudStart();
extern "C" void Voice_HudStop();

class CHud {
public:
    static jobject thiz;
    static bool bIsShow;

    static void toggleAll(bool toggle, bool isChat);

    static void UpdateHudInfo();
    static void UpdateWanted();
    static int iWantedLevel;

    static void UpdateMoney();
    static int iLocalMoney;

    static void toggleGps(bool toggle);
    static void toggleGreenZone(bool toggle);

    static void updatePlayerInfo(int id, char *name);

    static void toggleLogo(bool toggle);

    static CVector2D radarBgPos1;
    static CVector2D radarBgPos2;
    static CVector2D radarPos;
    static float radarSize;

    static void toggleVoice(bool toggle);

    static void UpdateAmmo();
};

#endif //SA_MP_LAUNCHER_CHUD_H
