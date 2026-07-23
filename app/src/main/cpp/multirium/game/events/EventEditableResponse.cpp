//
// Created by viski on 15.05.2025.
//

#include "EventEditableResponse.h"
/*
void CEventEditableResponse::InjectHooks() {
    RH_ScopedVirtualClass(CEventEditableResponse, 0x85AB80, 17);
    RH_ScopedCategory("Events");

    RH_ScopedInstall(Constructor, 0x4AC450);
    RH_ScopedVMTInstall(Clone, 0x420ED0);
    RH_ScopedVMTInstall(HasEditableResponse, 0x420EF0);
    RH_ScopedInstall(WillRespond, 0x4AC490);
    RH_ScopedInstall(InformVehicleOccupants, 0x4AC4A0);
    RH_ScopedInstall(InformRespectedFriends, 0x4B2B00);
    RH_ScopedInstall(InformGroup, 0x4B7DF0);
    RH_ScopedInstall(TriggerLookAt, 0x4AC5A0);
    RH_ScopedOverloadedInstall(ComputeResponseTaskType, "ped", 0x4B56C0, void(CEventEditableResponse::*)(CPed * ped, bool));
    RH_ScopedOverloadedInstall(ComputeResponseTaskType, "group", 0x4B57A0, void(CEventEditableResponse::*)(CPedGroup*));
    RH_ScopedInstall(ComputeResponseTaskOfType, 0x4B5730);
}

CEventEditableResponse::CEventEditableResponse(eTaskType taskType) :
        CEvent() {
    m_bAddToEventGroup     = true;
    m_TaskId               = taskType;
    m_FacialExpressionType = eFacialExpression::NONE;
}

CEventEditableResponse* CEventEditableResponse::Constructor() {
    this->CEventEditableResponse::CEventEditableResponse();
    return this;
}

CEvent* CEventEditableResponse::Clone() const noexcept {
    CEventEditableResponse* clone = CloneEditable();
    clone->m_TaskId               = m_TaskId;
    clone->m_FacialExpressionType = m_FacialExpressionType;
    clone->m_bAddToEventGroup     = m_bAddToEventGroup;
    return clone;
}

bool CEventEditableResponse::WillRespond() const {
    return m_TaskId != TASK_NONE;
}

void CEventEditableResponse::InformVehicleOccupants(CPed* ped) {
    CVehicle* vehicle = ped->m_pVehicle;
    if (!ped->bInVehicle || !DoInformVehicleOccupants(ped) || !m_bAddToEventGroup) {
        return;
    }
    const auto InformOccupant = [this, ped](CPed* o) {
        if (!o || o == ped) {
            return;
        }
        const auto clone          = static_cast<CEventEditableResponse*>(Clone());
        clone->m_TaskId           = TASK_NONE;
        clone->m_bAddToEventGroup = false;
        o->GetEventGroup().Add(clone);
        delete clone;
    };
    InformOccupant(vehicle->GetDriver());
    rng::for_each(vehicle->GetPassengers(), InformOccupant);
}*/