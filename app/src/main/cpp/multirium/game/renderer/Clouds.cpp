//
// Created by viski on 20.04.2025.
//

#include "Clouds.h"
#include "game/core/Camera.h"
#include "game/core/game.h"
#include "TimeCycle.h"
#include "game/core/Clock.h"
#include "Coronas.h"
#include "Lines.h"
#include "game/core/rgba.h"
#include "Weather.h"
#include "game/custom/SkyBox.h"
#include "Sprite.h"
#include "game/core/General.h"
#include "RenderBuffer.hpp"
#include "game/control/Scene.h"
#include "game/core/fx/Fx.h"
#include "game/core/World.h"

extern CGame pGame;

float  CClouds::m_fWaterFXStartUnderWaterness = 0.0f;

float  CClouds::m_fVolumetricCloudMaxDistance = 0.0f;
uint32 CClouds::m_VolumetricCloudsUsedNum = 0;
float  CClouds::m_fVolumetricCloudDensity = 0.0f;
bool   CClouds::m_bVolumetricCloudHeightSwitch = false;
float  CClouds::m_fVolumetricCloudWindMoveFactor = 0.0f;

float CClouds::ms_cameraRoll = 0.0f;

int32 CClouds::IndividualRotation = 0;
float CClouds::CloudRotation = 0.0f;

CClouds::volumeClouds CClouds::ms_vc = {};
CClouds::movingFog CClouds::ms_mf = {};

float CClouds::CurrentFogIntensity = 0.0f;

RwTexture *CClouds::gpCloudTex = nullptr;
RwTexture *CClouds::gpCloudMaskTex = nullptr;
RwTexture *CClouds::gpMoonMask = nullptr;
RwTexture *CClouds::ms_pHighCloudTex = nullptr;

void CClouds::Init() {
    Log(OBFUSCATE("CClouds::Init()"));

    CTxdStore::PushCurrentTxd();
    CTxdStore::SetCurrentTxd(CTxdStore::FindTxdSlot("particle"));

    CClouds::gpCloudTex = RwTextureRead("cloud1", nullptr);
    *reinterpret_cast<RwTexture **>(g_libGTASA + (VER_x32 ? 0xA23DE8 : 0xCC4CD8)) = CClouds::gpCloudTex;
    CClouds::gpCloudMaskTex = RwTextureRead("cloudmasked", nullptr);
    *reinterpret_cast<RwTexture **>(g_libGTASA + (VER_x32 ? 0xA23DE8 : 0xCC4CE0)) = CClouds::gpCloudMaskTex;
    CClouds::gpMoonMask = RwTextureRead("lunar", "lunarm");
    CClouds::ms_pHighCloudTex = RwTextureRead("cloudhigh", "cloudhighm");
    *reinterpret_cast<RwTexture **>(g_libGTASA + (VER_x32 ? 0xA23DE8 : 0xCC6240)) = CClouds::ms_pHighCloudTex;

    CTxdStore::PopCurrentTxd();

    CloudRotation = 0.0f;

    VolumetricCloudsInit();

    ms_mf = {};

    MovingFogInit();
}

void CClouds::Update() {
    static CCamera &TheCamera = *reinterpret_cast<CCamera *>(g_libGTASA +
                                                             (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

    CloudRotation =
            std::sin(TheCamera.m_fOrientation - 0.85f) * CWeather::Wind * 0.001f + CloudRotation;
    IndividualRotation += (int32) ((CTimer::GetTimeStep() * CWeather::Wind * 0.5f + 0.3f) * 60.0f);
}

void CClouds::Shutdown() {
    RwTextureDestroy(CClouds::gpCloudTex);
    CClouds::gpCloudTex = nullptr;

    RwTextureDestroy(CClouds::gpCloudMaskTex);
    CClouds::gpCloudMaskTex = nullptr;

    RwTextureDestroy(CClouds::ms_pHighCloudTex);
    CClouds::ms_pHighCloudTex = nullptr;
}

void CClouds::MovingFog_Create(CVector *posn) {
    int32 slotId = MovingFog_GetFirstFreeSlot();
    if (slotId == -1)
        return;

    ms_mf.pos[slotId] = CVector{
            CGeneral::GetRandomNumberInRange(-58.0f, 58.0f),
            CGeneral::GetRandomNumberInRange(-58.0f, 58.0f),
            CGeneral::GetRandomNumberInRange(-5.0f, 5.0f)
    };
    ms_mf.pos[slotId] += *posn;
    ms_mf.fSize[slotId] = CGeneral::GetRandomNumberInRange(4.0f, 10.0f);
    ms_mf.fAlpha[slotId] = 1.0f;
    ms_mf.fAlphaMax[slotId] = CGeneral::GetRandomNumberInRange(8.0f, 20.0f);
    ms_mf.fSpeedFactor[slotId] = CGeneral::GetRandomNumberInRange(0.5f, 1.2f);
    ms_mf.bUsed[slotId] = true;
}

void CClouds::MovingFog_Delete(int32 fogSlotIndex) {
    fogSlotIndex = std::clamp(fogSlotIndex, 0, MAX_MOVING_FOG - 1);
    ms_mf.bUsed[fogSlotIndex] = false;
}

void CClouds::MovingFog_Update() {
    static CCamera &TheCamera = *reinterpret_cast<CCamera *>(g_libGTASA +
                                                             (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

    if (MovingFog_GetFXIntensity() == 0.f)
        return;

    CVector camPos = TheCamera.GetPosition();
    for (int32 i = 0; i < MAX_MOVING_FOG; i++) {
        if (!ms_mf.bUsed[i]) {
            MovingFog_Create(&camPos);
            continue;
        }

        CVector &fogPosn = ms_mf.pos[i];
        CVector offset = fogPosn - camPos;

        fogPosn.x += MovingFog_GetWind().x * ms_mf.fSpeedFactor[i];
        fogPosn.y += MovingFog_GetWind().y * ms_mf.fSpeedFactor[i];

        if (offset.Magnitude() <= 60.f) {
            ms_mf.fAlpha[i] = std::min(ms_mf.fAlpha[i] + CTimer::GetTimeStep(), ms_mf.fAlphaMax[i]);
        } else {
            ms_mf.fAlpha[i] -= CTimer::GetTimeStep();
            if (ms_mf.fAlpha[i] <= 0.f) {
                MovingFog_Delete(i);
            }
        }
    }
}

void CClouds::MovingFogInit() {
    ms_mf = {};
    ms_mf.windVector = CVector(0.06f, 0.06f, 0.0f);
    ms_mf.quadVT[0] = 0;
    ms_mf.quadVT[1] = 1;
    ms_mf.quadVT[2] = 2;
    ms_mf.quadVT[3] = 0;
    ms_mf.quadVT[4] = 2;
    ms_mf.quadVT[5] = 3;
}

float CClouds::MovingFog_GetFXIntensity() {
    return CWeather::Foggyness_SF;
}

CVector CClouds::MovingFog_GetWind() {
    return ms_mf.windVector;
}

int32 CClouds::MovingFog_GetFirstFreeSlot() {
    int32 result = 0;
    while (ms_mf.bUsed[result]) {
        if (++result >= MAX_MOVING_FOG)
            return -1;
    }
    return result;
}

void CClouds::MovingFogRender() {

}

uint8 CalculateColorWithBalance(uint8 blue, float colorBalance) {
    return lerp<uint8>(blue, 0u, colorBalance);
}

const auto colorBalance = std::max(CWeather::Foggyness, CWeather::CloudCoverage);

void CClouds::Render() {
    if (CHook::CallFunction<bool>("_ZN5CGame25CanSeeOutSideFromCurrAreaEv")) {
        if (CWeather::UnderWaterness < 0.5)
            CSkyBox::Process();
        CCoronas::SunBlockedByClouds = false;

        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, RWRSTATE(FALSE));
        RwRenderStateSet(rwRENDERSTATEZTESTENABLE, RWRSTATE(FALSE));
        RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(TRUE));
        RwRenderStateSet(rwRENDERSTATESRCBLEND, (void *) rwBLENDSRCALPHA);
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void *) rwBLENDINVSRCALPHA);

        CSprite::InitSpriteBuffer();

        Render_MaybeRenderMoon();
        //  Render_MaybeRenderRockstarLogo(colorBalance);
        // Render_RenderLowClouds(std::max(colorBalance, CWeather::ExtraSunnyness));
        // Render_MaybeRenderRainbows();
        // Render_MaybeRenderStreaks();
    }
}

