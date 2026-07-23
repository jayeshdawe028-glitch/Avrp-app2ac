#pragma once
#include <cstdint>

class CPed;

class CActor
{
public:
    CActor(int iSkin, float fX, float fY, float fZ, float fAngle);
    ~CActor();

    void Destroy();

    void SetHealth(float fHealth);
    void SetInvulnerable(bool bInvulnerable);
    void ApplyAnimation(const char* szAnimName, const char* szAnimLib, float fDelta,
                        int bLoop, int bLockX, int bLockY, int bFreeze, int iTime);
    void ClearAnimation();
    void SetFacingAngle(float fAngle);
    void ForceTargetRotation(float fRotation);

public:
    CPed*     m_pPed = nullptr;
    bool      m_bInvulnerable = false;
    uintptr_t m_dwGTAId = 0;   // <-- FIX
};
