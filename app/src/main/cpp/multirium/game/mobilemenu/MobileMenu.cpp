//
// Created by viski on 04.05.2025.
//

#include "MobileMenu.h"
#include "main.h"
#include <jni.h>
#include "vendor/armhook/patch.h"
#include "game/core/Timer.h"
#include "game/core/game.h"
#include "java/jniutil.h"
#include "java/CJavaGui.h"
#include "java/CPauseMenu.h"
#include "net/netgame.h"
#include "game/core/widgets/WidgetGta.h"
#include "game/core/widgets/TouchInterface.h"

extern CJavaWrapper *pJavaWrapper;
extern CNetGame *pNetGame;
extern CGame *pGame;

MobileMenu gMobileMenu;

void Menu_SwitchOnFromGame() {
    CHook::CallFunction<void>("_Z21Menu_SwitchOnFromGamev");
}

void Menu_MapInitPause() {
    CHook::CallFunction<void>("_Z17Menu_MapInitPausev");
}

void MobileMenu::Load() {
    if (g_libGTASA <= 0x1000) return;
    CHook::CallFunction<void>("_ZN10MobileMenu4LoadEv");
}
/*
void MobileMenu::InitForPause_orig()
{
    thiz->InitializedForSignOut = 0;

    AudioEngine = *(CAudioEngine*)(g_libGTASA + 0xBB65F8);

    CHook::CallFunction<void>("_ZN12CAudioEngine24ReportFrontendAudioEventEiff", &AudioEngine, 0, 0.0, 1.0);
    CHook::CallFunction<void>("_ZN12CAudioEngine7ServiceEv", &AudioEngine);

    Menu_SwitchOnFromGame();
    Menu_MapInitPause();

    MobileMenu::Load(thiz);

   // void* v2 = operator new(0x78uLL);
   // CHook::CallFunction<void>("_ZN10FlowScreenC2Ebb", (FlowScreen *)v2, 1, 1);
  //  FlowScreen::FlowScreen((FlowScreen *)v2, 1, 1);
   // *(uint64_t *)v2 = (uint64_t)(g_libGTASA + 0x825AB0);
    void* v2 = operator new(0x78u);
    CHook::CallFunction<void>("_ZN10FlowScreenC2Ebb", (FlowScreen *)v2, 1, 1);
    *(void **)v2 = (void *)(g_libGTASA + 0x825AB0);

    MainMenuScreen::AddAllItems((MainMenuScreen *)v2);
    if ( thiz->pendingScreen )
        MobileMenu::ProcessPending(thiz);
    thiz->pendingScreen = (MenuScreen *)v2;
    if ( CTouchInterface::m_pWidgets[161]
         && (unsigned int)(CEntryExitManager::ms_exitEnterState - 1) >= 2
         && CTouchInterface::m_pWidgets[161]->m_fTapHoldTime > 0.66 )
    {
        thiz->isMapMode = 1;
        *(_DWORD *)(v2 + 16) = 1065353216;
    }
}*/

void (*MobileMenu::InitForPause_orig)(MobileMenu* thiz);
void MobileMenu::InitForPause() {
    CJavaGui::Create(PAUSE);
    if ( pJavaWrapper ) {
        pJavaWrapper->SetPauseState(true);
    }

    pGame->FindPlayerPed()->ToggleControllable(false);
    pGame->DisplayHUD(false);
    CTimer::ms_fTimeScale = 0.2f; // 0.4f
    CPauseMenu::bIsShow = true;
    CLocalPlayer::m_bDontSendSync = true;
}

CVector2D MobileMenu::CalcBGUVSize() {
    float aspectRatioWidth = (static_cast<float>(RsGlobal->screenWidth) * 0.4f)
                             / static_cast<float>(RsGlobal->screenHeight);

    return CVector2D(aspectRatioWidth, 0.4f);
}

CVector2D MobileMenu::GetRandomBGCoords()
{
    float availableWidth = 1.0f - gMobileMenu.bgUVSize.x;
    float availableHeight = 1.0f - gMobileMenu.bgUVSize.y;

    return CVector2D(
            availableWidth * (rand() / static_cast<float>(RAND_MAX)),
            availableHeight * (rand() / static_cast<float>(RAND_MAX))
    );
}

void MobileMenu::InitGameVars()
{
    gMobileMenu.waypoint_blip = 0;
    gMobileMenu.m_WantsToRestartGame = 0;
}

