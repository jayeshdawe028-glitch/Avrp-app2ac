#pragma once

#include "../game/net/ObjectMP.h"

#define INVALID_OBJECT_ID	0xFFFF

class CObjectPool
{
public:
    CObjectPool();
    ~CObjectPool();

    bool New(OBJECTID ObjectID, int iModel, CVector vecPos, CVector vecRot, float fDrawDistance = 0);
    bool Delete(OBJECTID ObjectID);

    bool GetSlotState(uint16_t ObjectID)
    {
        if(ObjectID >= MAX_OBJECTS) return false;
        return m_bObjectSlotState[ObjectID];
    };

    void Process();

    // 0.3.7
    CObject* GetAt(OBJECTID ObjectID) {
        if (ObjectID >= MAX_OBJECTS || ObjectID < 0) return nullptr;
        if(!m_bObjectSlotState[ObjectID]) return 0;
        return m_pObjects[ObjectID];
    }

    CObject *GetObjectFromGtaPtr(CEntity *pGtaObject)
    {
        uint16_t x=1;

        while(x!=MAX_OBJECTS)
        {
            if(m_pObjects[x])
                if(pGtaObject == m_pObjects[x]->m_pEntity) return m_pObjects[x];

            x++;
        }

        return 0;
    }

    OBJECTID FindIDFromGtaPtr(CPhysical* pGtaObject);

    CObject* FindObjectFromGtaPtr(CPhysical* pGtaObject);

private:
    int			m_iObjectCount;
    bool		m_bObjectSlotState[MAX_OBJECTS];
    CObject*	m_pObjects[MAX_OBJECTS];
};