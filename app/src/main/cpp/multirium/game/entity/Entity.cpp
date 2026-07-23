//
// Created by viski on 17.04.2025.
//

#include "Entity.h"
#include "game/core/game.h"
#include "game/Collision/ColStore.h"
#include "patch.h"
#include "game/core/General.h"
#include "game/modelinfo/BaseModelInfo.h"
#include "game/modelinfo/ModelInfo.h"
#include "game/core/References.h"
#include "game/pools/Pools.h"
#include "../util/CUtil.h"

extern CGame* pGame;

CEntity::CEntity() : CPlaceable() {
    m_nStatus = STATUS_ABANDONED;
    m_nType = ENTITY_TYPE_NOTHING;

    m_nFlags = 0;
    m_bIsVisible = true;
    m_bBackfaceCulled = true;

    m_nScanCode = 0;
    m_nAreaCode = eAreaCodes::AREA_CODE_NORMAL_WORLD;
    m_nModelIndex = 0xFFFF;
    m_pRwObject = nullptr;
    m_nIplIndex = 0;
    m_nRandomSeed = CGeneral::GetRandomNumber();
    m_pReferences = nullptr;
    m_pStreamingLink = nullptr;
    m_nNumLodChildren = 0;
    m_nNumLodChildrenRendered = 0;
    m_pLod = nullptr;
}

CEntity::~CEntity()
{
    if (m_pLod)
        m_pLod->m_nNumLodChildren--;

    CEntity::DeleteRwObject();
    CEntity::ResolveReferences();
}

void CEntity::ResolveReferences()
{
    auto refs = m_pReferences;
    while (refs) {
        if (*refs->m_ppEntity == this)
            *refs->m_ppEntity = nullptr;

        refs = refs->m_pNext;
    }

    refs = m_pReferences;
    if (!refs)
        return;

    refs->m_ppEntity = nullptr;
    while (refs->m_pNext)
        refs = refs->m_pNext;

    refs->m_pNext = CReferences::pEmptyList;
    CReferences::pEmptyList = m_pReferences;
    m_pReferences = nullptr;
}

void CEntity::Add()
{
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x3ED8D8 + 1 : 0x4CD574), this);
}

void CEntity::SetInterior(int interiorId, bool needRefresh)
{
    m_nAreaCode = static_cast<eAreaCodes>(interiorId);

    if ( this == (CEntity*)pGame->FindPlayerPed()->m_pPed )
    {
        CGame::currArea = interiorId;

        auto pos = GetPosition();
        CColStore::RequestCollision(&pos, m_nAreaCode);

        if(interiorId == 0) {
            CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x00420898 + 1 : 0x504194), false);
            // CTimeCycle::StopExtraColour(0);
        }

    }

}

float CEntity::GetDistanceFromLocalPlayerPed() const
{
    auto pLocalPlayerPed = pGame->FindPlayerPed();

    return DistanceBetweenPoints(GetPosition(), pLocalPlayerPed->m_pPed->GetPosition());
}

float CEntity::GetDistanceFromCamera()
{
    if(!this)
        return 0;

    CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));
    return DistanceBetweenPoints(GetPosition(), TheCamera.GetPosition());
}

float CEntity::GetDistanceFromPoint(float X, float Y, float Z) const
{
    CVector vec(X, Y, Z);

    return DistanceBetweenPoints(GetPosition(), vec);
/*
    float* entityPos;

    if (this->positionData != nullptr) {
        entityPos = (float*)((char*)this->positionData + 48);
    } else {
        entityPos = &this->X;
    }

    CVector distanceVector;
    distanceVector.x = X - entityPos[0];
    distanceVector.y = Y - entityPos[1];
    distanceVector.z = Z - entityPos[2];

    return CVector::Magnitude(&distanceVector);*/
}

void CEntity::SetCollisionChecking(bool bCheck)
{
    m_bCollisionProcessed = bCheck;
}

void CEntity::UpdateRpHAnim() {
    if (const auto atomic = GetFirstAtomic(m_pRwClump)) {
        if (RpSkinGeometryGetSkin(RpAtomicGetGeometry(atomic)) && !m_bDontUpdateHierarchy) {
            RpHAnimHierarchyUpdateMatrices(GetAnimHierarchyFromSkinClump(m_pRwClump));
        }
    }
}

void CEntity::UpdateRwFrame()
{
    if (!m_pRwObject)
        return;

    RwFrameUpdateObjects(static_cast<RwFrame*>(rwObjectGetParent(m_pRwObject)));
}

void CEntity::DeleteRwObject()
{
   // if(!*(uintptr*)this) return;

    //(( void (*)(CEntity*))(*(void**)(*(uintptr*)this + (VER_x32 ? 0x24 : 0x24 * 2))))(this);
}

void CEntity::UpdateRW() {
    if (!m_pRwObject)
        return;

    auto parentMatrix = GetModellingMatrix();
    if (m_matrix)
        m_matrix->UpdateRwMatrix(parentMatrix);
    else
        m_placement.UpdateRwMatrix(parentMatrix);
}

void CEntity::UpdateRwMatrixAndFrame()
{
    //if (!IsAdded())
   // {
        if (m_pRwObject)
        {
            auto parent = (char *)m_pRwObject->parent;
            auto m_pMat = m_matrix;

            auto pMatrix = (RwMatrix *)(parent + (VER_x32 ? 0x10 : 0x20));

            if (m_pMat)
                m_pMat->UpdateRwMatrix(pMatrix);
            else
                m_placement.UpdateRwMatrix(pMatrix);

           // CHook::CallFunction<void>("_ZN7CEntity13UpdateRwFrameEv", m_pEntity);
            UpdateRwFrame();
        }
    //}
   /* if (!IsGameEntityArePlaceable(this)) // m_pEntity
    {
        if (m_pRwObject)
        {
            auto parent = (char *)m_pRwObject->parent;
            auto m_pMat = m_matrix;

            auto pMatrix = (RwMatrix *)(parent + (VER_x32 ? 0x10 : 0x20));

            if (m_pMat)
                m_pMat->UpdateRwMatrix(pMatrix);
            else
                m_placement.UpdateRwMatrix(pMatrix);


            CHook::CallFunction<void>("_ZN7CEntity13UpdateRwFrameEv", this); // m_pEntity
        }
    }*/
}

