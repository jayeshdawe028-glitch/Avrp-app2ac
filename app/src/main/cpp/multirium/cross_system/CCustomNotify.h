#pragma once

#include <jni.h>

class CCustomNotify
{
public:
    static void initFromJava(JNIEnv* env, jclass clazzIn);
    static void show(const char* text, int npcId, int actionId);
    static void hide();

private:
    static jclass clazz;
};
