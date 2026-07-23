#pragma once

#include <cstring>
#include <stdint.h>
#include "tHandlingData.h"
#include "game/Enums/eVehicleHandlingFlags.h"
#include "game/Enums/eVehicleType.h"
#include "game/animation/RideAnimData.h"
#include "game/entity/Physical.h"
#include "game/net/ObjectMP.h"

struct CPed;

enum eCarWeapon : uint8 {
    CAR_WEAPON_NOT_USED       = 0,
    CAR_WEAPON_HEAVY_GUN      = 1,
    CAR_WEAPON_FREEFALL_BOMB  = 2,
    CAR_WEAPON_LOCK_ON_ROCKET = 3,
    CAR_WEAPON_DOUBLE_ROCKET  = 4,
};

enum eCarLock : uint32 {
    CARLOCK_NOT_USED,
    CARLOCK_UNLOCKED,
    CARLOCK_LOCKED,
    CARLOCK_LOCKOUT_PLAYER_ONLY,
    CARLOCK_LOCKED_PLAYER_INSIDE,
    CARLOCK_COP_CAR,
    CARLOCK_FORCE_SHUT_DOORS,
    CARLOCK_SKIP_SHUT_DOORS
};

enum eVehicleAppearance {
    VEHICLE_APPEARANCE_NONE = 0,
    VEHICLE_APPEARANCE_AUTOMOBILE = 1,
    VEHICLE_APPEARANCE_BIKE = 2,
    VEHICLE_APPEARANCE_HELI = 3,
    VEHICLE_APPEARANCE_BOAT = 4,
    VEHICLE_APPEARANCE_PLANE = 5,
};

enum eVehicleLightsFlags : uint32 {
    VEHICLE_LIGHTS_TWIN = 1,
    VEHICLE_LIGHTS_IGNORE_DAMAGE = 4,
    VEHICLE_LIGHTS_DISABLE_FRONT = 16,
    VEHICLE_LIGHTS_DISABLE_REAR = 32
};

enum eVehicleCreatedBy : uint8 {
    RANDOM_VEHICLE = 1,
    MISSION_VEHICLE = 2,
    PARKED_VEHICLE = 3,
    PERMANENT_VEHICLE = 4
};

enum eBombState {
    BOMB_TIMED_NOT_ACTIVATED = 1,
    BOMB_IGNITION = 2,
    BOMB_STICKY = 3,
    BOMB_TIMED_ACTIVATED = 4,
    BOMB_IGNITION_ACTIVATED = 5
};

enum eVehicleOverrideLightsState {
    NO_CAR_LIGHT_OVERRIDE = 0,
    FORCE_CAR_LIGHTS_OFF = 1,
    FORCE_CAR_LIGHTS_ON = 2
};

enum eCarPiece {
    CAR_PIECE_DEFAULT     = 0,
    CAR_PIECE_BONNET      = 1,
    CAR_PIECE_BOOT        = 2,
    CAR_PIECE_BUMP_FRONT  = 3,
    CAR_PIECE_BUMP_REAR   = 4,
    CAR_PIECE_DOOR_LF     = 5,
    CAR_PIECE_DOOR_RF     = 6,
    CAR_PIECE_DOOR_LR     = 7,
    CAR_PIECE_DOOR_RR     = 8,
    CAR_PIECE_WING_LF     = 9,
    CAR_PIECE_WING_RF     = 10,
    CAR_PIECE_WING_LR     = 11,
    CAR_PIECE_WING_RR     = 12,
    CAR_PIECE_WHEEL_LF    = 13, // front wheel for 2 wheel bike
    CAR_PIECE_WHEEL_RF    = 14,
    CAR_PIECE_WHEEL_RL    = 15, // rear wheel for 2 wheel bike
    CAR_PIECE_WHEEL_RR    = 16,
    CAR_PIECE_BIKEWHEEL_F = 17,
    CAR_PIECE_BIKEWHEEL_R = 18,
    CAR_PIECE_WINDSCREEN  = 19,
};
constexpr inline eCarPiece eCarPiece_WheelPieces[]{ CAR_PIECE_WHEEL_LF, CAR_PIECE_WHEEL_RF, CAR_PIECE_WHEEL_RL, CAR_PIECE_WHEEL_RR };

// Do not change the order!
enum eRotationAxis : int32 {
    AXIS_X = 0,
    AXIS_Y = 1,
    AXIS_Z = 2
};

