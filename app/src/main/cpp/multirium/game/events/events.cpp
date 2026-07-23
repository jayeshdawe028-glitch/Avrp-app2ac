#include "main.h"
#include "game.h"
#include "net/netgame.h"
#include "Streaming.h"

extern CGame* pGame;
extern CNetGame* pNetGame;

#define EVENT_TYPE_PAINTJOB			1
#define EVENT_TYPE_CARCOMPONENT		2
#define EVENT_TYPE_CARCOLOR			3
#define EVENT_ENTEREXIT_MODSHOP		4

extern bool bFirstSpawn;

void ProcessIncommingEvent(PLAYERID playerId, int iEventType,
	uint32_t dwParam1, uint32_t dwParam2, uint32_t dwParam3)
{
	if (pNetGame == nullptr) return;
	if (bFirstSpawn) return;

	CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
	CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();

	CVehicle* pVehicle = nullptr;
	CRemotePlayer* pPlayer = nullptr;

    int iVehicleID;
    int iPaintJob;
    int iComponent;
    uint32_t v;

    CVehicle* v9; // x0

    // у лр не юзаеться нигде
    /*if(iEventType == 2) {
        Log("RPC: EVENT_TYPE_CARCOMPONENT");
        iVehicleID = pNetGame->GetVehiclePool()->FindGtaIDFromID(dwParam1);
        if ( (CStreaming::TryLoadModel((CStreaming *)a4, v8) & 1) != 0 )
        {
            v9 = GamePool_Vehicle_GetAt(iVehicleID);
            if ( v9 )
                CHook::CallFunction<uint64_t>("_ZN8CVehicle17AddVehicleUpgradeEi", v9, v4);
        }
    }*/
}