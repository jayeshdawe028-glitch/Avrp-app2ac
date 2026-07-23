/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "game/core/common.h"
#include "game/core/core/Quaternion.h"
// Fuck Rw Types
struct RpHAnimBlendInterpFrame {
    CQuaternion orientation;
    CVector     translation;
};
VALIDATE_SIZE(RpHAnimBlendInterpFrame, 0x1C);