typedef int32 eOrdnanceType;
typedef int32 eBikeWheelSpecial;

enum eFlightModel : int32 {
    FLIGHT_MODEL_UNK = 0,
    FLIGHT_MODEL_BARON = 1,
    FLIGHT_MODEL_RC = 2,
    FLIGHT_MODEL_PLANE = 3, // also used for cars
    FLIGHT_MODEL_UNK2 = 4,
    FLIGHT_MODEL_BOAT = 5,
    FLIGHT_MODEL_HELI = 6, // also used for hydra
};

enum tWheelState : int32 {
    WHEEL_STATE_NORMAL,   // standing still or rolling normally
    WHEEL_STATE_SPINNING, // rotating but not moving
    WHEEL_STATE_SKIDDING,
    WHEEL_STATE_FIXED,    // not rotating
};

enum class eVehicleCollisionComponent : uint16
{
    DEFAULT = 0x0,
    BONNET = 0x1,
    BOOT = 0x2,
    BUMP_FRONT = 0x3,
    BUMP_REAR = 0x4,
    DOOR_LF = 0x5,
    DOOR_RF = 0x6,
    DOOR_LR = 0x7,
    DOOR_RR = 0x8,
    WING_LF = 0x9,
    WING_RF = 0xA,
    WING_LR = 0xB,
    WING_RR = 0xC,
    WINDSCREEN = 0x13,
};
enum eTurnState
{
    TURN_OFF,
    TURN_LEFT,
    TURN_RIGHT,
    TURN_ALL
};
struct tHydraulicData {
    // applied when the vehicle is moving
    // or when hopping keys are pressed (numpad keys)
    float m_fSuspensionNormalUpperLimit;
    float m_fSuspensionNormalLowerLimit;

    // applied when the suspension is extended, like when you press the horn button (caps lock on pc)
    // or when numpad keys are pressed
    float m_fSuspensionExtendedUpperLimit;
    float m_fSuspensionExtendedLowerLimit;

    // applied when the vehicle is at rest (idle/not moving)
    // and does NOT apply if numpad keys are pressed (car hopping)
    float m_fSuspensionNormalIdleUpperLimit;
    float m_fSuspensionNormalIdleLowerLimit;
    float m_aWheelSuspension[4];
};

VALIDATE_SIZE(tHydraulicData, 0x28);

struct SCustomCarShadow
{
    RwTexture* pTexture;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    float fSizeX;
    float fSizeY;
};
enum eNeonMode : uint8_t {
    NEON_OFF        = 0,  // неон выключен
    NEON_STATIC     = 1,  // статичный цвет (color1 = eNeonPreset)
    NEON_RAINBOW    = 2,  // плавный RGB круг (speed 0..3)
    NEON_BREATHE    = 3,  // "дыхание" (color1 = eNeonPreset, speed влияет на период)
    NEON_STROBE     = 4,  // строб (color1 = eNeonPreset, speed влияет на частоту)
    NEON_POLICE     = 5,  // красно/синий (speed 0..3)
    NEON_DUALFADE   = 6,  // плавный переход color1 <-> color2 (оба eNeonPreset)
    NEON_RANDOM     = 7,  // случайный цвет (каждые N мс)
    NEON_PULSE      = 8   // пульсация яркости (color1 = eNeonPreset)
};

enum eNeonPreset : uint8_t {
    NEON_WHITE  = 0,
    NEON_RED    = 1,
    NEON_GREEN  = 2,
    NEON_BLUE   = 3,
    NEON_YELLOW = 4,
    NEON_CYAN   = 5,
    NEON_PURPLE = 6,
    NEON_PINK   = 7,
    NEON_ORANGE = 8,

    // дополнительные "штатные" цвета
    NEON_LIME   = 9,
    NEON_AQUA   = 10,
    NEON_ICE    = 11,
    NEON_GOLD   = 12,
    NEON_AMBER  = 13,
    NEON_MINT   = 14,
    NEON_VIOLET = 15,
    NEON_ROSE   = 16,
    NEON_TEAL   = 17,
    NEON_SKY    = 18
};

class CVehicle : public CPhysical {
//public:
    //   using Ref = notsa::EntityRef<CVehicle>;
public:
    CVehicle(int iType, float fX, float fY, float fZ, float fRotation, bool bPreloaded, bool bSiren);
    virtual ~CVehicle();
    void SetNeonMode(uint8_t mode, uint8_t color1, uint8_t color2, uint8_t speed);
    void SetVinyl(uint8_t vinylId);