void CClouds::Render_MaybeRenderMoon() {
    static CCamera &TheCamera = *reinterpret_cast<CCamera *>(g_libGTASA +
                                                             (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

    RwV3d In;
    RwV3d pResult;
    float pScaleY;
    float pScaleX;
    float ms_fFarClipZ = *(float *) (g_libGTASA + (VER_x32 ? 0xA26A88 : 0xCC7EF8));

    In.z = 15.0;
    CSimpleTransform *p_m_transform = &TheCamera.m_placement;

    if (TheCamera.m_matrix)
        p_m_transform = (CSimpleTransform *) &TheCamera.m_matrix->m_pos.x;

    In.x = p_m_transform->m_vPosn.x + 0.0;
    In.y = p_m_transform->m_vPosn.y + -100.0;
    In.z = p_m_transform->m_vPosn.z + 55.0;

    if (CClock::GetIsTimeInRangeWithMinutes(20, 30, 3, 30)) {
        int r = 255;
        int g = 240;
        int b = 200;

        if (CClock::ms_nGameClockHours >= 23 || CClock::ms_nGameClockHours <= 1) {
            r = 255;
            g = 255;
            b = 255;
        }

        if (CSprite::CalcScreenCoors(&In, &pResult, &pScaleX, &pScaleY, false, true)) {
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RWRSTATE(NULL));
            RwRenderStateSet(rwRENDERSTATESRCBLEND, RWRSTATE(rwBLENDSRCALPHA));
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDONE));

            CSprite::RenderOneXLUSprite(
                    pResult.x,
                    pResult.y,
                    ms_fFarClipZ,
                    pScaleX * ((CCoronas::MoonSize * 1.5f) + 4.0f),
                    pScaleY * ((CCoronas::MoonSize * 1.5f) + 4.0f),
                    0, 0, 0, 255,
                    1.0f / ms_fFarClipZ,
                    255, false, false, 0.0f, 0.0f);

            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, gpMoonMask->raster);
            RwRenderStateSet(rwRENDERSTATESRCBLEND, RWRSTATE(rwBLENDSRCALPHA));
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDONE));

            float phase = (float) CClock::ms_nGameClockDays / 31.0f;
            float moonSize = (CCoronas::MoonSize * 2.0f) + 4.0f;

            CSprite::RenderOneXLUSprite(
                    pResult.x + ((phase - 0.5f) * ((pScaleX * moonSize * 2.7f) * 2.0f)),
                    pResult.y + ((pScaleY * moonSize) * 0.7f),
                    ms_fFarClipZ,
                    pScaleX * (moonSize * 1.7f),
                    pScaleY * (moonSize * 1.7f),
                    0, 0, 0, 0,
                    1.0f / ms_fFarClipZ,
                    255, false, false, 0.0f, 0.0f);

            RwRenderStateSet(rwRENDERSTATESRCBLEND, RWRSTATE(rwBLENDDESTALPHA));
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDONE));
            RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, RWRSTATE(FALSE));
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, gpCoronaTexture[2]->raster);

            CSprite::RenderOneXLUSprite(
                    pResult.x,
                    pResult.y,
                    ms_fFarClipZ,
                    pScaleX * ((CCoronas::MoonSize * 1.5f) + 4.0f),
                    pScaleY * ((CCoronas::MoonSize * 1.5f) + 4.0f),
                    r,
                    g,
                    b,
                    255,
                    1.0f / ms_fFarClipZ,
                    255, false, false, 0.0f, 0.0f);

            RwRenderStateSet(rwRENDERSTATESRCBLEND, RWRSTATE(rwBLENDSRCALPHA));
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDINVSRCALPHA));
        }
    }
}


