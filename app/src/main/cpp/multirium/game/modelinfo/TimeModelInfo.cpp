//
// Created by viski 12.05.2025.
//

#include "TimeModelInfo.h"

#include "ModelInfo.h"
#include "patch.h"

void CTimeModelInfo::InjectHooks() {
    CHook::Redirect("_ZN14CTimeModelInfo12GetModelTypeEv", &CTimeModelInfo::GetModelType);
    CHook::Redirect("_ZN14CTimeModelInfo11GetTimeInfoEv", &CTimeModelInfo::GetTimeInfo_);
}

CTimeInfo* CTimeModelInfo::GetTimeInfo_(CTimeModelInfo* a) {
    return a->GetTimeInfo();
}

ModelInfoType CTimeModelInfo::GetModelType() {
    return ModelInfoType::MODEL_INFO_TIME;
}

CTimeInfo* CTimeModelInfo::GetTimeInfo() {
    return &m_timeInfo;
}