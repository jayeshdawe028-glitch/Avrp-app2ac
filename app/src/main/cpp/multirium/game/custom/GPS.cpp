//
// Created by viski on 10.04.2025.
//

#include "GPS.h"
#include "game/peds/playerped.h"
#include "game.h"
#include "gui/gui.h"
#include "CUtil.h"
#include "net/netgame.h"
#include "Radar.h"
#include "game/core/widgets/TouchInterface.h"
#include "Streaming.h"
#include "game/control/PathFind.h"
#include "game/mobilemenu/MobileMenu.h"

extern CGame *pGame;
extern CGUI *pGUI;
extern CNetGame *pNetGame;

RwTexture* marker;

CVector GPS::to;
CNodeAddress resultNodes[MAX_NODE_POINTS];
//uint16_t resultNodes[MAX_NODE_POINTS];
CVector2D nodePoints[MAX_NODE_POINTS];
RwIm2DVertex lineVerts[MAX_NODE_POINTS * 4] {0};
bool GPS::enabled = false;

float* NearScreenZ;
float* RecipNearClip;

void Setup2DVertex(RwD3D9Vertex vertex, float x, float y, RwUInt32 color)
{
    vertex.x = x;
    vertex.y = y;
    vertex.z = *NearScreenZ + 0.0001f;
    vertex.rhw = *RecipNearClip;
    vertex.emissiveColor = color;
    vertex.u = 0.0f;
    vertex.v = 0.0f;
}

inline bool LaneDirectionRespected()
{
    return false;
}

inline bool IsRadarVisible()
{
    CWidgetGta *radar = CTouchInterface::m_pWidgets[0xA1];
    return (radar != NULL && radar->m_bEnabled);
}

float lineWidth = 3.5f, textOffset, textScale, flMenuMapScaling;
CRect emptyRect, radarRect;
unsigned int gpsLineColor = RWRGBALONG(GPS_LINE_R, GPS_LINE_G, GPS_LINE_B, GPS_LINE_A);

int* ThePaths;

