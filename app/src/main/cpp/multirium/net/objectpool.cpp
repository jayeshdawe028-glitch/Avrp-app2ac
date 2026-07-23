#include "../main.h"
#include "game/core/game.h"
#include "netgame.h"

extern CGame *pGame;

// 0.3.7
CObjectPool::CObjectPool()
{
    for (OBJECTID ObjectID = 0; ObjectID < MAX_OBJECTS; ObjectID++)
    {
        m_bObjectSlotState[ObjectID] = false;
        m_pObjects[ObjectID] = nullptr;
    }

    m_iObjectCount = 0;
}
// 0.3.7
CObjectPool::~CObjectPool()
{
    for (OBJECTID ObjectID = 0; ObjectID < MAX_OBJECTS; ObjectID++)
    {
        Delete(ObjectID);
    }
}

bool CObjectPool::New(OBJECTID ObjectID, int iModel, CVector vecPos, CVector vecRot, float fDrawDistance)
{
    if(m_pObjects[ObjectID] != 0)
        Delete(ObjectID);

    m_pObjects[ObjectID] = pGame->NewObject(iModel, vecPos, vecRot, fDrawDistance);

    if(m_pObjects[ObjectID]) {
        m_bObjectSlotState[ObjectID] = true;

        return true;
    }

    return false;
}

bool CObjectPool::Delete(OBJECTID ObjectID)
{
    if (ObjectID < 0 || ObjectID >= MAX_OBJECTS) return false;
    if(!GetSlotState(ObjectID) || !m_pObjects[ObjectID])
        return false;

    m_bObjectSlotState[ObjectID] = false;
    delete m_pObjects[ObjectID];
    m_pObjects[ObjectID] = 0;

    return true;
}

void CObjectPool::Process()
{
    static unsigned long s_ulongLastCall = 0;
    if (!s_ulongLastCall) s_ulongLastCall = GetTickCount();
    unsigned long ulongTick = GetTickCount();
    float fElapsedTime = ((float)(ulongTick - s_ulongLastCall)) / 1000.0f;
    for (int i = 0; i < MAX_OBJECTS; i++)
    {
        if (m_bObjectSlotState[i]) m_pObjects[i]->Process(fElapsedTime);
    }
    s_ulongLastCall = ulongTick;
}

CObject* CObjectPool::FindObjectFromGtaPtr(CPhysical* pGtaObject)
{
    for (OBJECTID ObjectID = 0; ObjectID < MAX_OBJECTS; ObjectID++)
    {
        if (m_pObjects[ObjectID] && m_pObjects[ObjectID]->m_pEntity && m_pObjects[ObjectID]->m_pEntity == pGtaObject)
            return m_pObjects[ObjectID];
    }

    return nullptr;
}

OBJECTID CObjectPool::FindIDFromGtaPtr(CPhysical* pGtaObject)
{
    for (OBJECTID ObjectID = 0; ObjectID < MAX_OBJECTS; ObjectID++)
    {
        if (m_pObjects[ObjectID] && m_pObjects[ObjectID]->m_pEntity && m_pObjects[ObjectID]->m_pEntity == pGtaObject)
            return ObjectID;
    }

    return INVALID_OBJECT_ID;
}