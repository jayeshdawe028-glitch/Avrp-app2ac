//
// Created by viski on 01.05.2025.
//

#ifndef MULTIRIUM_RENDERER_H
#define MULTIRIUM_RENDERER_H

#include "game/core/common.h"
#include "game/core/core/PtrListDoubleLink.h"
#include "game/core/core/PtrListSingleLink.h"
#include "game/entity/Entity.h"

#include "game/vehicles/vehicle.h"
#include "game/buildings/Building.h"
#include "game/peds/CopPed.h"
#include "game/entity/Dummy.h"
#include "game/objects/Object.h"

class CVehicle;
class CBaseModelInfo;

class CBuilding;
class CObject;
class CVehicle;
class CPed;
class CDummy;

enum eRendererVisibility {
    RENDERER_INVISIBLE = 0,
    RENDERER_VISIBLE,
    RENDERER_CULLED,
    RENDERER_STREAMME
};

//struct tScanLists {
    /*CPtrListSingleLink<CBuilding*> *buildingsList;
    CPtrListDoubleLink<CObject*> *objectsList;
    CPtrListDoubleLink<CVehicle*> *vehiclesList;
    CPtrListDoubleLink<CPed*> *pedsList;
    CPtrListDoubleLink<CDummy*> *dummiesList;

    template<typename Visitor>
    void VisitLists(Visitor&& visitor) {
        if (buildingsList) {
            std::invoke(visitor, *buildingsList);
        }
        if (objectsList) {
            std::invoke(visitor, *objectsList);
        }
        if (vehiclesList) {
            std::invoke(visitor, *vehiclesList);
        }
        if (pedsList) {
            std::invoke(visitor, *pedsList);
        }
        if (dummiesList) {
            std::invoke(visitor, *dummiesList);
        }
    }*/
//};
//VALIDATE_SIZE(tScanLists, (VER_x32 ? 0x14 : 0x28));

struct tRenderListEntry {
    CEntity* entity;
    float distance;
};
VALIDATE_SIZE(tRenderListEntry, (VER_x32 ? 8 : 16));

constexpr auto MAX_INVISIBLE_ENTITY_PTRS = 150u;
constexpr auto MAX_VISIBLE_ENTITY_PTRS   = 1000u;
constexpr auto MAX_VISIBLE_LOD_PTRS      = 1000u;
constexpr auto MAX_VISIBLE_SUPERLOD_PTRS = 50u;

class CWorldScan {
public:
    using tScanFunction = void(__cdecl*)(int32, int32);
    static void ScanWorld(CVector2D* points, int32 pointsCount, tScanFunction scanFunction);
    static void SetExtraRectangleToScan(float minX, float maxX, float minY, float maxY);
};

class CRenderer {
public:
    static bool ms_bRenderTunnels;
    static bool ms_bRenderOutsideTunnels;
    static tRenderListEntry* ms_pLodDontRenderList;
    static tRenderListEntry* ms_pLodRenderList;
    static CVehicle* m_pFirstPersonVehicle;
    static CEntity* (ms_aInVisibleEntityPtrs)[MAX_INVISIBLE_ENTITY_PTRS];
    static CEntity* (ms_aVisibleSuperLodPtrs)[MAX_VISIBLE_SUPERLOD_PTRS];
    static CEntity* (ms_aVisibleLodPtrs)[MAX_VISIBLE_LOD_PTRS];
    static CEntity* (ms_aVisibleEntityPtrs)[MAX_VISIBLE_ENTITY_PTRS];
    static uint32_t ms_nNoOfVisibleSuperLods;
    static uint32_t ms_nNoOfInVisibleEntities;
    static uint32_t ms_nNoOfVisibleLods;
    static uint32_t ms_nNoOfVisibleEntities;
    static float ms_fFarClipPlane;
    static float ms_fCameraHeading;
    static bool m_loadingPriority;
    static bool ms_bInTheSky;
    static CVector ms_vecCameraPosition;
    static float ms_lodDistScale;    // default 1.2
    static float ms_lowLodDistScale; // default 1.0

public:
    static void InjectHooks();

    static void RenderEverythingBarRoads();

    static void Init();
    static void PreRender();
    static void Shutdown();

    static void RenderOneNonRoad(CEntity *pEntity);

    static void RenderFadingInEntities();

    static auto GetVisibleLodPtrs()      { return std::span{ ms_aVisibleLodPtrs,      (size_t)ms_nNoOfVisibleLods }; }
    static auto GetVisibleEntityPtrs()   { return std::span{ ms_aVisibleEntityPtrs,   (size_t)ms_nNoOfVisibleEntities }; }
    static auto GetVisibleSuperLodPtrs() { return std::span{ ms_aVisibleSuperLodPtrs, (size_t)ms_nNoOfVisibleSuperLods }; }
    static auto GetInVisibleEntityPtrs() { return std::span{ ms_aInVisibleEntityPtrs, (size_t)ms_nNoOfInVisibleEntities }; }
};

#endif //MULTIRIUM_RENDERER_H
