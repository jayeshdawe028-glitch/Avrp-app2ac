/* ============================================================================

    Echo Cross
    Telegram: @tasjJson

============================================================================ */

#include "CHud.h"
#include <jni.h>

#include "main.h"
#include "game/core/game.h"
#include "net/netgame.h"
#include "java/jniutil.h"
#include "settings.h"
#include "vendor/armhook/patch.h"
#include "CSpeedometr.h"

extern CNetGame* pNetGame;
extern CJavaWrapper* g_pJavaWrapper;
extern CGame* pGame;

bool CHud::bIsShow = false;
jobject CHud::thiz = nullptr;

int CHud::iWantedLevel = 0;
int CHud::iLocalMoney = 0;

CVector2D CHud::radarBgPos1;
CVector2D CHud::radarBgPos2;
CVector2D CHud::radarPos;
float CHud::radarSize;

static jmethodID jUpdateHudInfo = nullptr;
static jmethodID jUpdateAmmo = nullptr;
static jmethodID jToggleAll = nullptr;
static jmethodID jUpdateWanted = nullptr;
static jmethodID jUpdateMoney = nullptr;
static jmethodID jToggleGps = nullptr;
static jmethodID jToggleGreenZone = nullptr;
static jmethodID jToggleLogo = nullptr;
static jmethodID jToggleVoice = nullptr;
static jmethodID jUpdatePlayerInfo = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_hud_HudManager_HudInit(JNIEnv* env, jobject thiz)
{
    if (CHud::thiz)
    {
        env->DeleteGlobalRef(CHud::thiz);
        CHud::thiz = nullptr;
    }

    CHud::thiz = env->NewGlobalRef(thiz);

    jclass clazz = env->GetObjectClass(thiz);
    if (!clazz) return;

    jUpdateHudInfo    = env->GetMethodID(clazz, "UpdateHudInfo", "(II)V");
    jUpdateAmmo       = env->GetMethodID(clazz, "UpdateAmmo", "(III)V");
    jToggleAll        = env->GetMethodID(clazz, "toggleAll", "(ZZ)V");
    jUpdateWanted     = env->GetMethodID(clazz, "UpdateWanted", "(I)V");
    jUpdateMoney      = env->GetMethodID(clazz, "updateMoney", "(I)V");
    jToggleGps        = env->GetMethodID(clazz, "toggleGps", "(Z)V");
    jToggleGreenZone  = env->GetMethodID(clazz, "toggleGreenZone", "(Z)V");
    jToggleLogo       = env->GetMethodID(clazz, "toggleLogo", "(Z)V");
    jToggleVoice      = env->GetMethodID(clazz, "toggleVoice", "(Z)V");
    jUpdatePlayerInfo = env->GetMethodID(clazz, "updatePlayerInfo", "(Ljava/lang/String;I)V");

    env->DeleteLocalRef(clazz);
}

void CHud::toggleAll(bool toggle, bool isChat)
{
    if (toggle == bIsShow) return;
    bIsShow = toggle;

    if (pNetGame && pNetGame->GetPlayerPool() &&
        pNetGame->GetPlayerPool()->GetLocalPlayer() &&
        pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed() &&
        pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed()->IsInVehicle())
    {
        if (!toggle)
        {
            if (CSpeedometr::bIsShow) CSpeedometr::tempToggle(false);
        }
        else
        {
            if (CSpeedometr::bIsShow) CSpeedometr::tempToggle(true);
        }
    }

    JNIEnv* env = CJavaWrapper::GetEnv();
    if (!env) return;

    if (jToggleAll && CHud::thiz)
        env->CallVoidMethod(CHud::thiz, jToggleAll, (jboolean)toggle, (jboolean)isChat);
}

static int tickHudInfo = 0;
void CHud::UpdateHudInfo()
{
    tickHudInfo++;
    if (tickHudInfo < 20) return;
    tickHudInfo = 0;

    CPlayerPed* pPed = pGame ? pGame->FindPlayerPed() : nullptr;
    if (!pPed) return;

    JNIEnv* env = CJavaWrapper::GetEnv();
    if (!env) return;

    if (jUpdateHudInfo && CHud::thiz)
        env->CallVoidMethod(CHud::thiz, jUpdateHudInfo, (jint)pPed->GetHealth(), (jint)pPed->GetArmour());

    if (pNetGame && pNetGame->GetPlayerPool())
    {
        CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
        CHud::updatePlayerInfo(pPlayerPool->GetLocalPlayerID(), pSettings->Get().szNickName);
    }

    CHud::UpdateAmmo();
}

