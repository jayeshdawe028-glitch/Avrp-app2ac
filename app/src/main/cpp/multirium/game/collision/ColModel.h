/*
    Plugin-SDK file
    Authors: GTA Community. See more here
    https://github.com/DK22Pac/plugin-sdk
    Do not delete this comment block. Respect others' work!
*/
#pragma once

#include "BoundingBox.h"
#include "ColSphere.h"
#include "CollisionData.h"
#include "game/core/common.h"

class CColModel {
    uint8        m_nColSlot;
    union {
        struct {
            uint8 m_bHasCollisionVolumes : 1; // AKA `m_bNotEmpty`
            uint8 m_bIsSingleColDataAlloc : 1;
            uint8 m_bIsActive : 1;
        };
        uint8 m_nFlags;
    };
    uint8 pad0[2];
    CCollisionData* m_pColData;

    CBox m_boxBound;
    CSphere m_sphereBound;

public:
    static void InjectHooks();

    CColModel();
    ~CColModel();

    static void* operator new(size_t size);
    static void operator delete(void* data);
    CColModel& operator=(const CColModel& colModel);

public:
    void AllocateData();
    void AllocateData(int32 numSpheres, int32 numBoxes, int32 numLines, int32 numVertices, int32 numTriangles, bool bUsesDisks);
    void MakeMultipleAlloc();
    void RemoveCollisionVolumes();
    void CalculateTrianglePlanes();
    void RemoveTrianglePlanes();

private:
    void AllocateData(int32 size);

public:
    // HELPERS
    [[nodiscard]] auto GetTriCount() const noexcept     { return m_pColData ? m_pColData->m_nNumTriangles : 0u; }
    [[nodiscard]] float GetBoundRadius() const noexcept { return m_sphereBound.m_fRadius; }
    auto& GetBoundCenter() { return m_sphereBound.m_vecCenter; }
    auto& GetBoundingBox() { return m_boxBound; }
    auto& GetBoundingSphere() { return m_sphereBound; }
    CCollisionData* GetData() const { return m_pColData; }
};

VALIDATE_SIZE(CColModel, (VER_x32 ? 0x30 : 0x38));