// From `CClouds::Render`
// Draws the R* logo on the sky
void CClouds::Render_MaybeRenderRockstarLogo(float colorBalance) {
    /* static CCamera &TheCamera = *reinterpret_cast<CCamera *>(g_libGTASA +
                                                              (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

     constexpr auto R_OFFSET_FROM_CAMERA = CVector{100.f, 0.f,
                                                   10.f}; // Letter `R` offset from camera
     constexpr auto STAR_OFFSET_FROM_CAMERA = CVector{100.f, -90.f,
                                                      10.f}; // `*` [As in R*] offset from camera

     constexpr float STARS_Y_POSITIONS[9] = {0.00f, 0.05f, 0.13f, 0.40f, 0.70f, 0.60f, 0.27f, 0.55f,
                                             0.75f};
     constexpr float STARS_Z_POSITIONS[9] = {0.00f, 0.45f, 0.90f, 1.00f, 0.85f, 0.52f, 0.48f, 0.35f,
                                             0.20f};
     constexpr float STARS_SIZES[9] = {1.00f, 1.40f, 0.90f, 1.00f, 0.60f, 1.50f, 1.30f, 1.00f,
                                       0.80f};

     const uint32_t t = CClock::GetGameClockHours() == 22u
                        ? CClock::GetGameClockMinutes()
                        : CClock::GetGameClockHours() == 6u - 1
                          ? 60u - CClock::GetGameClockMinutes()
                          : 60u;

     if (t == 0) {
         return;
     }

     const auto colorB = (uint8) (255u * t / 60u);
     const auto colorRG = CalculateColorWithBalance(colorB, colorBalance);
     RwRenderStateSet(rwRENDERSTATETEXTURERASTER,
                      RWRSTATE(RwTextureGetRaster((*reinterpret_cast<RwTexture ***>(g_libGTASA +
                                                                                    0xCC6A18))[CORONATYPE_SHINYSTAR])));
     //RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RWRSTATE(RwTextureGetRaster(gpCoronaTexture[CORONATYPE_SHINYSTAR])));

     const auto camPos = TheCamera.GetPosition();

     for (auto i = 0; i < 11; i++) {
         CVector offset = R_OFFSET_FROM_CAMERA;
         if (i >= 9) {
             offset.x = -offset.x;
         }

         const auto posIdx = i % 9;
         offset.y -= STARS_Y_POSITIONS[posIdx] * 90.f;
         offset.z += STARS_Z_POSITIONS[posIdx] * 80.f;

         CVector starPosScr;
         CVector2D starSizeScr;
         if (!CSprite::CalcScreenCoors(camPos + offset, &starPosScr, &starSizeScr.x, &starSizeScr.y,
                                       false, true)) {
             continue;
         }

         const auto cc = CalculateColorWithBalance(colorB, (float) (rand() % 32) * 0.015f);

         starSizeScr *= STARS_SIZES[posIdx] * 0.8f;

         CSprite::RenderBufferedOneXLUSprite(
                 starPosScr,
                 starSizeScr,
                 cc, cc, cc, 255,
                 1.f / starPosScr.z,
                 255
         );
     }

     CVector lastStarPosScr;
     CVector2D lastStarSizeScr;
     if (CSprite::CalcScreenCoors(camPos + STAR_OFFSET_FROM_CAMERA, &lastStarPosScr,
                                  &lastStarSizeScr.x, &lastStarSizeScr.y, false, true)) {
         const auto cc = CalculateColorWithBalance(colorB,
                                                   (float) (rand() % 128) * 0.0015625f + 0.5f);

         lastStarSizeScr.x *= 5.f;
         lastStarSizeScr.y *= 5.f;

         CSprite::RenderOneXLUSprite(
                 lastStarPosScr,
                 lastStarSizeScr,
                 cc, cc, cc, 255,
                 1.f / lastStarPosScr.z,
                 255, 0, 0
         );
     }

     CSprite::FlushSpriteBuffer();*/
}

// From `CClouds::Render`
void CClouds::Render_MaybeRenderRainbows() {
    /*  constexpr size_t NUM_RAINBOW_LINES = 6;
      const CRGBA RAINBOW_LINES_COLOR[]{
              {30, 0,  0,  255},
              {30, 15, 0,  255},
              {30, 30, 0,  255},
              {10, 30, 10, 255},
              {0,  0,  30, 255},
              {15, 0,  30, 255}
      };
      //RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RWRSTATE(RwTextureGetRaster(gpCoronaTexture[CORONATYPE_SHINYSTAR])));
      RwRenderStateSet(rwRENDERSTATETEXTURERASTER,
                       RWRSTATE(RwTextureGetRaster((*reinterpret_cast<RwTexture ***>(g_libGTASA +
                                                                                     0xCC6A18))[CORONATYPE_SHINYSTAR])));
      static CCamera &TheCamera = *reinterpret_cast<CCamera *>(g_libGTASA +
                                                               (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

      const auto camPos = TheCamera.GetPosition();

      for (auto i = 0; i < NUM_RAINBOW_LINES; i++) {
          const auto offset = CVector{
                  (float) i * 1.5f,
                  100.f,
                  5.f
          };

          CVector rblinePosScr;
          CVector2D rblineSizeScr;
          if (!CSprite::CalcScreenCoors(camPos + offset, &rblinePosScr, &rblineSizeScr.x,
                                        &rblineSizeScr.y, false, true)) {
              continue;
          }
          rblineSizeScr *= CVector2D{2.f, 50.f};

          const auto &clr = RAINBOW_LINES_COLOR[i];
          CSprite::RenderBufferedOneXLUSprite(
                  rblinePosScr,
                  rblineSizeScr,
                  (uint8) ((float) clr.r * CWeather::Rainbow),
                  (uint8) ((float) clr.g * CWeather::Rainbow),
                  (uint8) ((float) clr.b * CWeather::Rainbow),
                  clr.a,
                  1.f / rblinePosScr.z,
                  255
          );
      }

      CSprite::FlushSpriteBuffer();*/
}

