//
// Created by x1y2z on 20.09.2023.
//

#ifndef LIVERUSSIA_TIMECYCLE_H
#define LIVERUSSIA_TIMECYCLE_H


#include "ColourSet.h"
#include "game/collision/Box.h"

class CTimeCycleBox {
public:
    CBox  m_Box;
    uint16_t m_FarClip;
    uint8_t m_LodDistMult;
    uint32_t m_ExtraColor;
    float m_Strength;
    float m_Falloff;
};

class CTimeCycle {
public:
    static void Update();
    static void CalcColoursForPoint(CVector point, CColourSet* set);
public:
    static inline CColourSet m_CurrentColours;
    static inline RwRGBA m_BelowHorizonGrey;

public:
    static void InjectHooks();
};


#endif //LIVERUSSIA_TIMECYCLE_H
