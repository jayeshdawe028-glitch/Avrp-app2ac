//
// Created by viski on 20.04.2025.
//

#ifndef X64_CLOUDS_H
#define X64_CLOUDS_H

#include "jni.h"
#include "game/core/Streaming.h"
#include "game/RW/TxdStore.h"
#include "game/fakerw/RenderWare.h"

#include "game/core/core/Vector.h"
//#include <rwplcore.h>

constexpr auto MAX_MOVING_FOG = 350;
constexpr auto MAX_VOLUMETRIC_CLOUDS = 180;

constexpr auto SKYP_WIDTH_MULT = 1.4f;
constexpr auto SKYP_CAM_DIST = 30.f;

constexpr auto SKYP_ABOVE_HORIZON_Z = 0.5f;
constexpr auto SKYP_HORIZON_Z = 0.0f;
constexpr auto SKYP_SEA_HORIZON_Z = -0.1f;
constexpr auto SKYP_BELOW_HORIZON_Z = -0.3f;

struct tMovingFog {
    bool    m_bFogSlots[MAX_MOVING_FOG];
    CVector m_vecPosn[MAX_MOVING_FOG];
    float   m_fSize[MAX_MOVING_FOG];
    float   m_fIntensity[MAX_MOVING_FOG];
    float   m_fMaxIntensity[MAX_MOVING_FOG];
    CVector m_vecWind;
    float   m_fSpeed[MAX_MOVING_FOG];
    int     field_27B4[12];
    std::array<uint32, 6> m_nPrimIndices;
};

struct tVolumetricClouds {
    bool       bUsed[MAX_VOLUMETRIC_CLOUDS];
    bool       bJustCreated[MAX_VOLUMETRIC_CLOUDS];

    CVector    pos[MAX_VOLUMETRIC_CLOUDS];
    CVector    size[MAX_VOLUMETRIC_CLOUDS];

    int32      alpha[MAX_VOLUMETRIC_CLOUDS];

    RwTexture* texture;

    CVector    quadNormal[3];

    float      modelX[18];
    float      modelY[18];
    float      modelZ[18];

    float      modelU[18];
    float      modelV[18];
};

class CClouds {
public:
    typedef struct {
        bool bUsed[MAX_MOVING_FOG]; // m_bFogSlots
        uint8_t pad[2]; // xz
        CVector pos[MAX_MOVING_FOG]; // m_vecPosn
        float fSize[MAX_MOVING_FOG]; // m_fSize
        float fAlpha[MAX_MOVING_FOG]; // m_fIntensity
        float fAlphaMax[MAX_MOVING_FOG]; // m_fMaxIntensity
        CVector windVector; // m_vecWind
        float fSpeedFactor[MAX_MOVING_FOG]; // m_fSpeed
        float modelU[6]; // xz
        float modelV[6]; // xz
        int quadVT[6]; // navernoe m_nPrimIndices
    } movingFog;
    typedef struct {
        bool bUsed[180];
        bool bJustCreated[180];
        CVector pos[180];
        CVector size[180];
        uint32_t alpha[180];
        RwTexture* pTexture;
        CVector quadNormal[3];
        float modelX[18];
        float modelY[18];
        float modelZ[18];
        float modelU[18];
        float modelV[18];
    } volumeClouds;
    static RwTexture* gpCloudTex;
    static RwTexture* gpMoonMask;
    static RwTexture* gpCloudMaskTex;
    static RwTexture* ms_pHighCloudTex;
    static float CurrentFogIntensity;
    //static CVector PlayerCoords;
    //static CVector CameraCoors;
    static volumeClouds ms_vc;
    static movingFog ms_mf;
    static float CloudRotation;
    static int32 IndividualRotation;
    static float ms_cameraRoll;
    static float m_fWaterFXStartUnderWaterness;
    static float m_fVolumetricCloudMaxDistance;
    static uint32 m_VolumetricCloudsUsedNum;
    static float m_fVolumetricCloudDensity;
    static bool m_bVolumetricCloudHeightSwitch;
    static float m_fVolumetricCloudWindMoveFactor;

    static void Render_MaybeRenderMoon();

    static void InjectHooks();

    static void Init();
    static void Update();
    static void Shutdown();
    static void MovingFogInit();
    static void VolumetricCloudsInit();
    static void SetUpOneSkyPoly(CVector vert1pos, CVector vert2pos, CVector vert3pos, CVector vert4pos, uint8 topRed, uint8 topGreen, uint8 topBlue,
                                uint8 bottomRed, uint8 bottomGreen, uint8 bottomBlue);
    static void MovingFog_Create(CVector* posn);
    static float MovingFog_GetFXIntensity();
    static CVector MovingFog_GetWind();
    static int32 MovingFog_GetFirstFreeSlot();

    static void MovingFog_Delete(int32 fogSlotIndex);
    static void MovingFog_Update();
    static void MovingFogRender();
    static void Render_MaybeRenderRockstarLogo(float colorBalance);
    static void Render_RenderLowClouds(float colorBalance);
    static void Render_MaybeRenderRainbows();
    static void Render_MaybeRenderStreaks();

    static void Render();
    static void RenderSkyPolys();
    static void RenderBottomFromHeight();
    static void VolumetricClouds_Create(CVector* posn);
    static void VolumetricClouds_Delete(int32 vcSlotIndex);
    static int32 VolumetricClouds_GetFirstFreeSlot();
    static float VolumetricCloudsGetMaxDistance();
    static void VolumetricCloudsRender();
};

#endif //X64_CLOUDS_H