// From `CClouds::Render`
void CClouds::Render_MaybeRenderStreaks() {
    static CCamera &TheCamera = *reinterpret_cast<CCamera *>(g_libGTASA +
                                                             (VER_x32 ? 0x00951FA8 : 0xBBA8D0));


    static_assert(8192u >= 800u);

    RwRenderStateSet(rwRENDERSTATESRCBLEND, RWRSTATE(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDINVSRCALPHA));

    const auto timeMS = CTimer::GetTimeInMS();

    const auto repeatDelta = timeMS % 8192u;
    if (repeatDelta >= 800u) {
        return;
    }

    const auto repeat64 = (timeMS / 8192u) % 64;

    const auto size = CVector{
            (float) ((int32) (repeat64 % 7) - 3) * 0.1f,
            (float) ((int32) ((timeMS & 0xFFFF) / 8192u) - 4) * 0.1f,
            1.f
    }.Normalized();

    const auto offsetDir = CVector{
            (float) ((int32) (repeat64 % 9) - 5),
            (float) ((int32) (repeat64 % 10) - 5),
            0.1f
    }.Normalized();

    RenderBuffer::ClearRenderBuffer();

    const auto PushVertex = [
            basePos = offsetDir * 1000.f + TheCamera.GetPosition(),
            size
    ](float scale, CRGBA color) {
        RenderBuffer::PushVertex(
                basePos + size * scale,
                color
        );
    };

    const auto scale = (float) ((800u / 2 - (int32) repeatDelta) * 2);
    PushVertex(scale, {255, 255, 255, 225});
    PushVertex(scale + 50.f, {255, 255, 255, 0});

    RenderBuffer::PushIndices({1, 0}, false);

    RenderBuffer::Render(rwPRIMTYPEPOLYLINE, nullptr, rwIM3D_VERTEXRGBA | rwIM3D_VERTEXXYZ);
}

void
CClouds::SetUpOneSkyPoly(CVector vert1pos, CVector vert2pos, CVector vert3pos, CVector vert4pos,
                         uint8 topRed, uint8 topGreen, uint8 topBlue,
                         uint8 bottomRed, uint8 bottomGreen, uint8 bottomBlue) {
    return CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x0059F924 + 1 : 0x6C3700), vert1pos, vert2pos, vert3pos, vert4pos, topRed, topGreen, topBlue, bottomRed, bottomGreen, bottomBlue);
    uint16 uiStartVertex = uiTempBufferVerticesStored;
    uint16 iStartIndex = uiTempBufferIndicesStored;

    aTempBufferIndices[iStartIndex + 0] = uiStartVertex;
    aTempBufferIndices[iStartIndex + 1] = uiStartVertex + 2;
    aTempBufferIndices[iStartIndex + 2] = uiStartVertex + 1;
    aTempBufferIndices[iStartIndex + 3] = uiStartVertex + 1;
    aTempBufferIndices[iStartIndex + 4] = uiStartVertex + 2;
    aTempBufferIndices[iStartIndex + 5] = uiStartVertex + 3;

    uiTempBufferIndicesStored += 6;
    uiTempBufferVerticesStored += 4;
}

void CClouds::RenderSkyPolys() {
    CCamera &TheCamera = *reinterpret_cast<CCamera *>(g_libGTASA +
                                                      (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

    CVector norm{}, pos{};

    if (TheCamera.m_matrix) {
        pos = TheCamera.m_matrix->GetPosition();
        norm = TheCamera.m_matrix->GetForward();
    } else {
        pos = TheCamera.m_placement.m_vPosn;
        float fHeading = TheCamera.m_placement.m_fHeading;
        norm.x = -sin(fHeading);
        norm.y = cos(fHeading);
    }

    norm.z = 0.f;
    norm.Normalise();

    CVector invNorm = -norm;
    float fBlendFactor = (pos.z - 25.0f) * (1.0f / 80.0f);
    fBlendFactor = std::clamp(fBlendFactor, 0.f, 1.f);
    fBlendFactor = std::max(fBlendFactor, CWeather::Foggyness);

    RwRGBA belowHorizonGrey = CTimeCycle::m_BelowHorizonGrey;
    belowHorizonGrey.red   += static_cast<RwUInt8>(fBlendFactor * static_cast<float>(CTimeCycle::m_CurrentColours.m_nSkyBottomRed - belowHorizonGrey.red));
    belowHorizonGrey.green += static_cast<RwUInt8>(fBlendFactor * static_cast<float>(CTimeCycle::m_CurrentColours.m_nSkyBottomGreen - belowHorizonGrey.green));
    belowHorizonGrey.blue  += static_cast<RwUInt8>(fBlendFactor * static_cast<float>(CTimeCycle::m_CurrentColours.m_nSkyBottomBlue - belowHorizonGrey.blue));

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     RWRSTATE(NULL));
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       RWRSTATE(NULL));
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      RWRSTATE(NULL));
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(NULL));
    RwRenderStateSet(rwRENDERSTATESRCBLEND,          RWRSTATE(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,         RWRSTATE(rwBLENDINVSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEFOGENABLE,         RWRSTATE(NULL));
    RwRenderStateSet(rwRENDERSTATECULLMODE,          RWRSTATE(rwCULLMODECULLNONE));

    uiTempBufferIndicesStored = 0;
    uiTempBufferVerticesStored = 0;

    CVector2D scale{norm.y * SKYP_WIDTH_MULT, invNorm.x * SKYP_WIDTH_MULT}, aPos2D[4] = {};
    CVector aVertices[4] = {};
    float aPosZ[6] = {};

    aPos2D[0].x = pos.x + (invNorm.x - scale.x) * SKYP_CAM_DIST;
    aPos2D[0].y = pos.y + (invNorm.y - scale.y) * SKYP_CAM_DIST;
    aPos2D[1].x = pos.x + (invNorm.x + scale.x) * SKYP_CAM_DIST;
    aPos2D[1].y = pos.y + (invNorm.y + scale.y) * SKYP_CAM_DIST;
    aPos2D[2].x = pos.x + (norm.x - scale.x) * SKYP_CAM_DIST;
    aPos2D[2].y = pos.y + (norm.y - scale.y) * SKYP_CAM_DIST;
    aPos2D[3].x = pos.x + (norm.x + scale.x) * SKYP_CAM_DIST;
    aPos2D[3].y = pos.y + (norm.y + scale.y) * SKYP_CAM_DIST;

    aPosZ[0] = pos.z + (invNorm.z + SKYP_ABOVE_HORIZON_Z) * SKYP_CAM_DIST;
    aPosZ[1] = pos.z + (norm.z + SKYP_ABOVE_HORIZON_Z) * SKYP_CAM_DIST;
    aPosZ[2] = pos.z + (norm.z + SKYP_HORIZON_Z) * SKYP_CAM_DIST;
    aPosZ[3] = pos.z + (norm.z + SKYP_SEA_HORIZON_Z) * SKYP_CAM_DIST;
    aPosZ[4] = pos.z + (norm.z + SKYP_BELOW_HORIZON_Z) * SKYP_CAM_DIST;
    aPosZ[5] = pos.z + (invNorm.z + SKYP_BELOW_HORIZON_Z) * SKYP_CAM_DIST;

    aVertices[0].x = aPos2D[2].x;
    aVertices[0].y = aPos2D[2].y;
    aVertices[0].z = aPosZ[1];

    aVertices[1].x = aPos2D[3].x;
    aVertices[1].y = aPos2D[3].y;
    aVertices[1].z = aPosZ[1];

    aVertices[2].x = aPos2D[2].x;
    aVertices[2].y = aPos2D[2].y;
    aVertices[2].z = aPosZ[2];

    aVertices[3].x = aPos2D[3].x;
    aVertices[3].y = aPos2D[3].y;
    aVertices[3].z = aPosZ[2];

    SetUpOneSkyPoly(
            aVertices[0], aVertices[1], aVertices[2], aVertices[3],
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyTopRed),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyTopGreen),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyTopBlue),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyBottomRed),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyBottomGreen),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyBottomBlue)
    );

    aVertices[0].z = aPosZ[2];
    aVertices[1].z = aPosZ[2];
    aVertices[2].z = aPosZ[3];
    aVertices[3].z = aPosZ[3];

    SetUpOneSkyPoly(
            aVertices[0], aVertices[1], aVertices[2], aVertices[3],
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyBottomRed),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyBottomGreen),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyBottomBlue),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyBottomRed),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyBottomGreen),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyBottomBlue)
    );

    aVertices[0].z = aPosZ[3];
    aVertices[1].z = aPosZ[3];
    aVertices[2].z = aPosZ[4];
    aVertices[3].z = aPosZ[4];

    SetUpOneSkyPoly(
            aVertices[0], aVertices[1], aVertices[2], aVertices[3],
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyBottomRed),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyBottomGreen),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyBottomBlue),
            belowHorizonGrey.red,
            belowHorizonGrey.green,
            belowHorizonGrey.blue
    );

    aVertices[0].z = aPosZ[1];
    aVertices[1].z = aPosZ[1];
    aVertices[2].x = aPos2D[0].x;
    aVertices[2].y = aPos2D[0].y;
    aVertices[2].z = aPosZ[0];
    aVertices[3].x = aPos2D[1].x;
    aVertices[3].y = aPos2D[1].y;
    aVertices[3].z = aPosZ[0];

    SetUpOneSkyPoly(
            aVertices[0], aVertices[1], aVertices[2], aVertices[3],
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyTopRed),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyTopGreen),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyTopBlue),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyTopRed),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyTopGreen),
            static_cast<uint8>(CTimeCycle::m_CurrentColours.m_nSkyTopBlue)
    );

    aVertices[0].z = aPosZ[4];
    aVertices[1].x = aPos2D[3].x;
    aVertices[1].y = aPos2D[3].y;
    aVertices[1].z = aPosZ[4];
    aVertices[2].z = aPosZ[5];
    aVertices[3].z = aPosZ[5];

    SetUpOneSkyPoly(
            aVertices[0], aVertices[1], aVertices[2], aVertices[3],
            belowHorizonGrey.red,
            belowHorizonGrey.green,
            belowHorizonGrey.blue,
            belowHorizonGrey.red,
            belowHorizonGrey.green,
            belowHorizonGrey.blue
    );

    CHook::CallFunction<void>("_ZN13CBrightLights23RenderOutGeometryBufferEv");
}

