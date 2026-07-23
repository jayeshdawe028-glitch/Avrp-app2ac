//
// Created by viski on 28.03.2025.
//
#include "CChat.h"
#include <jni.h>

#include "main.h"

#include "game/core/game.h"
#include "net/netgame.h"
#include "java/jniutil.h"
#include "settings.h"
#include "vendor/armhook/patch.h"


extern CNetGame* pNetGame;
extern CJavaWrapper *g_pJavaWrapper;
extern CGame *pGame;
extern CGUI *pGUI;

bool  CChat::bIsShow = false;
jobject CChat::thiz = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_chat_Chat_nativeInit(JNIEnv *env, jobject thiz) {
    // TODO: implement nativeInit()

    CChat::thiz = env->NewGlobalRef(thiz);
}

// -------------- CHAT START -----------------
void CChat::AddChatMessage(const char msg[], uint32_t textcolor)
{
    if(!thiz)return;

    char msg_utf[1024];
    cp1251_to_utf8(msg_utf, msg);

    JNIEnv* env = CJavaWrapper::GetEnv();

    jstring jmsg = env->NewStringUTF(msg_utf);

    jclass clazz = env->GetObjectClass(thiz);
    jmethodID AddChatMessage = env->GetMethodID(clazz, "AddChatMessage", "(Ljava/lang/String;I)V");

    env->CallVoidMethod(thiz, AddChatMessage, jmsg, textcolor);
    env->DeleteLocalRef(jmsg);
}
bool CChat::bIsShowChat = true;
void CChat::ToggleChat(bool toggle){
    bIsShowChat = toggle;

    JNIEnv* env = CJavaWrapper::GetEnv();

    jclass clazz = env->GetObjectClass(thiz);
    jmethodID ToggleChat = env->GetMethodID(clazz, "ToggleChat" , "(Z)V");
    env->CallVoidMethod(thiz, ToggleChat, toggle);
}
bool ProcessLocalCommands(const char str[])
{
    if (strcmp(str, "/neonrgb") == 0)
    {
        CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
        if(!pVehiclePool) return true;

        auto pVeh = pNetGame->GetVehiclePool()->GetAt(pVehiclePool->FindIDFromGtaPtr(pGame->FindPlayerPed()->GetGtaVehicle()));
        if (!pVeh)
            return true;
        if (!pGame->FindPlayerPed()->IsInVehicle()) {
            pGUI->AddDebugMessage("[NEON] Вы должны находиться в машине!");
            return true;
        }
        //pVeh->neonColor.r = static_cast<uint8_t>(r);
        //pVeh->neonColor.g = static_cast<uint8_t>(g);
        //pVeh->neonColor.b = static_cast<uint8_t>(b);

        pVeh->neonType = static_cast<CVehicle::eNeonTypes>(2);

        pVeh->m_bShadow = true;
        return true;
    }
    /*bool a = true;
    if (strstr(str, "/timecyc"))
    {
        if(a) {
            Log("show");
            a = false;
            st.isShouldBeShown == true;
        } else {
            Log("hide");
            a = true;
            st.isShouldBeShown == false;
        }

        return true;
    }*/
    if (strcmp(str, "/neon") == 0)
    {
        int r, g, b;
        if (sscanf(str, "/neon %d %d %d", &r, &g, &b) != 3)
        {
            pGUI->AddDebugMessage("[NEON] Используйте: /neon [r] [g] [b]");
            return true;
        }
        CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
        if(!pVehiclePool) return true;

        auto pVeh = pNetGame->GetVehiclePool()->GetAt(pVehiclePool->FindIDFromGtaPtr(pGame->FindPlayerPed()->GetGtaVehicle()));
        if (!pVeh)
            return true;
        if (!pGame->FindPlayerPed()->IsInVehicle()) {
            pGUI->AddDebugMessage("[NEON] Вы должны находиться в машине!");
            return true;
        }
        pVeh->neonColor.r = static_cast<uint8_t>(r);
        pVeh->neonColor.g = static_cast<uint8_t>(g);
        pVeh->neonColor.b = static_cast<uint8_t>(b);

        pVeh->neonType = static_cast<CVehicle::eNeonTypes>(1);

        pVeh->m_bShadow = true;
        return true;
    }
    return false;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_chat_Chat_nativeSendChatMessage(JNIEnv *env, jobject thiz, jbyteArray str) {
    // TODO: implement nativeSendChatMessage()

    jbyte* pMsg = env->GetByteArrayElements(str, nullptr);
    jsize length = env->GetArrayLength(str);

    std::string szStr((char*)pMsg, length);

    if (pNetGame) {
        if (!szStr.empty()) {
            if (szStr[0] == '/')
                if (ProcessLocalCommands(szStr.c_str())) {
                    return;
                }
            if (szStr[0] == '/')
                pNetGame->SendChatCommand(szStr.c_str());
            else
                pNetGame->SendChatMessage(szStr.c_str());
        }
    }
    env->ReleaseByteArrayElements(str, pMsg, JNI_ABORT);
}
// -------------- CHAT END -----------------