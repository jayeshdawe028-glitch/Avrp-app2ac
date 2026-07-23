#include "CMinerIndicator.h"
#include "java/jniutil.h"
#include "main.h"

jobject CMinerIndicator::thiz = nullptr;
static jmethodID jUpdateMiner = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_miner_MinerIndicatorManager_nativeInit(JNIEnv *env, jobject thiz)
{
    CMinerIndicator::thiz = env->NewGlobalRef(thiz);

    jclass clazz = env->GetObjectClass(thiz);
    jUpdateMiner = env->GetMethodID(clazz, "update", "(ZZII)V");
}

void CMinerIndicator::Update(bool show, bool inWork, int ore, int ingot)
{
    JNIEnv* env = CJavaWrapper::GetEnv();
    if (!env || !thiz || !jUpdateMiner) return;

    env->CallVoidMethod(thiz, jUpdateMiner,
                        (jboolean)show,
                        (jboolean)inWork,
                        (jint)ore,
                        (jint)ingot);
}