void CClouds::RenderBottomFromHeight() {
    /****
   * Code below should be good
   * but it isn't complete...
   ****** */

//    const auto camPos = TheCamera.GetPosition();
//    if (camPos.z < -90.f) { // 0x71557D [Moved up here]
//        return;
//    }
//
//    const auto& cc = CTimeCycle::m_CurrentColours; // cc = color component
//    const auto ClampClr = [](float clr) {
//        return std::min(clr, 255.f);
//    };
//    const auto fcClr = CRGBA{ // fc = fluffy clouds
//        (uint8)ClampClr(cc.m_nFluffyCloudsBottomRed * 2.f + 20.f),
//        (uint8)ClampClr(cc.m_nFluffyCloudsBottomGreen * 1.5f),
//        (uint8)ClampClr(cc.m_nFluffyCloudsBottomBlue * 1.5f),
//        (uint8)255
//    };
//
//    auto lowZ = 160.f, highZ = 190.f;
//
//    auto& windShift = StaticRef<float, 0xC6E954>();
//
//    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      RWRSTATE(TRUE));
//    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       RWRSTATE(TRUE));
//    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(TRUE));
//    RwRenderStateSet(rwRENDERSTATEFOGENABLE,         RWRSTATE(TRUE));
//    RwRenderStateSet(rwRENDERSTATESRCBLEND,          RWRSTATE(rwBLENDSRCALPHA));
//    RwRenderStateSet(rwRENDERSTATEDESTBLEND,         RWRSTATE(rwBLENDINVSRCALPHA));
//    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     RWRSTATE(RwTextureGetRaster(gpCloudMaskTex)));

    // TODO....
}

//
// -- VOLUMETRIC CLOUDS --
//

