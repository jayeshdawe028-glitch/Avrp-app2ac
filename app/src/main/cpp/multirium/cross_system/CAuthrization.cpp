#include "CAuthrization.h"
#include "main.h"

#include "net/netgame.h"
#include "java/jniutil.h"

extern CNetGame* pNetGame;

jobject CAuthrization::thiz = nullptr;
jclass  CAuthrization::clazz = nullptr;
bool    CAuthrization::bIsShow = false;

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_auth_Autorization_nativeInit(JNIEnv* env, jclass /*clazz*/)
{
    if (CAuthrization::clazz) return;

    jclass localClazz = env->FindClass("com/multirium/mobile/gui/auth/Autorization");
    if (!localClazz) {
        Log("CAuthrization::nativeInit -> FindClass Autorization failed");
        return;
    }

    CAuthrization::clazz = (jclass)env->NewGlobalRef(localClazz);
    env->DeleteLocalRef(localClazz);

    Log("CAuthrization::nativeInit OK");
}

void CAuthrization::show(bool isEmail, bool isAutoAuth, int skinId)
{
    extern CJavaWrapper* pJavaWrapper;
    if (pJavaWrapper) pJavaWrapper->HideLoadingScreen();

    JNIEnv* env = CJavaWrapper::GetEnv();
    if (!env) return;

    if (!CAuthrization::clazz) {
        Log("CAuthrization::show -> clazz null (nativeInit not called)");
        return;
    }

    if (!CAuthrization::thiz) {
        jmethodID ctor = env->GetMethodID(CAuthrization::clazz, "<init>", "(ZZI)V");
        if (!ctor) {
            Log("CAuthrization::show -> ctor (ZZI)V not found!");
            return;
        }

        jobject localObj = env->NewObject(
                CAuthrization::clazz,
                ctor,
                (jboolean)isEmail,
                (jboolean)isAutoAuth,
                (jint)skinId
        );

        if (!localObj) {
            Log("CAuthrization::show -> NewObject failed");
            return;
        }

        CAuthrization::thiz = env->NewGlobalRef(localObj);
        env->DeleteLocalRef(localObj);
    }

    jmethodID showMethod = env->GetMethodID(CAuthrization::clazz, "show", "(ZZI)V");
    if (showMethod) {
        env->CallVoidMethod(CAuthrization::thiz, showMethod,
                            (jboolean)isEmail, (jboolean)isAutoAuth, (jint)skinId);
    }

    CAuthrization::bIsShow = true;
}

void CAuthrization::hide()
{
    if (!CAuthrization::thiz) return;

    JNIEnv* env = CJavaWrapper::GetEnv();
    if (!env || !CAuthrization::clazz) return;

    jmethodID method = env->GetMethodID(CAuthrization::clazz, "destroy", "()V");
    if (method) env->CallVoidMethod(CAuthrization::thiz, method);

    env->DeleteGlobalRef(CAuthrization::thiz);
    CAuthrization::thiz = nullptr;

    CAuthrization::bIsShow = false;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_auth_Autorization_onLoginClick(JNIEnv* env, jobject /*thiz*/, jstring pass)
{
    Log("AUTH: onLoginClick JNI called");

    if (!pass) {
        Log("AUTH: pass == null");
        return;
    }

    if (!pNetGame) {
        Log("AUTH: pNetGame == null");
        return;
    }

    const char* inputPassword = env->GetStringUTFChars(pass, nullptr);
    if (!inputPassword) {
        Log("AUTH: GetStringUTFChars failed");
        return;
    }

    Log("AUTH: sending login packet, len=%d", (int)strlen(inputPassword));

    pNetGame->SendLoginPacket(inputPassword);

    env->ReleaseStringUTFChars(pass, inputPassword);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_auth_Autorization_ToggleAutoLogin(JNIEnv* env, jobject /*thiz*/, jboolean toggle)
{
    if (!pNetGame) return;

    RakNet::BitStream bsSend;
    bsSend.Write((uint8_t)PACKET_AUTH);
    bsSend.Write((uint8_t)6);
    bsSend.Write((uint8_t)toggle);

    pNetGame->GetRakClient()->Send(&bsSend, HIGH_PRIORITY, RELIABLE, 0);
}
