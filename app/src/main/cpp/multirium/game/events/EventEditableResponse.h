//
// Created by viski on 15.05.2025.
//

#ifndef MULTIRIUM_EVENTEDITABLERESPONSE_H
#define MULTIRIUM_EVENTEDITABLERESPONSE_H

#include "Event.h"
#include "game/Enums/eTaskType.h"
#include "main.h"
#include "game/tasks/TaskTypes/TaskSimpleFacial.h"
/*
class CPedGroup;

class CEventEditableResponse : public CEvent {
public:
    CEventEditableResponse(eTaskType taskType = TASK_NONE);
    ~CEventEditableResponse() override = default;

    CEvent* Clone() const noexcept override;
    bool HasEditableResponse() const override { return true; } // 0x420EF0;
    virtual CEventEditableResponse* CloneEditable() const noexcept = 0;

    bool WillRespond() const;
    void InformVehicleOccupants(CPed* ped);
    void InformRespectedFriends(CPed* ped);
    void InformGroup(CPed* ped);
    void TriggerLookAt(CPed* ped) const;
    void ComputeResponseTaskType(CPed* ped, bool bDecisionMakerTypeInGroup);
    void ComputeResponseTaskType(CPedGroup* pedGroup);
    bool ComputeResponseTaskOfType(CPed* ped, eTaskType taskId);

    auto GetFacialExpressionType() const { return (eFacialExpression)m_FacialExpressionType; }

private:
    static void InjectHooks();

    CEventEditableResponse* Constructor();

public:
    bool                               m_bAddToEventGroup;
    notsa::WEnumS16<eTaskType>         m_TaskId;
    notsa::WEnumS16<eFacialExpression> m_FacialExpressionType;
};
VALIDATE_SIZE(CEventEditableResponse, (VER_x32 ? 0x14 : 0x28));*/

#endif //MULTIRIUM_EVENTEDITABLERESPONSE_H
