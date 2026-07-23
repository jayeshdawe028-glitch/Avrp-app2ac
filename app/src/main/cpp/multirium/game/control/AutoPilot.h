//
// Created by viski on 26.04.2025.
//

#ifndef MULTIRIUM_AUTOPILOT_H
#define MULTIRIUM_AUTOPILOT_H

#include "PathFind.h"
#include "NodeAddress.h"
#include "game/core/core/Vector.h"
#include "../game/Enums/eCarMission.h"
#include "../game/Enums/eCarDrivingStyle.h"

class CVehicle;
class CEntity;

enum eAutoPilotTempAction : int8 {
    TEMPACT_NONE                     = 0,
    TEMPACT_WAIT                     = 1,
    TEMPACT_EMPTYTOBEREUSED          = 2,
    TEMPACT_REVERSE                  = 3,
    TEMPACT_HANDBRAKETURNLEFT        = 4,
    TEMPACT_HANDBRAKETURNRIGHT       = 5,
    TEMPACT_HANDBRAKESTRAIGHT        = 6,
    TEMPACT_TURNLEFT                 = 7,
    TEMPACT_TURNRIGHT                = 8,
    TEMPACT_GOFORWARD                = 9,
    TEMPACT_SWIRVELEFT               = 10,
    TEMPACT_SWIRVERIGHT              = 11,
    TEMPACT_STUCKINTRAFFIC           = 12,
    TEMPACT_REVERSE_LEFT             = 13,
    TEMPACT_REVERSE_RIGHT            = 14,
    TEMPACT_PLANE_FLY_UP             = 15,
    TEMPACT_PLANE_FLY_STRAIGHT       = 16,
    TEMPACT_PLANE_SHARP_LEFT         = 17,
    TEMPACT_PLANE_SHARP_RIGHT        = 18,
    TEMPACT_HEADON_COLLISION         = 19,
    TEMPACT_SWIRVELEFT_STOP          = 20,
    TEMPACT_SWIRVERIGHT_STOP         = 21,
    TEMPACT_REVERSE_STRAIGHT         = 22,
    TEMPACT_BOOST_USE_STEERING_ANGLE = 23,
    TEMPACT_BRAKE                    = 24,
};

class CAutoPilot {
public:
    CNodeAddress        m_currentAddress;
    CNodeAddress        m_startingRouteNode;
    CNodeAddress        m_endingRouteNode;
    int32               field_C;
    uint32              m_nSpeedScaleFactor;
    CCarPathLinkAddress m_nCurrentPathNodeInfo;
    CCarPathLinkAddress m_nNextPathNodeInfo;
    CCarPathLinkAddress m_nPreviousPathNodeInfo;
    char                field_1A[2];
    uint32              m_nTimeToStartMission;
    uint32              m_nTimeSwitchedToRealPhysics;
    int8                _smthPrev;
    int8                _smthCurr;
    int8                _smthNext;
    int8                m_nCurrentLane;
    int8                m_nNextLane;
    eCarDrivingStyle    m_nCarDrivingStyle;
    eCarMission         m_nCarMission;
    eAutoPilotTempAction m_nTempAction;
    uint32              m_nTempActionTime;
    uint32              m_LastUpdateTimeMs; //
    uint8               m_ucTempActionMode;
    uint8               m_ucCarMissionModeCounter;
    char                field_36[2];
    float               m_speed;
    float               m_fMaxTrafficSpeed;
    uint8               m_nCruiseSpeed;
    char                field_41;
    char                field_42[2];
    float               m_SpeedMult;
    uint8               m_ucHeliTargetDist;
    uint8               m_ucHeliSpeedMult;
    char                field_4A;
    struct CarCtrlFlags {
        uint8 bHonkAtCar : 1;
        uint8 bHonkAtPed : 1;
        uint8 bAvoidLevelTransitions : 1;
        uint8 bStayInFastLane : 1;
        uint8 bStayInSlowLane : 1;
        uint8 bDoTargetCatchupCheck : 1;
        uint8 bCantGoAgainstTraffic : 1;
        uint8 bHeliFollowTarget : 1;
    };

    union {
        uint8 m_nCarCtrlFlags;
        CarCtrlFlags carCtrlFlags;
    };

    struct MovementFlags {
        uint8 bIsStopped : 1;
        uint8 bIsParked : 1;
    };

    union {
        uint8 m_nMovementFlags;
        MovementFlags movementFlags;
    };
    uint8           m_nStraightLineDistance;
    uint8           m_ucCarFollowDist;
    uint8           m_ucHeliTargetDist2;
    char            field_50;
    char            field_51;
    char            field_52[10];
    CVector         m_vecDestinationCoors;
    CNodeAddress    m_aPathFindNodesInfo[8];
    uint16          m_nPathFindNodesCount;
    char            field_8A[2];
    CVehicle*       m_TargetEntity;
    CEntity*        m_ObstructingEntity; // Entity to slow down for
    int8            m_vehicleRecordingId;
    bool            m_bPlaneDogfightSomething;
    int16           field_96;

    CAutoPilot();

    void ModifySpeed(float target);
    void RemoveOnePathNode();

    void SetCarMission(eCarMission carMission) { m_nCarMission = carMission; }

    void SetCarMission(eCarMission carMission, uint32 timeOffsetMs);

    void ClearCarMission() { m_nCarMission = MISSION_NONE; }

    void SetCruiseSpeed(uint32 s) { assert(s <= UINT8_MAX); m_nCruiseSpeed = (uint8)s; }

    /*!
     * @notsa
     * @brief Set the temporary action
     * @param action The action set add
     * @param durMs  The action's duration in milliseconds
    */
    void SetTempAction(eAutoPilotTempAction action, uint32 durMs);

    /*!
     * @brief Clear current temp. act.
    */
    void ClearTempAct() { m_nTempAction = TEMPACT_NONE; }

    void SetDrivingStyle(eCarDrivingStyle s) { m_nCarDrivingStyle = s; }
};
VALIDATE_SIZE(CAutoPilot, (VER_x32 ? 0x98 : 0xA8));

#endif //MULTIRIUM_AUTOPILOT_H
