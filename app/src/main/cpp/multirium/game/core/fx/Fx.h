//
// Created by viski on 20.04.2025.
//
#ifndef X64_FX_H
#define X64_FX_H

#include "main.h"
#include "game/fakerw/RenderWare.h"

enum FxQuality_e : uint32 {
    FX_QUALITY_LOW = 0,
    FX_QUALITY_MEDIUM = 1,
    FX_QUALITY_HIGH = 2,
    FX_QUALITY_VERY_HIGH = 3,
};

enum eSparkType : uint8 {
    SPARK_PARTICLE_SPARK2 = 0,
    SPARK_PARTICLE_SPARK  = 1
};

class Fx_c {
public:
    static void InjectHooks();
};

#endif //X64_FX_H