#include <GLES2/gl2.h>
#include "main.h"
#include "vendor/armhook/patch.h"
#include "game.h"
#include "net/netgame.h"
#include "gui/gui.h"
#include "game/core/textures/TextureDatabase.h"
#include "game/core/textures/TextureDatabaseEntry.h"
#include "game/core/textures/TextureDatabaseRuntime.h"
#include "game/control/Scene.h"
#include "game/renderer/sprite2d.h"
#include "game/peds/PlayerPedGta.h"
#include "game/pools/Pools.h"
#include "java/jniutil.h"
#include "game/modelinfo/ModelInfo.h"
#include "game/core/core/MatrixLink.h"
#include "game/core/core/MatrixLinkList.h"
#include "game/collision/Collision.h"
#include "TxdStore.h"
#include "util/CUtil.h"
#include "game/renderer/Coronas.h"
#include "game/control/multitouch.h"
#include "Streaming.h"
#include "References.h"
#include "VisibilityPlugins.h"
#include "game/animation/AnimManager.h"
#include "FileLoader.h"
#include "game/renderer/Renderer.h"
#include "CrossHair.h"
#include "World.h"
#include "Clock.h"
#include "game/renderer/Clouds.h"
#include "game/renderer/Weather.h"
#include "game/weapons/Weapon.h"
#include "game/control/PathFind.h"
#include "RQShader.h"
#include "game/renderer/Sprite.h"
#include "game/core/widgets/TouchInterface.h"
#include "game/core/widgets/WidgetRadar.h"
#include "java/CPauseMenu.h"
#include "game/audio/CAudioSystem.h"
#include "game/renderer/Shadows.h"
#include "game/custom/GPS.h"
#include "game/custom/SkyBox.h"
#include "game/custom/VehicleNeon.h"
#include "game/renderer/RenderBuffer.hpp"
#include "game/core/fx/Fx.h"
#include "FirstPersonCamera.h"
#include "CFPSFix.h"
#include "ES2VertexBuffer.h"
#include "RQ_Commands.h"
#include "game/control/Pickups.h"
#include "game/renderer/TimeCycle.h"
#include "game/core/pipelines/CustomCar/CustomCarEnvMapPipeline.h"
#include "game/core/pipelines/CustomBuilding/CustomBuildingDNPipeline.h"
#include "game/renderer/Occlusion.h"
#include "game/renderer/RealTimeShadowManager.h"
#include "game/core/widgets/WidgetGta.h"
#include "game/control/CarEnterExit.h"
#include "game/mobilemenu/MobileMenu.h"
#include "game/control/Bridge.h"
#include "game/core/Messages.h"
#include "game/mobilemenu/screens/MainMenuScreen.h"

extern "C" {
#include "game/encryption/aes.h"
}
#include "game/encryption/CTinyEncrypt.h"
#include "game/encryption/encrypt.h"
#include "game/encryption/CGameResourcesDecryptor.cpp"
#include "game/encryption/CGameResourcesDecryptor.h"
#include "game/encryption/str_obfuscator_no_template.hpp"

extern CGUI *pGUI;
extern CGame *pGame;
extern CNetGame *pNetGame;

#define MAX_ENCRYPTED_TXD 2

const cryptor::string_encryptor encrArch[MAX_ENCRYPTED_TXD] = {
        cryptor::create("texdb/gta3/gta3.txt", 21),
        cryptor::create("texdb/gta_int/gta_int.txt", 27),
};

bool isEncrypted(const char* szArch)
{
    //return false;
    for (int64 i = 0; i < MAX_ENCRYPTED_TXD; i++)
    {
        if (!strcmp(encrArch[i].decrypt(), szArch))
            return true;
    }

    return false;
}
void InitCTX(AES_ctx& ctx, const uint8_t* pKey)
{
    uint8_t key[16];
    memcpy(&key[0], &pKey[0], 16);
    for (int i = 0; i < 16; i++)
    {
        key[i] = XOR_UNOBFUSCATE(key[i]);
    }
    uint8_t iv[16];
    memcpy(&iv[0], &g_iIV, 16);
    for (int i = 0; i < 16; i++)
    {
        iv[i] = XOR_UNOBFUSCATE(iv[i]);
    }
    AES_init_ctx_iv(&ctx, &key[0], &iv[0]);
}

uint8_t byteInternalPlayer = 0;
CPed *dwCurPlayerActor = 0;
uint8_t byteCurPlayer = 0;
uint8_t byteCurDriver = 0;

extern "C" uintptr_t get_lib() {
    return g_libGTASA;
}

// 0.3.7
PLAYERID FindPlayerIDFromGtaPtr(CEntity *pEntity) {
    if (pEntity == nullptr) return INVALID_PLAYER_ID;

    CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
    CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();

    PLAYERID PlayerID = pPlayerPool->FindRemotePlayerIDFromGtaPtr((CPed *) pEntity);
    if (PlayerID != INVALID_PLAYER_ID) return PlayerID;

    VEHICLEID VehicleID = pVehiclePool->FindIDFromGtaPtr((CVehicle *) pEntity);
    if (VehicleID != INVALID_VEHICLE_ID) {
        for (PLAYERID i = 0; i < MAX_PLAYERS; i++) {
            CRemotePlayer *pRemotePlayer = pPlayerPool->GetAt(i);
            if (pRemotePlayer && pRemotePlayer->CurrentVehicleID() == VehicleID) {
                return i;
            }
        }
    }

    return INVALID_PLAYER_ID;
}

// 0.3.7
PLAYERID FindActorIDFromGtaPtr(CPed *pPed) {
    if (pPed) {
        return pNetGame->GetActorPool()->FindIDFromGtaPtr(pPed);
    }

    return INVALID_PLAYER_ID;
}

/* =============================================================================== */

void RenderEffects() {

    CClouds::MovingFogRender();
    //CClouds::VolumetricCloudsRender();
    //CHook::CallFunction<void>("_ZN7CClouds22VolumetricCloudsRenderEv");

    CHook::CallFunction<void>("_ZN13CBrightLights6RenderEv");

    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x5D6EC5
                                                    : 0x6FC3DC)); // _ZN18CVisibilityPlugins27RenderReallyDrawLastObjectsEv
    CHook::CallFunction<void>(
            g_libGTASA + (VER_x32 ? 0x5A23B9 : 0x6C5CD4)); // _ZN8CCoronas6RenderEv


//	RenderEffects();
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x0059DA40 + 1 : 0x6C1D6C));
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x005BE914 + 1 : 0x6E2FB4));
//    CRopes::Render();
//    CGlass::Render();
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x005A6BC8 + 1 : 0x6CA5D0));
    CVisibilityPlugins::RenderReallyDrawLastObjects();
    CCoronas::Render();

    static CCamera &TheCamera = *reinterpret_cast<CCamera *>(g_libGTASA +
                                                             (VER_x32 ? 0x00951FA8 : 0xBBA8D0));
    auto g_fx = *(uintptr_t *) (g_libGTASA + (VER_x32 ? 0x00820520 : 0xA062A8));
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x00363DF0 + 1 : 0x433F54), &g_fx,
                              TheCamera.m_pRwCamera, false);

    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x005CBBAC + 1 : 0x6F054C));
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x0059BF84 + 1 : 0x6C0268));
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x005A1C38 + 1 : 0x6C552C));
////    if (CHeli::NumberOfSearchLights || CTheScripts::NumberOfScriptSearchLights) {
////        CHeli::Pre_SearchLightCone();
////        CHeli::RenderAllHeliSearchLights();
////        CTheScripts::RenderAllSearchLights();
////        CHeli::Post_SearchLightCone();
////    }
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x005E3390 + 1 : 0x708DF0));
////    if (CReplay::Mode != MODE_PLAYBACK && !CPad::GetPad(0)->DisablePlayerControls) {
////        FindPlayerPed()->DrawTriangleForMouseRecruitPed();
////    }
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x005C0B14 + 1 : 0x6E50CC));
//    //CVehicleRecording::Render();
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x005B19D0 + 1 : 0x6D6068));
//    //CRenderer::RenderFirstPersonVehicle();
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x005B5F78 + 1 : 0x6DA2B8));

    //DebugModules::Render3D();
}

int IsLowSpecDevice(void) {
    return 1;
}

int GetMaxStorage(void) {
    int result; // r0

    if (IsLowSpecDevice())
        result = 0xF00000;
    else
        result = 0x2D00000;
    return result;
}

#include "java/CHud.h"
#include "game/core/widgets/TouchInterface.h"

