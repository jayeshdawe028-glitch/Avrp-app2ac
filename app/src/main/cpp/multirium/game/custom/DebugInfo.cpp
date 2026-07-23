//
// Created by viski on 06.05.2025.
//

#include "main.h"
#include "gui/gui.h"
#include "game/core/game.h"
#include "net/netgame.h"
#include "game/fakerw/RenderWare.h"
#include "playertags.h"
#include "settings.h"
#include "game/core/Timer.h"
#include "game/core/Streaming.h"
#include "net/netgame.h"
#include "game/core/textures/TextureDatabaseRuntime.h"
#include "DebugInfo.h"
#include "vendor/raknet/RakNetStatistics.h"
#include "java/CHud.h"

extern CNetGame* pNetGame;

extern CGUI* pGUI;
extern CGame *pGame;

bool CDebugInfo::m_uiDrawDebug = false;

void CDebugInfo::ToggleDebugDraw(bool toggle) {
    CDebugInfo::m_uiDrawDebug = toggle;
}

/*
// отключено временно
void format(char* out, ...)
{
    va_list va;
    va_start(va, out);

    va_list arg;
    va_copy(arg, va);

    vsnprintf(
            out,
            0x100u,
            "%s FPS: %.0f\n"
            "Packetloss: %d\n"
            "MEM: %d mb (stream %d/%d) (Tex %d MB)\n"
            "POS: %.2f, %.2f, %.2f",
            arg
    );

    va_end(arg);
    va_end(va);
}
*/

void CDebugInfo::Draw()
{
    // DEBUG DRAW ВРЕМЕННО ОТКЛЮЧЕН ПОЛНОСТЬЮ
    return;

    /*
    // скрыто по умолчанию — рисуем ТОЛЬКО когда включено вручную
    if(!CDebugInfo::m_uiDrawDebug)
        return;

    ImVec2 screenPos = ImVec2(40.0f, 1080.0f - pGUI->GetFontSize() * 11.0f);
    ImVec2 pos = ImVec2(
            pGUI->ScaleX(screenPos.x),
            pGUI->ScaleY(screenPos.y)
    );

    char debugText[256];

    struct mallinfo memInfo = mallinfo();

#if VER_x32
    const char* arch = "x32";
#else
    const char* arch = "x64";
#endif

    RakNetStatisticsStruct *s;

    format(
            debugText,
            arch,
            CTimer::game_FPS,
            100.0f * ( float ) s->messagesTotalBitsResent / ( float ) s->totalBitsSent,
            memInfo.uordblks >> 20,
            CStreaming::ms_memoryUsed >> 20,
            CStreaming::ms_memoryAvailable >> 20,
            TextureDatabaseRuntime::storedTexels >> 20,
            pGame->FindPlayerPed()->m_pPed->m_matrix->m_pos.x,
            pGame->FindPlayerPed()->m_pPed->m_matrix->m_pos.y,
            pGame->FindPlayerPed()->m_pPed->m_matrix->m_pos.z
    );

    pGUI->RenderText(pos, (ImU32)0xFFFFFFFF, true, &debugText[0]);
    */
}
