//
// Created by viski on 07.05.2025.
//

#ifndef MULTIRIUM_CSPEEDOMETR_H
#define MULTIRIUM_CSPEEDOMETR_H

#include <jni.h>

static class CSpeedometr {
public:

    enum
    {
        BUTTON_ENGINE,
        BUTTON_LIGHT,
        BUTTON_TURN_LEFT,
        BUTTON_TURN_RIGHT,
        BUTTON_TURN_ALL,
        BUTTON_DOOR
    };

    static bool bIsShow;

    static void hide();

    static void show();

    static jclass clazz;

    static void update();

    static float fFuel;
    static int iMilliage;

    static void tempToggle(bool toggle);

   // static void updateTurn(bool isTurn);

    static void updateSpeed();
    static void updateTurn(bool isTurn);

    static jobject thiz;
};


#endif //MULTIRIUM_CSPEEDOMETR_H
