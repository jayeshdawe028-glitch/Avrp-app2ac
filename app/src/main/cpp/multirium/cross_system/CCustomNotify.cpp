#include "../main.h"
#include "CCustomNotify.h"
#include "java/jniutil.h"
#include "../net/netgame.h"
extern CNetGame* pNetGame;

jclass CCustomNotify::clazz = nullptr;

extern CJavaWrapper* g_pJavaWrapper;

void CCustomNotify::initFromJava(JNIEnv* env, jclass clazzIn)
{
    if (!env || !clazzIn) return;

    if (clazz)
    {
        env->DeleteGlobalRef(clazz);
        clazz = nullptr;
    }

    clazz = (jclass)env->NewGlobalRef(clazzIn);
}

void CCustomNotify::show(const char* text, int npcId, int actionId)
{
    if (!clazz)
    {
        Log("CCustomNotify: clazz not inited (call CustomNotify.init())");
        return;
    }

    JNIEnv* env = g_pJavaWrapper->GetEnv();

    jmethodID method = env->GetStaticMethodID(clazz, "show", "(Ljava/lang/String;II)V");
    if (!method)
    {
        if (env->ExceptionCheck()) env->ExceptionClear();
        Log("CCustomNotify: method show not found");
        return;
    }

    jstring jText = env->NewStringUTF(text ? text : "");
    env->CallStaticVoidMethod(clazz, method, jText, (jint)npcId, (jint)actionId);

    if (env->ExceptionCheck()) env->ExceptionClear();
    env->DeleteLocalRef(jText);
}

void CCustomNotify::hide()
{
    if (!clazz) return;

    JNIEnv* env = g_pJavaWrapper->GetEnv();

    jmethodID method = env->GetStaticMethodID(clazz, "hide", "()V");
    if (!method)
    {
        if (env->ExceptionCheck()) env->ExceptionClear();
        Log("CCustomNotify: method hide not found");
        return;
    }

    env->CallStaticVoidMethod(clazz, method);
    if (env->ExceptionCheck()) env->ExceptionClear();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_notify_CustomNotify_nativeInit(JNIEnv* env, jclass clazzIn)
{
    CCustomNotify::initFromJava(env, clazzIn);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_notify_CustomNotify_nativeOnClick(JNIEnv* env, jclass, jint npcId, jint actionId)
{
    Log("NotifyClick UI npc=%d action=%d", (int)npcId, (int)actionId);
    if (!pNetGame) return;
    pNetGame->SendNotifyClick((uint32_t)npcId, (uint32_t)actionId);
}
