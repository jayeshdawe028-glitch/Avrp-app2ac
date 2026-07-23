//
// Created by viski on 17.04.2025.
//

#include "Weather.h"
#include "main.h"
#include "patch.h"

void CWeather::InjectHooks() {
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00A7D16C : 0xD21728), &Wind);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00A7D168 : 0xD21724), &Sandstorm);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00A7D164 : 0xD21720), &Rain);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00A7D160 : 0xD2171C), &InTunnelness);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x006760FC : 0x84A278), &UnderWaterness);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00A7D15C : 0xD21718), &WaterDepth);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00A7D154 : 0xD21710), &ExtraSunnyness);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00A7D150 : 0xD2170C), &Foggyness_SF);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00A7D14C : 0xD21708), &Foggyness);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00A7D148 : 0xD21704), &CloudCoverage);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x00A7D170 : 0xD2172C), &WindDir);
}