MobileMenu::MobileMenu()
{
    gMobileMenu.WantsToLoad = false;
    gMobileMenu.isMapMode = 0;

    gMobileMenu.screenStack.Clear();

    gMobileMenu.pendingScreen = 0;
    gMobileMenu.NEW_MAP_SCALE = 0x43A0000044898000LL;
    gMobileMenu.MAP_OFFSET_Y = 224.0f;
    gMobileMenu.CurrentGameNotResumable = 0;
    MobileMenu::InitGameVars();
    gMobileMenu.pointerMode = 1;
}

void MobileMenu::ClampBGCoords(CVector2D* toClamp) {
    toClamp->x = std::min(toClamp->x, 1.0f - gMobileMenu.bgUVSize.x);
    toClamp->y = std::min(toClamp->y, 1.0f - gMobileMenu.bgUVSize.y);
}

float VectorDistance(const CVector2D& a, const CVector2D& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

void MobileMenu::Initialize(MobileMenu* menu) {
    float screenWidth = RsGlobal->screenWidth ? (float)RsGlobal->screenWidth : 2436.0f;
    float screenHeight = RsGlobal->screenHeight ? (float)RsGlobal->screenHeight : 1125.0f;

    menu->bgUVSize.x = (screenWidth * 0.4f) / screenHeight;
    menu->bgUVSize.y = 0.4f;

    do {
        menu->bgCurCoords.x = (1.0f - menu->bgUVSize.x) * (rand() / (float)RAND_MAX);
        menu->bgCurCoords.y = (1.0f - menu->bgUVSize.y) * (rand() / (float)RAND_MAX);

        menu->bgStartCoords = menu->bgCurCoords;

        menu->bgTargetCoords.x = (1.0f - menu->bgUVSize.x) * (rand() / (float)RAND_MAX);
        menu->bgTargetCoords.y = (1.0f - menu->bgUVSize.y) * (rand() / (float)RAND_MAX);

    } while (VectorDistance(menu->bgStartCoords, menu->bgTargetCoords) < 0.35f);

    CVector2D direction = {
            menu->bgTargetCoords.x - menu->bgCurCoords.x,
            menu->bgTargetCoords.y - menu->bgCurCoords.y
    };

    float invLength = 1.0f / sqrtf(direction.x * direction.x + direction.y * direction.y);
    menu->bgCurCoords.x += direction.x * invLength * 0.15f;
    menu->bgCurCoords.y += direction.y * invLength * 0.15f;
}

bool MobileMenu::IsMainMenuScreen(MobileMenu *menu) {
    return menu->screenStack.numEntries == 1;
}

bool Menu_IsPlayingGame(bool checkThatGameIsResumable) {
    return *(uint32_t*)(g_libGTASA + 0xD3D640) == 9;
}

MobileMenu::~MobileMenu()
{
    MenuScreen** dataPtr = gMobileMenu.screenStack.dataPtr;
    gMobileMenu.screenStack.numEntries = 0;
    if (dataPtr) {
        free(dataPtr);
        gMobileMenu.screenStack.dataPtr = nullptr;
    }
    gMobileMenu.screenStack.numAlloced = 0;
}

void Menu_MapRender() {
    gMobileMenu.DisplayingMap = 1;

    CHook::CallFunction<void>("_ZN6CRadar15InitFrontEndMapEv");

    float mapScale = gMobileMenu.NEW_MAP_SCALE;
    int screenWidth = RsGlobal->screenWidth;
    int screenHeight = RsGlobal->screenHeight;
    float scaleFactor = mapScale / 5.0f;

    if (CTouchInterface::m_pWidgets[150]) {
        CRect widgetRect(0.0f, 0.0f, (float)screenWidth, (float)screenHeight);
        CHook::CallFunction<void>("_ZN7CWidget13SetScreenRectERK5CRect", CTouchInterface::m_pWidgets[150], &widgetRect);
    }

    float mapOffsetX = (gMobileMenu.MAP_OFFSET_X - mapScale) / 448.0f * screenHeight;
    float mapOffsetY = (gMobileMenu.MAP_OFFSET_Y - mapScale) / 448.0f * screenHeight;
    float cellSize = (mapScale / 448.0f) * screenHeight;

    CRect fullScreenRect(0.0f, 0.0f, (float)screenWidth, (float)screenHeight);
    CRGBA black(0, 0, 0, 255);
    CSprite2d::DrawRect(&fullScreenRect, &black);

    RwRenderStateSet(rwRENDERSTATESRCBLEND, RWRSTATE(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDINVSRCALPHA));
    CHook::CallFunction<void>("_Z12emu_glEnablej", 0xBE2u);
/*
    for (int y = 0; y < 10; y++) {
        for (int x = 0; x < 10; x++) {
            if (CTheZones::ZonesVisited[0][y * 10 + x]) {
                // Calculate cell coordinates
                float left = mapOffsetX + cellSize * y;
                float top = mapOffsetY + cellSize * (x + 1);
                float right = mapOffsetX + cellSize * (y + 1);
                float bottom = mapOffsetY + cellSize * x;
                float borderSize = cellSize * 0.125f;

                // Draw cell with borders
                CRGBA white(255, 255, 255, 255);
                CRGBA dark(0, 0, 0, 255);

                // Main cell
                CRect cellRect(left, top, right, bottom);
                CSprite2d::DrawRect(&cellRect, &white);

                // Draw borders around the cell
                // (Left, right, top, bottom borders omitted for brevity)
                // ...
            }
        }
    }*/

    Log("draw back map");
    RwRenderStateSet(rwRENDERSTATESRCBLEND, RWRSTATE(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDINVSRCALPHA));

    CRGBA grayBg1(96, 96, 96, 255);
    CSprite2d::DrawRect(&fullScreenRect, &grayBg1);

    CRGBA grayBg2(160, 160, 160, 255);
    CSprite2d::DrawRect(&fullScreenRect, &grayBg2);

    Log("draw radar sections");
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            float sectionLeft = mapOffsetX + cellSize * y;
            float sectionTop = mapOffsetY + cellSize * x;

            if (sectionLeft > -cellSize && sectionTop > -cellSize &&
                sectionLeft + cellSize < screenWidth + cellSize &&
                sectionTop + cellSize < screenHeight + cellSize) {
                CHook::CallFunction<void>("_ZN6CRadar19DrawRadarSectionMapEii5CRectf", y, x, CRect(sectionLeft, sectionTop,
                                                                                              sectionLeft + cellSize, sectionTop + cellSize), 1.0f);
            }
        }
    }

   /* Log("Draw gang overlay and blips");
    RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)5);
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)6);

    CRadar::DrawRadarGangOverlay(true);

    if (!(CTheScripts::bPlayerIsOffTheMap || CTheScripts::HideAllFrontEndMapBlips)) {
        float scrollOffset = fmodf(mapScroll, 1.0f);
        CRadar::DrawBlips(scrollOffset);
    }

    GxtChar* locationName = CPlaceName::GetForMap(&CUserDisplay::PlaceName,
        gMobileMenu.MAP_AREA_X, gMobileMenu.MAP_AREA_Y);

    */

    Log("draw legend and other UI elements");

    Log("draw crosshair for joystick control");
    if (CHook::CallFunction<HIDInputType>("_ZN4CHID12GetInputTypeEv") == HID_INPUT_TYPE_JOYPAD && gMobileMenu.isMapMode) {
        float centerX = screenWidth * 0.5f;
        float centerY = screenHeight * 0.5f;

        CRect hLine(centerX - 16.0f, centerY, centerX + 16.0f, centerY);
        CSprite2d::DrawRect(&hLine, &black);

        CRect vLine(centerX, centerY - 16.0f, centerX, centerY + 16.0f);
        CSprite2d::DrawRect(&vLine, &black);
    }

    gMobileMenu.DisplayingMap = 0;
}

