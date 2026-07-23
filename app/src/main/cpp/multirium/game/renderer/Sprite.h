//
// Created by viski on 17.04.2025.
//

#ifndef X64_SPRITE_H
#define X64_SPRITE_H

#include "main.h"

class CSprite {
public:
    static void InjectHooks();

    static void InitSpriteBuffer();
    static void FlushSpriteBuffer();
    static void RenderOneXLUSprite_Rotate_Dimension(float ScreenX, float ScreenY, float ScreenZ, float SizeX, float SizeY, uint8 R, uint8 G, uint8 B, int16 Intensity16, float RecipZ, float Rotation, char Alpha);
    static void RenderBufferedOneXLUSprite(CVector pos, CVector2D size, uint8 r, uint8 g, uint8 b, int16 intensity, float recipNearZ, uint8 a11);

    static void
    RenderOneXLUSprite(CVector pos, CVector2D halfSize, uint8 r, uint8 g, uint8 b, int16 intensity,
                       float rhw, uint8 a, uint8 udir, uint8 vdir);

    static void
    RenderOneXLUSprite(float ScreenX, float ScreenY, float ScreenZ, float SizeX, float SizeY,
                       uint8_t R,
                       uint8_t G, uint8_t B, uint16_t Intensity16, float RecipZ, uint8_t Alpha,
                       bool FlipU, bool FlipV, float uvPad1, float uvPad2);

    static bool
    CalcScreenCoors(const RwV3d *In, RwV3d *pResult, float *pScaleX, float *pScaleY,
                    bool ClipFarPlane,
                    bool ClipNearPlane);
};


#endif //X64_SPRITE_H