    void SetNeonCustom(uint8_t effect, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness, uint8_t speed);
#if VER_x32
    uint8_t m_VehicleAudioEntity[588];
#else
    uint8_t m_VehicleAudioEntity[0x310];
#endif
    tHandlingData* m_pHandlingData;
    uintptr_t* pFlyingHandling;
    union{
        eVehicleHandlingFlags  m_nHandlingFlagsIntValue;
        struct {
            uint32_t b1gBoost : 1;
            uint32_t b2gBoost : 1;
            uint32_t bNpcAntiRoll : 1;
            uint32_t bNpcNeutralHandl : 1;
            uint32_t bNoHandbrake : 1;
            uint32_t bSteerRearwheels : 1;
            uint32_t bHbRearwheelSteer : 1;
            uint32_t bAltSteerOpt : 1;
            uint32_t bWheelFNarrow2 : 1;
            uint32_t bWheelFNarrow : 1;
            uint32_t bWheelFWide : 1;
            uint32_t bWheelFWide2 : 1;
            uint32_t bWheelRNarrow2 : 1;
            uint32_t bWheelRNarrow : 1;
            uint32_t bWheelRWide : 1;
            uint32_t bWheelRWide2 : 1;
            uint32_t bHydraulicGeom : 1;
            uint32_t bHydraulicInst : 1;
            uint32_t bHydraulicNone : 1;
            uint32_t bNosInst : 1;
            uint32_t bOffroadAbility : 1;
            uint32_t bOffroadAbility2 : 1;
            uint32_t bHalogenLights : 1;
            uint32_t bProcRearwheelFirst : 1;
            uint32_t bUseMaxspLimit : 1;
            uint32_t bLowRider : 1;
            uint32_t bStreetRacer : 1;
            uint32_t bSwingingChassis : 1;
        } handlingFlags;
    };
#if VER_x32
    uint8_t AutoPilot[152];
#else
    uint32_t padflagsx64;
    uint8_t AutoPilot[0xA8];
#endif
    union
    {
        struct
        {
            uint32_t m_nVehicleUpperFlags;
            uint32_t m_nVehicleLowerFlags;
        };
        struct {
            uint32_t bIsLawEnforcer : 1; // Is this guy chasing the player at the moment
            uint32_t bIsAmbulanceOnDuty : 1; // Ambulance trying to get to an accident
            uint32_t bIsFireTruckOnDuty : 1; // Firetruck trying to get to a fire
            uint32_t bIsLocked : 1; // Is this guy locked by the script (cannot be removed)
            uint32_t bEngineOn : 1; // For sound purposes. Parked cars have their engines switched off (so do destroyed cars)
            uint32_t bIsHandbrakeOn : 1; // How's the handbrake doing ?
            uint32_t bLightsOn : 1; // Are the lights switched on ?
            uint32_t bFreebies : 1; // Any freebies left in this vehicle ?

            uint32_t bIsVan : 1; // Is this vehicle a van (doors at back of vehicle)
            uint32_t bIsBus : 1; // Is this vehicle a bus
            uint32_t bIsBig : 1; // Is this vehicle a bus
            uint32_t bLowVehicle : 1; // Need this for sporty type cars to use low getting-in/out anims
            uint32_t bComedyControls : 1; // Will make the car hard to control (hopefully in a funny way)
            uint32_t bWarnedPeds : 1; // Has scan and warn peds of danger been processed?
            uint32_t bCraneMessageDone : 1; // A crane message has been printed for this car allready
            uint32_t bTakeLessDamage : 1; // This vehicle is stronger (takes about 1/4 of damage)

            uint32_t bIsDamaged : 1; // This vehicle has been damaged and is displaying all its components
            uint32_t bHasBeenOwnedByPlayer : 1;// To work out whether stealing it is a crime
            uint32_t bFadeOut : 1; // Fade vehicle out
            uint32_t bIsBeingCarJacked : 1; // Fade vehicle out
            uint32_t bCreateRoadBlockPeds : 1;// If this vehicle gets close enough we will create peds (coppers or gang members) round it
            uint32_t bCanBeDamaged : 1; // Set to FALSE during cut scenes to avoid explosions
            uint32_t bOccupantsHaveBeenGenerated : 1; // Is true if the occupants have already been generated. (Shouldn't happen again)
            uint32_t bGunSwitchedOff : 1; // Level designers can use this to switch off guns on boats

