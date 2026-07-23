#include "CChooseSpawn.h"
#include "main.h"
#include "net/netgame.h"
#include "java/jniutil.h"

jobject CChooseSpawn::thiz = nullptr;
jclass  CChooseSpawn::clazz = nullptr;

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

static void EnsureSpawnSelectClass(JNIEnv* env)
{
    if (CChooseSpawn::clazz) return;

    jclass local = LoadClass(env, "com.multirium.mobile.gui.auth.SpawnSelect");
    if (!local)
    {
        Log("CChooseSpawn: LoadClass SpawnSelect failed");
        return;
    }

    CChooseSpawn::clazz = (jclass)env->NewGlobalRef(local);
    env->DeleteLocalRef(local);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_auth_SpawnSelectNative_nativeInit(JNIEnv* env, jobject thiz, jobject activity)
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

    Log("SpawnSelectNative.nativeInit OK");
}

void CChooseSpawn::show(int organization, int home, int family, int pos)
{
    JNIEnv* env = CJavaWrapper::GetEnv(); // declared in your existing jniutil/CJavaWrapper
    if (!env) return;

    if (!gClassLoader || !gLoadClass)
    {
        Log("CChooseSpawn::show -> SpawnSelectNative.nativeInit not called");
        return;
    }

    EnsureSpawnSelectClass(env);
    if (!CChooseSpawn::clazz) return;

    jmethodID ctor = env->GetMethodID(CChooseSpawn::clazz, "<init>", "(IIII)V");
    if (!ctor)
    {
        Log("CChooseSpawn::show -> ctor <init>(IIII)V not found");
        env->ExceptionClear();
        return;
    }

    if (CChooseSpawn::thiz) return;

    jobject obj = env->NewObject(CChooseSpawn::clazz, ctor,
                                 (jint)organization, (jint)home, (jint)family, (jint)pos);
    if (!obj)
    {
        Log("CChooseSpawn::show -> NewObject failed");
        env->ExceptionClear();
        return;
    }

    CChooseSpawn::thiz = env->NewGlobalRef(obj);
    env->DeleteLocalRef(obj);

    Log("CChooseSpawn::show OK org=%d home=%d family=%d pos=%d", organization, home, family, pos);
}

void CChooseSpawn::hide()
{
    if (!CChooseSpawn::thiz || !CChooseSpawn::clazz) return;

    JNIEnv* env = CJavaWrapper::GetEnv();
    if (!env) return;

    jmethodID method = env->GetMethodID(CChooseSpawn::clazz, "destroy", "()V");
    if (method) env->CallVoidMethod(CChooseSpawn::thiz, method);
    env->ExceptionClear();

    env->DeleteGlobalRef(CChooseSpawn::thiz);
    CChooseSpawn::thiz = nullptr;

    Log("CChooseSpawn::hide OK");
}

/*
 * Kotlin:
 * external fun SendSpawn(spawnid: Int)
 * => JNI имя ДОЛЖНО быть ..._SpawnSelect_SendSpawn
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_auth_SpawnSelect_SendSpawn(JNIEnv* env, jobject thiz, jint spawnid)
{
    if (!pNetGame) return;
    pNetGame->SendCustomPacketSpawn(251, 60, (uint32_t)spawnid);
}

// совместимость (если где-то осталось nativeSendSpawn)
extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_auth_SpawnSelect_nativeSendSpawn(JNIEnv* env, jobject thiz, jint spawnid)
{
    Java_com_multirium_mobile_gui_auth_SpawnSelect_SendSpawn(env, thiz, spawnid);
}
