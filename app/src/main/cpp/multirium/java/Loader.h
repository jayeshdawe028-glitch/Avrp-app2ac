//
// Created by viski on 05.04.2025.
//

#ifndef X64_CLOADER_H
#define X64_CLOADER_H

#include <jni.h>

static class CLoader {
public:

    static void initJavaClasses(JavaVM *pjvm);

    static jobject thiz;
    static jclass clazz;
};


#endif //X64_CLOADER_H