            uint32_t bVehicleColProcessed : 1;// Has ProcessEntityCollision been processed for this car?
            uint32_t bIsCarParkVehicle : 1; // Car has been created using the special CAR_PARK script command
            uint32_t bHasAlreadyBeenRecorded : 1; // Used for replays
            uint32_t bPartOfConvoy : 1;
            uint32_t bHeliMinimumTilt : 1; // This heli should have almost no tilt really
            uint32_t bAudioChangingGear : 1; // sounds like vehicle is changing gear
            uint32_t bIsDrowning : 1; // is vehicle occupants taking damage in water (i.e. vehicle is dead in water)
            uint32_t bTyresDontBurst : 1; // If this is set the tyres are invincible

            uint32_t bCreatedAsPoliceVehicle : 1;// True if this guy was created as a police vehicle (enforcer, policecar, miamivice car etc)
            uint32_t bRestingOnPhysical : 1; // Dont go static cause car is sitting on a physical object that might get removed
            uint32_t bParking : 1;
            uint32_t bCanPark : 1;
            uint32_t bFireGun : 1; // Does the ai of this vehicle want to fire it's gun?
            uint32_t bDriverLastFrame : 1; // Was there a driver present last frame ?
            uint32_t bNeverUseSmallerRemovalRange : 1;// Some vehicles (like planes) we don't want to remove just behind the camera.
            uint32_t bIsRCVehicle : 1; // Is this a remote controlled (small) vehicle. True whether the player or AI controls it.

            uint32_t bAlwaysSkidMarks : 1; // This vehicle leaves skidmarks regardless of the wheels' states.
            uint32_t bEngineBroken : 1; // Engine doesn't work. Player can get in but the vehicle won't drive
            uint32_t bVehicleCanBeTargetted : 1;// The ped driving this vehicle can be targetted, (for Torenos plane mission)
            uint32_t bPartOfAttackWave : 1; // This car is used in an attack during a gang war
            uint32_t bWinchCanPickMeUp : 1; // This car cannot be picked up by any ropes.
            uint32_t bImpounded : 1; // Has this vehicle been in a police impounding garage
            uint32_t bVehicleCanBeTargettedByHS : 1;// Heat seeking missiles will not target this vehicle.
            uint32_t bSirenOrAlarm : 1; // Set to TRUE if siren or alarm active, else FALSE

            uint32_t bHasGangLeaningOn : 1;
            uint32_t bGangMembersForRoadBlock : 1;// Will generate gang members if NumPedsForRoadBlock > 0
            uint32_t bDoesProvideCover : 1; // If this is false this particular vehicle can not be used to take cover behind.
            uint32_t bMadDriver : 1; // This vehicle is driving like a lunatic
            uint32_t bUpgradedStereo : 1; // This vehicle has an upgraded stereo
            uint32_t bConsideredByPlayer : 1; // This vehicle is considered by the player to enter
            uint32_t bPetrolTankIsWeakPoint : 1;// If false shootong the petrol tank will NOT Blow up the car
            uint32_t bDisableParticles : 1; // Disable particles from this car. Used in garage.

            uint32_t bHasBeenResprayed : 1; // Has been resprayed in a respray garage. Reset after it has been checked.
            uint32_t bUseCarCheats : 1; // If this is true will set the car cheat stuff up in ProcessControl()
            uint32_t bDontSetColourWhenRemapping : 1;// If the texture gets remapped we don't want to change the colour with it.
            uint32_t bUsedForReplay : 1; // This car is controlled by replay and should be removed when replay is done.
        } m_nVehicleFlags;
    };

