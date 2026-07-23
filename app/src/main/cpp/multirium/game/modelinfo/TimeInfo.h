//
// Created by viski on 12.05.2025.
//

#ifndef MULTIRIUM_TIMEINFO_H
#define MULTIRIUM_TIMEINFO_H

#include "../main.h"
#include "Clock.h"
#include "game/core/common.h"

class CTimeInfo {
public:
    uint8_t m_nTimeOn;
    uint8_t m_nTimeOff;
    int16 m_nOtherTimeModel = -1;

public:
    static void InjectHooks();

public:
    static CTimeInfo* FindOtherTimeModel(const char* modelName);

    int32 GetOtherTimeModel() const { return m_nOtherTimeModel; }

    void SetOtherTimeModel(int16 otherTimeModel) { m_nOtherTimeModel = otherTimeModel; }

    uint8_t GetTimeOn() const { return m_nTimeOn; }

    uint8_t GetTimeOff() const { return m_nTimeOff; }

    void SetTimes(uint8_t timeOn, uint8_t timeOff) { m_nTimeOn = timeOn; m_nTimeOff = timeOff; }

    bool IsVisibleNow() const noexcept { return CClock::GetIsTimeInRange(GetTimeOn(), GetTimeOff()); }
};
VALIDATE_SIZE(CTimeInfo, 0x4);

#endif //MULTIRIUM_TIMEINFO_H