void GPS::DoPathDraw() {
    if(!GPS::enabled) return;

    if(!marker) return;

    if(!pNetGame) return;

    CLocalPlayer* pLocalPlayer = pNetGame->GetPlayerPool()->GetLocalPlayer();
    if(!pLocalPlayer) return;

    CPlayerPed* pPlayerPed = pLocalPlayer->GetPlayerPed();
    if(!pPlayerPed) return;
/*
    short nodesCount = 0;
    bool isGamePaused = CTimer::m_CodePause || CTimer::m_UserPause;
    CVector playerPos;
    playerPos.x = pGame->FindPlayerPed()->m_pPed->m_matrix->m_pos.x;
    playerPos.y = pGame->FindPlayerPed()->m_pPed->m_matrix->m_pos.y;
    playerPos.z = pGame->FindPlayerPed()->m_pPed->m_matrix->m_pos.z;

    CHook::CallFunction<int64>("_ZN9CPathFind12DoPathSearchEh7CVector12CNodeAddressS0_PS1_PsiPffS2_fbS1_bb",
                                 (uintptr_t)ThePaths,
                                 0,
                                 0xFFFFFFFFLL,
                                 resultNodes,
                                 &nodesCount,
                                 MAX_NODE_POINTS,
                                 0,
                                 0,
                                 playerPos.x,
                                 playerPos.y,
                                 playerPos.z,
                                 GPS::to.x,
                                 GPS::to.y,
                                 GPS::to.z,
                                 1000000.0f,
                                 1000000.0f,
                                 0,
                                 0xFFFFFFFFLL,
                                 0,
                                 0
    );

    Log("nodesCount: %d", nodesCount);

    if (nodesCount >= 1) {
        for (int i = 0; i < nodesCount; ++i)
        {
            Log("cheto");
            auto* node = (CPathNode*)(ThePaths[513 + resultNodes[i].m_wAreaId] + 28 * resultNodes[i].m_wNodeId);
            CVector2D nodePos = node->GetPosition();
            CHook::CallFunction<float>("_ZN6CRadar35TransformRealWorldPointToRadarSpaceER9CVector2DRKS0_", nodePos, nodePos);
            if (!isGamePaused) {
                CRadar::TransformRadarPointToScreenSpace(nodePoints[i], nodePos);
            } else {
                CHook::CallFunction<float*>("_ZN6CRadar15LimitRadarPointER9CVector2D", nodePos);
                CRadar::TransformRadarPointToScreenSpace(nodePoints[i], nodePos);
                nodePoints[i].x *= flMenuMapScaling;
                nodePoints[i].y *= flMenuMapScaling;
            }
        }
        Log("otrisovka");
        if(IsRadarVisible() || isGamePaused) {
            if (!isGamePaused) SetScissorRect(&radarRect);
            RwRenderStateSet(rwRENDERSTATETEXTURERASTER, NULL);
            unsigned int vertIndex = 0;
            --nodesCount;
            for (short i = 0; i < nodesCount; i++)
            {
                CVector2D point[4], shift[2];
                CVector2D dir = nodePoints[i + 1] - nodePoints[i];
                float angle = atan2(dir.y, dir.x);
                if (!isGamePaused)
                {
                    shift[0].x = cosf(angle - 1.5707963f) * lineWidth;
                    shift[0].y = sinf(angle - 1.5707963f) * lineWidth;
                    shift[1].x = cosf(angle + 1.5707963f) * lineWidth;
                    shift[1].y = sinf(angle + 1.5707963f) * lineWidth;
                }
                else
                {
                    float mp = 200.0f - 140.0f;
                    if (mp < 140.0f) mp = 140.0f;
                    else if (mp > 960.0f) mp = 960.0f;
                    mp = mp / 960.0f + 0.4f;

                    shift[0].x = cosf(angle - 1.5707963f) * lineWidth * mp;
                    shift[0].y = sinf(angle - 1.5707963f) * lineWidth * mp;
                    shift[1].x = cosf(angle + 1.5707963f) * lineWidth * mp;
                    shift[1].y = sinf(angle + 1.5707963f) * lineWidth * mp;
                }
                Setup2DVertex(lineVerts[vertIndex], nodePoints[i].x + shift[0].x, nodePoints[i].y + shift[0].y, gpsLineColor);
                Setup2DVertex(lineVerts[++vertIndex], nodePoints[i + 1].x + shift[0].x, nodePoints[i + 1].y + shift[0].y, gpsLineColor);
                Setup2DVertex(lineVerts[++vertIndex], nodePoints[i].x + shift[1].x, nodePoints[i].y + shift[1].y, gpsLineColor);
                Setup2DVertex(lineVerts[++vertIndex], nodePoints[i + 1].x + shift[1].x, nodePoints[i + 1].y + shift[1].y, gpsLineColor);
                ++vertIndex;
            }
            RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, lineVerts, 4 * nodesCount);
            if (!isGamePaused) SetScissorRect(&emptyRect);
        }
    }
*/
    CVector TagPos;

    TagPos = GPS::to;

    TagPos.z += 0.25f + (pPlayerPed->m_pPed->GetDistanceFromCamera() * 0.0475f);

    CVector Out;
    // CSprite::CalcScreenCoors
    ((void (*)(CVector *, CVector *, float *, float *, bool, bool)) (g_libGTASA + (VER_x32 ? 0x005C57E8 + 1 : 0x6E9DF8)))(
            &TagPos, &Out, 0, 0, 0, 0);

    if (Out.z < 1.0f)
        return;

    ImVec2 pos = ImVec2(Out.x, Out.y);
    pos.x -= 70.0f / 2;
    pos.y -= 70.0f / 2;

    ImVec2 a = ImVec2(pos.x, pos.y);
    ImVec2 b = ImVec2(pos.x + 70.0f ,
                      pos.y + 70.0f);
    ImGui::GetForegroundDrawList()->AddImage((ImTextureID) marker->raster, a, b);

    char szStr[30];

    snprintf(&szStr[0], 30, "%d М", (int)pPlayerPed->m_pPed->GetDistanceFromPoint(GPS::to.x, GPS::to.y, GPS::to.z));
    ImVec2 vpos = ImVec2(Out.x - strlen(szStr) * (pGUI->GetFontSize() / 2.0f) / 2, pos.y + 70.0f + 4.0f);
    pGUI->RenderText(vpos, (ImU32)0xFFFFFFFF, true, &szStr[0]);
}

void GPS::Set(CVector pos, bool toggle) {
    GPS::to = pos;
    GPS::enabled = toggle;

    if(!marker)
        marker = CUtil::LoadTextureFromDB("txd", "radar_waypoint");
}

void GPS::injectHooks() {
    NearScreenZ = reinterpret_cast<float *>(g_libGTASA + (VER_x32 ? 0x00675F8C : 0x849FA0));
    RecipNearClip = reinterpret_cast<float*>(g_libGTASA + (VER_x32 ? 0x006766AC : 0x84ADC8));
    ThePaths = reinterpret_cast<int*>(CHook::getSym("ThePaths"));
}
