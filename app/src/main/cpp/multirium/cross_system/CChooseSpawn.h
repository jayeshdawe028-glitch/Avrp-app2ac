#pragma once
#include <jni.h>

class CChooseSpawn {
public:
    static void show(int organization, int home, int family, int pos);
    static void hide();

    static jclass  clazz;
private:
    static jobject thiz;
};
