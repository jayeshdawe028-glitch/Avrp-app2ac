#include "../main.h"
#include "game/core/game.h"
#include "netgame.h"

// 0.3.7
CActorPool::CActorPool()
{
    for (PLAYERID ActorID = 0; ActorID < MAX_ACTORS; ActorID++)
    {
        m_pActors[ActorID] = nullptr;
        m_bActorSlotState[ActorID] = false;
        m_pGtaPed[ActorID] = nullptr;
    }
}

// 0.3.7
CActorPool::~CActorPool()
{
    for (PLAYERID ActorID = 0; ActorID < MAX_ACTORS; ActorID++)
    {
        Delete(ActorID);
    }
}

// 0.3.7
void CActorPool::New(NEW_ACTOR* pNewActor)
{
    if (!pNewActor) return;

    const PLAYERID id = (PLAYERID)pNewActor->ActorID;
    if (id >= MAX_ACTORS) return;

    if (m_pActors[id])
    {
        Delete(id);
    }

    CActor* pActor = new CActor(
            (int)pNewActor->iSkin,
            pNewActor->vecPos.x,
            pNewActor->vecPos.y,
            pNewActor->vecPos.z,
            pNewActor->fAngle
    );

    // ВАЖНО: если актёр не создался (скипнули из-за модели/скина) — не занимаем слот
    if (!pActor || !pActor->m_pPed || pActor->m_dwGTAId == 0)
    {
        if (pActor) delete pActor;
        m_pActors[id] = nullptr;
        m_pGtaPed[id] = nullptr;
        m_bActorSlotState[id] = false;
        return;
    }

    m_pActors[id] = pActor;
    m_pGtaPed[id] = pActor->m_pPed;
    m_bActorSlotState[id] = true;

    pActor->SetHealth(pNewActor->fHealth);
    pActor->SetInvulnerable(pNewActor->bInvulnerable != 0);
}

// 0.3.7
void CActorPool::Delete(PLAYERID ActorID)
{
    if (ActorID >= MAX_ACTORS) return;

    CActor* pActor = GetAt(ActorID);
    if (pActor)
    {
        m_bActorSlotState[ActorID] = false;
        delete m_pActors[ActorID];
        m_pActors[ActorID] = nullptr;
        m_pGtaPed[ActorID] = nullptr;
    }
}
