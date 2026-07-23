//
// Created by viski on 12.05.2025.
//

#include "TimeInfo.h"
#include "core/KeyGen.h"
#include "ModelInfo.h"
#include "patch.h"
#include <cstring>

void CTimeInfo::InjectHooks() {
    //CHook::Redirect("FindOtherTimeModel", &CTimeInfo::FindOtherTimeModel);
}

CTimeInfo* CTimeInfo::FindOtherTimeModel(const char* modelName) {
    char timeSwitchModelName[24];
    strcpy(timeSwitchModelName, modelName);

    char* nightSuffix = strstr(timeSwitchModelName, "_nt");

  /*  if (nightSuffix) {
       // strncpy_s(nightSuffix, 4, "_dy", 4);
        *reinterpret_cast<int*>(nightSuffix) = 0x795F6479; // "yd_y"
    }
    else {
        char* daySuffix = strstr(timeSwitchModelName, "_dy");

        if (!daySuffix)
            return nullptr;

       // strncpy_s(daySuffix, 4, "_nt", 4);
        *reinterpret_cast<int*>(daySuffix) = 0x746E5F; // "_nt"
    }*/
    if (nightSuffix) {
        memcpy(nightSuffix, "_dy", 3);
        nightSuffix[3] = '\0';
        //strncpy(nightSuffix, "_nt", 4);
    } else {
        char* daySuffix = strstr(timeSwitchModelName, "_dy");
        if (!daySuffix)
            return nullptr;

        memcpy(daySuffix, "_nt", 3);
        daySuffix[3] = '\0';
        //strncpy(daySuffix, "_nt", 4);
    }

    int32 index = 0;
    //CBaseModelInfo* modelInfo = CModelInfo::GetModelInfoFromHashKey(CKeyGen::GetUppercaseKey(timeSwitchModelName), &index);
   // CTimeInfo* timeInfo = modelInfo ? modelInfo->GetTimeInfo() : nullptr;

   // if (timeInfo) {
    //    timeInfo->m_nOtherTimeModel = index;
    //    return timeInfo;
   // }

    return nullptr;
}