void Render2dStuff() {
    if (!pGUI || !pGame || !pNetGame) {
        return;
    }

    if (CHook::CallFunction<bool>(
            g_libGTASA + (VER_x32 ? 0x001BB7F4 + 1 : 0x24EA90))) // emu_IsAltRenderTarget()
        CHook::CallFunction<void>(
                g_libGTASA + (VER_x32 ? 0x001BC20C + 1 : 0x24F5B8)); // emu_FlushAltRenderTarget()

    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, RWRSTATE(FALSE));
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, RWRSTATE(FALSE));
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATESRCBLEND, RWRSTATE(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDINVSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, RWRSTATE(rwRENDERSTATENARENDERSTATE));
    RwRenderStateSet(rwRENDERSTATECULLMODE, RWRSTATE(rwCULLMODECULLNONE));

    // uint32_t reserve = GetMaxStorage();
    //  uint32_t usedMemory = *(uint32_t*)(g_libGTASA + (VER_x32 ? 0x00679EB4 : 0x851D80));
    //  uint32_t maxMemory = *(uint32_t*)(g_libGTASA + (VER_x32 ? 0x006791EC : 0x850408)) + 0xA00000;

    // if (usedMemory > maxMemory)
    //      CStreaming::MakeSpaceFor(reserve);

    //((void(*)())(g_libGTASA + (VER_x32 ? 0x00437200 + 1: 0x0051C694)))();

    if (!Weapon::ms_bTakePhoto) {
        if (CHud::bIsShow) {
            CHud::UpdateHudInfo();

            CWidgetGta *v3 = CTouchInterface::m_pWidgets[0xA1];
            if (v3) {
                v3->m_fOriginX = CHud::radarPos.x;
                v3->m_fOriginY = CHud::radarPos.y;

                v3->m_fScaleX = CHud::radarSize;
                v3->m_fScaleY = CHud::radarSize;
            }
            // call CHud::DrawRadar
            ((void (*)()) (g_libGTASA + (VER_x32 ? 0x437B0D : 0x51CFF0)))();
        }
    }

    CHook::CallFunction<void>("_ZN4CHud4DrawEv");

    // TODO: by viski
    *(uint8_t *) (g_libGTASA + (VER_x32 ? 0x677CF4 : 0x9FF3A8)) = 0;

    ((void (*)(bool)) (g_libGTASA + (VER_x32 ? 0x002B0BD8 + 1 : 0x36FB00)))(
            false); // CTouchInterface::DrawAll

    CHook::CallFunction<void>("_Z12emu_GammaSeth", 1);

    ((void (*)(bool)) (g_libGTASA + (VER_x32 ? 0x0054BDD4 + 1 : 0x66B678)))(
            true); // CMessages::Display - gametext
    ((void (*)(bool)) (g_libGTASA + (VER_x32 ? 0x005A9120 + 1 : 0x6CCEA0)))(
            true); // CFont::RenderFontBuffer
    CHook::CallFunction<void>("_Z12emu_GammaSeth", 0);

    if (pNetGame) {
        CTextDrawPool *pTextDrawPool = pNetGame->GetTextDrawPool();
        if (pTextDrawPool) pTextDrawPool->Draw();
    }

    if (pGUI && pGame) pGUI->Render();
}

/* =============================================================================== */

typedef struct {
    CVector vecPosObject;
    CQuaternion m_qRotation;
    int32 wModelIndex;
    union {
        struct { // CFileObjectInstanceType
            uint32 m_nAreaCode: 8;
            uint32 m_bRedundantStream: 1;
            uint32 m_bDontStream: 1;
            uint32 m_bUnderwater: 1;
            uint32 m_bTunnel: 1;
            uint32 m_bTunnelTransition: 1;
            uint32 m_nReserved: 19;
        };
        uint32 m_nInstanceType;
    };
    int32 m_nLodInstanceIndex;
} stLoadObjectInstance;
VALIDATE_SIZE(stLoadObjectInstance, 0x28);

void (*CEntity_Render)(CEntity *pEntity);

int g_iLastRenderedObject;

void CEntity_Render_hook(CEntity *pEntity) {
    if (CSkyBox::GetSkyObject() &&
        CSkyBox::GetSkyObject()->m_pEntity == pEntity &&
        !CSkyBox::IsNeedRender()) {
        return;
    }
    g_iLastRenderedObject = pEntity->GetModelId();
    CEntity_Render(pEntity);
}

#include "game/custom/SkyBox.h"
#include "Radar.h"
/* =============================================================================== */

#if VER_x32
uint32_t CHudColours__GetIntColour(uint32 colour_id)
{
    return TranslateColorCodeToRGBA(colour_id);
}
#else

uint32_t CHudColours__GetIntColour(uintptr *thiz, uint8 colour_id) {
    return TranslateColorCodeToRGBA(colour_id);
}

#endif

/* =============================================================================== */
void (*AND_TouchEvent)(int type, int num, int posX, int posY);

void AND_TouchEvent_hook(int type, int num, int posX, int posY) {
    // пхаххпх
    if (CTimer::m_UserPause)
        return AND_TouchEvent(type, num, posX, posY);

    bool bRet = pGUI->OnTouchEvent(type, num, posX, posY);
    if (!bRet) return;

    ImGuiIO &io = ImGui::GetIO();

    if (bRet)
        return AND_TouchEvent(type, num, posX, posY);

    CTouchInterface::lastPosX = posX;
    CTouchInterface::lastPosY = posY;
}

/* =============================================================================== */

extern CPlayerPed *g_pCurrentFiredPed;
extern BULLET_DATA *g_pCurrentBulletData;

extern int g_iLagCompensationMode;

void SendBulletSync(CVector *vecOrigin, CVector *vecEnd, CVector *vecPos, CEntity **ppEntity) {
    BULLET_DATA bulletData;
    memset(&bulletData, 0, sizeof(BULLET_DATA));

    bulletData.vecOrigin.x = vecOrigin->x;
    bulletData.vecOrigin.y = vecOrigin->y;
    bulletData.vecOrigin.z = vecOrigin->z;

    bulletData.vecPos.x = vecPos->x;
    bulletData.vecPos.y = vecPos->y;
    bulletData.vecPos.z = vecPos->z;

    if (ppEntity) {
        static CEntity *pEntity;
        pEntity = *ppEntity;
        if (pEntity) {
            if (pEntity->m_matrix) {
                if (pNetGame->m_iLagCompensation) {
                    bulletData.vecOffset.x = vecPos->x - pEntity->m_matrix->GetPosition().x;
                    bulletData.vecOffset.y = vecPos->y - pEntity->m_matrix->GetPosition().y;
                    bulletData.vecOffset.z = vecPos->z - pEntity->m_matrix->GetPosition().z;
                } else {
                    static CMatrix mat1;
                    memset(&mat1, 0, sizeof(mat1));

                    static CMatrix mat2;
                    memset(&mat2, 0, sizeof(mat2));

                    auto entMat = pEntity->GetMatrix().ToRwMatrix();
                    RwMatrixOrthoNormalize(reinterpret_cast<RwMatrix *>(&mat2), &entMat);
                    // RwMatrixInvert
                    Invert(mat1, mat2);

                    ProjectMatrix(&bulletData.vecOffset, &mat1, vecPos);
                }
            }

            bulletData.pEntity = pEntity;
        }
    }

    pGame->FindPlayerPed()->ProcessBulletData(&bulletData);

}

extern bool g_customFire;

/* 0.3.7 */
uint32_t (*CWeapon__FireInstantHit)(CWeapon *thiz, CPed *pFiringEntity, CVector *vecOrigin,
                                    CVector *muzzlePosn, CEntity *targetEntity,
                                    CVector *target, CVector *originForDriveBy, bool arg6,
                                    bool muzzle);

uint32_t CWeapon__FireInstantHit_hook(CWeapon *thiz, CPed *pFiringEntity, CVector *vecOrigin,
                                      CVector *muzzlePosn, CEntity *targetEntity,
                                      CVector *target, CVector *originForDriveBy, bool arg6,
                                      bool muzzle) {
    if (pNetGame &&
        pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed()->m_pPed)        // CWeapon::Fire
    {
        if (pFiringEntity != GamePool_FindPlayerPed())
            return muzzle;

        if (pNetGame) {
            pNetGame->GetPlayerPool()->ApplyCollisionChecking();
        }

        if (pGame) {
            CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
            if (pPlayerPed)
                pPlayerPed->FireInstant();
        }

        if (pNetGame) {
            pNetGame->GetPlayerPool()->ResetCollisionChecking();
        }

        return muzzle;
    }

    return CWeapon__FireInstantHit(thiz, pFiringEntity, vecOrigin, muzzlePosn, targetEntity,
                                   target, originForDriveBy, arg6, muzzle);
}

