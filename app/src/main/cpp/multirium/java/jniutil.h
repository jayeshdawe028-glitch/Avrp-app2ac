// jniutil.h
#pragma once

#include "main.h"
#include <jni.h>
#include <string>

#define EXCEPTION_CHECK(env) \
    if ((env) && (env)->ExceptionCheck()) \
    { \
        (env)->ExceptionDescribe(); \
        (env)->ExceptionClear(); \
        return; \
    }

#define EXCEPTION_CHECK_RET(env, ret) \
    if ((env) && (env)->ExceptionCheck()) \
    { \
        (env)->ExceptionDescribe(); \
        (env)->ExceptionClear(); \
        return (ret); \
    }

class CJavaWrapper
{
public:
    CJavaWrapper(JNIEnv *env, jobject activity);
    ~CJavaWrapper() {};

    static JNIEnv *GetEnv();

    std::string GetClipboardString();
    void SetPauseState(bool pause);
    void ShowLoadingScreen();
    void HideLoadingScreen();
    void exitGame();

    jbyteArray as_byte_array(unsigned char* buf, int len)
    {
        JNIEnv* env = GetEnv();
        if(!env) return nullptr;

        jbyteArray array = env->NewByteArray(len);
        if(!array) return nullptr;

        env->SetByteArrayRegion(array, 0, len, reinterpret_cast<jbyte*>(buf));
        EXCEPTION_CHECK_RET(env, nullptr);
        return array;
    }

public:
    jobject activity = nullptr;

    jmethodID s_setPauseState = nullptr;
    jmethodID s_showLoadingScreen = nullptr;
    jmethodID s_hideLoadingScreen = nullptr;
    jmethodID s_exitGame = nullptr;

    jmethodID s_showAuthorization = nullptr;
    jmethodID s_hideAuthorization = nullptr;
    jmethodID s_showRegistration = nullptr;
    jmethodID s_hideRegistration = nullptr;
    jmethodID s_GetClipboardText = nullptr;

    void HideAuthorization();
    void ShowAuthorization(char *nick, int id, bool ip_match, bool toggleAutoLogin, bool email_acvive);
};
