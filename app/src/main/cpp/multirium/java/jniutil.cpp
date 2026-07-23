// jniutil.cpp
#include "jniutil.h"
#include "game/core/game.h"
#include "..//net/netgame.h"

extern CGame *pGame;
extern CNetGame *pNetGame;

CJavaWrapper *g_pJavaWrapper = nullptr;

JNIEnv *CJavaWrapper::GetEnv()
{
    if(!javaVM) return nullptr;

    JNIEnv *env = nullptr;
    jint stat = javaVM->GetEnv((void **)&env, JNI_VERSION_1_6);

    if(stat == JNI_OK) return env;

    if(stat == JNI_EDETACHED)
    {
        // ВАЖНО: этот поток не прикреплён к JVM — прикрепляем
#if defined(__ANDROID__)
        if(javaVM->AttachCurrentThread(&env, nullptr) != JNI_OK)
#else
            if(javaVM->AttachCurrentThread((void**)&env, nullptr) != JNI_OK)
#endif
        {
            Log("GetEnv: Failed to attach current thread");
            return nullptr;
        }
        return env;
    }

    if(stat == JNI_EVERSION)
    {
        Log("GetEnv: JNI version not supported");
        return nullptr;
    }

    Log("GetEnv: JNI_ERR");
    return nullptr;
}

CJavaWrapper::CJavaWrapper(JNIEnv *env, jobject activity)
{
    g_pJavaWrapper = this;

    this->activity = env->NewGlobalRef(activity);

    jclass clas = env->GetObjectClass(activity);
    if(!clas)
    {
        Log("CJavaWrapper: no class");
        return;
    }

    s_showLoadingScreen = env->GetMethodID(clas, "showLoadingScreen", "()V");
    s_hideLoadingScreen = env->GetMethodID(clas, "hideLoadingScreen", "()V");
    s_setPauseState     = env->GetMethodID(clas, "setPauseState", "(Z)V");
    s_exitGame          = env->GetMethodID(clas, "exitGame", "()V");

    env->DeleteLocalRef(clas);
}

void CJavaWrapper::ShowLoadingScreen()
{
    JNIEnv* env = GetEnv();
    if(!env) { Log("ShowLoadingScreen: No env"); return; }

    if(s_showLoadingScreen)
        env->CallVoidMethod(activity, s_showLoadingScreen);

    EXCEPTION_CHECK(env);
}

void CJavaWrapper::HideLoadingScreen()
{
    JNIEnv* env = GetEnv();
    if(!env) { Log("HideLoadingScreen: No env"); return; }

    if(s_hideLoadingScreen)
        env->CallVoidMethod(activity, s_hideLoadingScreen);

    EXCEPTION_CHECK(env);
}

void CJavaWrapper::SetPauseState(bool pause)
{
    JNIEnv* env = GetEnv();
    if(!env) { Log("SetPauseState: No env"); return; }

    if(s_setPauseState)
        env->CallVoidMethod(activity, s_setPauseState, (jboolean)pause);

    EXCEPTION_CHECK(env);
}

void CJavaWrapper::exitGame()
{
    JNIEnv* env = GetEnv();
    if(!env) { Log("exitGame: No env"); return; }

    if(s_exitGame)
        env->CallVoidMethod(activity, s_exitGame);

    EXCEPTION_CHECK(env);
}