bool g_bForceWorldProcessLineOfSight = false;

uint32_t (*CWeapon__ProcessLineOfSight)(CVector *vecOrigin, CVector *vecEnd, CVector *vecPos,
                                        CPed **ppEntity, CWeapon *pWeaponSlot, CPed **ppEntity2,
                                        bool b1, bool b2, bool b3, bool b4, bool b5, bool b6,
                                        bool b7);

uint32_t CWeapon__ProcessLineOfSight_hook(CVector *vecOrigin, CVector *vecEnd, CVector *vecPos,
                                          CPed **ppEntity, CWeapon *pWeaponSlot, CPed **ppEntity2,
                                          bool b1, bool b2, bool b3, bool b4, bool b5, bool b6,
                                          bool b7) {
    uintptr_t dwRetAddr = 0;
    GET_LR(dwRetAddr);

    Log(OBFUSCATE("dwRetAddr CWeapon__ProcessLineOfSight_hook 0x%llx"), dwRetAddr);
#if VER_x32
    if(dwRetAddr >= 0x005DC178 && dwRetAddr <= 0x005DD684)
        g_bForceWorldProcessLineOfSight = true;
#else
    if (dwRetAddr >= 0x701494 && dwRetAddr <= 0x702B18)
        g_bForceWorldProcessLineOfSight = true;
#endif

    return CWeapon__ProcessLineOfSight(vecOrigin, vecEnd, vecPos, ppEntity, pWeaponSlot, ppEntity2,
                                       b1, b2, b3, b4, b5, b6, b7);
}


uintptr_t
(*CWorld__ProcessLineOfSight)(CVector *vecOrigin, CVector *vecEnd, CVector *vecPos, CPed **ppEntity,
                              bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7,
                              bool b8);

uintptr_t CWorld__ProcessLineOfSight_hook(CVector *vecOrigin, CVector *vecEnd, CVector *vecPos,
                                          CPed **ppEntity, bool b1, bool b2, bool b3, bool b4,
                                          bool b5, bool b6, bool b7, bool b8) {
    uintptr_t dwRetAddr = 0;
    GET_LR(dwRetAddr);
    if (dwRetAddr == (VER_x32 ? 0x005dd0b0 + 1 : 0x70253C) || g_bForceWorldProcessLineOfSight) {
        g_bForceWorldProcessLineOfSight = false;

        CEntity *pEntity = nullptr;
        CVector vecPosPlusOffset;

        if (g_iLagCompensationMode != 2) {
            if (g_pCurrentFiredPed != pGame->FindPlayerPed()) {
                if (g_pCurrentBulletData) {
                    if (g_pCurrentBulletData->pEntity) {
                        if (!IsGameEntityArePlaceable(g_pCurrentBulletData->pEntity)) {
                            auto pMatrix = g_pCurrentBulletData->pEntity->GetMatrix();

                            if (g_iLagCompensationMode) {
                                vecPosPlusOffset.x =
                                        pMatrix.GetPosition().x + g_pCurrentBulletData->vecOffset.x;
                                vecPosPlusOffset.y =
                                        pMatrix.GetPosition().y + g_pCurrentBulletData->vecOffset.y;
                                vecPosPlusOffset.z =
                                        pMatrix.GetPosition().z + g_pCurrentBulletData->vecOffset.z;
                            } else
                                ProjectMatrix(&vecPosPlusOffset, &pMatrix,
                                              &g_pCurrentBulletData->vecOffset);

                            vecEnd->x = vecPosPlusOffset.x - vecOrigin->x + vecPosPlusOffset.x;
                            vecEnd->y = vecPosPlusOffset.y - vecOrigin->y + vecPosPlusOffset.y;
                            vecEnd->z = vecPosPlusOffset.z - vecOrigin->z + vecPosPlusOffset.z;

                        }
                    }
                }
            }
        }

        uint32_t result = 0;
        result = CWorld__ProcessLineOfSight(vecOrigin, vecEnd, vecPos, ppEntity, b1, b2, b3, b4, b5,
                                            b6, b7, b8);

        if (g_iLagCompensationMode == 2) {
            if (g_pCurrentFiredPed) {
                if (g_pCurrentFiredPed == pGame->FindPlayerPed())
                    SendBulletSync(vecOrigin, vecEnd, vecPos, (CEntity **) ppEntity);
            }

            return result;
        }

        if (g_pCurrentFiredPed) {
            if (g_pCurrentFiredPed != pGame->FindPlayerPed()) {
                if (g_pCurrentBulletData) {
                    if (!g_pCurrentBulletData->pEntity) {
                        CPed *pLocalPed = GamePool_FindPlayerPed();
                        if (*ppEntity == pLocalPed ||
                            (CVehicle *) ppEntity == (CVehicle *) pLocalPed->pVehicle) {
                            *ppEntity = nullptr;
                            vecPos->x = 0.0f;
                            vecPos->y = 0.0f;
                            vecPos->z = 0.0f;
                            return 0;
                        }
                    }
                }
            }
        }

        if (g_pCurrentFiredPed) {
            if (g_pCurrentFiredPed == pGame->FindPlayerPed())
                SendBulletSync(vecOrigin, vecEnd, vecPos, (CEntity **) ppEntity);
        }

        return result;
    }

    return CWorld__ProcessLineOfSight(vecOrigin, vecEnd, vecPos, ppEntity, b1, b2, b3, b4, b5, b6,
                                      b7, b8);
}

// реверснуть с лр
uint32_t
(*CWeapon__FireSniper)(CWeapon *thiz, CPed *pFiringEntity, CEntity *victim, CVector *target);

uint32_t
CWeapon__FireSniper_hook(CWeapon *thiz, CPed *pFiringEntity, CEntity *victim, CVector *target) {
    if (pFiringEntity == GamePool_FindPlayerPed()) {
        if (pGame) {
            CPlayerPed *pPlayerPed = pGame->FindPlayerPed();
            if (pPlayerPed) {
                pPlayerPed->FireInstant();
            }
        }
    }

    return true;
}

#pragma pack(push, 1)
struct stPedDamageResponse {
    CEntity *pEntity;
    float fDamage;
    int iBodyPart;
    int iWeaponType;
    bool bSpeak;
};
#pragma pack(pop)
extern float m_fWeaponDamages[43 + 1];

void onDamage(CPed *issuer, CPed *damaged) {
    if (!pNetGame) return;
    CPed *pPedPlayer = GamePool_FindPlayerPed();
    if (damaged && (pPedPlayer == issuer)) {
        if (pNetGame->GetPlayerPool()->FindRemotePlayerIDFromGtaPtr((CPed *) damaged) !=
            INVALID_PLAYER_ID) {
            CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
            CAMERA_AIM *caAim = pPlayerPool->GetLocalPlayer()->GetPlayerPed()->GetCurrentAim();

            CVector aim;
            aim.x = caAim->f1x;
            aim.y = caAim->f1y;
            aim.z = caAim->f1z;

            pPlayerPool->GetLocalPlayer()->SendBulletSyncData(
                    pPlayerPool->FindRemotePlayerIDFromGtaPtr((CPed *) damaged),
                    BULLET_HIT_TYPE_PLAYER, aim);
        }
    }
}

//void (*CPedDamageResponseCalculator__ComputeDamageResponse)(CPedDamageResponseCalculator* thiz, CPed* pPed, uintptr_t* a3, uint32_t a4);
//void CPedDamageResponseCalculator__ComputeDamageResponse_hook(CPedDamageResponseCalculator* thiz, CPed* pPed, uintptr_t *a3, uint32_t a4)
void
(*CPedDamageResponseCalculator__ComputeDamageResponse)(stPedDamageResponse *thiz, CEntity *pEntity,
                                                       uintptr_t pDamageResponse, bool bSpeak);

