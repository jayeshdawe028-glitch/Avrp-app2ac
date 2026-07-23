//
// fixed by cross on 21.01.2026.
//

#include "Radar.h"
#include "net/netgame.h"
#include "World.h"
#include "game/custom/GPS.h"

#include <atomic>
#include <cstdint>

extern CNetGame* pNetGame;
extern CGame* pGame;

class CRunningScript;

static std::atomic<int>       g_LastCmd300To399{-1};
static std::atomic<uintptr_t> g_LastScriptThis{0};

static int (*CRunningScript_Process300To399)(CRunningScript* thiz, int cmd) = nullptr;
static int CRunningScript_Process300To399_hook(CRunningScript* thiz, int cmd)
{
    g_LastCmd300To399.store(cmd, std::memory_order_relaxed);
    g_LastScriptThis.store((uintptr_t)thiz, std::memory_order_relaxed);
    return CRunningScript_Process300To399 ? CRunningScript_Process300To399(thiz, cmd) : 0;
}

float CRadar::m_fMapZoom = 2.0f;
float CRadar::m_fMapOffsetX = 0.0f;
float CRadar::m_fMapOffsetY = 0.0f;
float CRadar::m_radarRange = 350.0f;
bool  CRadar::m_bMenuMapActive = false;

float CRadar::cachedCos = 1.0f;
float CRadar::cachedSin = 0.0f;

void CRadar::Initialise() {}

CVector2D CRadar::GetMapFullscreenPosFromRadarSpace(const CVector2D* posInput)
{
    CVector2D result{};
    if (!posInput || !RsGlobal) return result;

    float v1 = (CRadar::m_fMapZoom * ((float)RsGlobal->screenWidth / 14.0f)) * 14.0f;
    float v2 = (v1 - (float)RsGlobal->screenWidth) * -0.5f;
    float v3 = v2 + (CRadar::m_fMapOffsetY * CRadar::m_fMapZoom);

    float v4 = ((((posInput->x * CRadar::m_radarRange) + 3500.0f) / 7000.0f) * v1)
               + (v2 + (CRadar::m_fMapOffsetX * CRadar::m_fMapZoom));

    float v5 = (((7000.0f - ((posInput->y * CRadar::m_radarRange) + 3500.0f)) / 7000.0f) * v1) + v3;

    result.x = v4;
    result.y = v5;
    return result;
}

void CRadar::CalculateCachedSinCos()
{
    if (CRadar::m_bMenuMapActive) {
        CRadar::cachedSin = 0.0f;
        CRadar::cachedCos = 1.0f;
    }
}

float CRadar::TransformRadarPointToScreenSpace(CVector2D& out, CVector2D const& in)
{
    return CHook::CallFunction<float>(
            "_ZN6CRadar32TransformRadarPointToScreenSpaceER9CVector2DRKS0_",
            out,
            in
    );
}

static uint32_t (*CRadar__GetRadarTraceColor)(uint32_t color, uint8_t bright, uint8_t friendly) = nullptr;
static uint32_t CRadar__GetRadarTraceColor_hook(uint32_t color, uint8_t bright, uint8_t friendly)
{
    (void)bright;
    (void)friendly;
    return TranslateColorCodeToRGBA(color);
}

static int (*CRadar__SetCoordBlip)(int blipType, CVector pos, uint32_t arg3, int blipDisplay, char* name) = nullptr;
static std::atomic<int32_t> g_LastWaypointHandle{-1};

static int CRadar__SetCoordBlip_hook(int blipType, CVector pos, uint32_t arg3, int blipDisplay, char* name)
{
    int blipId = CRadar__SetCoordBlip
                 ? CRadar__SetCoordBlip(blipType, pos, arg3, blipDisplay, name)
                 : 0;

    if (blipType == 4 && blipDisplay == 2) {
        g_LastWaypointHandle.store((int32_t)blipId, std::memory_order_relaxed);
    }

    if (pNetGame && name && !strncmp(name, "CODEWAY", 7))
    {
        float findZ = CWorld::FindGroundZForCoord(pos.x, pos.y) + 1.5f;

        CVector gpsPos = { pos.x, pos.y, findZ - 1.5f };
        GPS::Set(gpsPos, true);

        if (pNetGame->GetGameState() == GAMESTATE_CONNECTED && pNetGame->GetRakClient())
        {
            RakNet::BitStream bsSend;
            bsSend.Write(pos.x);
            bsSend.Write(pos.y);
            bsSend.Write(findZ);

            pNetGame->GetRakClient()->RPC(
                    &RPC_MapMarker,
                    &bsSend,
                    HIGH_PRIORITY,
                    RELIABLE,
                    0,
                    false,
                    UNASSIGNED_NETWORK_ID,
                    nullptr
            );
        }
    }

    return blipId;
}

static void (*CRadar_DrawRadarGangOverlay)(bool unk) = nullptr;
static void CRadar_DrawRadarGangOverlay_hook(bool unk)
{
    if (pNetGame)
    {
        CGangZonePool* pGangZonePool = pNetGame->GetGangZonePool();
        if (pGangZonePool) {
            pGangZonePool->Draw((uint32_t)unk);
        }
    }
}

static void (*CRadar_ClearBlip)(int32_t index) = nullptr;

static void CRadar_ClearBlip_hook(int32_t index)
{
    auto safeCall = [&](int32_t v)
    {
        if (CRadar_ClearBlip) CRadar_ClearBlip(v);
    };

    constexpr int32_t SAFE_HANDLE_MAX = 0x100000;

    if (index >= 0 && index < SAFE_HANDLE_MAX)
    {
        safeCall(index);
        return;
    }

    int lastCmd = g_LastCmd300To399.load(std::memory_order_relaxed);

    if (lastCmd == 356 || lastCmd == 357)
    {
        int32_t wp = g_LastWaypointHandle.load(std::memory_order_relaxed);
        if (wp >= 0 && wp < SAFE_HANDLE_MAX) {
            safeCall(wp);
            return;
        }
    }
}

void CRadar::InjectHooks()
{
    CHook::InlineHook(
            "_ZN14CRunningScript23ProcessCommands300To399Ei",
            (void*)CRunningScript_Process300To399_hook,
            (void**)&CRunningScript_Process300To399
    );

    CHook::InlineHook(
            "_ZN6CRadar9ClearBlipEi",
            (void*)CRadar_ClearBlip_hook,
            (void**)&CRadar_ClearBlip
    );

    CHook::InlineHook(
            "_ZN6CRadar20DrawRadarGangOverlayEb",
            (void*)CRadar_DrawRadarGangOverlay_hook,
            (void**)&CRadar_DrawRadarGangOverlay
    );

    CHook::InlineHook(
            "_ZN6CRadar12SetCoordBlipE9eBlipType7CVectorj12eBlipDisplayPc",
            (void*)CRadar__SetCoordBlip_hook,
            (void**)&CRadar__SetCoordBlip
    );

    CHook::InlineHook(
            "_ZN6CRadar19GetRadarTraceColourEjhh",
            (void*)CRadar__GetRadarTraceColor_hook,
            (void**)&CRadar__GetRadarTraceColor
    );
}
