//
// Created by viski on 01.05.2025.
//

#pragma once

#include "game/peds/Ped.h"
#include "game/core/common.h"
#include "game/core/core/LinkList.h"

class CEntity;
class CClumpModelInfo;
class CAtomicModelInfo;

enum eAtomicComponentFlag {
    ATOMIC_IS_NOT_PRESENT = 0x0,
    ATOMIC_IS_OK_STATE = 0x1,
    ATOMIC_IS_DAM_STATE = 0x2,
    ATOMIC_IS_LEFT = 0x4,
    ATOMIC_IS_RIGHT = 0x8,
    ATOMIC_IS_FRONT = 0x10,
    ATOMIC_IS_REAR = 0x20,
    ATOMIC_HAS_ALPHA = 0x40,
    ATOMIC_CULL = 0x80,
    ATOMIC_IS_REAR_DOOR = 0x100,
    ATOMIC_IS_FRONT_DOOR = 0x200,
    ATOMIC_RENDER_ALWAYS = 0x400, // enabled for opened components (doors)
    ATOMIC_IS_REPLACEMENT_UPGRADE = 0x800,
    ATOMIC_IS_DOOR_WINDOW_OPENED = 0x1000,
    ATOMIC_DISABLE_REFLECTIONS = 0x2000,
    // ATOMIC_HAS_DYNAMIC_TEXTURES  = 0x2000, // TODO: WTF is this?
    ATOMIC_IS_BLOWN_UP = 0x4000,
    ATOMIC_VEHCOMP_15 = 0x8000
};

struct tAtomicVisibilityPlugin {
    int16 m_modelId;
    union {
        uint16 m_flags; // eAtomicComponentFlag
        int16 m_userValue;
    };
};
VALIDATE_SIZE(tAtomicVisibilityPlugin, 0x4);

struct tClumpVisibilityPlugin {
    void* m_visibilityCallBack;
    int32 m_alpha;
};
VALIDATE_SIZE(tClumpVisibilityPlugin, (VER_x32 ? 0x8 : 0x10));

struct tFrameVisibilityPlugin {
    union {
        int32 m_hierarchyId;
        CClumpModelInfo* m_modelInfo;
    };
};
VALIDATE_SIZE(tFrameVisibilityPlugin, (VER_x32 ? 0x4 : 0x8));

class CVisibilityPlugins {
public:
    using RenderFunction = void (*)(void*, float);

    struct AlphaObjectInfo {
        union {
            RpAtomic* m_atomic;
            CEntity* m_entity;
        };
        void* m_pCallback;
        CVisibilityPlugins::RenderFunction RenderFn;
        float m_distance; // alpha
    };

    static RwInt32 ms_atomicPluginOffset;
    static RwInt32 ms_clumpPluginOffset;
    static RwInt32 ms_framePluginOffset;
    static RwCamera* ms_pCamera;
    static inline float ms_pedLodDist{};
    static RwV3d* ms_pCameraPosn;

    static CLinkList<CVisibilityPlugins::AlphaObjectInfo> m_alphaList;
    static CLinkList<CVisibilityPlugins::AlphaObjectInfo> m_alphaBoatAtomicList;
    static CLinkList<CVisibilityPlugins::AlphaObjectInfo> m_alphaEntityList;
    static CLinkList<CVisibilityPlugins::AlphaObjectInfo> m_alphaUnderwaterEntityList;
    static CLinkList<CVisibilityPlugins::AlphaObjectInfo> m_alphaReallyDrawLastList;
    static CLinkList<CPed*> ms_weaponPedsForPC;
public:
    static void InjectHooks();

    static void Initialise();
    static void SetRenderWareCamera(RwCamera* camera);
    static float GetDistanceSquaredFromCamera(RwFrame* frame);
    static inline float GetDistanceSquaredFromCamera1(RwFrame* frame) { return GetDistanceSquaredFromCamera(frame); };

    static float GetDistanceSquaredFromCamera(const CVector* pos);

    static RpAtomic *RenderPedCB(RpAtomic *atomic);

    static int32 GetClumpAlpha(RpClump *clump);
    static int32 GetAtomicId(RpAtomic *pAtomic);

    static void RenderAlphaAtomic(RpAtomic *atomic, int32 alpha);
    static void SetupVehicleVariables(RpClump* clump);