void CPedDamageResponseCalculator__ComputeDamageResponse_hook(stPedDamageResponse *thiz,
                                                              CEntity *pEntity,
                                                              uintptr_t pDamageResponse,
                                                              bool bSpeak) {
    if (thiz && pEntity) onDamage((CPed *) *(uintptr_t *) thiz, (CPed *) pEntity);
    int weaponid = thiz->iWeaponType;
    float fDamage;
    Log("%d", fDamage);
    if (weaponid < 0 || weaponid >= static_cast<int>(std::size(m_fWeaponDamages))) {
        fDamage = thiz->fDamage;
    } else {
        fDamage = m_fWeaponDamages[weaponid];
    }

    int bodypart = thiz->iBodyPart;

    if (pNetGame) {
        CPlayerPool *pPlayerPool = pNetGame->GetPlayerPool();
        if (pPlayerPool) {
            if (weaponid < 0 || weaponid > 255 || (weaponid > 54 && weaponid < 200) ||
                (weaponid > 201 && weaponid < 255))
                weaponid = 255; // suicide
            else if (weaponid == 18)
                weaponid = 37; // flamethower
            else if (weaponid == 35 || weaponid == 16)
                weaponid = 51; // explosion

            PLAYERID damagedid = pPlayerPool->FindRemotePlayerIDFromGtaPtr(
                    (CPed *) thiz->pEntity); // отправитель урона
            PLAYERID issuerid = pPlayerPool->FindRemotePlayerIDFromGtaPtr(
                    (CPed *) pEntity); // получатель

            PLAYERID byteLocalId = pPlayerPool->GetLocalPlayerID();

            // give player damage
            if ((CPed *) thiz->pEntity == pGame->FindPlayerPed()->m_pPed &&
                issuerid != INVALID_PLAYER_ID) {
                //CHUD::addGiveDamageNotify(issuerid, weaponid, fDamage);
                pPlayerPool->GetLocalPlayer()->GiveTakeDamage(false, issuerid, fDamage, weaponid,
                                                              bodypart);
                Log(OBFUSCATE("GiveDamage Name: %s, weaponid: %d, damage: %f"),
                    pPlayerPool->GetPlayerName(issuerid), weaponid, fDamage);
            }

                // player take damage
            else if (damagedid != INVALID_PLAYER_ID && issuerid == INVALID_PLAYER_ID) {
                pPlayerPool->GetLocalPlayer()->GiveTakeDamage(true, damagedid, fDamage, weaponid,
                                                              bodypart);

                char nick[MAX_PLAYER_NAME];
                strcpy(nick, pPlayerPool->GetPlayerName(damagedid));

                Log(OBFUSCATE("TakeDamage %s, %d, %f"), pPlayerPool->GetPlayerName(damagedid),
                    weaponid, fDamage);

                //CHUD::addTakeDamageNotify(pPlayerPool->GetPlayerName(damagedid), weaponid, fDamage);
            }
            if ((CPed *) thiz->pEntity == pGame->FindPlayerPed()->m_pPed) {
                CHud::UpdateHudInfo();
            }


            Log(OBFUSCATE("%d, %f, %d, %d"), damagedid, fDamage, weaponid, bodypart);
        }
    }

    CPedDamageResponseCalculator__ComputeDamageResponse(thiz, pEntity, pDamageResponse, bSpeak);
/*

	if (thiz == nullptr || pPed == nullptr || a3 == nullptr) return;

    if (ComputeDamageResponse(thiz, pPed))
        return;

	CPedDamageResponseCalculator__ComputeDamageResponse(thiz, pPed, a3, a4);*/
}

int (*CTextureDatabaseRuntime__GetEntry)(uintptr_t thiz, const char *a2, bool *a3);

int CTextureDatabaseRuntime__GetEntry_hook(uintptr_t thiz, const char *a2, bool *a3) {
    int result;

    if (thiz)
        result = CTextureDatabaseRuntime__GetEntry(thiz, a2, a3);
    else
        result = -1;

    return result;
}

extern CJavaWrapper *pJavaWrapper;

void (*MainMenuScreen__OnExit)();

void MainMenuScreen__OnExit_hook() {
    pGame->bIsGameExiting = true;

    pNetGame->GetRakClient()->Disconnect(0);

    pJavaWrapper->exitGame();
}

/* =============================================================================== */

void ReadSettingFile();

void (*NvUtilInit)();

void NvUtilInit_hook() {
    Log(OBFUSCATE("NvUtilInit"));

    NvUtilInit();

    g_pszStorage = (char *) (g_libGTASA + (VER_x32 ? 0x6D687C : 0x8B46A8)); // StorageRootBuffer
    ReadSettingFile();
}

struct stFile {
    int isFileExist;
    FILE *f;
};

char lastFile[123];

stFile *NvFOpen(const char *r0, const char *r1, int r2, int r3) {
    strcpy(lastFile, r1);

    static char path[255]{};
    memset(path, 0, sizeof(path));

    sprintf(path, "%s%s", g_pszStorage, r1);

    // ----------------------------
    if (!strncmp(r1 + 12, "mainV1.scm", 10)) {
        sprintf(path, "%sSAMP/main.scm", g_pszStorage);
        Log("Loading %s", path);
    }
    // ----------------------------
    if (!strncmp(r1 + 12, "SCRIPTV1.IMG", 12)) {
        sprintf(path, "%sSAMP/script.img", g_pszStorage);
        Log("Loading script.img..");
    }
    // ----------------------------
    if (!strncmp(r1, "DATA/PEDS.IDE", 13)) {
        sprintf(path, "%sSAMP/peds.ide", g_pszStorage);
        Log("Loading peds.ide..");
    }
    // ----------------------------
    if (!strncmp(r1, "DATA/VEHICLES.IDE", 17)) {
        sprintf(path, "%sSAMP/vehicles.ide", g_pszStorage);
        Log("Loading vehicles.ide..");
    }

    if (!strncmp(r1, "DATA/GTA.DAT", 12)) {
        sprintf(path, "%sSAMP/gta.dat", g_pszStorage);
        Log("Loading gta.dat..");
    }

#if VER_x32
    auto *st = (stFile*)malloc(8);
#else
    auto *st = (stFile *) malloc(0x10);
#endif
    st->isFileExist = false;

    FILE *f = fopen(path, "rb");

    if (f) {
        st->isFileExist = true;
        st->f = f;
        return st;
    } else {
        Log("NVFOpen hook | Error: file not found (%s)", path);
        free(st);
        return nullptr;
    }
}

bool g_bPlaySAMP = false;

void (*MainMenuScreen__Update)(uintptr_t thiz, float deltaTime);

void MainMenuScreen__Update_hook(uintptr_t thiz, float deltaTime) {
    MainMenuScreen__Update(thiz, deltaTime);

    if (g_bPlaySAMP) return;

    pGame->StartGame();

    // StartGameScreen::OnNewGameCheck()
    ((void (*)()) (g_libGTASA + (VER_x32 ? 0x002A7270 + 1 : 0x365EA0)))();
    //*(uint32_t*)(g_libGTASA + 0x009E75B8) = 8;

    g_bPlaySAMP = true;
}

void (*StartGameScreen__OnNewGameCheck)();

void StartGameScreen__OnNewGameCheck_hook() {
    if (g_bPlaySAMP)
        return;

    StartGameScreen__OnNewGameCheck();
}

int
(*CCollision__ProcessVerticalLine)(float *a1, float *a2, int a3, int a4, int *a5, int a6, int a7,
                                   int a8);

int
CCollision__ProcessVerticalLine_hook(float *a1, float *a2, int a3, int a4, int *a5, int a6, int a7,
                                     int a8) {
    int result; // x0

    if (a3)
        result = CCollision__ProcessVerticalLine(a1, a2, a3, a4, a5, a6, a7, a8);
    else
        result = 0;
    return result;
}

int (*CUpsideDownCarCheck__IsCarUpsideDown)(int, int);

int CUpsideDownCarCheck__IsCarUpsideDown_hook(int a1, int a2) {
    return 0;
}

int64 getmip() {
    return 1;
}

uint64_t *RQCommand_rqSetAlphaTest(uint64_t *result) {
    *result += 8;
    return result;
}

int64 GetInputType(void) {
    return 0LL;
}

void (*FxEmitterBP_c__Render)(uintptr_t *a1, int a2, int a3, float a4, char a5);

void FxEmitterBP_c__Render_hook(uintptr_t *a1, int a2, int a3, float a4, char a5) {
    if (!a1 || !a2) return;
    uintptr_t *temp = *((uintptr_t **) a1 + 3);
    if (!temp) {
        return;
    }
    FxEmitterBP_c__Render(a1, a2, a3, a4, a5);
}

bool (*RwResourcesFreeResEntry)(void *entry);

bool RwResourcesFreeResEntry_hook(void *entry) {
    bool result;
    if (entry) result = RwResourcesFreeResEntry(entry);
    else result = false;
    return result;
}