    unsigned int m_nCreationTime;
    uint8_t m_nPrimaryColor;			// 1076-1077	;byteColor1
    uint8_t m_nSecondaryColor;			// 1077-1078	;byteColor2
    uint8_t m_colour3;
    uint8_t m_colour4;
    uint8_t m_comp1;
    uint8_t m_comp2;
    short  m_anUpgrades[15];
    float m_wheelScale;
    unsigned short m_nAlarmState;
    short  m_nForcedRandomRouteSeed; // if this is non-zero the random wander gets deterministic
    CPed* pDriver;			// 1120-1124	;driver
    CPed* m_apPassengers[8];	// 1124-1152	;m_apPassengers
    unsigned char  m_nNumPassengers;
    unsigned char  m_nNumGettingIn;
    unsigned char  m_nGettingInFlags;
    unsigned char  m_nGettingOutFlags;
    uint8_t             m_nMaxPassengers;
    uint8_t             m_nWindowsOpenFlags; // initialised, but not used?
    uint8_t             m_nNitroBoosts;
    int8_t              m_vehicleSpecialColIndex;
    uintptr* m_pEntityWeAreOn;
    uintptr* m_pFire;
    float  m_fSteerAngle;
    float  m_f2ndSteerAngle; // used for steering 2nd set of wheels or elevators etc..
    float  m_fGasPedal;
    float  m_fBreakPedal;
    eVehicleCreatedBy  m_nCreatedBy; // see eVehicleCreatedBy
    unsigned char  abc1;
    unsigned char  abc2;
    unsigned char  abc3;
    uint32_t cachedTotalSteer;
    short m_nExtendedRemovalRange;
    unsigned char m_nBombOnBoard : 3; // 0 = None
    // 1 = Timed
    // 2 = On ignition
    // 3 = remotely set ?
    // 4 = Timed Bomb has been activated
    // 5 = On ignition has been activated
    unsigned char m_nOverrideLights : 2; // uses enum NO_CAR_LIGHT_OVERRIDE, FORCE_CAR_LIGHTS_OFF, FORCE_CAR_LIGHTS_ON
    unsigned char m_nWinchType : 2; // Does this vehicle use a winch?
    unsigned char m_nGunsCycleIndex : 2; // Cycle through alternate gun hardpoints on planes/helis
    unsigned char m_nOrdnanceCycleIndex : 2; // Cycle through alternate ordnance hardpoints on planes/helis
    uint8_t nUsedForCover;
    uint8_t AmmoInClip;
    uint8_t PacMansCollected;
    uint8_t PedsPositionForRoadBlock;
    uint8_t NumPedsForRoadBlock[4];
    float   m_fDirtLevel; // Dirt level of vehicle body texture: 0.0f=fully clean, 15.0f=maximum dirt visible
    unsigned char m_nCurrentGear;
    uint8_t padd_1[3];
    float   m_fGearChangeCount; // used as parameter for cTransmission::CalculateDriveAcceleration, but doesn't change
    float   m_fWheelSpinForAudio;
    float fHealth;				// 1224-1228	;m_fHealth
    CVehicle* m_pTowingVehicle;
    CVehicle* m_pTrailer;
    uint8_t padd_2[3];
    bool m_bFireAutoFlare;
    CEntity *pBombOwner;
    uint32_t DontUseSmallerRemovalRange;
    uint32_t TimeOfLastShotFired;
    uint32_t m_nTimeOfDeath;
    uint16_t GetOutOfCarTimer;
    uint16_t DelayedExplosion;
    CEntity *pDelayedExplosionInflictor;
    float LastFrontHeight;
    float LastRearHeight;
    uint8_t NumOilSpillsToDo;
    uint8_t skip14[3]; // wtf?
    float OilSpillLastX;
    float OilSpillLastY;
    eCarLock m_nDoorLock;
    uint32_t m_LastTimePrimaryFired;
    uint32_t m_LastTimeSecondaryFired;
    uint32_t m_LastTimeGunFired;
    int8_t LastDamagedWeaponType;
    uint8_t skip15[3]; // wtf?
    CEntity *pLastDamageEntity;
    uint8_t m_nRadioStation;
    uint8_t m_nRainHitCount;
    uint8_t m_nSoundIndex;
    uint8_t m_SelectedWeapon;
    uint32_t m_cHorn;
    uint8_t m_nHornPattern;
    uint8_t m_NoHornCount;
    uint8_t ComedyControlsState;
    uint8_t m_hassleStatus;

