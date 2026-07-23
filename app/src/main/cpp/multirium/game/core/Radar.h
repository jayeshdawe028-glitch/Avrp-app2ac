//
// Created by viski on 10.04.2025.
//

#ifndef X64_RADAR_H
#define X64_RADAR_H

#include "game.h"
#include "patch.h"

struct RadarLocalUsage {
     uint8 bUsed;
     uint8 bLoaded;
};

class CRadar {
public:
    static float TransformRadarPointToScreenSpace(CVector2D& out, CVector2D const& in);
    static void InjectHooks();

public: // br
    static float m_fMapZoom;
    static float m_fMapOffsetX;
    static float m_fMapOffsetY;
    static float m_radarRange;

    static float cachedCos;
    static float cachedSin;

    static bool m_bMenuMapActive;

    static CVector2D GetMapFullscreenPosFromRadarSpace(const CVector2D *posInput);

    static void DrawMapFullscreen();
    static void CalculateCachedSinCos();
    static void UpdateInputMapFullscreen();
    static void Initialise();
};


#endif //X64_RADAR_H
