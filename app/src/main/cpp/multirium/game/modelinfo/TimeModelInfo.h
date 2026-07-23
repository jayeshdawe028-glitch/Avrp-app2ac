//
// Created by viski on 12.05.2025.
//

#ifndef MULTIRIUM_TIMEMODELINFO_H
#define MULTIRIUM_TIMEMODELINFO_H


#include "TimeInfo.h"
#include "AtomicModelInfo.h"

#pragma pack(push, 1)
class CTimeModelInfo : public CAtomicModelInfo {
public:
    CTimeInfo m_timeInfo;

public:
    static void InjectHooks();

    static ModelInfoType GetModelType();
    CTimeInfo* GetTimeInfo();
    static CTimeInfo* GetTimeInfo_(CTimeModelInfo* self);
};
#pragma pack(pop)
//VALIDATE_SIZE(CTimeModelInfo, (VER_x32 ? 0x44 : 0x50));

#endif //MULTIRIUM_TIMEMODELINFO_H
