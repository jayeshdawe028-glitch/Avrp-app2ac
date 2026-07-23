#pragma once

#include "game/core/core/PtrListDoubleLink.h"
#include "game/vehicles/vehicle.h"
#include "game/objects/Object.h"

class CObject;
class CVehicle;
class CPed;

enum eRepeatSectorList : int32 {
    REPEATSECTOR_VEHICLES = 0,
    REPEATSECTOR_PEDS = 1,
    REPEATSECTOR_OBJECTS = 2
};

class CRepeatSector {
public:
    CPtrListDoubleLink& GetList(eRepeatSectorList type) noexcept {
        return m_lists[type];
    }

    const CPtrListDoubleLink& GetList(eRepeatSectorList type) const noexcept {
        return m_lists[type];
    }
public:
    CPtrListDoubleLink m_lists[3];
};
VALIDATE_SIZE(CRepeatSector, (VER_x32 ? 0xC : 0x18));