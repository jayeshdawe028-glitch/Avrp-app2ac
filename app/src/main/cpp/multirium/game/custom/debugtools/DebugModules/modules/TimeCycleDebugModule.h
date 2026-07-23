#pragma once

#include "../DebugModule.h"

class TimeCycleDebugModule final : public DebugModule {
public:
    void RenderWindow() override final;
    void Render3D() override final;
    void RenderMenuEntry() override final;

private:
    void SyncFromGame();
    void RenderBoxesGUI();

private:
    struct Color3 {
        float r, g, b;
    };

    struct Color4 : Color3 {
        float a;
    };

private:
     int    m_OldWeatherTypeId{};
     int    m_NewWeatherType{};
     int    m_Hours{};
     int    m_Minutes{};
     Color3 m_CurrentRGB1{};
     Color3 m_CurrentRGB2{};
     Color3 m_AmbRGB{};
     Color3 m_AmbObjRGB{};
     Color3 m_SkyTopRGB{};
     Color3 m_SkyBottomRGB{};
     Color3 m_SunCoreRGB{};
     Color3 m_SunCoronaRGB{};
     uint32_t  m_SunSize{};
     Color4 m_BelowHorizonGrey{};
     uint32_t  m_SpriteSize{};
     uint32_t  m_SpriteBrightness{};
     uint32_t  m_ShadowStrength{};
     uint32_t  m_PoleShadowStrength{};
     uint32_t  m_LightShadowStrength{};
     uint32_t  m_FarClip{};
     uint32_t  m_FogStart{};
     uint32_t  m_LightsOnGroundBrightness{};
     Color3 m_LowCloudsRGB{};
     Color3 m_FluffyCloudsBottom{};
     Color4 m_WaterRGBA{};
     Color4 m_PostFx1RGBA{};
     Color4 m_PostFx2RGBA{};
     uint32_t  m_CloudAlpha{};
     uint32_t  m_HighLightMinIntensity{};
     uint32_t  m_WaterFogAlpha{};
     uint32_t  m_DirectionalMult{};
     Color3 m_BrightnessAddedToAmbient{};
     uint32_t  m_TimeId{};
     uint32_t  m_WeatherId{};
     bool   m_ShowBoxes{};
     bool   m_IsOpen{};
};