    uint8_t StoredCollPolys[0x58]; // CStoredCollPoly StoredCollPolys[2];
    uint8_t m_storedCollisionLighting[4];
    uintptr_t *m_fxSysEngOverheat;
    uintptr_t *m_fxSysEngFire;
    uintptr_t *m_fxSysHeliDust;
    union {
        uint8_t m_nRenderLightsFlags;
        struct {
            uint8_t m_bRightFront : 1;
            uint8_t m_bLeftFront : 1;
            uint8_t m_bRightRear : 1;
            uint8_t m_bLeftRear : 1;
        } m_renderLights;
    };
    uint8_t skip16[3]; // wtf?
    RwTexture*   m_pCustomCarPlate;
    float        m_fRawSteerAngle; // AKA m_fSteeringLeftRight
    eVehicleType m_nVehicleType;    // Theory by forkerer:
    eVehicleType m_nVehicleSubType; // Hack to have stuff be 2 classes at once, like vortex which can act like a car and a boat
    const char* m_remapTxdName;
    const char* m_newRemapTxdName;
    RwTexture *m_pRemapTexture;

public:
public: // NOTSA functions
    // m_nVehicleType start
    [[nodiscard]] bool IsVehicleTypeValid()     const { return m_nVehicleType != VEHICLE_TYPE_IGNORE; }
    [[nodiscard]] bool IsAutomobile()           const { return m_nVehicleType == VEHICLE_TYPE_AUTOMOBILE; }
    [[nodiscard]] bool IsMonsterTruck()         const { return m_nVehicleType == VEHICLE_TYPE_MTRUCK; }
    [[nodiscard]] bool IsQuad()                 const { return m_nVehicleType == VEHICLE_TYPE_QUAD; }
    [[nodiscard]] bool IsHeli()                 const { return m_nVehicleType == VEHICLE_TYPE_HELI; }
    [[nodiscard]] bool IsPlane()                const { return m_nVehicleType == VEHICLE_TYPE_PLANE; }
    [[nodiscard]] bool IsBoat()                 const { return m_nVehicleType == VEHICLE_TYPE_BOAT; }
    [[nodiscard]] bool IsTrain()                const { return m_nVehicleType == VEHICLE_TYPE_TRAIN; }
    [[nodiscard]] bool IsFakeAircraft()         const { return m_nVehicleType == VEHICLE_TYPE_FHELI || m_nVehicleType == VEHICLE_TYPE_FPLANE; }
    [[nodiscard]] bool IsBike()                 const { return m_nVehicleType == VEHICLE_TYPE_BIKE; }
    [[nodiscard]] bool IsBMX()                  const { return m_nVehicleType == VEHICLE_TYPE_BMX; }
    [[nodiscard]] bool IsTrailer()              const { return m_nVehicleType == VEHICLE_TYPE_TRAILER ||  m_nVehicleSubType == VEHICLE_TYPE_TRAILER;}
    [[nodiscard]] bool IsRCVehicleModelID();
    // m_nVehicleType end

    // m_nVehicleSubType start
    [[nodiscard]] bool IsSubVehicleTypeValid() const { return m_nVehicleSubType != VEHICLE_TYPE_IGNORE; }
    [[nodiscard]] bool IsSubAutomobile()       const { return m_nVehicleSubType == VEHICLE_TYPE_AUTOMOBILE; }
    [[nodiscard]] bool IsSubMonsterTruck()     const { return m_nVehicleSubType == VEHICLE_TYPE_MTRUCK; }
    [[nodiscard]] bool IsSubQuad()             const { return m_nVehicleSubType == VEHICLE_TYPE_QUAD; }
    [[nodiscard]] bool IsSubHeli()             const { return m_nVehicleSubType == VEHICLE_TYPE_HELI; }
    [[nodiscard]] bool IsSubPlane()            const { return m_nVehicleSubType == VEHICLE_TYPE_PLANE; }
    [[nodiscard]] bool IsSubBoat()             const { return m_nVehicleSubType == VEHICLE_TYPE_BOAT; }
    [[nodiscard]] bool IsSubTrain()            const { return m_nVehicleSubType == VEHICLE_TYPE_TRAIN; }
    [[nodiscard]] bool IsSubFakeAircraft()     const { return m_nVehicleSubType == VEHICLE_TYPE_FHELI || m_nVehicleSubType == VEHICLE_TYPE_FPLANE; }
    [[nodiscard]] bool IsSubBike()             const { return m_nVehicleSubType == VEHICLE_TYPE_BIKE; }
    [[nodiscard]] bool IsSubBMX()              const { return m_nVehicleSubType == VEHICLE_TYPE_BMX; }
    [[nodiscard]] bool IsSubTrailer()          const { return m_nVehicleSubType == VEHICLE_TYPE_TRAILER; }

    [[nodiscard]] bool IsSubRoadVehicle()      const { return !IsSubHeli() && !IsSubPlane() && !IsSubTrain(); }
    [[nodiscard]] bool IsSubFlyingVehicle()    const { return IsSubHeli() && IsSubPlane(); }
    // m_nVehicleSubType end

