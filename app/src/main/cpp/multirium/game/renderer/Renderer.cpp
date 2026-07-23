//
// Created by viski on 18.04.2025.
//

#include "Renderer.h"
#include "vendor/armhook/patch.h"
#include "game/core/game.h"
#include "game/RW/VisibilityPlugins.h"
#include "game/modelinfo/ModelInfo.h"
#include "java/CPauseMenu.h"
#include "Shadows.h"

bool CRenderer::ms_bRenderTunnels;
bool CRenderer::ms_bRenderOutsideTunnels;
tRenderListEntry* CRenderer::ms_pLodDontRenderList;
tRenderListEntry* CRenderer::ms_pLodRenderList;
CVehicle* CRenderer::m_pFirstPersonVehicle;
CEntity* (CRenderer::ms_aInVisibleEntityPtrs)[MAX_INVISIBLE_ENTITY_PTRS];
CEntity* (CRenderer::ms_aVisibleSuperLodPtrs)[MAX_VISIBLE_SUPERLOD_PTRS];
CEntity* (CRenderer::ms_aVisibleLodPtrs)[MAX_VISIBLE_LOD_PTRS];
CEntity* (CRenderer::ms_aVisibleEntityPtrs)[MAX_VISIBLE_ENTITY_PTRS];

uint32_t CRenderer::ms_nNoOfVisibleSuperLods;
uint32_t CRenderer::ms_nNoOfInVisibleEntities;
uint32_t CRenderer::ms_nNoOfVisibleLods;
uint32_t CRenderer::ms_nNoOfVisibleEntities;

float CRenderer::ms_fFarClipPlane;
float CRenderer::ms_fCameraHeading;
bool CRenderer::m_loadingPriority;
bool CRenderer::ms_bInTheSky;

CVector CRenderer::ms_vecCameraPosition;
float CRenderer::ms_lodDistScale;
float CRenderer::ms_lowLodDistScale;

void CRenderer::Init() {
    // empty
}

void CRenderer::Shutdown() {
    // empty
}

void CRenderer::PreRender() {
   /* float v0; // w9
    uint64_t v1; // x21
    uint64_t v2; // x21
    uint64_t v3; // x21
    uint64_t v4; // x21
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *i; // x20
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *j; // x20

    FadeDistMult = 1.0;
    bool* RQCaps = reinterpret_cast<bool*>(g_libGTASA + 0x896130);
    if ( RQCaps[5] || RQCaps[4] || RQCaps[14] )
    {
        if ( CPauseMenu::Settings[0].value == 2 )
        {
            v0 = 0.6;
            goto LABEL_10;
        }
        if ( CPauseMenu::Settings[0].value == 1 )
        {
            v0 = 0.25;
            LABEL_10:
            FadeDistMult = v0;
            goto LABEL_11;
        }
    }
    else if ( CPauseMenu::Settings[0].value == 1 )
    {
        v0 = 0.5;
        goto LABEL_10;
    }
    if ( !CPauseMenu::Settings[0].value )
        FadeDistMult = 0.0;
    LABEL_11:
    if ( CRenderer::ms_nNoOfVisibleLods >= 1 )
    {
        v1 = 0LL;
        do
        {
            (*(void (__fastcall **)(uint64_t))(*(uint64_t *)CRenderer::ms_aVisibleLodPtrs[v1] + 144LL))(CRenderer::ms_aVisibleLodPtrs[v1]);
            ++v1;
        }
        while ( v1 < CRenderer::ms_nNoOfVisibleLods );
    }
    if ( CRenderer::ms_nNoOfVisibleEntities >= 1 )
    {
        v2 = 0LL;
        do
        {
            (*(void (__fastcall **)(uint64_t))(*(uint64_t *)CRenderer::ms_aVisibleEntityPtrs[v2] + 144LL))(CRenderer::ms_aVisibleEntityPtrs[v2]);
            ++v2;
        }
        while ( v2 < CRenderer::ms_nNoOfVisibleEntities );
    }
    if ( CRenderer::ms_nNoOfVisibleSuperLods >= 1 )
    {
        v3 = 0LL;
        do
        {
            (*(void (__fastcall **)(uint64_t))(*(uint64_t *)CRenderer::ms_aVisibleSuperLodPtrs[v3] + 144LL))(CRenderer::ms_aVisibleSuperLodPtrs[v3]);
            ++v3;
        }
        while ( v3 < CRenderer::ms_nNoOfVisibleSuperLods );
    }
    if ( CRenderer::ms_nNoOfInVisibleEntities >= 1 )
    {
        v4 = 0LL;
        do
        {
            (*(void (__fastcall **)(uint64_t))(*(uint64_t *)CRenderer::ms_aInVisibleEntityPtrs[v4] + 144LL))(CRenderer::ms_aInVisibleEntityPtrs[v4]);
            ++v4;
        }
        while ( v4 < CRenderer::ms_nNoOfInVisibleEntities );
    }*/
   /* for ( i = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaEntityList.m_firstLink.m_pNext;
          i != (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaEntityList.m_lastLink;
          i = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)i->m_firstLink.m_pNext )
    {
        if ( i->m_firstLink.item.RenderFn == CVisibilityPlugins::RenderEntity )
        {
            *((uint64_t *)i->m_firstLink.item.pObj + 5) &= ~0x20000uLL;
            (*(void (__fastcall **)(void *))(*(uint64_t *)i->m_firstLink.item.pObj + 144LL))(i->m_firstLink.item.pObj);
        }
    }
    for ( j = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstLink.m_pNext;
          j != (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaUnderwaterEntityList.m_lastLink;
          j = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)j->m_firstLink.m_pNext )
    {
        if ( j->m_firstLink.item.RenderFn == CVisibilityPlugins::RenderEntity )
        {
            *((uint64_t *)j->m_firstLink.item.pObj + 5) &= ~0x20000uLL;
            (*(void (__fastcall **)(void *))(*(uint64_t *)j->m_firstLink.item.pObj + 144LL))(j->m_firstLink.item.pObj);
        }
    }*/
   // if ( /*MobileSettings::settings[5].value != 2*/CPauseMenu::Settings[5].value != 2 )
        //CShadows::RenderExtraPlayerShadows();
}

