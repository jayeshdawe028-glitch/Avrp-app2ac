//
// Created by roman on 11/24/2024.
//

#pragma once


#include "Entity.h"
#include "game/core/core/PtrNodeDoubleLink.h"
#include "game/core/core/EntryInfoList.h"
#include "game/renderer/RealTimeShadow.h"

enum ePhysicalFlags {
    PHYSICAL_b01                     = 0x1,
    PHYSICAL_APPLY_GRAVITY           = 0x2,
    PHYSICAL_DISABLE_COLLISION_FORCE = 0x4,
    PHYSICAL_COLLIDABLE              = 0x8,
    PHYSICAL_DISABLE_TURN_FORCE      = 0x10,
    PHYSICAL_DISABLE_MOVE_FORCE      = 0x20,
    PHYSICAL_INFINITE_MASS           = 0x40,
    PHYSICAL_DISABLE_Z               = 0x80,

    PHYSICAL_SUBMERGED_IN_WATER      = 0x100,
    PHYSICAL_ON_SOLID_SURFACE        = 0x200,
    PHYSICAL_BROKEN                  = 0x400,
    PHYSICAL_b12                     = 0x800,
    PHYSICAL_b13                     = 0x1000,
    PHYSICAL_DONT_APPLY_SPEED        = 0x2000,
    PHYSICAL_b15                     = 0x4000,
    PHYSICAL_b16                     = 0x8000,

    PHYSICAL_17                      = 0x10000,
    PHYSICAL_18                      = 0x20000,
    PHYSICAL_BULLETPROOF             = 0x40000,
    PHYSICAL_FIREPROOF               = 0x80000,
    PHYSICAL_COLLISIONPROOF          = 0x100000,
    PHYSICAL_MEELEPROOF              = 0x200000,
    PHYSICAL_INVULNERABLE            = 0x400000,
    PHYSICAL_EXPLOSIONPROOF          = 0x800000,

    PHYSICAL_DONTCOLLIDEWITHFLYERS   = 0x1000000,
    PHYSICAL_ATTACHEDTOENTITY        = 0x2000000,
    PHYSICAL_27                      = 0x4000000,
    PHYSICAL_TOUCHINGWATER           = 0x8000000,
    PHYSICAL_CANBECOLLIDEDWITH       = 0x10000000,
    PHYSICAL_DESTROYED               = 0x20000000,
    PHYSICAL_31                      = 0x40000000,
    PHYSICAL_32                      = 0x80000000,
};

struct CPhysical : public CEntity {
    float       m_fPrevDistFromCam;
    uint32_t    m_LastCollisionTime;
    union {
        struct {
            uint32_t bMakeMassTwiceAsBig : 1;
            uint32_t bApplyGravity : 1;
            uint32_t bDisableCollisionForce : 1;
            uint32_t bCollidable : 1;
            uint32_t bDisableTurnForce : 1;
            uint32_t bDisableMoveForce : 1;
            uint32_t bInfiniteMass : 1;
            uint32_t bDisableZ : 1;

            uint32_t bSubmergedInWater : 1;
            uint32_t bOnSolidSurface : 1;
            uint32_t bBroken : 1;
            uint32_t bProcessCollisionEvenIfStationary : 1; // ref @ 0x6F5CF0
            uint32_t bSkipLineCol : 1;                               // only used for peds
            uint32_t bDontApplySpeed : 1;
            uint32_t b15 : 1;
            uint32_t bProcessingShift : 1;

            uint32_t b17 : 1;
            uint32_t bDisableSimpleCollision : 1; // ref @ CPhysical::ProcessCollision
            uint32_t bBulletProof : 1;
            uint32_t bFireProof : 1;
            uint32_t bCollisionProof : 1;
            uint32_t bMeleeProof : 1;
            uint32_t bInvulnerable : 1;
            uint32_t bExplosionProof : 1;

            uint32_t bDontCollideWithFlyers : 1;
            uint32_t bAttachedToEntity : 1;
            uint32_t bAddMovingCollisionSpeed : 1;
            uint32_t bTouchingWater : 1;
            uint32_t bCanBeCollidedWith : 1;
            uint32_t bDestroyed : 1;
            uint32_t b31 : 1;
            uint32_t b32 : 1;
        } physicalFlags;
        uint32_t m_nPhysicalFlags;
    };
    CVector         m_vecMoveSpeed;
    CVector         m_vecTurnSpeed;
    CVector         m_vecFrictionMoveSpeed;
    CVector         m_vecFrictionTurnSpeed;
    CVector         m_vecForce;
    CVector         m_vecTorque;
    float           m_fMass;
    float           m_fTurnMass;
    float           m_fVelocityFrequency;
    float           m_fAirResistance;
    float           m_fElasticity;
    float           m_fBuoyancyConstant;
    CVector         m_vecCentreOfMass;
    CEntryInfoList       m_pCollisionList;

    uintptr_t       m_pMovingList;

    uint8_t         m_nFakePhysics;
    uint8_t         m_nNumEntitiesCollided;
    eSurfaceType    m_nContactSurface;
    uint8_t         _pad4;

    CEntity*        m_apCollidedEntities[6];
    float           m_fMovingSpeed; // m_fTrueDistanceTravelled
    float           m_fDamageIntensity; // m_fDamageImpulseMagnitude
    CEntity         *m_pDamageEntity;
    CVector         m_vecLastCollisionImpactVelocity;
    CVector         m_vecLastCollisionPosn;
    uint16_t        m_nPieceType;
    uint8_t         _pad5[2];
    CEntity         *m_pAttachedTo;
    CVector         m_vecAttachPosnOffset;
    CVector         m_vecAttachTiltOffset;
    CQuaternion     m_qAttachedEntityRotation;
    CEntity         *m_pEntityIgnoredCollision;
    float           m_fContactSurfaceBrightness;
    float           m_fDynamicLighting;
    CRealTimeShadow*       m_pShadowData;

public:
    CPhysical();
    ~CPhysical() override;

    // originally virtual functions
    void Add() override;
    void Remove() override;
    bool IsAdded();

    CVector GetSpeed( CVector point);
    void ApplyMoveSpeed();
    void ApplyTurnSpeed();

    CVector& GetMoveSpeed()                 { return m_vecMoveSpeed; }
    void GetMoveSpeedVector(CVector *vec)
    {
        vec->x = m_vecMoveSpeed.x;
        vec->y = m_vecMoveSpeed.y;
        vec->z = m_vecMoveSpeed.z;
    }
    void     SetVelocity(CVector velocity)  { m_vecMoveSpeed = velocity; } // 0x441130

    CVector& GetTurnSpeed()         { return m_vecTurnSpeed; }
    void ResetTurnSpeed()           { m_vecTurnSpeed = CVector(); }
    void SetTurnSpeed(CVector vec)  { m_vecTurnSpeed = vec; }

    void ResetMoveSpeed()           { SetVelocity(CVector{}); }

    void ApplyMoveForce(float x, float y, float z);
    void ApplyMoveForce(CVector force);
    void ApplyTurnForce(CVector force, CVector point);
};

static_assert(sizeof(CPhysical) == (VER_x32 ? 0x13C : 0x198), "Invalid size CPhysical");