    [[nodiscard]] bool IsTransportVehicle()    const { return m_nModelIndex == MODEL_TAXI    || m_nModelIndex == MODEL_CABBIE; }
    [[nodiscard]] bool IsAmphibiousHeli()      const { return m_nModelIndex == MODEL_SEASPAR || m_nModelIndex == MODEL_LEVIATHN; }
    [[nodiscard]] bool IsConstructionVehicle() const { return m_nModelIndex == MODEL_DUMPER  || m_nModelIndex == MODEL_DOZER || m_nModelIndex == MODEL_FORKLIFT; }

    [[nodiscard]] bool IsRealBike() const { return m_pHandlingData->m_bIsBike;  }
    [[nodiscard]] bool IsRealHeli() const { return m_pHandlingData->m_bIsHeli;  }
    [[nodiscard]] bool IsRealPlane()const { return m_pHandlingData->m_bIsPlane; }
    [[nodiscard]] bool IsRealBoat() const { return m_pHandlingData->m_bIsBoat;  }

    [[nodiscard]] eVehicleCreatedBy GetCreatedBy() const      { return m_nCreatedBy; }
    [[nodiscard]] bool IsCreatedBy(eVehicleCreatedBy v) const { return v == m_nCreatedBy; }
    [[nodiscard]] bool IsMissionVehicle() const { return m_nCreatedBy == MISSION_VEHICLE; }
    auto GetMaxPassengerSeats() { return std::span{ m_apPassengers, m_nMaxPassengers }; }

    void SetGettingInFlags(uint8 doorId);
    void SetGettingOutFlags(uint8 doorId);
    void ClearGettingInFlags(uint8 doorId);
    void ClearGettingOutFlags(uint8 doorId);

    bool GetLightsStatus() { return m_nVehicleFlags.bLightsOn;}
    void SetLightStatus(bool status) { m_nVehicleFlags.bLightsOn = status; }
public:
    static inline auto m_aSpecialColModel = std::array<CColModel, 4>();

public:
    static void InjectHooks();

    bool AddPassenger(CPed* passenger);
    bool AddPassenger(CPed* passenger, uint8 seatNumber);
    void RemovePassenger(CPed* passenger);
    void SetDriver(CPed* driver);
    void RemoveDriver(bool arg0);
    void ApplyTurnForceToOccupantOnEntry(CPed* passenger);
    void RenderDriverAndPassengers();
    void PreRenderDriverAndPassengers();
    // get special ride anim data for bile or quad
    CRideAnimData* GetRideAnimData() { return nullptr; }
    int GetPassengerIndex(const CPed* ped);

    void AddVehicleUpgrade(int32 modelId);
    void RemoveVehicleUpgrade(int32 upgradeModelIndex);
    void SetNumberPlate(const char* plate);
    const char* GetNumberPlate() const { return m_szPlateText; }
    int GetVehicleSubtype();
    void AddComponent(int iComponentID);
    void RemoveComponent(int iComponentID);
    void SetPaintJob(uint8_t bytePaintJobID);
    void SetColor(uint8_t iColor1, uint8_t iColor2);
    void LinkToInterior(int iInterior);
    void SetDamageStatus(uint32_t dwPanelDamageStatus, uint32_t dwDoorDamageStatus, uint8_t byteLightDamageStatus);
    void SetTireDamageStatus(uint8_t byteTireDamageStatus);
    void SetPlateText(const char* szPlateText) { strncpy(m_szPlateText, szPlateText, 32); }
    void SetZAngle(float fAngle);
    void ProcessMarkers();
    bool IsOccupied();

    void AttachTrailer();
    void DetachTrailer();
    void SetTrailer(CVehicle* pTrailer);

    bool IsRCVehicle();
    float GetHealth();
    void SetHealth(float fHealth);
    CVehicle* GetTrailer();
    CVehicle* GetTractor();
    float GetTrainSpeed();
    uint16_t GetHydraThrusters();
    float GetBikeLean();
    bool IsATrainPart();
    bool IsATowTruck();
    bool IsATrailer();
    bool VerifyInstance();
    bool IsDriverLocalPlayer();
    bool GetNeonUIState(bool& enabled, int& effect, int& r, int& g, int& b, int& brightness, int& speed);
    void SetInvulnerable(bool bInv);
    bool HasSunk();
    bool HasADriver();

    int GetEngineState() { return m_iEngineState; }
    int GetLightState() { return m_iLightState; }

    void ApplyEngineState(int iState);
    void ApplyLightState(int state);