    static void *AtomicConstructor(void *atom, RwInt32 offset, RwInt32 size);

    static void *
    AtomicCopyConstructor(void *pDestAtom, const void *pSrcAtom, RwInt32 offset, RwInt32 size);

    static void *AtomicDestructor(void *atom, RwInt32 offset, RwInt32 size);

    static uint64_t CalculateFadingAtomicAlpha(CBaseModelInfo *pModelInfo, CEntity *pEntity, float dist);

    static void ClearAtomicFlag(RpAtomic *pAtomic, uint16 flag);

    static void ClearClumpForAllAtomicsFlag(RpClump *pClump, int32 id);

    static void *ClumpConstructor(void *pClump, RwInt32 offset, RwInt32 size);

    static RwBool DefaultVisibilityCB(RpClump *pClump);

    static void *
    ClumpCopyConstructor(void *pDestClump, const void *pSrcClump, RwInt32 offset, RwInt32 size);

    static void *ClumpDestructor(void *pClump, RwInt32 offset, RwInt32 size);

    static void *FrameConstructor(void *pFrame, RwInt32 offset, RwInt32 size);

    static void *
    FrameCopyConstructor(void *pDestFrame, const void *pSrcFrame, RwInt32 offset, RwInt32 size);

    static void *FrameDestructor(void *pFrame, RwInt32 offset, RwInt32 size);

    static RwBool FrustumSphereCB(RpClump *pClump);

    static CClumpModelInfo *GetClumpModelInfo(RpClump *pClump);

    static float GetDistanceSquaredFromCamera(RwV3d *pVec);

    static float GetDotProductWithCameraVector(RwMatrix *pMatrix, RwMatrix *pCarMatrix, uint32_t atomicId);

    static uint64_t GetFrameHierarchyId(RwFrame *pFrame);

    static CBaseModelInfo *GetModelInfo(RpAtomic *pAtomic);

    static int32 GetModelInfoIndex(RpAtomic *pAtomic);

    static uint16 GetUserValue(RpAtomic *pAtomic);

    static void InitAlphaAtomicList();

    static void InitAlphaEntityList();

    static bool InsertAtomicIntoBoatSortedList(RpAtomic *pAtomic, float dist);

    static bool InsertAtomicIntoReallyDrawLastList(RpAtomic *pAtomic, float dist);

    static bool InsertAtomicIntoSortedList(RpAtomic *pAtomic, float dist);

    static bool InsertEntityIntoReallyDrawLastList(CEntity *pEntity, float dist);

    static bool InsertEntityIntoSortedList(CEntity *pEntity, float dist);

    static bool InsertEntityIntoUnderwaterList(CEntity *pEntity, float dist);

    static bool InsertObjectIntoSortedList(void *pObj, float dist, RenderFunction fn);

    static RwBool IsAtomicVisible(RpAtomic *pAtomic);

    static RwBool IsClumpVisible(RpClump *pClump);

    static RwBool PluginAttach();

    static void RenderAlphaAtomics();

    static void RenderAtomic(RpAtomic *pObj, float dist);

    static RpAtomic *RenderAtomicWithAlphaCB(RpAtomic *pAtomic, void *pAlpha);

    static void RenderBoatAlphaAtomics();

    static void RenderFadingAtomic(CBaseModelInfo *pModelInfo, RpAtomic *pAtomic, int32 alpha);

    static void RenderFadingClump(CBaseModelInfo *pModelInfo, RpClump *pClump, int32 alpha);

    static RpAtomic *RenderFadingClumpCB(RpAtomic *pAtomic);

    static void RenderFadingEntities();

    static void RenderFadingUnderwaterEntities();

    static RpAtomic *RenderHeliRotorAlphaCB(RpAtomic *pAtomic);

    static float ms_bigVehicleLod0Dist;
    static float ms_vehicleLod0Dist;
    static float ms_vehicleLod0RenderMultiPassDist;
    static float ms_cullCompsDist;
    static float ms_cullBigCompsDist;
    static float ms_vehicleLod1Dist;
    static float ms_pedFadeDist;

    static RpAtomic *RenderHeliTailRotorAlphaCB(RpAtomic *pAtomic);

    static RpAtomic *RenderObjNormalAtomic(RpAtomic *pAtomic);

    static void RenderOrderedList(CLinkList<AlphaObjectInfo> *list);