void MobileMenu::PrepareToRender()
{
    CFont::SetAlphaFade(255.0);
//    CSprite2d::InitPerFrame();
    CFont::InitPerFrame();
    CFont::SetBackground(0, 0);
    CFont::SetWrapx((float)RsGlobal->screenWidth);
    CFont::SetOrientation(1u);
    CFont::SetCentreSize((float)RsGlobal->screenWidth);
    CFont::SetProportional(1u);
    DefinedState2d();
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       RWRSTATE(FALSE));
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      RWRSTATE(FALSE));
    RwRenderStateSet(rwRENDERSTATEFOGENABLE,         RWRSTATE(FALSE));
    RwRenderStateSet(rwRENDERSTATESRCBLEND,          RWRSTATE(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,         RWRSTATE(rwBLENDINVSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER,     RWRSTATE(rwFILTERLINEAR));
    RwRenderStateSet(rwRENDERSTATETEXTUREADDRESS,    RWRSTATE(rwTEXTUREADDRESSCLAMP));
   // RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     RWRSTATE(NULL));
}

float mapModeOpacity;

void SetupVertices(float x1, float y1, float x2, float y2, CRGBA color1, CRGBA color2)
{
    RwUInt32 color1Value = (color1.a << 24) | (color1.r << 16) | (color1.g << 8) | color1.b;
    RwUInt32 color2Value = (color2.a << 24) | (color2.r << 16) | (color2.g << 8) | color2.b;

    CSprite2d::maVertices[0].x = x1;
    CSprite2d::maVertices[0].y = y1;
    CSprite2d::maVertices[0].z = 0.0f;
    CSprite2d::maVertices[0].rhw = 1.0f;
    CSprite2d::maVertices[0].emissiveColor = color1Value;
    CSprite2d::maVertices[0].u = 0.0f;
    CSprite2d::maVertices[0].v = 0.0f;

    CSprite2d::maVertices[1].x = x2;
    CSprite2d::maVertices[1].y = y1;
    CSprite2d::maVertices[1].z = 0.0f;
    CSprite2d::maVertices[1].rhw = 1.0f;
    CSprite2d::maVertices[1].emissiveColor = color1Value;
    CSprite2d::maVertices[1].u = 1.0f;
    CSprite2d::maVertices[1].v = 0.0f;

    CSprite2d::maVertices[2].x = x1;
    CSprite2d::maVertices[2].y = y2;
    CSprite2d::maVertices[2].z = 0.0f;
    CSprite2d::maVertices[2].rhw = 1.0f;
    CSprite2d::maVertices[2].emissiveColor = color2Value;
    CSprite2d::maVertices[2].u = 0.0f;
    CSprite2d::maVertices[2].v = 1.0f;

    CSprite2d::maVertices[3].x = x2;
    CSprite2d::maVertices[3].y = y2;
    CSprite2d::maVertices[3].z = 0.0f;
    CSprite2d::maVertices[3].rhw = 1.0f;
    CSprite2d::maVertices[3].emissiveColor = color2Value;
    CSprite2d::maVertices[3].u = 1.0f;
    CSprite2d::maVertices[3].v = 1.0f;
}

RwBool RwIm2DRenderPrimitive_BUGFIX(RwPrimitiveType primType, RwIm2DVertex *vertices, RwInt32 numVertices)
{
    return RwIm2DRenderPrimitive(primType, vertices, numVertices);
}

void RenderFadeOverlay(CRGBA upperColor, CRGBA lowerColor, float screenWidth, float upperHeight) {
    SetupVertices(0.0f, 0.0f, screenWidth, upperHeight,
                  upperColor, upperColor);
    RwIm2DRenderPrimitive_BUGFIX(rwPRIMTYPETRISTRIP, CSprite2d::maVertices, 4);

    SetupVertices(0.0f, upperHeight, screenWidth, RsGlobal->screenHeight,
                  lowerColor, lowerColor);
    RwIm2DRenderPrimitive_BUGFIX(rwPRIMTYPETRISTRIP, CSprite2d::maVertices, 4);
}

void MobileMenu::Render(MobileMenu* menu) {
    // We do not want libGTASA's default singleplayer menu rendering to execute and crash,
    // especially with the custom frontend hook flows.
    return;
}

void MobileMenu::InjectHooks() {
    gMobileMenu = *(MobileMenu*)(g_libGTASA + (VER_x32 ? 0x6E0074 : 0x8BE780));
 //   CHook::Redirect("_ZN10MobileMenu10InitializeEv", &MobileMenu::Initialize);
    mapModeOpacity = *(float*)(g_libGTASA + (VER_x32 ? 0x6E0128 : 0x8BE854));
    CHook::InlineHook("_ZN10MobileMenu12InitForPauseEv", MobileMenu::InitForPause, &MobileMenu::InitForPause_orig);

    // Redirect Render to our safe no-op to bypass the SIGSEGV
    CHook::Redirect("_ZN10MobileMenu6RenderEv", &MobileMenu::Render);
}