    void SetDoorState(int state);
    void SetComponentOpenState(int iDoor, int iComponent, float fDoorOpenRatio);
    void SetAlarmState(uint16_t wAlarmState) { m_pVehicle->m_nVehicleFlags.bSirenOrAlarm = wAlarmState; }
    void OpenWindow(uint8_t component);
    void CloseWindow(uint8_t component);

    void RemoveEveryoneFromVehicle();

    void UpdateColor();
    bool UpdateLastDrivenTime();

public:
    CVehicle* m_pVehicle;

    CObject*        m_pLeftFrontTurnLighter = nullptr;
    CObject*	    m_pRightFrontTurnLighter = nullptr;
    CObject*	    m_pLeftRearTurnLighter = nullptr;
    CObject*	    m_pRightRearTurnLighter = nullptr;
    eTurnState 		m_iTurnState = TURN_OFF;
    bool 			m_bIsOnRightTurnLight = false;
    bool 			m_bIsOnLeftTurnLight = false;
    bool 			m_bIsOnAllTurnLight = false;

    bool            m_bIsTurnLight;
    uint32_t        m_iTickTurnLight;
    uint32_t        lasttick;

    // ================= NEON ===================
    enum eNeonTypes {
        ON_TYPE_DEF,
        ON_TYPE_RAINBOW
    };
    CRGBA neonColor		{};
    bool m_bShadow;
    SCustomCarShadow m_Shadow;
    eNeonTypes neonType;
    uint8_t neonStage;
    // ==========================================

    uintptr 		m_dwGTAId;
    bool m_bPreloaded;
    int m_iEngineState;
    int m_iLightState;
    bool m_bIsInvulnerable;
    bool m_bIsLocked;
    bool m_bDoorsLocked;
    uint8_t m_byteObjectiveVehicle;
    bool m_bSpecialMarkerEnabled;
    uint8_t m_byteColor1;
    uint8_t m_byteColor2;
    bool m_bHaveColor;
    bool m_bSiren;
    char m_szPlateText[32];
    uint32_t m_dwMarkerID;

    bool m_bWheelAlignmentX;
    float m_fWheelAlignmentX;

    bool m_bWheelAlignmentY;
    float m_fWheelAlignmentY;

    bool m_bWheelSize;
    float m_fWheelSize;

    bool m_bWheelWidth;
    float m_fWheelWidth;

    bool m_bWheelOffsetX;
    float m_fWheelOffsetX;

    bool m_bWheelOffsetY;
    float m_fWheelOffsetY;

    float m_fNewOffsetX;
    float m_fNewOffsetY;

    bool m_bWasWheelOffsetProcessedX;
    bool m_bWasWheelOffsetProcessedY;
    uint32_t m_uiLastProcessedWheelOffset;

    void SetWheelAlignment(int iWheel, float angle);
    void SetWheelOffset(int iWheel, float offset);
    void SetWheelWidth(float fValue);
    void ProcessWheelsOffset();
    void ProcessWheelOffset(RwFrame* pFrame, bool bLeft, float fValue, int iID);

    RwMatrix m_vInitialWheelMatrix[4];

    void UpdateDamageStatus(uint32_t dwPanelDamage, uint32_t dwDoorDamage, uint8_t byteLightDamage);
    void SetPanelDamageStatus(uint32_t dwPanelStatus);
    uint32_t GetPanelDamageStatus();
    void SetDoorDamageStatus(uint32_t dwDoorStatus);
    uint32_t GetDoorDamageStatus();
    void SetLightDamageStatus(uint8_t byteLightStatus);
    uint8_t GetLightDamageStatus();
    void SetWheelPoppedStatus(uint8_t byteWheelStatus);
    uint8_t GetWheelPoppedStatus();

    bool SirenEnabled();

    void EnableSiren(bool bState);

    bool IsLandingGearNotUp();

    bool			m_bHasBeenDriven;
    uint32_t		m_dwTimeSinceLastDriven;\


    void ApplyTexture(const char *szTexture, const char *szNew);

    bool IsRetextured(const char *szOldTexture);

    void NeonProcess();
    void NeonRender();

    void ProcessTurnLight(int isRight, int lightId);
    static void ProcessShadowLight(CVehicle* m_pVehicle, CVector coords, int IsRight, int id);

    void
    SetCustomShadow(uint8_t r, uint8_t g, uint8_t b, float fSizeX, float fSizeY, const char *szTex);

};

//VALIDATE_SIZE(CVehicle, (VER_x32 ? 0x760 : 0x918));