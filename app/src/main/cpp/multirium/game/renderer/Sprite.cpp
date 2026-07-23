//
// Created by viski on 17.04.2025.
//

#include "Sprite.h"
#include "patch.h"
#include "game/core/Camera.h"

bool CSprite::CalcScreenCoors(const RwV3d *In, RwV3d *pResult, float *pScaleX, float *pScaleY, bool ClipFarPlane, bool ClipNearPlane) {
    return CHook::CallFunction<bool>("_ZN7CSprite15CalcScreenCoorsERK5RwV3dPS0_PfS4_bb", In, pResult, pScaleX, pScaleY, ClipFarPlane, ClipNearPlane);
}

void CSprite::InitSpriteBuffer() {
    CHook::CallFunction<void>("_ZN7CSprite16InitSpriteBufferEv");
}

void CSprite::FlushSpriteBuffer() {
    CHook::CallFunction<void>("_ZN7CSprite17FlushSpriteBufferEv");
}

void CSprite::RenderBufferedOneXLUSprite(CVector pos, CVector2D size, uint8 r, uint8 g, uint8 b, int16 intensity, float recipNearZ, uint8 a11) {
    CHook::CallFunction<void>("_ZN7CSprite26RenderBufferedOneXLUSpriteEfffffhhhsfh",
                              pos,
                              size,
                              r, g, b, intensity,
                              recipNearZ,
                              a11);
}

void CSprite::RenderOneXLUSprite(CVector pos, CVector2D halfSize, uint8 r, uint8 g, uint8 b, int16 intensity, float rhw, uint8 a, uint8 udir, uint8 vdir) {
    CHook::CallFunction<void>(OBFUSCATE("_ZN7CSprite18RenderOneXLUSpriteEfffffhhhsfhhhff"),
                              halfSize,
                              r, g, b,
                              intensity,
                              rhw, a, udir,
                              vdir);
}

void CSprite::RenderOneXLUSprite(float ScreenX, float ScreenY, float ScreenZ, float SizeX, float SizeY, uint8_t R, uint8_t G, uint8_t B, uint16_t Intensity16, float RecipZ, uint8_t Alpha, bool FlipU, bool FlipV, float uvPad1, float uvPad2) {
    CHook::CallFunction<void>("_ZN7CSprite18RenderOneXLUSpriteEfffffhhhsfhhhff",
            ScreenX,
            ScreenY,
            ScreenZ,
            SizeX,
            SizeY,
            R,
            G,
            B,
            Intensity16,
            RecipZ,
            Alpha,
            FlipU,
            FlipV,
            uvPad1,
            uvPad2);
}

void CSprite::RenderOneXLUSprite_Rotate_Dimension(float ScreenX, float ScreenY, float ScreenZ, float SizeX, float SizeY, uint8 R, uint8 G, uint8 B, int16 Intensity16, float RecipZ, float Rotation, char Alpha) {
    //assert(false);
    CHook::CallFunction<void>("_ZN7CSprite43RenderBufferedOneXLUSprite_Rotate_DimensionEfffffhhhsffh", ScreenX, ScreenY, ScreenZ, SizeX, SizeY, R, G, B, Intensity16, RecipZ, Rotation, Alpha);
}

void CSprite::InjectHooks() {
    Log(__FUNCTION__);
}
