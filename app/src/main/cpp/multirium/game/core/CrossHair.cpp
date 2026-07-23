//
// Created by x1y2z on 10.11.2023.
//

#include "CrossHair.h"
#include "common.h"
#include "util/CUtil.h"
#include "main.h"
#include "vendor/armhook/patch.h"
#include "Camera.h"
#include "game/core/core/Quaternion.h"
#include "game/peds/playerped.h"
#include "game/vehicles/vehicle.h"
#include "game/vehicles/vehicle.h"
#include "game/entity/Physical.h"
#include "game/net/ObjectMP.h"
#include "aimstuff.h"
#include "net/netgame.h"

extern CNetGame *pNetGame;

void CCrossHair::Init()
{
    Log("CCrossHair::Init");
    pCircleTex = new CSprite2d();
    pCircleTex->m_pTexture = CUtil::LoadTextureFromDB("txd", "siteM16");
    Log("CCrossHair::Init 2");
}

void CCrossHair::Render()
{
    if(!pNetGame || pNetGame->GetPlayerPool())
        return;

    auto pPed = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();

    int iCamState = GameGetLocalPlayerCameraMode();

    int v1 = iCamState & 0xFFFD;
    if(iCamState == 53 || iCamState == 39 || v1 == 40) //CPlayerPed::GetWeaponRadiusOnScreen(void)	00000000005C4528
    {
        static float fCHairScreenMultX = (RsGlobal->screenWidth - (RsGlobal->screenHeight / 9 * 16)) / 2 + ((RsGlobal->screenHeight / 9 * 16) * 0.524);
        static float fFixedOffset = RsGlobal->screenWidth / (RsGlobal->screenWidth - (RsGlobal->screenHeight / 9 * 16)) * 2.0;
        auto gunRadius = CHook::CallFunction<float>(g_libGTASA + (VER_x32 ? 0x004C69E8 + 1:0x5C4528), pPed->m_pPed);
        static float fCHairScreenMultY = (RsGlobal->screenHeight / 9 * 16) / 10 * 6 * 0.4 + fFixedOffset;

        RwRenderStateSet(rwRENDERSTATETEXTUREFILTER, RWRSTATE(rwFILTERLINEAR));
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, RWRSTATE(FALSE));

        float f1 = ((RsGlobal->screenHeight / 448.0) * 64.0) * gunRadius;

        float fPosX1 = ((f1 * 0.5) + fCHairScreenMultX) - f1;
        float fPosY1 = ((f1 * 0.5) + fCHairScreenMultY) - f1;

        float fPosX2 = (f1 * 0.5) + fPosX1;
        float fPosY2 = (f1 * 0.5) + fPosY1;

        pCircleTex->Draw(CRect{fPosX1, fPosY1, fPosX2, fPosY2}, CRGBA(255, 255, 255, 255));
        pCircleTex->Draw(CRect{fPosX1 + f1, fPosY1, fPosX2, fPosY2}, CRGBA(255, 255, 255, 255));
        pCircleTex->Draw(CRect{fPosX1, fPosY1 + f1, fPosX2, fPosY2}, CRGBA(255, 255, 255, 255));
        pCircleTex->Draw(CRect{fPosX1 + f1, fPosY1 + f1, fPosX2, fPosY2}, CRGBA(255, 255, 255, 255));
        return;
    }
}