static uint32_t dwRLEDecompressSourceSize = 0;
OSFile lastOpenedFile;

size_t (*OS_FileOpen)(uintptr_t a1, OSFile *a2, const char *a3, int a4);
size_t OS_FileOpen_hook(uintptr_t a1, OSFile *a2, const char *a3, int a4)
{
    size_t retn = OS_FileOpen(a1, a2, a3, a4);

    if (isEncrypted(a3))
    {
        lastOpenedFile = *a2;
    }
  //  Log("%s%s", g_pszStorage, a3);
    return retn;
}

size_t (*OS_FileRead)(FILE *file, void *buffer, size_t numBytes);
size_t OS_FileRead_hook(FILE *file, void *buffer, size_t numBytes)
{

    uintptr_t calledFrom = 0;

    GET_LR(calledFrom);
    dwRLEDecompressSourceSize = numBytes;
    if(!numBytes)
        return 0;

    if (calledFrom == (VER_x32 ? 0x1E8142 + 1 : 0x2842B0) && file == (void*)lastOpenedFile)
    {
        lastOpenedFile = 0;

        AES_ctx ctx;
        InitCTX(ctx, &g_iEncryptionKeyTXD[0]);

        size_t retv = OS_FileRead(file, buffer, numBytes);
        int fileSize = numBytes;
        int iters = fileSize / PART_SIZE_TXD;
        uintptr_t pointer = (uintptr_t)buffer;
        for (int i = 0; i < iters; i++)
        {
            AES_CBC_decrypt_buffer(&ctx, (uint8_t *)pointer, PART_SIZE_TXD);
            pointer += PART_SIZE_TXD;
        }
        return retv;
    }
    return OS_FileRead(file, buffer, numBytes);
}

void (*RLEDecompress)(uint8_t *pDest, size_t uiDestSize, uint8_t const *pSrc, size_t uiSegSize,
                      uint32_t uiEscape);

void RLEDecompress_hook(uint8_t *pDest, size_t uiDestSize, const uint8_t *pSrc, size_t uiSegSize,
                        uint32_t uiEscape) {
    if (!pDest || !pSrc || uiDestSize == 0 || uiSegSize == 0)
        return;

    const uint8_t *pTempSrc = pSrc;
    const uint8_t *const pEndOfDest = pDest + uiDestSize;
    const uint8_t *const pEndOfSrc = pSrc + dwRLEDecompressSourceSize;

    try {
        while (pDest < pEndOfDest && pTempSrc < pEndOfSrc) {
            if (*pTempSrc == uiEscape) {
                if (pTempSrc + 1 >= pEndOfSrc || pTempSrc[1] == 0 ||
                    pTempSrc + 2 + uiSegSize > pEndOfSrc)
                    throw std::runtime_error(OBFUSCATE("rled error 1"));

                uint8_t ucCurSeg = pTempSrc[1];
                while (ucCurSeg--) {
                    if (pDest + uiSegSize > pEndOfDest)
                        throw std::runtime_error(OBFUSCATE("rled error 2"));
                    memcpy(pDest, pTempSrc + 2, uiSegSize);
                    pDest += uiSegSize;
                }
                pTempSrc += 2 + uiSegSize;
            } else {
                if (pDest + uiSegSize > pEndOfDest || pTempSrc + uiSegSize > pEndOfSrc)
                    throw std::runtime_error(OBFUSCATE("rled error 3"));
                memcpy(pDest, pTempSrc, uiSegSize);
                pDest += uiSegSize;
                pTempSrc += uiSegSize;
            }
        }

        dwRLEDecompressSourceSize = 0;
    } catch (const std::exception &e) {
        Log(OBFUSCATE("%s"), e.what());
    }
}

float (*CDraw__SetFOV)(float fFOV, bool isCinematic);

float CDraw__SetFOV_hook(float fFOV, bool isCinematic) {
    float aspectRatio = *(float *) (g_libGTASA + (VER_x32 ? 0x00A26A90 : 0xCC7F00));
    float result = ((aspectRatio - 1.3333f) * 11.0f) / 0.44444f + fFOV;
    *(float *) (g_libGTASA + (VER_x32 ? 0x006B1CB8 : 0x88E6BC)) = result;
    return result;
}

void (*CCam__Process)(uintptr_t);

