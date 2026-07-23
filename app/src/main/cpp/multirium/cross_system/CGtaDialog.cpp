#include "../main.h"
#include "CGtaDialog.h"
#include "java/jniutil.h"
#include "../net/netgame.h"

extern CJavaWrapper* g_pJavaWrapper;
extern CNetGame* pNetGame;

jclass CGtaDialog::clazz = nullptr;

void CGtaDialog::initFromJava(JNIEnv* env, jclass clazzIn)
{
    if (!env || !clazzIn) return;

    if (clazz)
    {
        env->DeleteGlobalRef(clazz);
        clazz = nullptr;
    }

    clazz = (jclass)env->NewGlobalRef(clazzIn);
}

void CGtaDialog::show(int dialogId, const char* title, const char* text, const char* left, const char* right)
{
    if (!clazz)
    {
        Log("CGtaDialog: clazz not inited (call GtaDialog.init())");
        return;
    }

    JNIEnv* env = g_pJavaWrapper->GetEnv();

    jmethodID method = env->GetStaticMethodID(
            clazz,
            "show",
            "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V"
    );

    if (!method)
    {
        if (env->ExceptionCheck()) env->ExceptionClear();
        Log("CGtaDialog: method show not found");
        return;
    }

    jstring jTitle = env->NewStringUTF(title ? title : "");
    jstring jText  = env->NewStringUTF(text ? text : "");
    jstring jLeft  = env->NewStringUTF(left ? left : "");
    jstring jRight = env->NewStringUTF(right ? right : "");

    env->CallStaticVoidMethod(clazz, method, (jint)dialogId, jTitle, jText, jLeft, jRight);

    if (env->ExceptionCheck()) env->ExceptionClear();

    env->DeleteLocalRef(jTitle);
    env->DeleteLocalRef(jText);
    env->DeleteLocalRef(jLeft);
    env->DeleteLocalRef(jRight);
}

void CGtaDialog::hide()
{
    if (!clazz) return;

    JNIEnv* env = g_pJavaWrapper->GetEnv();

    jmethodID method = env->GetStaticMethodID(clazz, "hide", "()V");
    if (!method)
    {
        if (env->ExceptionCheck()) env->ExceptionClear();
        Log("CGtaDialog: method hide not found");
        return;
    }

    env->CallStaticVoidMethod(clazz, method);
    if (env->ExceptionCheck()) env->ExceptionClear();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_dialog_GtaDialog_nativeInit(JNIEnv* env, jclass clazzIn)
{
    CGtaDialog::initFromJava(env, clazzIn);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_dialog_GtaDialog_nativeOnButton(JNIEnv* env, jclass, jint dialogId, jint button)
{
    Log("GtaDialogClick UI id=%d button=%d", (int)dialogId, (int)button);
    if (!pNetGame) return;
    pNetGame->SendGtaDialogClick((uint32_t)dialogId, (uint8_t)button);
}
