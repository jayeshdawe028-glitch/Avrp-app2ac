//
// Created by viski on 17.04.2025.
//

#ifndef X64_WEATHER_H
#define X64_WEATHER_H

#include "main.h"
#include "game/renderer/Clouds.h"

enum eWeatherRegion : int16 {
    WEATHER_REGION_DEFAULT = 0,
    WEATHER_REGION_LA = 1,
    WEATHER_REGION_SF = 2,
    WEATHER_REGION_LV = 3,
    WEATHER_REGION_DESERT = 4
};

class CWeather {
public:
    static inline uint8_t UnderWaterness;
    static inline float Wind;
    static inline float Sandstorm;
    static inline float Rain;
    static inline float InTunnelness;
    static inline float WaterDepth;
    static inline float ExtraSunnyness;
    static inline float Foggyness_SF;
    static inline float Foggyness;
    static inline float CloudCoverage;
    static inline float Rainbow;
    static inline CVector WindDir;

    static void InjectHooks();
};


#endif //X64_WEATHER_H
