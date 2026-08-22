//
// Created by viski on 31.03.2025.
//
#include "../main.h"
#include <jni.h>
#include "../vendor/armhook/patch.h"
#include "CPauseMenu.h"
#include "jniutil.h"
#include "game/core/Timer.h"
#include "game/core/game.h"
#include "net/netgame.h"
#include "CJavaGui.h"
#include "Loader.h"
#include "../game/mobilemenu/MobileMenu.h"
#include "../game/core/widgets/WidgetGta.h"
#include "game/core/widgets/TouchInterface.h"

bool  CPauseMenu::bIsShow = false;

extern CJavaWrapper *pJavaWrapper;
extern CNetGame *pNetGame;
extern CGUI *pGUI;
extern CGame *pGame;

void (*CTouchInterface_LoadFromFile)(const char *a1, int a2);
void CTouchInterface_LoadFromFile_hooked(const char *a1, int a2) {
    JNIEnv *env = CJavaWrapper::GetEnv();

    jmethodID method = env->GetStaticMethodID(CLoader::clazz, "loadSettings", "()V");
    env->CallStaticVoidMethod(CLoader::clazz, method);

    CTouchInterface_LoadFromFile(a1, a2);
}

void CPauseMenu::InjectHooks() {
    uintptr_t *setting = (uintptr_t *) (g_libGTASA + (VER_x32 ? 0x6E03FC : 0x8BEB38));
    CHook::Write(g_libGTASA + (VER_x32 ? 0x6E03FC : 0x8BEB38), &CPauseMenu::Settings);
    CPauseMenu::Settings = reinterpret_cast<Setting *>(setting);

    CHook::RET(OBFUSCATE("_ZN10MobileMenu10DrawSpriteEP9RwTextureP9CVector2DS3_P5CRGBA")); // MobileMenu::DrawSprite
    CHook::RET(OBFUSCATE("_ZN10MobileMenu10DrawSpriteEP9RwTexture5CRGBA9CVector2DS3_")); // MobileMenu::DrawSprite
    CHook::RET(OBFUSCATE("_ZN10MobileMenu10DrawSpriteEP9RwTexture5CRGBA9CVector2DS3_S3_S3_")); // MobileMenu::DrawSprite
    CHook::RET(OBFUSCATE("_ZN10MenuScreen10DrawSpriteEP9RwTexture5CRGBA9CVector2DS3_S3_S3_")); // MenuScreen::DrawSprite

    CHook::RET(OBFUSCATE("_ZN6CRadar10DrawLegendEiii")); // CRadar::DrawLegend
    CHook::RET(OBFUSCATE("_ZN10MenuScreen6OnBackEv")); // MenuScreen::OnBack
    CHook::RET(OBFUSCATE("_ZN10MenuScreen8DrawBackEb")); // MenuScreen::DrawBack
    CHook::RET(OBFUSCATE("_ZN14MainMenuScreen6OnBackEv")); // MainMenuScreen::OnBack
    CHook::RET(OBFUSCATE("_Z12ScreenToMenuRK9CVector2D")); // ScreenToMenu
    CHook::RET(OBFUSCATE("_ZN10MobileMenu6UnloadEv")); // MobileMenu::Unload
    // MobileMenu::PrepareToRender is implemented in our client and must not be
    // replaced with an unconditional native RET. The native Render path relies
    // on the 2D RenderWare state being prepared before it draws the menu.
    CHook::RET(OBFUSCATE("_ZN10MenuScreen8DrawQuadE9CVector2DS0_P5CRGBA")); // MobileMenu::DrawQuad
    CHook::RET(OBFUSCATE("_ZN10MenuScreen10DrawSliderE9CVector2DS0_fbhb")); // MobileMenu::DrawSlider
    CHook::RET(OBFUSCATE("_ZN14MainMenuScreen11AddAllItemsEv")); // MainMenuScreen::AddAllItems
#if VER_x32
    CHook::NOP(g_libGTASA + 0x2AB4A6, 2);
#else
    CHook::NOP(g_libGTASA + 0x36A190, 1);

    CHook::NOP(g_libGTASA + 0x36A090, 1); // dark background below city labels

    // map zoom speed
    //CHook::WriteMemory(g_libGTASA + 0x368F38, "\x00\x00\xF0\x41", 4);

    // map zoom max
    // default: 1100.0f
    //CHook::WriteMemory(g_libGTASA + 0x368F7C, "\x00\x80\xBB\x44", 4);
    //CHook::WriteMemory(g_libGTASA + 0x368F88, "\x00\x80\xBB\x44", 4);
    //CHook::WriteMemory(g_libGTASA + 0x368F80, "\x00\x80\xBB\x44", 4);

    uintptr_t viski_addr[] = {
            0x369CF8, 0x369CFC, 0x369D08,
            0x369D4C, 0x369D50, 0x369D5C,
            0x369DB0, 0x369DB4, 0x369DB8,
            0x369DF4, 0x369E10, 0x369E14,
            0x369E3C, 0x369E40, 0x369E50,
            0x369E7C, 0x369E80, 0x369E9C,
            0x369D84, 0x369D90
    };

    const char* viski_src[] = {
            "\x01\x00\x80\x52",
            "\x62\x13\x80\x52",
            "\xC3\x1D\x80\x52",
            "\x01\x00\x80\x52",
            "\x62\x13\x80\x52",
            "\xC3\x1D\x80\x52",
            "\x01\x00\x80\x52",
            "\x62\x13\x80\x52",
            "\xC3\x1D\x80\x52",
            "\x01\x00\x80\x52",
            "\x62\x13\x80\x52",
            "\xC3\x1D\x80\x52",
            "\x01\x00\x80\x52",
            "\x62\x13\x80\x52",
            "\xC3\x1D\x80\x52",
            "\x01\x00\x80\x52",
            "\x62\x13\x80\x52",
            "\xC3\x1D\x80\x52",
            "\xA1\x00\x80\x52",
            "\xC1\x00\x80\x52"
    };

    for (int i = 0; i < 20; i++) {
        CHook::WriteMemory(g_libGTASA + viski_addr[i], viski_src[i], 4);
    }
#endif

    CHook::RET(OBFUSCATE("_Z11BackPressedv"));

    CHook::InlineHook("_ZN15CTouchInterface12LoadFromFileEPKcb", CTouchInterface_LoadFromFile_hooked, &CTouchInterface_LoadFromFile);
}