static int tickAmmo = 0;
void CHud::UpdateAmmo()
{
    if (!CHud::bIsShow) return;
    tickAmmo++;
    if (tickAmmo < 20) return;
    tickAmmo = 0;

    JNIEnv* env = CJavaWrapper::GetEnv();
    if (!env) return;

    CPlayerPed* pPed = pGame ? pGame->FindPlayerPed() : nullptr;
    if (!pPed || !pPed->m_pPed) return;

    int slot = (int)pPed->m_pPed->m_nActiveWeaponSlot;
    if (slot < 0) slot = 0;

    if (!jUpdateAmmo || !CHud::thiz) return;

    env->CallVoidMethod(CHud::thiz, jUpdateAmmo,
                        (jint)pPed->m_pPed->m_aWeapons[slot].dwType,
                        (jint)pPed->m_pPed->m_aWeapons[slot].dwAmmo,
                        (jint)pPed->m_pPed->m_aWeapons[slot].dwAmmoInClip);
}

void CHud::UpdateWanted()
{
    JNIEnv* env = CJavaWrapper::GetEnv();
    if (!env) return;

    if (jUpdateWanted && CHud::thiz)
        env->CallVoidMethod(CHud::thiz, jUpdateWanted, (jint)iWantedLevel);
}

void CHud::UpdateMoney()
{
    JNIEnv* env = CJavaWrapper::GetEnv();
    if (!env) return;

    if (jUpdateMoney && CHud::thiz)
        env->CallVoidMethod(CHud::thiz, jUpdateMoney, (jint)iLocalMoney);
}

void CHud::toggleGps(bool toggle)
{
    JNIEnv* env = CJavaWrapper::GetEnv();
    if (!env) return;

    if (jToggleGps && CHud::thiz)
        env->CallVoidMethod(CHud::thiz, jToggleGps, (jboolean)toggle);
}

void CHud::toggleGreenZone(bool toggle)
{
    JNIEnv* env = CJavaWrapper::GetEnv();
    if (!env) return;

    CHud::UpdateAmmo();

    if (jToggleGreenZone && CHud::thiz)
        env->CallVoidMethod(CHud::thiz, jToggleGreenZone, (jboolean)toggle);
}

void CHud::toggleLogo(bool toggle)
{
    JNIEnv* env = CJavaWrapper::GetEnv();
    if (!env) return;

    if (jToggleLogo && CHud::thiz)
        env->CallVoidMethod(CHud::thiz, jToggleLogo, (jboolean)toggle);
}

void CHud::toggleVoice(bool toggle)
{
    JNIEnv* env = CJavaWrapper::GetEnv();
    if (!env) return;

    if (jToggleVoice && CHud::thiz)
        env->CallVoidMethod(CHud::thiz, jToggleVoice, (jboolean)toggle);
}

void CHud::updatePlayerInfo(int id, char* name)
{
    JNIEnv* env = CJavaWrapper::GetEnv();
    if (!env) return;

    if (!jUpdatePlayerInfo || !CHud::thiz) return;

    jstring jname = env->NewStringUTF(name ? name : "");
    env->CallVoidMethod(CHud::thiz, jUpdatePlayerInfo, jname, (jint)id);
    env->DeleteLocalRef(jname);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_hud_HudManager_alt(JNIEnv* env, jobject thiz)
{
    LocalPlayerKeys.bKeys[ePadKeys::KEY_WALK] = true;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_hud_HudManager_N(JNIEnv* env, jobject thiz)
{
    LocalPlayerKeys.bKeys[ePadKeys::KEY_NO] = true;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_hud_HudManager_SetRadarBgPos(JNIEnv* env, jobject thiz,
                                                           jfloat x1, jfloat y1,
                                                           jfloat x2, jfloat y2)
{
    CHud::radarBgPos1.x = x1;
    CHud::radarBgPos1.y = y1;
    CHud::radarBgPos2.x = x2;
    CHud::radarBgPos2.y = y2;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_hud_HudManager_SetRadarPos(JNIEnv* env, jobject thiz,
                                                         jfloat x1, jfloat y1, jfloat size)
{
    CHud::radarPos.x = x1;
    CHud::radarPos.y = y1;
    CHud::radarSize = size;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_hud_HudManager_nativeBoolShowHud(JNIEnv* env, jobject thiz,
                                                               jboolean toggle)
{
    CHud::bIsShow = (bool)toggle;
}

extern "C" void Voice_HudStart();
extern "C" void Voice_HudStop();

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_hud_HudManager_nativeVoiceStart(JNIEnv* env, jobject thiz)
{
    LogVoice("[sv:dbg:hudvoice] : Java button -> nativeVoiceStart()");
    Voice_HudStart();
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_hud_HudManager_nativeVoiceStop(JNIEnv* env, jobject thiz)
{
    LogVoice("[sv:dbg:hudvoice] : Java button -> nativeVoiceStop()");
    Voice_HudStop();
}