    static RpMaterial *SetTextureCB(RpMaterial *material, void *texture);

    static RpAtomic *RenderPlayerCB(RpAtomic *pAtomic);
    static void RenderReallyDrawLastObjects();

    static RpAtomic* RenderTrainHiDetailCB(RpAtomic* pAtomic);

    static RpAtomic* RenderTrainHiDetailAlphaCB(RpAtomic* pAtomic);

    static RpAtomic *RenderVehicleHiDetailAlphaCB(RpAtomic *pAtomic);

    static RpAtomic *RenderVehicleHiDetailAlphaCB_BigVehicle(RpAtomic *pAtomic);

    static RpAtomic *RenderVehicleHiDetailAlphaCB_Boat(RpAtomic *pAtomic);

    static RpAtomic *RenderVehicleHiDetailCB(RpAtomic *pAtomic);

    static RpAtomic *RenderVehicleHiDetailCB_BigVehicle(RpAtomic *pAtomic);

    static RpAtomic *RenderVehicleHiDetailCB_Boat(RpAtomic *pAtomic);

    static RpAtomic *RenderVehicleLoDetailCB_Boat(RpAtomic *pAtomic);

    static RpAtomic *RenderVehicleReallyLowDetailCB(RpAtomic *pAtomic);

    static RpAtomic *RenderVehicleReallyLowDetailCB_BigVehicle(RpAtomic *pAtomic);

    static void RenderWeaponPedsForPC();

    static RpAtomic *RenderWeaponCB(RpAtomic *pAtomic);

    static RpAtomic *RenderWheelAtomicCB(RpAtomic *pAtomic);

    static void ResetRenderFadingAtomic(CBaseModelInfo *pModelInfo);

    static void SetAtomicFlag(RpAtomic *pAtomic, uint16 flag);

    static void SetAtomicId(RpAtomic *pAtomic, int32 id);

    static void SetAtomicRenderCallback(RpAtomic *pAtomic, RpAtomicCallBackRender renderCB);

    static void SetClumpAlpha(RpClump *pClump, int32 alpha);

    static void SetClumpForAllAtomicsFlag(RpClump *pClump, int32 id);

    static void SetClumpModelInfo(RpClump *pClump, CClumpModelInfo *pModelInfo);

    static void SetFrameHierarchyId(RwFrame *pFrame, uint64_t value);

    static void SetModelInfoIndex(RpAtomic *pAtomic, int32 index);

    static void SetUserValue(RpAtomic *pAtomic, uint16 value);

    static void SetupRenderFadingAtomic(CBaseModelInfo *pModelInfo, int32 alpha);

    static void Shutdown();

    static RwBool VehicleVisibilityCB(RpClump *pClump);

    static RwBool VehicleVisibilityCB_BigVehicle(RpClump *pClump);

    static void RenderEntity(CEntity *pObj, float dist);
};

template<> struct CLink<CVisibilityPlugins::AlphaObjectInfo> {
    CVisibilityPlugins::AlphaObjectInfo item;

    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pPrev;

    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pNext;
};

template<> struct CLink<CPed *> {
    CPed *item;
    CLink<CPed *> *m_pPrev;
    CLink<CPed *> *m_pNext;
};
VALIDATE_SIZE(CLink<CPed *>, (VER_x32 ? 0xC : 0x18));

template <>
struct CLinkList<CPed*> {
    CLink<CPed*> m_firstLink;
    CLink<CPed*> m_lastLink;
    CLink<CPed*> m_firstFreeLink;
    CLink<CPed*> m_lastFreeLink;
    CLink<CPed*> *m_pStore;
};
VALIDATE_SIZE(CLink<CPed *>, (VER_x32 ? 0xC : 0x18));

template<> struct CLinkList<CVisibilityPlugins::AlphaObjectInfo> {
    CLink<CVisibilityPlugins::AlphaObjectInfo> m_firstLink;
    CLink<CVisibilityPlugins::AlphaObjectInfo> m_lastLink;
    CLink<CVisibilityPlugins::AlphaObjectInfo> m_firstFreeLink;
    CLink<CVisibilityPlugins::AlphaObjectInfo> m_lastFreeLink;
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pStore;
};

#define RpAtomicGetVisibilityPlugin(atomic) ((tAtomicVisibilityPlugin*)((uint32)atomic + CVisibilityPlugins::ms_atomicPluginOffset))