void CRenderer::RenderOneNonRoad(CEntity* pEntity)
{
    CHook::CallFunction<void>("_ZN9CRenderer16RenderOneNonRoadEP7CEntity", pEntity);
}

void CRenderer::RenderFadingInEntities()
{
    RwRenderStateSet(rwRENDERSTATEFOGENABLE,         RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATECULLMODE,          RWRSTATE(rwCULLMODECULLBACK));
   // DeActivateDirectional();
   // SetAmbientColours();
    ((void(*)())(g_libGTASA + (VER_x32 ? 0x5D1F98 + 1 : 0x6F6664)))(); // -- DeActivateDirectional
    ((void(*)())(g_libGTASA + (VER_x32 ? 0x5D2068 + 1 : 0x6F6738)))(); // -- SetAmbientColours
    CVisibilityPlugins::RenderFadingEntities();
}

#include "game/control/Scene.h"

void CRenderer::RenderEverythingBarRoads()
{
    CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

    RwRenderStateSet(rwRENDERSTATEFOGENABLE,         RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATECULLMODE,          RWRSTATE(rwCULLMODECULLBACK));
    if ( !CGame::currArea )
        RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, RWRSTATE(140u));

    assert(ms_nNoOfVisibleEntities <= MAX_VISIBLE_ENTITY_PTRS);
    for (auto& entity : GetVisibleEntityPtrs()) {
        auto* vehicle = entity->AsVehicle();
        if (entity->IsBuilding() && CModelInfo::GetModelInfo(entity->m_nModelIndex)->IsRoad())
            continue;

        bool bInserted = false;
        if (entity->IsVehicle() || (entity->IsPed() && CVisibilityPlugins::GetClumpAlpha(entity->m_pRwClump) != 255)) {
            // todo: R* nice check | or we missed smth here?
            if (entity->IsVehicle()) {
                bool bInsertIntoSortedList = false;
                if (vehicle->IsBoat()) {
                    const auto& camMode = CCamera::GetActiveCamera().m_nMode;
                    const auto& lookDirection = TheCamera.GetLookDirection();
                    if (camMode == MODE_WHEELCAM || camMode == MODE_1STPERSON &&
                                                    lookDirection != LOOKING_DIRECTION_FORWARD && lookDirection != LOOKING_DIRECTION_UNKNOWN_1 ||
                        CVisibilityPlugins::GetClumpAlpha(entity->m_pRwClump) != 255
                            )
                    {
                        bInsertIntoSortedList = true;
                    }
                }
                else if (!vehicle->physicalFlags.bTouchingWater) {
                    bInsertIntoSortedList = true;
                }
                const float fMagnitude = DistanceBetweenPoints(entity->GetPosition(), ms_vecCameraPosition);
                if (bInsertIntoSortedList)
                    bInserted = CVisibilityPlugins::InsertEntityIntoSortedList(entity, fMagnitude);
                else
                    bInserted = CVisibilityPlugins::InsertEntityIntoUnderwaterList(entity, fMagnitude);
            }
        }
        if (!bInserted)
            RenderOneNonRoad(entity);
    }
    float oldzShift = Scene.m_pRwCamera->zShift;
    RwCameraEndUpdate(Scene.m_pRwCamera);

    Scene.m_pRwCamera->zShift -= 100.0f;
    RwCameraBeginUpdate(Scene.m_pRwCamera);
    for (auto& entity : GetVisibleLodPtrs()) {
        RenderOneNonRoad(entity);
    }
    RwCameraEndUpdate(Scene.m_pRwCamera);

    Scene.m_pRwCamera->zShift = oldzShift;
    RwCameraBeginUpdate(Scene.m_pRwCamera);
}

void CRenderer::InjectHooks() {
  //  CHook::Write(g_libGTASA + 0xBCF8E8, &ms_nNoOfVisibleLods);
   // CHook::Write(g_libGTASA + 0xBD1840, &ms_aVisibleLodPtrs);
   // CHook::Write(g_libGTASA + 0xBCF8F0, &ms_nNoOfVisibleSuperLods);
  // CHook::Write(g_libGTASA + 0xBD3780, &ms_aVisibleSuperLodPtrs);
  //  CHook::Write(g_libGTASA + 0xBCF8EC, &ms_nNoOfInVisibleEntities);
   // CHook::Write(g_libGTASA + 0xBD3940, &ms_aInVisibleEntityPtrs);
   // CHook::Write(g_libGTASA + 0xBCF8F0, &ms_nNoOfVisibleSuperLods);

   // CHook::Redirect("_ZN9CRenderer24RenderEverythingBarRoadsEv", &CRenderer::RenderEverythingBarRoads);

    CHook::Write(g_libGTASA + (VER_x32 ? 0x6764D0 : 0x84AA10), &ms_bRenderOutsideTunnels);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x67914C : 0x8502C8), &m_loadingPriority);

    CHook::Write(g_libGTASA + (VER_x32 ? 0x6778EC : 0x84D210), &ms_aVisibleEntityPtrs);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x6771F0 : 0x84C428), &ms_nNoOfVisibleEntities);
}
