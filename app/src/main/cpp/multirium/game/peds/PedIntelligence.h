//
// Created by viski on 18.04.2025.
//

#pragma once

#include "main.h"
#include "game/tasks/TaskManager.h"
#include "game/events/EventHandler.h"
#include "game/events/EventGroup.h"

class CTaskSimpleJetPack;

class CPedIntelligence
{
public:
    class CPed* m_pPed;
    CTaskManager   m_TaskMgr;
    CEventHandler  m_eventHandler;
    CEventGroup    m_eventGroup;

    int     m_iDecisionMakerType;
    int     m_iDecisionMakerTypeInGroup;
    float   m_fHearingRange;
    float   m_fSeeingRange;

    int     m_iMaxNumFriendsToInform;
    float   m_fMaxInformFriendDistance;
    float   m_fFollowNodeThresholdDistance;

    int8    m_iNextEventResponseSequence;
    uint8   m_iHighestPriorityEventType;
    uint8   m_iHighestPriorityEventPriority;
    uint8   pad0;

#if VER_x32
    uint8   m_vehicleScanner[0x1C8];
#else
    uint8   pad1[0x2C8];
#endif

public:
    static void InjectHooks();

    CPedIntelligence(CPed* ped);
    ~CPedIntelligence();

    void Process();
    void FlushImmediately(bool bSetPrimaryDefaultTask);

    void FlushIntelligence();
    void ProcessAfterProcCol();
    void ProcessAfterPreRender();

    CTaskSimpleJetPack* GetTaskJetPack();
};
VALIDATE_SIZE(CPedIntelligence, (VER_x32 ? 0x298 : 0x440));

