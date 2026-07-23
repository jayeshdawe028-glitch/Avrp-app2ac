
#include "game/core/common.h"
#include "game/pools/Pools.h"
#include "Building.h"
#include "game/modelinfo/ModelInfo.h"
#include "vendor/armhook/patch.h"
#include "game/Enums/eEntityType.h"
#include "Streaming.h"

int32 gBuildings;

CBuilding::CBuilding() : CEntity()
{
    m_nType = ENTITY_TYPE_BUILDING;
    m_bUsesCollision = true;
}

void* CBuilding::operator new(size_t size)
{
    return GetBuildingPool()->New();
}

void CBuilding::operator delete(void* data)
{
    GetBuildingPool()->Delete(static_cast<CBuilding*>(data));
}
CBuilding g_Building;
void CBuilding__ReplaceWithNewModel(int32 newModelIndex) {
    g_Building.ReplaceWithNewModel(newModelIndex);
}

void CBuilding::ReplaceWithNewModel(int32 newModelIndex)
{
    DeleteRwObject();
    if (!CModelInfo::GetModelInfo(m_nModelIndex)->m_nRefCount)
        CStreaming::RemoveModel(m_nModelIndex);

    m_nModelIndex = newModelIndex;
}

bool IsBuildingPointerValid(CBuilding* building)
{
    if (!building)
        return false;

    return GetBuildingPool()->IsObjectValid(building);
}

void CBuilding::InjectHooks() {
    gBuildings = *(int32*)(g_libGTASA + (VER_x32 ? 0x9584E8 : 0xBC0FBC));

    CHook::Redirect("_ZN9CBuilding19ReplaceWithNewModelEi", &CBuilding__ReplaceWithNewModel);
    CHook::Redirect("_Z22IsBuildingPointerValidP9CBuilding", &IsBuildingPointerValid);
}
