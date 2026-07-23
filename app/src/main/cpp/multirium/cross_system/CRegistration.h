//
// Created by Error on 21.05.2025.
//

#ifndef CRIMINAL_MOSCOW_CREGISTRATION_H
#define CRIMINAL_MOSCOW_CREGISTRATION_H

#include <jni.h>
#include <stdint.h>

class CRegistration {
public:
    static bool bIsShow;
    static int RegisterSexMale;
    static int RegisterSkinValue;
    static int RegisterSkinId;

    static jclass  clazz;
    static jobject thiz;

    static void show();
    static void hide();

    static uint32_t ChangeRegisterSkin(int delta);
};

#endif //CRIMINAL_MOSCOW_CREGISTRATION_H
