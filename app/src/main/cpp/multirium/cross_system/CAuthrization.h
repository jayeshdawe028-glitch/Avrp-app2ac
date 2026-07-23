#pragma once
#include <jni.h>

class CAuthrization {
public:
    static jobject thiz;
    static jclass  clazz;
    static bool    bIsShow;

    static void show(bool isEmail, bool isAutoAuth, int skinId);
    static void hide();
};