void CEntity::UpdateMatrix(RwMatrix mat)
{
    if (m_matrix)
    {
        CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x3FD02C + 1 : 0x4E07EC), this);

        SetMatrix((CMatrix&)mat);
        UpdateRwMatrixAndFrame();

        CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x3FCE3C + 1 : 0x4E0608), this);
    }
}

void CEntity::Render()
{
    // CEntity::PreRender
    ((void (*)(CEntity*))(*(uintptr_t*)(*(uintptr*)(this) + (VER_x32 ? 0x48 : 0x48 * 2) )))(this);

    // CRenderer::RenderOneNonRoad
    CHook::CallFunction<void>("_ZN9CRenderer16RenderOneNonRoadEP7CEntity", this);
}

RwMatrix* CEntity::GetModellingMatrix() {
    if (!m_pRwObject)
        return nullptr;

    return RwFrameGetMatrix(RwFrameGetParent(m_pRwObject));
}

CBaseModelInfo* CEntity::GetModelInfo() const {
    return CModelInfo::GetModelInfo(m_nModelIndex);
}

void CEntity::Add(const CRect* rect) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x3ED8FC + 1 : 0x4CD5C0), this, rect);
}

void CEntity::Remove() {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x3EDBE8 + 1 : 0x4CD888), this);
}

void CEntity::SetModelIndex(uint32 index) {
    CEntity::SetModelIndexNoCreate(index);
    //CEntity::CreateRwObject();
}

void CEntity::SetModelIndexNoCreate(uint32 index) {

}

bool CEntity::HasPreRenderEffects()
{

}

void CEntity::SetIsStatic(bool isStatic) {
    m_bIsStatic = isStatic;
}

void CEntity::RegisterReference(CEntity** entity)
{
    if (IsBuilding() && !m_bIsTempBuilding && !m_bIsProcObject && !m_nIplIndex)
        return;

    auto refs = m_pReferences;
    while (refs) {
        if (refs->m_ppEntity == entity) {
            return;
        }
        refs = refs->m_pNext;
    }

    if (!m_pReferences && !CReferences::pEmptyList) {
        auto iPedsSize = GetPedPoolGta()->GetSize();
        for (int32 i = 0; i < iPedsSize; ++i) {
            auto ped = GetPedPoolGta()->GetAt(i);
            if (ped) {
                ped->PruneReferences();
                if (CReferences::pEmptyList)
                    break;
            }

        }

        if (!CReferences::pEmptyList) {
            auto iVehsSize = GetVehiclePoolGta()->GetSize();
            for (int32 i = 0; i < iVehsSize; ++i) {
                auto vehicle = GetVehiclePoolGta()->GetAt(i);
                if (vehicle) {
                    vehicle->PruneReferences();
                    if (CReferences::pEmptyList)
                        break;
                }

            }
        }

        if (!CReferences::pEmptyList) {
            auto iObjectsSize = GetObjectPoolGta()->GetSize();
            for (int32 i = 0; i < iObjectsSize; ++i) {
                auto obj = GetObjectPoolGta()->GetAt(i);
                if (obj) {
                    obj->PruneReferences();
                    if (CReferences::pEmptyList)
                        break;
                }
            }
        }
    }

    if (CReferences::pEmptyList) {
        auto pEmptyRef = CReferences::pEmptyList;
        CReferences::pEmptyList = pEmptyRef->m_pNext;
        pEmptyRef->m_pNext = m_pReferences;
        m_pReferences = pEmptyRef;
        pEmptyRef->m_ppEntity = entity;
    }
}

void CEntity::PruneReferences()
{
    if (!m_pReferences)
        return;

    auto refs = m_pReferences;
    auto ppPrev = &m_pReferences;
    while (refs) {
        if (*refs->m_ppEntity == this) {
            ppPrev = &refs->m_pNext;
            refs = refs->m_pNext;
        }
        else {
            auto refTemp = refs->m_pNext;
            *ppPrev = refs->m_pNext;
            refs->m_pNext = CReferences::pEmptyList;
            CReferences::pEmptyList = refs;
            refs->m_ppEntity = nullptr;
            refs = refTemp;
        }
    }
}

void CEntity::CleanUpOldReference(CEntity** entity)
{
    if (!m_pReferences)
        return;

    auto refs = m_pReferences;
    auto ppPrev = &m_pReferences;
    while (refs->m_ppEntity != entity) {
        ppPrev = &refs->m_pNext;
        refs = refs->m_pNext;
        if (!refs)
            return;
    }

    *ppPrev = refs->m_pNext;
    refs->m_pNext = CReferences::pEmptyList;
    refs->m_ppEntity = nullptr;
    CReferences::pEmptyList = refs;
}

CVector CEntity::TransformFromObjectSpace(CVector a3)
{
    CVector *v2;
    CVector v7;

    v7.x = 0.0;
    v7.y = 0.0;
    v7.z = 0.0;
    if (m_matrix) {
        v7 = *m_matrix * a3;
        return v7;
    } else {
        return CUtil::TransformPoint(m_placement, a3);
    }
}

bool CEntity::IsInCurrentArea() const {
    return m_nAreaCode == CGame::currArea;
}