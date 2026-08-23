#include "CRegistration.h"
#include "main.h"
#include "net/netgame.h"
#include "java/jniutil.h"

jobject CRegistration::thiz = nullptr;
jclass  CRegistration::clazz = nullptr;

bool CRegistration::bIsShow = false;
int  CRegistration::RegisterSexMale = 1;
int  CRegistration::RegisterSkinValue = 0;
int  CRegistration::RegisterSkinId = 0;

extern CNetGame* pNetGame;

// --- saved from Java init ---
static jobject gActivity = nullptr;
static jobject gClassLoader = nullptr;
static jmethodID gLoadClass = nullptr;

static jclass LoadClass(JNIEnv* env, const char* dotName)
{
    if (!gClassLoader || !gLoadClass) return nullptr;

    jstring jName = env->NewStringUTF(dotName);
    jobject clsObj = env->CallObjectMethod(gClassLoader, gLoadClass, jName);
    env->DeleteLocalRef(jName);

    if (env->ExceptionCheck())
    {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }
    return (jclass)clsObj; // local ref
}

static void EnsureRegistrationClass(JNIEnv* env)
{
    if (CRegistration::clazz) return;

    jclass local = LoadClass(env, "com.multirium.mobile.gui.auth.Registration");
    if (!local)
    {
        Log("CRegistration: LoadClass Registration failed");
        return;
    }

    CRegistration::clazz = (jclass)env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_auth_RegistrationNative_nativeInit(JNIEnv* env, jclass, jobject activity)
{
    if (gActivity) return;

    gActivity = env->NewGlobalRef(activity);

    jclass actCls = env->GetObjectClass(activity);
    jmethodID midGetCL = env->GetMethodID(actCls, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject loader = env->CallObjectMethod(activity, midGetCL);
    env->DeleteLocalRef(actCls);

    gClassLoader = env->NewGlobalRef(loader);
    env->DeleteLocalRef(loader);

    jclass clCls = env->FindClass("java/lang/ClassLoader");
    gLoadClass = env->GetMethodID(clCls, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    env->DeleteLocalRef(clCls);

    Log("RegistrationNative.nativeInit OK");
}

void CRegistration::show()
{
    extern CJavaWrapper* pJavaWrapper;
    if (pJavaWrapper) pJavaWrapper->HideLoadingScreen();

    if (bIsShow) return;

    JNIEnv* env = CJavaWrapper::GetEnv();
    if (!env) return;

    if (!gClassLoader || !gLoadClass)
    {
        Log("CRegistration::show -> RegistrationNative.nativeInit not called");
        return;
    }

    EnsureRegistrationClass(env);
    if (!CRegistration::clazz) return;

    jmethodID ctor = env->GetMethodID(CRegistration::clazz, "<init>", "()V");
    if (!ctor)
    {
        Log("CRegistration::show -> ctor <init>()V not found");
        env->ExceptionClear();
        return;
    }

    jobject obj = env->NewObject(CRegistration::clazz, ctor);
    if (!obj)
    {
        Log("CRegistration::show -> NewObject failed");
        env->ExceptionClear();
        return;
    }

    CRegistration::thiz = env->NewGlobalRef(obj);
    env->DeleteLocalRef(obj);

    bIsShow = true;

    Log("CRegistration::show OK");
}

void CRegistration::hide()
{
    if (!bIsShow) return;

    JNIEnv* env = CJavaWrapper::GetEnv();
    if (!env) return;

    if (CRegistration::thiz && CRegistration::clazz)
    {
        jmethodID method = env->GetMethodID(CRegistration::clazz, "destroy", "()V");
        if (method) env->CallVoidMethod(CRegistration::thiz, method);
        env->ExceptionClear();

        env->DeleteGlobalRef(CRegistration::thiz);
        CRegistration::thiz = nullptr;
    }

    bIsShow = false;

    Log("CRegistration::hide OK");
}

// skin change helper (optional)
uint32_t CRegistration::ChangeRegisterSkin(int delta)
{
    RegisterSkinValue += delta;

    if (RegisterSkinValue < 0) RegisterSkinValue = 0;
    if (RegisterSkinValue > 299) RegisterSkinValue = 299;

    RegisterSkinId = RegisterSkinValue;

    if (pNetGame)
        pNetGame->SendRegisterSkinPacket((uint32_t)RegisterSkinId);

    return (uint32_t)RegisterSkinId;
}

/* ===================== JNI from Kotlin Registration ===================== */

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_auth_Registration_onRegisterClick(JNIEnv* env, jobject thiz,
                                                                jstring pass, jstring mail, jint sex)
{
    if (!pNetGame) return;

    const char* cPass = env->GetStringUTFChars(pass, nullptr);
    const char* cMail = env->GetStringUTFChars(mail, nullptr);

    // sex: 1 = male, 2 = female (как у тебя в Kotlin)
    uint8_t uSex = (uint8_t)sex;
    uint8_t uSkin = (uint8_t)CRegistration::RegisterSkinId;

    pNetGame->SendRegisterPacket((char*)cPass, (char*)cMail, uSex, 18, (uint32_t)CRegistration::RegisterSkinId);

    env->ReleaseStringUTFChars(pass, cPass);
    env->ReleaseStringUTFChars(mail, cMail);

    CRegistration::hide();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_auth_Registration_onRegisterSkinBackClick(JNIEnv* env, jobject thiz)
{
    if (!pNetGame) return;

    // если у тебя спавн/регистрация завязаны на классы SA:MP
    pNetGame->SendPrevClass();

    // и дополнительно можно вести локальный индекс
    CRegistration::ChangeRegisterSkin(-1);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_auth_Registration_onRegisterSkinNextClick(JNIEnv* env, jobject thiz)
{
    if (!pNetGame) return;

    pNetGame->SendNextClass();
    CRegistration::ChangeRegisterSkin(+1);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_auth_Registration_onRegisterMaleSend(JNIEnv* env, jobject thiz, jint sex)
{
    // уведомляем клиент о выбранном полу
    CRegistration::RegisterSexMale = (int)sex;
    // можно дернуть сервер, чтобы он выставил стартовый скин
    if (pNetGame)
        pNetGame->SendRegisterSkinPacket((uint32_t)CRegistration::RegisterSkinId);
}