void CClouds::VolumetricCloudsInit() {
    ms_vc.quadNormal[0] = CVector(0.0f, 1.0f, 0.0f);
    ms_vc.quadNormal[1] = CVector(0.0f, 0.0f, 1.0f);
    ms_vc.quadNormal[2] = CVector(1.0f, 0.0f, 0.0f);

    ms_vc.modelX[0] = -0.5f;
    ms_vc.modelY[0] = 0.0f;
    ms_vc.modelZ[0] = 0.5f;
    ms_vc.modelU[0] = 0.0f;
    ms_vc.modelV[0] = 0.0f;

    ms_vc.modelX[1] = 0.5f;
    ms_vc.modelY[1] = 0.0f;
    ms_vc.modelZ[1] = 0.5f;
    ms_vc.modelU[1] = 1.0f;
    ms_vc.modelV[1] = 0.0f;

    ms_vc.modelX[2] = -0.5f;
    ms_vc.modelY[2] = 0.0f;
    ms_vc.modelZ[2] = -0.5f;
    ms_vc.modelU[2] = 0.0f;
    ms_vc.modelV[2] = 1.0f;

    ms_vc.modelX[3] = 0.5f;
    ms_vc.modelY[3] = 0.0f;
    ms_vc.modelZ[3] = 0.5f;
    ms_vc.modelU[3] = 1.0f;
    ms_vc.modelV[3] = 0.0f;

    ms_vc.modelX[4] = 0.5f;
    ms_vc.modelY[4] = 0.0f;
    ms_vc.modelZ[4] = -0.5f;
    ms_vc.modelU[4] = 1.0f;
    ms_vc.modelV[4] = 1.0f;

    ms_vc.modelX[5] = -0.5f;
    ms_vc.modelY[5] = 0.0f;
    ms_vc.modelZ[5] = -0.5f;
    ms_vc.modelU[5] = 0.0f;
    ms_vc.modelV[5] = 1.0f;

    ms_vc.modelX[6] = -0.5f;
    ms_vc.modelY[6] = 0.5f;
    ms_vc.modelZ[6] = 0.0f;
    ms_vc.modelU[6] = 0.0f;
    ms_vc.modelV[6] = 0.0f;

    ms_vc.modelX[7] = 0.5f;
    ms_vc.modelY[7] = 0.5f;
    ms_vc.modelZ[7] = 0.0f;
    ms_vc.modelU[7] = 1.0f;
    ms_vc.modelV[7] = 0.0f;

    ms_vc.modelX[8] = -0.5f;
    ms_vc.modelY[8] = -0.5f;
    ms_vc.modelZ[8] = 0.0f;
    ms_vc.modelU[8] = 0.0f;
    ms_vc.modelV[8] = 1.0f;

    ms_vc.modelX[9] = 0.5f;
    ms_vc.modelY[9] = 0.5f;
    ms_vc.modelZ[9] = 0.0f;
    ms_vc.modelU[9] = 1.0f;
    ms_vc.modelV[9] = 0.0f;

    ms_vc.modelX[10] = 0.5f;
    ms_vc.modelY[10] = -0.5f;
    ms_vc.modelZ[10] = 0.0f;
    ms_vc.modelU[10] = 1.0f;
    ms_vc.modelV[10] = 1.0f;

    ms_vc.modelX[11] = -0.5f;
    ms_vc.modelY[11] = -0.5f;
    ms_vc.modelZ[11] = 0.0f;
    ms_vc.modelU[11] = 0.0f;
    ms_vc.modelV[11] = 1.0f;

    ms_vc.modelX[12] = 0.0f;
    ms_vc.modelY[12] = -0.5f;
    ms_vc.modelZ[12] = 0.5f;
    ms_vc.modelU[12] = 0.0f;
    ms_vc.modelV[12] = 0.0f;

    ms_vc.modelX[13] = 0.0f;
    ms_vc.modelY[13] = 0.5f;
    ms_vc.modelZ[13] = 0.5f;
    ms_vc.modelU[13] = 1.0f;
    ms_vc.modelV[13] = 0.0f;

    ms_vc.modelX[14] = 0.0f;
    ms_vc.modelY[14] = -0.5f;
    ms_vc.modelZ[14] = -0.5f;
    ms_vc.modelU[14] = 0.0f;
    ms_vc.modelV[14] = 1.0f;

    ms_vc.modelX[15] = 0.0f;
    ms_vc.modelY[15] = 0.5f;
    ms_vc.modelZ[15] = 0.5f;
    ms_vc.modelU[15] = 1.0f;
    ms_vc.modelV[15] = 0.0f;

    ms_vc.modelX[16] = 0.0f;
    ms_vc.modelY[16] = 0.5f;
    ms_vc.modelZ[16] = -0.5f;
    ms_vc.modelU[16] = 1.0f;
    ms_vc.modelV[16] = 1.0f;

    ms_vc.modelX[17] = 0.0f;
    ms_vc.modelY[17] = -0.5f;
    ms_vc.modelZ[17] = -0.5f;
    ms_vc.modelU[17] = 0.0f;
    ms_vc.modelV[17] = 1.0f;

    for (auto i = 0u; i < MAX_VOLUMETRIC_CLOUDS; ++i) {
        ms_vc.bUsed[i] = false;
        ms_vc.bJustCreated[i] = 0;
    }
}

