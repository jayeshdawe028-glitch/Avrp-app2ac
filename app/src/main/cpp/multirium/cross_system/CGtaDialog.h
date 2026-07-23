#pragma once

#include <jni.h>

class CGtaDialog
{
public:
    static void initFromJava(JNIEnv* env, jclass clazzIn);

    static void show(int dialogId, const char* title, const char* text, const char* left, const char* right);
    static void hide();

private:
    static jclass clazz;
};
