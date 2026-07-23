//
// Created by viski on 05.04.2025.
//

#include <jni.h>
#include "Loader.h"
#include "java/CJavaGui.h"
#include "java/CDialog.h"
#include "java/CSpeedometr.h"

extern void Log(const char* fmt, ...);

jobject CLoader::thiz = nullptr;
jclass  CLoader::clazz = nullptr;

void CLoader::initJavaClasses(JavaVM* pjvm)
{
    JNIEnv* env = nullptr;
    pjvm->GetEnv((void**)& env, JNI_VERSION_1_6);

    CDialog::clazz = env->FindClass("com/multirium/mobile/gui/dialogs/Dialog");
    CDialog::clazz = (jclass) env->NewGlobalRef( CDialog::clazz );

    CJavaGui::clazz = env->FindClass("com/multirium/mobile/NativeGuiManager");
    CJavaGui::clazz = (jclass) env->NewGlobalRef(CJavaGui::clazz );

    CSpeedometr::clazz = env->FindClass("com/multirium/mobile/gui/speedometr/Speedometr");
    if (CSpeedometr::clazz == nullptr) {
        Log("Failed to create global ref for Speedometr class");
        return;
    }

    CSpeedometr::clazz = (jclass) env->NewGlobalRef( CSpeedometr::clazz );

    //CPauseMenu::clazz = env->FindClass("com/multirium/mobile/gui/main_menu/utils/SettingMenuUtil");
    //CPauseMenu::clazz = (jclass) env->NewGlobalRef( CPauseMenu::clazz );
    CLoader::clazz = env->FindClass("com/multirium/mobile/gui/main_menu/utils/SettingMenuUtil");
    CLoader::clazz = (jclass) env->NewGlobalRef( CLoader::clazz );
}