void CCam__Process_hook(uintptr_t thiz) {
    static CCamera &TheCamera = *reinterpret_cast<CCamera *>(g_libGTASA +
                                                             (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

    if (!CFirstPersonCamera::IsEnabled()) {
        CCam__Process(thiz);
        return;
    }
    CVector vecSpeed;

    CPlayerPed *pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
    auto m_CurrentVehicle = pNetGame->GetVehiclePool()->FindIDFromGtaPtr(pPed->GetGtaVehicle());
    auto pVehicle = pNetGame->GetVehiclePool()->GetAt(m_CurrentVehicle);

    float v6 = *(float *) (g_libGTASA + (VER_x32 ? 0x6A9FD0 : 0x8855D4));
    if (pNetGame && (*(uint16_t *) (thiz + 14) == 18 || *(uint16_t *) (thiz + 14) == 16)) {
        if (pNetGame->GetPlayerPool()) {
            if (pNetGame->GetPlayerPool()->GetLocalPlayer()) {
                if (pVehicle) {
                    pVehicle->m_pVehicle->GetMoveSpeedVector(&vecSpeed);
                    CVector vec;
                    vec.x = vecSpeed.x * 6.0f;
                    vec.y = vecSpeed.y * 6.0f;
                    vec.z = vecSpeed.z * 6.0f;
                    pVehicle->m_pVehicle->SetVelocity(vec);
                    *(float *) (g_libGTASA + (VER_x32 ? 0x6A9FD0 : 0x8855D4)) = 200.0f;
                }
            }
        }
    }
    *(float *) (g_libGTASA + (VER_x32 ? 0x6A9FD0 : 0x8855D4)) = 200.0f;
    CCam__Process(thiz);

    if (pVehicle) {
        pVehicle->m_pVehicle->SetVelocity(vecSpeed);
        *(float *) (g_libGTASA + (VER_x32 ? 0x6A9FD0 : 0x8855D4)) = v6;
    }
    if (*(uint16_t *) (thiz + 14) == 4 || *(uint16_t *) (thiz + 14) == 53) // 53 is weapon
    {
        if (pNetGame) {
            if (pNetGame->GetPlayerPool()) {
                if (pNetGame->GetPlayerPool()->GetLocalPlayer()) {
                    CPlayerPed *pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
                    if (pPed) {
#if VER_x32
                        *(uint32_t*)(g_libGTASA + 0x00951FA8 + 120) = 0xFFFFFFFF;
                        *(uint32_t*)(g_libGTASA + 0x00951FA8 + 124) = 0xFFFFFFFF;
                        *(uint8_t*)(g_libGTASA + 0x00951FA8 + 40) = 0;
#else
                        *(uint32_t *) (g_libGTASA + 0xBBA8D0 + 128) = 0xFFFFFFFFFFFFFFFFLL;
                        *(uint8_t *) (g_libGTASA + 0xBBA8D0 + 48) = 0;
#endif
                        CFirstPersonCamera::ProcessCameraOnFoot(thiz, pPed);
                        // weapon povorot??!
                        //  float v11 = atan2f(*(float *)(g_libGTASA + 0xBBA8D0 + 740), *(float *)(g_libGTASA + 0xBBA8D0 + 736));
                        // int v8 = *(uint16_t *)(thiz + 14);
                        //  float v12 = v11 + -1.57079633;
                        //  *(float *)(v10 + 1736) = v12;
                    }
                }
            }
        }
    }
    if (*(uint16_t *) (thiz + 14) == 18 || *(uint16_t *) (thiz + 14) == 16) // in car
    {
        if (pNetGame) {
            if (pNetGame->GetPlayerPool()) {
                if (pNetGame->GetPlayerPool()->GetLocalPlayer()) {
                    CPlayerPed *pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
                    if (pPed) {
#if VER_x32
                        *(uint32_t*)(g_libGTASA + 0x00951FA8 + 120) = 0xFFFFFFFF;
                        *(uint32_t*)(g_libGTASA + 0x00951FA8 + 124) = 0xFFFFFFFF;
                        *(uint8_t*)(g_libGTASA + 0x00951FA8 + 40) = 0;
#else
                        *(uint32_t *) (g_libGTASA + 0xBBA8D0 + 128) = 0xFFFFFFFFFFFFFFFFLL;
                        *(uint8_t *) (g_libGTASA + 0xBBA8D0 + 48) = 0;
#endif

                        CFirstPersonCamera::ProcessCameraInVeh(thiz, pPed, pVehicle);
                    }
                }
            }
        }
    }
}

void (*CWeapon__Fire)(CWeapon, CEntity *, CVector *, CVector *, CEntity *, CVector *, CVector *);

void
CWeapon__Fire_hook(CWeapon thiz, CEntity *a1, CVector *a2, CVector *a3, CEntity *a4, CVector *a5,
                   CVector *a6) {
    if (pNetGame) {
        CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
        if (pLocalPlayer) {
            CHud::UpdateAmmo();
            pLocalPlayer->ammoUpdated = 1;
        }
    }
    CWeapon__Fire(thiz, a1, a2, a3, a4, a5, a6);
}

void (*CPed__SetCurrentWeapon)(CPed *, int);

void CPed__SetCurrentWeapon_hook(CPed *pPed, int weaponType) {
    CHud::UpdateAmmo();
    CPed__SetCurrentWeapon(pPed, weaponType);
}

extern bool bDisableVehicleCollision;

uint16_t g_usLastProcessedModelIndexAutomobile = 0;
int g_iLastProcessedModelIndexAutoEnt = 0;

int32 (*CAutomobile__ProcessEntityCollision)(CVehicle *thiz, CEntity *ent, CColPoint *aColPoints);

int32
CAutomobile__ProcessEntityCollision_hook(CVehicle *thiz, CEntity *ent, CColPoint *aColPoints) {

    g_usLastProcessedModelIndexAutomobile = thiz->m_nModelIndex;
    g_iLastProcessedModelIndexAutoEnt = thiz->m_nModelIndex;
/*
    if (bDisableVehicleCollision) {
        if (ent->IsVehicle() || ent->m_nType == ENTITY_TYPE_PED) {
            return 0;
        }
    }

    bool bReplace = false;
    CColLine *pOld = nullptr;
    CCollisionData *pColData = nullptr;

    if (pNetGame) {
        CVehiclePool *pVehiclePool = pNetGame->GetVehiclePool();
        if(!pVehiclePool)
            Log("pizda nax");

        auto pVeh = pNetGame->GetVehiclePool()->GetAt(pVehiclePool->FindIDFromGtaPtr(pGame->FindPlayerPed()->GetGtaVehicle()));

        if (pVeh) {
            if (pVeh->bHasSuspensionLines && pVeh->GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR) {
                pColData = CModelInfo::GetModelInfo(
                        thiz->m_nModelIndex)->AsVehicleModelInfoPtr()->m_pColModel->m_pColData;
                if (pColData && pVeh->m_pSuspensionLines) {
                    if (pColData->m_pLines) {
                        pOld = pColData->m_pLines;
                        pColData->m_pLines = pVeh->m_pSuspensionLines;
                        bReplace = true;
                    }
                }
            }
        }
    }*/
    auto result = CAutomobile__ProcessEntityCollision(thiz, ent, aColPoints);

    //  if (bReplace) {
    //      pColData->m_pLines = pOld;
    //  }

    return result;
}

static CVehicle *g_pLastProcessedVehicleMatrix = nullptr;
static int g_iLastProcessedWheelVehicle = -1;

void (*CMatrix__Rotate)(void *thiz, float a1, float a2, float a3);

void CMatrix__Rotate_hook(void *thiz, float a1, float a2, float a3) {
    uintptr_t dwRetAddr = 0;
    GET_LR(dwRetAddr);

    if (dwRetAddr == 0x003A9D76 + 1) {
        CMatrix__Rotate(thiz, a1, a2, a3);
        return;
    }

    CMatrix__Rotate(thiz, a1, a2, a3);
    if (g_pLastProcessedVehicleMatrix && g_iLastProcessedWheelVehicle != -1) {
        if (g_pLastProcessedVehicleMatrix->m_bWheelAlignmentX ||
            g_pLastProcessedVehicleMatrix->m_bWheelAlignmentY) {
            if (g_iLastProcessedWheelVehicle == 2) {
                ((void (*)(void *, float)) (g_libGTASA + 0x003E8BE4 + 1))(thiz, 0.0f -
                                                                                g_pLastProcessedVehicleMatrix->m_fWheelAlignmentX); // CMatrix::RotateY
            }
            if (g_iLastProcessedWheelVehicle == 4) {
                ((void (*)(void *, float)) (g_libGTASA + 0x003E8BE4 + 1))(thiz, 0.0f -
                                                                                g_pLastProcessedVehicleMatrix->m_fWheelAlignmentY); // CMatrix::RotateY
            }
            if (g_iLastProcessedWheelVehicle == 5) {
                ((void (*)(void *, float)) (g_libGTASA + 0x003E8BE4 + 1))(thiz,
                                                                          g_pLastProcessedVehicleMatrix->m_fWheelAlignmentX); // CMatrix::RotateY
            }
            if (g_iLastProcessedWheelVehicle == 7) {
                ((void (*)(void *, float)) (g_libGTASA + 0x003E8BE4 + 1))(thiz,
                                                                          g_pLastProcessedVehicleMatrix->m_fWheelAlignmentY); // CMatrix::RotateY
            }
        }
    }
}

void (*CMatrix__SetScale)(void *thiz, float x, float y, float z);

void CMatrix__SetScale_hook(void *thiz, float x, float y, float z) {
    if (g_pLastProcessedVehicleMatrix && g_iLastProcessedWheelVehicle != -1) {
        if (g_iLastProcessedWheelVehicle >= 2 || g_iLastProcessedWheelVehicle <= 7) {
            // front wheel
            if (g_pLastProcessedVehicleMatrix->m_bWheelSize) {
                y *= g_pLastProcessedVehicleMatrix->m_fWheelSize * 1.3f;
                z *= g_pLastProcessedVehicleMatrix->m_fWheelSize * 1.3f;
            }
            if (g_pLastProcessedVehicleMatrix->m_bWheelWidth) {
                x = g_pLastProcessedVehicleMatrix->m_fWheelWidth;
            }
        }
    }

    CMatrix__SetScale(thiz, x, y, z);
}

void (*CAutomobile__UpdateWheelMatrix)(CVehicle *thiz, int, int);

void CAutomobile__UpdateWheelMatrix_hook(CVehicle *thiz, int nodeIndex, int flags) {
    if (g_pLastProcessedVehicleMatrix) {
        g_iLastProcessedWheelVehicle = nodeIndex;
    }

    CAutomobile__UpdateWheelMatrix(thiz, nodeIndex, flags);
}

int g_iLastProcessedSkinCollision = 228;
int g_iLastProcessedEntityCollision = 228;

int32_t (*CPed__ProcessEntityCollision)(CPed *thiz, CEntity *pEntity, CColPoint *aColPoints);

int32_t CPed__ProcessEntityCollision_hook(CPed *thiz, CEntity *pEntity, CColPoint *aColPoints) {
    g_iLastProcessedSkinCollision = thiz->m_nModelIndex;
    g_iLastProcessedEntityCollision = pEntity->m_nModelIndex;

    return CPed__ProcessEntityCollision(thiz, pEntity, aColPoints);
}

int g_iCounterVehicleCamera = 0;

int (*CPad__CycleCameraModeDownJustDown)(void *);

int CPad__CycleCameraModeDownJustDown_hook(void *thiz) {
    // if (!g_pWidgetManager)
    // {
    return 0;
    //}

    CPed *pPed = GamePool_FindPlayerPed();
    if (!pPed) {
        return 0;
    }

    static uint32_t lastTick = GetTickCount();
    bool bPressed = false;
    //if (CHUD::bIsTouchCameraButt && GetTickCount() - lastTick >= 250)
    // {
    //    CHUD::bIsTouchCameraButt = false;
    //   bPressed = true;
    //   lastTick = GetTickCount();
    //}

    if (pPed->bInVehicle) {
        if (bPressed) {
            g_iCounterVehicleCamera++;
        }
        if (g_iCounterVehicleCamera == 6) {
            CFirstPersonCamera::SetEnabled(true);
            return 0;
        } else if (g_iCounterVehicleCamera >= 7) {
            g_iCounterVehicleCamera = 0;
            CFirstPersonCamera::SetEnabled(false);
            return 1;

        } else {
            CFirstPersonCamera::SetEnabled(false);
        }

        return bPressed;
    }
    return 0;
}

bool g_bWeaponHudUpdated = false;

CHud *(*CWeapon__Update)(CWeapon *pWeapon, CPed *pPed);

CHud *CWeapon__Update_hook(CWeapon *pWeapon, CPed *pPed) {
    CHud *result = (CHud *) CWeapon__Update(pWeapon, pPed);

    if (!pNetGame) {return result;}

    CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
    if (!pLocalPlayer) return result;

    CPed *pLocalPed = reinterpret_cast<CPed *>(pLocalPlayer->GetPlayerPed());
    if (pLocalPed == pPed) {
        int weaponState = *((int *) pWeapon + 1);

        if (weaponState != 0) {
            if (weaponState != 2 || (g_bWeaponHudUpdated & 1) != 0)
                return result;

            g_bWeaponHudUpdated = 1;
        } else {
            if (g_bWeaponHudUpdated != 1)
                return result;

            //result = CHud::UpdateAmmo();
            CHud::UpdateAmmo();
            g_bWeaponHudUpdated = 0;
        }
    }

    return result;
}

CHud *(*CPlayerPed__MakeChangesForNewWeapon)(CPed *pPed);

CHud *CPlayerPed__MakeChangesForNewWeapon_hook(CPed *pPed) {
    CLocalPlayer *pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
    CPed *pLocalPed = reinterpret_cast<CPed *>(pLocalPlayer->GetPlayerPed());
    if (pLocalPed == pPed) {
        CHud::UpdateAmmo();
    }

    return CPlayerPed__MakeChangesForNewWeapon(pPed);
}

uint64_t (*CEventKnockOffBike__AffectsPed)();

uint64_t CEventKnockOffBike__AffectsPed_hook() {
    return 0;
}

void (*CAutomobile__PreRender)(CVehicle *thiz);

void CAutomobile__PreRender_hook(CVehicle *thiz) {
    //if (!pNetGame) {CAutomobile__PreRender(thiz);}

    CVehicleModelInfo *pModelInfoStart = static_cast<CVehicleModelInfo *>(CModelInfo::ms_modelInfoPtrs[thiz->m_nModelIndex]);

    if (pNetGame->GetVehiclePool()) {
        uint16_t vehid = pNetGame->GetVehiclePool()->FindIDFromGtaPtr(thiz);
        auto pVeh = pNetGame->GetVehiclePool()->GetAt(vehid);
        if (pVeh) {
            // синий
            pVeh->neonColor.r = static_cast<uint8_t>(0);
            pVeh->neonColor.g = static_cast<uint8_t>(0);
            pVeh->neonColor.b = static_cast<uint8_t>(255);
            pVeh->neonType = static_cast<CVehicle::eNeonTypes>(1);
            pVeh->m_bShadow = true;
            pVeh->NeonRender();
        }
    }
    //unsigned char orangeNeonData[] = {255, 165, 0, 1};
    // CVehicleNeon::Render(reinterpret_cast<const char*>(orangeNeonData), thiz);

    CAutomobile__PreRender(thiz);
}

CFPSFix g_fps;

void (*ANDRunThread)(void *a1);

void ANDRunThread_hook(void *a1) {
    g_fps.PushThread(gettid());

    ANDRunThread(a1);
}

void (*CStreaming__Init2)();

void CStreaming__Init2_hook() {
    CStreaming__Init2();
    *(uint32_t *) (g_libGTASA + (VER_x32 ? 0x00685FA0 : 0x85EBD8)) = 536870912;
}

bool (*CGame__Init2)(const uint8_t *);

bool CGame__Init2_hook(const uint8_t *pDatFile) {
    CGame__Init2(pDatFile);
    //CDraw__SetFOV_hook(120.0f, false);
    *(float *) (g_libGTASA + 0xCC7EFC) = 500.0f;
}

void InjectHooks() {
    Log("InjectHooks");
    CHook::Write(g_libGTASA + (VER_x32 ? 0x678954 : 0x84F2D0), &Scene);

#if !VER_x32 // mb all.. wtf crash x64?
    CHook::RET("_ZN11CPlayerInfo14LoadPlayerSkinEv");
    CHook::RET("_ZN11CPopulation10InitialiseEv");
#endif
    CCustomCarEnvMapPipeline::InjectHooks();
    CCamera::InjectHooks(); //
    CReferences::InjectHooks(); //
    CModelInfo::injectHooks(); //
    CTimer::InjectHooks(); //
    //cTransmission::InjectHooks(); //
    CAnimBlendAssociation::InjectHooks(); //
    //cHandlingDataMgr::InjectHooks(); //
    CPools::InjectHooks(); //
    CVehicle::InjectHooks(); //
    CMatrixLink::InjectHooks(); //
    CMatrixLinkList::InjectHooks(); //
    CStreaming::InjectHooks();
    CPlaceable::InjectHooks(); //
    CMatrix::InjectHooks(); //
    CCollision::InjectHooks(); //
    //CIdleCam::InjectHooks(); //
    CTouchInterface::InjectHooks(); //
    CWidgetGta::InjectHooks();
    CEntity::InjectHooks(); //
    CPhysical::InjectHooks(); //
    CAnimManager::InjectHooks(); //
    CCarEnterExit::InjectHooks();
    CPlayerPedGta::InjectHooks(); //
    CTaskManager::InjectHooks(); //
    CPedIntelligence::InjectHooks(); //
    CWorld::InjectHooks(); //
    CGame::InjectHooks();
    ES2VertexBuffer::InjectHooks();
    CRQ_Commands::InjectHooks();
    CTxdStore::InjectHooks();
    CVisibilityPlugins::InjectHooks();
    //CAdjustableHUD::InjectHooks();

    // new
    CClouds::InjectHooks();
    CWeather::InjectHooks();
    //RenderBuffer::InjectHooks();
    CTimeCycle::InjectHooks();
    CCoronas::InjectHooks();
    //CDraw::InjectHooks();
    CClock::InjectHooks();
    //CBirds::Init();
    CVehicleModelInfo::InjectHooks();
    CPathFind::InjectHooks();
    CSprite2d::InjectHooks();
    CFileLoader::InjectHooks();
    CShadows::InjectHooks();
    CPickups::InjectHooks();
    CRenderer::InjectHooks();
    CStreamingInfo::InjectHooks();
    TextureDatabase::InjectHooks();
    TextureDatabaseEntry::InjectHooks();
    TextureDatabaseRuntime::InjectHooks();
    CCustomBuildingDNPipeline::InjectHooks();

    RQShader::InjectHooks();
    CPauseMenu::InjectHooks();
  //  CAudioSystem::injectHooks();
    Weapon::InjectHooks();
    GPS::injectHooks();
    CWidgetRadar::InjectHooks();
    CSprite::InjectHooks();
    Fx_c::InjectHooks();
    CColourSet::InjectHooks();
    MobileMenu::InjectHooks();
    CRadar::InjectHooks();
    CBridge::InjectHooks();
    MainMenuScreen::InjectHooks();
    CMessages::InjectHooks();

    //CRealTimeShadowManager::InjectHooks();
}

#include "../renderer/WaterShader.h"

uintptr_t *g_WaterShaderClass = nullptr;

void (*emu_glEndInternal)();
void emu_glEndInternal_hook() {
    if (((*(uintptr_t *) (g_libGTASA + (VER_x32 ? 0x006B7094 : 0x8944A8))) & 0x80000) != 0) {
        if (g_WaterShaderClass == nullptr) {
            g_WaterShaderClass = (uintptr_t *) malloc(64);
            memset(g_WaterShaderClass, 1, 64);
//            WaterShader::BuildShadersSource1(g_WaterShaderClass);
        }

        //if (g_WaterShaderClass)
         //   WaterShader::EmuShader__Select3(g_WaterShaderClass, true);
    }
    emu_glEndInternal();
}

void InstallSpecialHooks() {
    Log("ok");
    // by viski
    InjectHooks();
    Log(OBFUSCATE("InstallSpecialHooks"));

    CHook::InlineHook("_Z11OS_FileOpen14OSFileDataAreaPPvPKc16OSFileAccessType", &OS_FileOpen_hook, &OS_FileOpen);

    if (!eglGetProcAddress("glAlphaFuncQCOM")) {
        if (eglGetProcAddress("glAlphaFunc")) {
            *((void **) (g_libGTASA +
                         (VER_x32 ? 0x6BCBF8 : 0x89A1B0))) = (void *) eglGetProcAddress(
                    "glAlphaFunc");
        } else {
            CHook::Redirect("_Z25RQ_Command_rqSetAlphaTestRPc", &RQCommand_rqSetAlphaTest);
        }
    }

    CHook::InlineHook("_Z10NvUtilInitv", NvUtilInit_hook, &NvUtilInit);

    CHook::InlineHook("_ZN22TextureDatabaseRuntime8GetEntryEPKcRb",
                      CTextureDatabaseRuntime__GetEntry_hook, &CTextureDatabaseRuntime__GetEntry);
    CHook::InstallPLT(g_libGTASA + (VER_x32 ? 0x6701D4 : 0x840708), &RLEDecompress_hook,
                      &RLEDecompress);
    CHook::InlineHook("_Z11OS_FileReadPvS_i", OS_FileRead_hook, &OS_FileRead);
    CHook::Redirect("_Z7NvFOpenPKcS0_bb", &NvFOpen);

    CHook::Redirect("_ZN5CGame20InitialiseRenderWareEv", &CGame::InitialiseRenderWare);
    CHook::InstallPLT(g_libGTASA + (VER_x32 ? 0x6785FC : 0x84EC20),
                      &StartGameScreen__OnNewGameCheck_hook, &StartGameScreen__OnNewGameCheck);
    CHook::InlineHook("_ZN14MainMenuScreen6UpdateEf", MainMenuScreen__Update_hook,
                      &MainMenuScreen__Update);

    //CHook::InlineHook("_Z32_rxOpenGLDefaultAllInOneRenderCBP10RwResEntryPvhj", rxOpenGLDefaultAllInOneRenderCB_hook, &rxOpenGLDefaultAllInOneRenderCB);
    //CHook::InlineHook("_ZN25CCustomBuildingDNPipeline18CustomPipeRenderCBEP10RwResEntryPvhj", CCustomBuildingDNPipeline__CustomPipeRenderCB_hook, &CCustomBuildingDNPipeline__CustomPipeRenderCB);
}

void InstallHooks() {
    // by viski
    CHook::Redirect("_ZN11CHudColours12GetIntColourEh", &CHudColours__GetIntColour);
    CHook::InlineHook("_Z14AND_TouchEventiiii", AND_TouchEvent_hook, &AND_TouchEvent);
    CHook::InlineHook("_Z12ANDRunThreadPv", ANDRunThread_hook, &ANDRunThread);
  //  CHook::InlineHook("_Z17emu_glEndInternalv", emu_glEndInternal_hook, &emu_glEndInternal);

    // from lr
    if (*(uint8_t *) (g_libGTASA + (VER_x32 ? 0x6B8B9C : 0x896135))) {
        CHook::Redirect("_ZNK14TextureListing11GetMipCountEv", &getmip);
    }

    //CHook::InlineHook("_ZN17CVehicleAnimGroup22ComputeAnimDoorOffsetsEi", ComputeAnimDoorOffsets_hook, &ComputeAnimDoorOffsets);
    CHook::Redirect("_ZN4CHID12GetInputTypeEv", &GetInputType);
    //CHook::Redirect("_ZN12CFireManager24ExtinguishPointWithWaterE7CVectorff",&CFireManager__ExtinguishPointWithWater_hook, &CFireManager__ExtinguishPointWithWater);

    CHook::InlineHook("_ZN7CEntity6RenderEv", CEntity_Render_hook, &CEntity_Render);

    CHook::Redirect("_Z13RenderEffectsv", &RenderEffects);
    CHook::InlineHook("_ZN7CWeapon4FireEP7CEntityP7CVectorS3_S1_S3_S3_", CWeapon__Fire_hook,
                      &CWeapon__Fire);
    CHook::InlineHook("_ZN4CPed16SetCurrentWeaponE11eWeaponType", CPed__SetCurrentWeapon_hook,
                      &CPed__SetCurrentWeapon);
    //CHook::InlineHook("_ZN7CWeapon6UpdateEP4CPed", CWeapon__Update_hook, &CWeapon__Update);
    CHook::InlineHook("_ZN10CPlayerPed23MakeChangesForNewWeaponE11eWeaponType",
                      CPlayerPed__MakeChangesForNewWeapon_hook,
                      &CPlayerPed__MakeChangesForNewWeapon);
    CHook::Redirect("_Z19PlayerIsEnteringCarv", &PlayerIsEnteringCar);
    CHook::InlineHook("_ZNK18CEventKnockOffBike10AffectsPedEP4CPed",
                      CEventKnockOffBike__AffectsPed_hook, &CEventKnockOffBike__AffectsPed);
    CHook::InlineHook("_ZN7CWeapon14FireInstantHitEP7CEntityP7CVectorS3_S1_S3_S3_bb",
                      CWeapon__FireInstantHit_hook, &CWeapon__FireInstantHit);
    CHook::InlineHook("_ZN7CWeapon10FireSniperEP4CPedP7CEntityP7CVector", CWeapon__FireSniper_hook,
                      &CWeapon__FireSniper);
    CHook::InlineHook("_ZN6CWorld18ProcessLineOfSightERK7CVectorS2_R9CColPointRP7CEntitybbbbbbbb",
                      CWorld__ProcessLineOfSight_hook, &CWorld__ProcessLineOfSight);
    CHook::InlineHook(
            "_ZN28CPedDamageResponseCalculator21ComputeDamageResponseEP4CPedR18CPedDamageResponseb",
            CPedDamageResponseCalculator__ComputeDamageResponse_hook,
            &CPedDamageResponseCalculator__ComputeDamageResponse);
    CHook::InlineHook(
            "_ZN7CWeapon18ProcessLineOfSightERK7CVectorS2_R9CColPointRP7CEntity11eWeaponTypeS6_bbbbbbb",
            CWeapon__ProcessLineOfSight_hook, &CWeapon__ProcessLineOfSight);
    CHook::InlineHook(
            "_ZN10CCollision19ProcessVerticalLineERK8CColLineRK7CMatrixR9CColModelR9CColPointRfbbP15CStoredCollPoly",
            CCollision__ProcessVerticalLine_hook, &CCollision__ProcessVerticalLine);
    CHook::Redirect("_Z13Render2dStuffv", &Render2dStuff);
    CHook::InlineHook("_ZN19CUpsideDownCarCheck15IsCarUpsideDownEPK8CVehicle",
                      CUpsideDownCarCheck__IsCarUpsideDown_hook,
                      &CUpsideDownCarCheck__IsCarUpsideDown);
    CHook::Redirect("_Z10GetTexturePKc", &CUtil::GetTexture);
    CHook::InlineHook("_ZN11CAutomobile22ProcessEntityCollisionEP7CEntityP9CColPoint",
                      CAutomobile__ProcessEntityCollision_hook,
                      &CAutomobile__ProcessEntityCollision);
    CHook::InlineHook("_ZN14MainMenuScreen6OnExitEv", MainMenuScreen__OnExit_hook,
                      &MainMenuScreen__OnExit);
    CHook::InlineHook("_ZN11CAutomobile9PreRenderEv", CAutomobile__PreRender_hook,
                      &CAutomobile__PreRender);
    CHook::InlineHook("_ZN11CAutomobile17UpdateWheelMatrixEii", CAutomobile__UpdateWheelMatrix_hook,
                      &CAutomobile__UpdateWheelMatrix);
    CHook::InlineHook("_ZN7CMatrix6RotateEfff", CMatrix__Rotate_hook, &CMatrix__Rotate);
    CHook::InlineHook("_ZN7CMatrix8SetScaleEfff", CMatrix__SetScale_hook, &CMatrix__SetScale);
    CHook::InlineHook("_ZN4CCam7ProcessEv", CCam__Process_hook, &CCam__Process);
    CHook::InlineHook("_ZN4CPad27CycleCameraModeDownJustDownEv",
                      CPad__CycleCameraModeDownJustDown_hook, &CPad__CycleCameraModeDownJustDown);
    CHook::InlineHook("_ZN13FxEmitterBP_c6RenderEP8RwCamerajfh", FxEmitterBP_c__Render_hook,
                      &FxEmitterBP_c__Render);
    CHook::InlineHook("_ZN4CPed22ProcessEntityCollisionEP7CEntityP9CColPoint",
                      CPed__ProcessEntityCollision_hook, &CPed__ProcessEntityCollision);
    CHook::InlineHook("_Z23RwResourcesFreeResEntryP10RwResEntry", RwResourcesFreeResEntry_hook,
                      &RwResourcesFreeResEntry);
    CHook::InlineHook("_ZN5CDraw6SetFOVEfb", CDraw__SetFOV_hook, &CDraw__SetFOV);
    HookCPad();
}