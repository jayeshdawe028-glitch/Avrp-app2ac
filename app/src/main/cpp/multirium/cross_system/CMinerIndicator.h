#pragma once
#include <jni.h>

class CMinerIndicator
{
public:
    static jobject thiz;
    static void Update(bool show, bool inWork, int ore, int ingot);
};