void openMap() {
    MobileMenu& gMobileMenu1 = *(MobileMenu*)(g_libGTASA + (VER_x32 ? 0x6E0074 : 0x8BE780));
    gMobileMenu1.isMapMode = true;
    MobileMenu::InitForPause_orig(&gMobileMenu1);
}

void closeMap() {
    MobileMenu& gMobileMenu1 = *(MobileMenu*)(g_libGTASA + (VER_x32 ? 0x6E0074 : 0x8BE780));
    CHook::CallFunction<uint64_t>("_ZN10MobileMenu4ExitEv", &gMobileMenu1);
    CTimer::m_UserPause = false;
    pJavaWrapper->SetPauseState(true);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_main_1menu_MainMenu_exit(JNIEnv *env, jclass clazz) {
    // TODO: implement exit()

    if ( pJavaWrapper ) {
        pJavaWrapper->SetPauseState(false);
    }

    CHook::CallFunction<void>(OBFUSCATE("_Z20Menu_SwitchOffToGamev"));
    //pGame->FindPlayerPed()->TogglePlayerControllable(true);
    pGame->FindPlayerPed()->ToggleControllable(true);
    pGame->DisplayHUD(true);
    CTimer::ms_fTimeScale = 1.0f;
    CPauseMenu::bIsShow = false;
    CLocalPlayer::m_bDontSendSync = false;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_main_1menu_MenuMap_toggleMap(JNIEnv *env, jobject thiz, jboolean toggle) {
    // TODO: implement toggleMap()

    if (toggle) {
        CGame::PostToMainThread(openMap);
    } else {
        CGame::PostToMainThread(closeMap);
    }
}
extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_main_1menu_utils_SettingGameButtonUtil_updateGameButton(JNIEnv *env,
                                                                                 jclass clazz,
                                                                                 jint id, jfloat x, jfloat y,
                                                                                 jfloat width, jfloat height) {
    // TODO: implement updateGameButton()

    if(id <= 190) {
        if (!CTouchInterface::m_pWidgets[id]) return;
        CTouchInterface::m_pWidgets[id]->m_fOriginX = x;
        CTouchInterface::m_pWidgets[id]->m_fOriginY = y;
        CTouchInterface::m_pWidgets[id]->m_fScaleX = width;
        CTouchInterface::m_pWidgets[id]->m_fScaleY = height;
    }
}
void ApplyFPSPatch(uint8_t fps);
extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_gui_main_1menu_utils_SettingMenuUtil_updateMenuParam(JNIEnv *env, jclass clazz,
                                                                          jint id, jint value) {
    // TODO: implement updateMenuParam()

    if(id <= 49) {
        CPauseMenu::Settings[id].value = value;

        switch (id) {
            case 2:
                *(float *) (g_libGTASA + (VER_x32 ? 0x6AAD54 : 0x886364)) = (float) ((float) ((float) CPauseMenu::Settings[2].value * 0.875f) * 0.01f) + 0.875f;
                break;
            case 11:
                CHook::CallFunction<void>(OBFUSCATE("_Z23Menu_ApplyAudioSettingsv"));
                break;
            case 39:
                ApplyFPSPatch(value);
                break;
        }
    }
}