void CClouds::VolumetricClouds_Create(CVector *posn) {
    static CCamera &TheCamera = *reinterpret_cast<CCamera *>(g_libGTASA +
                                                             (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

    using CGeneral::GetRandomNumberInRange;

    const auto rand1To5 = GetRandomNumberInRange(1.f, 5.f);

    const auto AddVolumetricCloud = [
            randMinSizeXYZ = rand1To5 * 20.f,
            randMaxSizeXY = rand1To5 * 100.f,
            randMaxSizeZ = rand1To5 * 40.f
    ](int32 vcidx, CVector pos) {
        ms_vc.bUsed[vcidx] = true;
        ms_vc.bJustCreated[vcidx] = true;
        ms_vc.alpha[vcidx] = GetRandomNumberInRange<int32>(36, 128);

        ms_vc.size[vcidx] = CVector{
                GetRandomNumberInRange(randMinSizeXYZ, randMaxSizeXY),
                GetRandomNumberInRange(randMinSizeXYZ, randMaxSizeXY),
                GetRandomNumberInRange(randMinSizeXYZ, randMaxSizeZ),
        };

        ms_vc.pos[vcidx] = pos;
    };

    if (posn) {
        const auto randMinMaxOffsetXYZ = rand1To5 * 3.f;
        for (auto i = 0; i < 5; i++) {
            const auto vcidx = VolumetricClouds_GetFirstFreeSlot();
            if (vcidx == -1) {
                return;
            }
            const auto pos = *posn = *posn +
                                     CVector::Random(-randMinMaxOffsetXYZ, randMinMaxOffsetXYZ);
            AddVolumetricCloud(
                    vcidx,
                    pos
            );
        }
    } else {
        const auto camPos = TheCamera.GetPosition();
        const auto maxDist = m_fVolumetricCloudMaxDistance;
        for (auto i = 0; i < MAX_VOLUMETRIC_CLOUDS; i++) {
            AddVolumetricCloud(
                    i,
                    camPos + CVector{
                            GetRandomNumberInRange(-maxDist, maxDist),
                            GetRandomNumberInRange(-maxDist, maxDist),
                            GetRandomNumberInRange(-maxDist * 0.25f, maxDist * 0.25f),
                    }
            );
        }
    }
}

void CClouds::VolumetricClouds_Delete(int32 vcSlotIndex) {
    vcSlotIndex = std::clamp(vcSlotIndex, 0, MAX_VOLUMETRIC_CLOUDS - 1);
    ms_vc.bUsed[vcSlotIndex] = false;
    ms_vc.bJustCreated[vcSlotIndex] = false;
}

// unused
// inlined into VolumetricClouds_Create
int32 CClouds::VolumetricClouds_GetFirstFreeSlot() {
    for (auto i = 0u; i < m_VolumetricCloudsUsedNum; i++) {
        if (!ms_vc.bUsed[i]) {
            return (int32) i;
        }
    }
    return -1;
}

float CClouds::VolumetricCloudsGetMaxDistance() {
    return std::min(RwCameraGetFarClipPlane(Scene.m_pRwCamera), 600.f);
}

void CClouds::VolumetricCloudsRender() {
    static CCamera &TheCamera = *reinterpret_cast<CCamera *>(g_libGTASA +
                                                             (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

    const auto plyr = pGame.FindPlayerPed();
    m_fVolumetricCloudDensity = [] {
        switch (CHook::CallFunction<int64>("_ZN4Fx_c12GetFxQualityEv")) {
            case FX_QUALITY_LOW:
                return 0.5f;
            case FX_QUALITY_MEDIUM:
                return 1.f / 3.f;
            default:
                return 1.f;
        }
    }();
    m_VolumetricCloudsUsedNum = lerp(0, MAX_VOLUMETRIC_CLOUDS, m_fVolumetricCloudDensity);

    if (!m_VolumetricCloudsUsedNum) {
        return;
    }

    m_fVolumetricCloudMaxDistance = CClouds::VolumetricCloudsGetMaxDistance();

    const auto fadeOutBeginDist = m_fVolumetricCloudMaxDistance - 100.f;
    const auto fadeOutDist = m_fVolumetricCloudMaxDistance + 200.f;

    const auto camPos = TheCamera.GetPosition();
    float &gfVolumetricCloudFader = *reinterpret_cast<float *>(g_libGTASA + (VER_x32 ? 0xA25AE4 : 0xCC69F0));
    if (m_bVolumetricCloudHeightSwitch) {
        const auto delta = CTimer::GetTimeStep() * 4.f;
        if (camPos.z < 220.f) {
            gfVolumetricCloudFader += delta;
            if (gfVolumetricCloudFader >= 255.f) {
                gfVolumetricCloudFader = 255.f;
                return;
            }
        } else {
            gfVolumetricCloudFader = std::max(0.f, gfVolumetricCloudFader - delta);
        }
    } else {
        gfVolumetricCloudFader = 0.f;
    }
    CHook::CallFunction<int64>("_ZN12CPostEffects30ImmediateModeRenderStatesStoreEv");
    CHook::CallFunction<int64>("_ZN12CPostEffects28ImmediateModeRenderStatesSetEv");
    //CPostEffects::ImmediateModeRenderStatesStore();
    //CPostEffects::ImmediateModeRenderStatesSet();

    RwRenderStateSet(rwRENDERSTATEZTESTENABLE, RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER, RWRSTATE(RwTextureGetRaster(ms_pHighCloudTex)));
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, RWRSTATE(rwFILTERLINEAR));
    //pGame.FindPlayerPed()->m_pPed->GetMoveSpeed() = CVector{0.f, 0.f, 0.f};
    Log("1");
    const auto plyrPos = FindPlayerCoors();
    const auto plyrVeh = FindPlayerVehicle();
    //const auto plyrPos = CHook::CallFunction<float>("_Z15FindPlayerCoorsi");
    //const auto plyrVeh = CHook::CallFunction<int64>("_Z17FindPlayerVehicleib");
    Log("2");
    CVector &gVecCameraCoors = *reinterpret_cast<CVector *>(g_libGTASA + 0xCC69E4);
    CVector &gVecPlayerCoors = *reinterpret_cast<CVector *>(g_libGTASA + 0xCC69FC);
    //auto& gVecCameraCoors = StaticRef<CVector, 0xC6E964>();
    //auto& gVecPlayerCoors = StaticRef<CVector, 0xC6E958>();

    const auto bIsCameraOrPlayerPosNotStatic =
            (camPos != gVecCameraCoors) || (plyrPos != gVecPlayerCoors);

    gVecCameraCoors = camPos;
    gVecPlayerCoors = plyrPos;
    Log("3");
    if (bIsCameraOrPlayerPosNotStatic) {
        Log("8");
        CVector &gVecCameraCoors = *reinterpret_cast<CVector *>(g_libGTASA + 0xCC69E4);
        const auto tPos = gVecCameraCoors + CVector::Random(
                CVector{-200.f, -200.f, -50.f},
                CVector{200.f, 200.f, 50.f}
        );
        /*const auto t = plyrVeh ? (CPlaceable*)plyrVeh : (CPlaceable*)&TheCamera;
        Log("9");
        auto pos = (
                t->GetPosition()
                + t->GetForward() * fadeOutDist
                + CVector::Random(
                        CVector{-200.f, -200.f, -50.f},
                        CVector{ 200.f,  200.f, 50.f}
                )
        );*/
        Log("10");
        VolumetricClouds_Create(const_cast<CVector *>(&tPos));
    }
    Log("4");
    const auto &cc = CTimeCycle::m_CurrentColours;
    const auto vcClr = (uint8) std::min((
                                                cc.m_nSkyTopRed
                                                + cc.m_nSkyTopGreen
                                                + cc.m_nSkyTopBlue
                                                + cc.m_nSkyBottomRed
                                                + cc.m_nSkyBottomGreen
                                                + cc.m_nSkyBottomBlue
                                        ) / 6 + 64, 255);
    Log("5");
    if (!m_VolumetricCloudsUsedNum) {
        CHook::CallFunction<int64>("_ZN12CPostEffects32ImmediateModeRenderStatesReStoreEv");
        //CPostEffects::ImmediateModeRenderStatesReStore();
        return;
    }
    Log("6");
    RenderBuffer::ClearRenderBuffer();

    const auto RenderOutBuffer = [] {
        RenderBuffer::Render(rwPRIMTYPETRILIST, nullptr, rwIM3D_VERTEXXYZ | rwIM3D_VERTEXUV, false);
    };

    for (auto vcidx = 0u; vcidx < m_VolumetricCloudsUsedNum; vcidx++) {
        if (!ms_vc.bUsed[vcidx]) {
            continue;
        }

        auto &vcPos = ms_vc.pos[vcidx];
        auto &vcSz = ms_vc.size[vcidx];

        const auto vcDistToCam = (camPos - vcPos).Magnitude();

        vcPos += CVector{m_fVolumetricCloudWindMoveFactor * CVector2D{CWeather::WindDir}};

        if (!ms_vc.bJustCreated[vcidx] && vcDistToCam > m_fVolumetricCloudMaxDistance) {
            VolumetricClouds_Delete(vcidx);
            continue;
        }

        if (vcDistToCam <= m_fVolumetricCloudMaxDistance || fadeOutDist <= vcDistToCam) {
            ms_vc.bJustCreated[vcidx] = false;
        }

        auto vcAlpha = std::max(0u,
                                ms_vc.alpha[vcidx] - static_cast<uint32_t>(gfVolumetricCloudFader));
        if (vcDistToCam > fadeOutBeginDist) {
            if (!vcAlpha) {
                continue;
            }
            if (vcDistToCam > m_fVolumetricCloudMaxDistance) {
                continue;
            }
            const auto distAlpha = std::max<int32>(0, (int32) (
                    ((m_fVolumetricCloudMaxDistance - fadeOutBeginDist) -
                     (vcDistToCam - fadeOutBeginDist)) * (float) vcAlpha /
                    (m_fVolumetricCloudMaxDistance - fadeOutBeginDist)));
            vcAlpha = std::min<int32>(vcAlpha, distAlpha);
            if (!vcAlpha) {
                continue;
            }
        }

        const auto vcToCamDir = (vcPos - camPos).Normalized();

        CRGBA quadColors[3];
        for (auto i = 0; i < 3; i++) {
            quadColors[i] = {
                    vcClr,
                    vcClr,
                    vcClr,
                    (uint8) (std::abs(vcToCamDir.Dot(ms_vc.quadNormal[i])) * (float) vcAlpha)
            };
        }

        for (auto k = 0; k < 3 * 6; k++) {
            const auto quadIdx = k / 6;

            if (!RenderBuffer::CanFitVertices(3)) {
                RenderOutBuffer();
            }

            RenderBuffer::PushVertex(
                    vcPos + CVector{ms_vc.modelX[k], ms_vc.modelY[k], ms_vc.modelZ[k]} * vcSz,
                    CVector2D{ms_vc.modelU[k], ms_vc.modelV[k]},
                    quadColors[k / 6]
            );
        }
    }

    RenderOutBuffer();

    CHook::CallFunction<int64>("_ZN12CPostEffects32ImmediateModeRenderStatesReStoreEv");
    //CPostEffects::ImmediateModeRenderStatesReStore();
}

void CClouds::InjectHooks() {
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00679880 : 0x851120), &m_fVolumetricCloudDensity);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x0067913C : 0x8502A8), &m_bVolumetricCloudHeightSwitch);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x006799E4 : 0x8513E8), &m_fVolumetricCloudWindMoveFactor);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00678594 : 0x84EB50), &m_fVolumetricCloudMaxDistance);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00679994 : 0x851348), &m_VolumetricCloudsUsedNum);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x006785DC : 0x84EBE0), &ms_cameraRoll);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x006785E8 : 0x84EBF8), &IndividualRotation);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00676DB8 : 0x84BBD0), &ms_vc);

    CClouds::m_fWaterFXStartUnderWaterness = *reinterpret_cast<float *>(g_libGTASA + (VER_x32 ? 0x6B2220 : 0x88EC24));

    CClouds::CloudRotation = *reinterpret_cast<float *>(g_libGTASA + (VER_x32 ? 0x00A23DF4 : 0xCC4CF0));

    CClouds::ms_mf = *reinterpret_cast<CClouds::movingFog *>(g_libGTASA + (VER_x32 ? 0x00A254D0 : 0xCC63D8));

    CClouds::CurrentFogIntensity = *reinterpret_cast<float *>(g_libGTASA + (VER_x32 ? 0x006B15E8 : 0x88DFEC));

    // Clouds
    CHook::Redirect("_ZN7CClouds4InitEv", CClouds::Init);
    CHook::Redirect("_ZN7CClouds6UpdateEv", CClouds::Update);
    CHook::Redirect("_ZN7CClouds8ShutdownEv", CClouds::Shutdown);
    CHook::Redirect("_ZN7CClouds6RenderEv", CClouds::Render);
    CHook::Redirect("_ZN7CClouds14RenderSkyPolysEv", CClouds::RenderSkyPolys);
    CHook::Redirect("_ZN7CClouds22RenderBottomFromHeightEv", CClouds::RenderBottomFromHeight);

    // Moving fog
    CHook::Redirect("_ZN7CClouds13MovingFogInitEv", CClouds::MovingFogInit);
    CHook::Redirect("_ZN7CClouds16MovingFog_CreateEP7CVector", CClouds::MovingFog_Create);
    CHook::Redirect("_ZN7CClouds16MovingFog_DeleteEi", CClouds::MovingFog_Delete);
    CHook::Redirect("_ZN7CClouds16MovingFog_UpdateEv", CClouds::MovingFog_Update);
    CHook::Redirect("_ZN7CClouds24MovingFog_GetFXIntensityEv", CClouds::MovingFog_GetFXIntensity);
    CHook::Redirect("_ZN7CClouds17MovingFog_GetWindEv", CClouds::MovingFog_GetWind);
    CHook::Redirect("_ZN7CClouds26MovingFog_GetFirstFreeSlotEv",
                    CClouds::MovingFog_GetFirstFreeSlot);

    // Volumetric clouds
    CHook::Redirect("_ZN7CClouds20VolumetricCloudsInitEv", CClouds::VolumetricCloudsInit);
    CHook::Redirect("_ZN7CClouds23VolumetricClouds_CreateEP7CVector",
                    CClouds::VolumetricClouds_Create);
    CHook::Redirect("_ZN7CClouds23VolumetricClouds_DeleteEi", CClouds::VolumetricClouds_Delete);
    CHook::Redirect("_ZN7CClouds33VolumetricClouds_GetFirstFreeSlotEv",
                    CClouds::VolumetricClouds_GetFirstFreeSlot);
    CHook::Redirect("_ZN7CClouds30VolumetricCloudsGetMaxDistanceEv",
                    CClouds::VolumetricCloudsGetMaxDistance);
    CHook::Redirect("_ZN7CClouds22VolumetricCloudsRenderEv", CClouds::VolumetricCloudsRender);
}