//
// Created by viski on 24.04.2025.
//

#include "World.h"
#include "vendor/armhook/patch.h"
#include "game.h"

extern CGame pGame;

// inlined
CPlayerInfoGta& FindPlayerInfo(int32 playerId) {
    return CWorld::Players[playerId < 0 ? CWorld::PlayerInFocus : playerId];
}

// Returns player ped
CPlayerPedGta* FindPlayerPed(int32 playerId) {
    if(playerId == -1)
        playerId = CWorld::PlayerInFocus;

    return FindPlayerInfo(playerId).m_pPed;
}

bool PlayerIsEnteringCar()
{
    // FIXME: �� ��������. ������� ���������� ������ ��� ��� �� �������� ����. �� ��������, ����� ���� ��� ������� ������ ������� ��� ����� � ����?
//    auto pPed = pGame.FindPlayerPed()->m_pPed;
//    if ( pPed->GetTaskManager().CTaskManager::FindActiveTaskByType(TASK_COMPLEX_ENTER_CAR_AS_DRIVER) )
//        return true;
//
//    return pPed->GetTaskManager().CTaskManager::FindActiveTaskByType(TASK_COMPLEX_ENTER_CAR_AS_PASSENGER) != nullptr;
    return false;
}

// Returns player vehicle
CVehicle* FindPlayerVehicle(int32 playerId, bool bIncludeRemote) {
    CPlayerPedGta* player = FindPlayerPed(playerId);
    if (!player || !player->bInVehicle)
        return nullptr;
    //if (bIncludeRemote) {
    //    CPlayerInfoGta* playerInfo = player->GetPlayerInfoForThisPlayerPed();
    //    if (playerInfo->pRemoteVehicle) {
    //        return playerInfo->pRemoteVehicle;
    //    }
    //}
    return player->pVehicle;
}

// Returns player coors
CVector FindPlayerCoors(int32 playerId) {
    CEntity* entity = FindPlayerEntity(playerId);
    return entity ? entity->GetPosition() : CVector{};
}

// Returns player ped or player vehicle if he's driving
CEntity* FindPlayerEntity(int32 playerId) {
    auto player = FindPlayerPed(playerId);
    if (!player)
        return nullptr;

    if (player->IsInVehicle())
        return player->pVehicle;

    return player;
}
