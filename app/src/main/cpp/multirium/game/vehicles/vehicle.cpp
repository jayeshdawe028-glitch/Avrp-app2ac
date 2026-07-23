#include "main.h"
#include "game.h"
#include "net/netgame.h"
#include "vehicle.h"
#include "Streaming.h"
#include "game/custom/VehicleNeon.h"
#include "game/custom/VehicleVinyl.h"
#include "game/modelinfo/ModelInfo.h"
#include "game/renderer/Shadows.h"
#include <unordered_map>
#include <cmath>
extern CGame* pGame;
extern CNetGame* pNetGame;

struct VinylState {
    uint8_t id = 0; // 0 = нет винила
};

static std::unordered_map<uint32_t, VinylState> g_vinylByGtaId;


static bool IsRemapTexture(RwTexture* tex)
{
    if (!tex) return false;
    const char* name = RwTextureGetName(tex);
    if (!name) return false;

    // case-insensitive "remap" в начале или внутри
    // (быстро, без tolower на каждую букву — просто проверим несколько вариантов)
    if (!strncasecmp(name, "remap", 5)) return true;
    if (strstr(name, "remap")) return true;
    if (strstr(name, "Remap")) return true;
    if (strstr(name, "REMAP")) return true;

    return false;
}
static bool IsBodyMaterialTexture(const char* texName)
{
    if (!texName || !texName[0]) return false;

    // запретные / точно не кузов
    if (!strcmp(texName, "plate_remap")) return false;
    if (!strcmp(texName, "vehiclelights128")) return false;
    if (!strcmp(texName, "damage")) return false;
    if (!strcmp(texName, "wheels")) return false;
    if (!strcmp(texName, "undercarriage")) return false;
    if (!strcmp(texName, "frame")) return false;
    if (!strcmp(texName, "radiator")) return false;
    if (!strcmp(texName, "grille_01")) return false;

    // КУЗОВ (самый частый кейс)
    if (!strcmp(texName, "white")) return true;

    // альтернативные варианты кузова
    if (!strcmp(texName, "body")) return true;

    // если модель уже использует remapbody* как материал на кузове
    if (!strncmp(texName, "remapbody", 8)) return true;

    return false;
}
static RpAtomic* ApplyRemapAtomicCB(RpAtomic* atomic, void* data)
{
    RwTexture* newTex = (RwTexture*)data;
    if (!atomic || !newTex) return atomic;

    RpGeometry* geom = RpAtomicGetGeometry(atomic);
    if (!geom) return atomic;

    RpMaterialList* ml = &geom->matList;
    if (!ml || !ml->materials || ml->numMaterials <= 0) return atomic;

    for (int i = 0; i < ml->numMaterials; ++i)
    {
        RpMaterial* mat = ml->materials[i];
        if (!mat) continue;

        RwTexture* cur = RpMaterialGetTexture(mat);
        if (!cur) continue;

        const char* tn = RwTextureGetName(cur);
        if (!tn) continue;

        // НЕ трогаем номер
        if (!strcmp(tn, "plate_remap"))
            continue;

        if (IsBodyMaterialTexture(tn))
        {
            RpMaterialSetTexture(mat, newTex);
        }
    }

    return atomic;
}

static void ApplyRemapToClump(RpClump* clump, RwTexture* remapTex)
{
    if (!clump || !remapTex) return;
    RpClumpForAllAtomics(clump, ApplyRemapAtomicCB, remapTex);
}
// ------------------------------------------------------------
// Neon state (храним отдельно, чтобы НЕ менять размер CVehicle)
// ------------------------------------------------------------
struct NeonState {
    uint8_t mode   = NEON_OFF;
    uint8_t c1     = NEON_WHITE;  // основной цвет / параметр
    uint8_t c2     = NEON_WHITE;  // второй цвет (для DUALFADE)
    uint8_t speed  = 1;           // 0..3 (медленно->быстро), по умолчанию 1
    uint32_t t0    = 0;           // базовое время для анимаций
    uint8_t  rndHue = 0;          // для RANDOM
    // custom RGB (для оттенка с колёсика)
    uint8_t custom = 0;           // 0=используем preset, 1=используем r/g/b
    uint8_t r = 255, g = 255, b = 255;
    uint8_t brightness = 100;     // 0..100
};
static std::unordered_map<uint32_t, NeonState> g_neonByGtaId;

static inline void NeonPresetRGB(uint8_t preset, uint8_t& r, uint8_t& g, uint8_t& b)
{
    switch (preset)
    {
        default:
        case NEON_WHITE:  r=255; g=255; b=255; break;
        case NEON_RED:    r=255; g= 40; b= 40; break;
        case NEON_GREEN:  r= 40; g=255; b= 60; break;
        case NEON_BLUE:   r= 60; g=140; b=255; break;
        case NEON_YELLOW: r=255; g=220; b= 40; break;
        case NEON_CYAN:   r= 40; g=255; b=255; break;
        case NEON_PURPLE: r=180; g= 60; b=255; break;
        case NEON_PINK:   r=255; g= 80; b=200; break;
        case NEON_ORANGE: r=255; g=140; b= 40; break;

        case NEON_LIME:   r=180; g=255; b= 40; break;
        case NEON_AQUA:   r= 40; g=220; b=255; break;
        case NEON_ICE:    r=160; g=210; b=255; break;
        case NEON_GOLD:   r=255; g=190; b= 80; break;
        case NEON_AMBER:  r=255; g=160; b= 40; break;
        case NEON_MINT:   r=120; g=255; b=190; break;
        case NEON_VIOLET: r=150; g= 80; b=255; break;
        case NEON_ROSE:   r=255; g= 90; b=140; break;
        case NEON_TEAL:   r= 40; g=255; b=190; break;
        case NEON_SKY:    r= 80; g=190; b=255; break;
    }
}

bool bInProcessDetachTrailer = false;

CVehicle::CVehicle(int iType, float fX, float fY, float fZ, float fRotation, bool bPreloaded, bool bSiren)
{
    RwMatrix matEnt;
    static CVehicle* pCreatedTrain = nullptr;

    m_pVehicle = nullptr;
    m_dwGTAId = 0;
    m_pTrailer = nullptr;

    if ((iType != TRAIN_PASSENGER_LOCO) &&
        (iType != TRAIN_FREIGHT_LOCO) &&
        (iType != TRAIN_PASSENGER) &&
        (iType != TRAIN_FREIGHT) &&
        (iType != TRAIN_TRAM)) {

        if (!CStreaming::TryLoadModel(iType))
            throw std::runtime_error(OBFUSCATE("Model not loaded"));

        ScriptCommand(&create_car, iType, fX, fY, fZ, &m_dwGTAId);

        Log("created car gtaId=%u model=%d", (unsigned)m_dwGTAId, iType);
        ScriptCommand(&set_car_z_angle, m_dwGTAId, fRotation);
        ScriptCommand(&car_gas_tank_explosion, m_dwGTAId, 0);
        ScriptCommand(&set_car_hydraulics, m_dwGTAId, 0);
        ScriptCommand(&toggle_car_tires_vulnerable, m_dwGTAId, 0);


        m_pVehicle = GamePool_Vehicle_GetAt(m_dwGTAId);

        if (m_pVehicle)
        {
            m_pVehicle->m_nDoorLock = (eCarLock)0;
            m_bIsLocked = false;
            m_pVehicle->GetMatrix(&matEnt);
            matEnt.pos.x = fX;
            matEnt.pos.y = fY;
            matEnt.pos.z = fZ;

            if( GetVehicleSubtype() != VEHICLE_SUBTYPE_BIKE &&
                GetVehicleSubtype() != VEHICLE_SUBTYPE_PUSHBIKE)
                matEnt.pos.z += 0.25f;

            m_pVehicle->SetMatrix((CMatrix&)matEnt);
            m_bPreloaded = bPreloaded;
        }

        //Remove();
        //pGame->RemoveModel(iType, false);
    }
    else if ((iType == TRAIN_PASSENGER_LOCO) ||
             (iType == TRAIN_FREIGHT_LOCO) ||
             (iType == TRAIN_TRAM))
    {
        if (iType == TRAIN_PASSENGER_LOCO) iType = 5;
        else if (iType == TRAIN_FREIGHT_LOCO) iType = 3;
        else if (iType == TRAIN_TRAM) iType = 9;

        uint32_t dwDirection = 0;
        if (fRotation > 180.0f) {
            dwDirection = 1;
        }

        if (!CStreaming::TryLoadModel(TRAIN_PASSENGER_LOCO))
            throw std::runtime_error(OBFUSCATE("Model not loaded"));

        if (!CStreaming::TryLoadModel(TRAIN_PASSENGER))
            throw std::runtime_error(OBFUSCATE("Model not loaded"));

        if (!CStreaming::TryLoadModel(TRAIN_FREIGHT_LOCO))
            throw std::runtime_error(OBFUSCATE("Model not loaded"));

        if (!CStreaming::TryLoadModel(TRAIN_FREIGHT))
            throw std::runtime_error(OBFUSCATE("Model not loaded"));
        if (!CStreaming::TryLoadModel(TRAIN_TRAM))
            throw std::runtime_error(OBFUSCATE("Model not loaded"));

        ScriptCommand(&create_train, iType, fX, fY, fZ, dwDirection, &m_dwGTAId);
        m_pVehicle = GamePool_Vehicle_GetAt(m_dwGTAId);

        pCreatedTrain = m_pVehicle;

        GamePrepareTrain(m_pVehicle);
    }
    else if ((iType == TRAIN_PASSENGER) ||
             (iType == TRAIN_FREIGHT))
    {
        if (!pCreatedTrain)
        {
            m_pVehicle = nullptr;
            return;
        }

        m_dwGTAId = GamePool_Vehicle_GetIndex(m_pVehicle);


        // init neon (default OFF)
        g_neonByGtaId[m_dwGTAId] = NeonState{};
        pCreatedTrain = m_pVehicle;
    }
    /*for (size_t i = 0; i < MAX_REPLACED_TEXTURES; i++)
    {
        m_bReplaceTextureStatus[i] = false;
        memset(&(m_szReplacedTextures[i].szOld[0]), 0, MAX_REPLACED_TEXTURE_NAME);
        m_szReplacedTextures[i].pTexture = nullptr;
    }

    m_bReplacedTexture = false;*/

    m_bIsInvulnerable = false;
    m_byteObjectiveVehicle = 0;
    m_bSpecialMarkerEnabled = false;
    m_dwMarkerID = 0;
    //_this->field_6D = 0;
    //_this->field_69 = GetTickCount();
    m_iEngineState = -1;
    m_iLightState = -1;
    m_bDoorsLocked = false;
    m_bHaveColor = false;
    m_bHasBeenDriven = false;
    m_dwTimeSinceLastDriven = GetTickCount();

    m_bWheelSize = false;
    m_bWheelWidth = false;
    m_bWheelAlignmentX = false;
    m_bWheelAlignmentY = false;
    m_bWheelOffsetX = false;
    m_bWheelOffsetY = false;
    m_fWheelOffsetX = 0.0f;
    m_fWheelOffsetY = 0.0f;
    m_fNewOffsetX = 0.0f;
    m_fNewOffsetY = 0.0f;
    m_bWasWheelOffsetProcessedX = true;
    m_bWasWheelOffsetProcessedY = true;
    m_uiLastProcessedWheelOffset = 0;

    m_bShadow = false;
    m_Shadow.pTexture = nullptr;
    neonStage = 0;
    neonType = eNeonTypes::ON_TYPE_DEF;
    m_iTickTurnLight = GetTickCount();
    m_bIsTurnLight = false;
    m_iTurnState = eTurnState::TURN_OFF;
    //_this->field_71 = 0;
    //_this->field_75 = 0;
    //_this->field_8F = 0;
    memset(m_szPlateText, 0, sizeof(m_szPlateText));
}

CVehicle::~CVehicle()
{
    if (m_dwGTAId)
        g_neonByGtaId.erase(m_dwGTAId);

    m_pVehicle = GamePool_Vehicle_GetAt(m_dwGTAId);

    if (m_pVehicle) {
        auto modelId = m_pVehicle->m_nModelIndex;
        if(m_dwMarkerID)
        {
            pGame->DisableMarker(m_dwMarkerID);
            m_dwMarkerID = 0;
        }

        RemoveEveryoneFromVehicle();

        if(SirenEnabled()) EnableSiren(false);

        if (m_pTrailer) {
            DetachTrailer();
            m_pTrailer = nullptr;
        }

        int iModel = m_pVehicle->m_nModelIndex;
        if (iModel == 538 || iModel == 537)
        {
            ScriptCommand(&destroy_train, m_dwGTAId);
        }
        else
        {
            ScriptCommand(&destroy_car, m_dwGTAId);
        }

        CStreaming::RemoveModelIfNoRefs(modelId);
    }

    if (m_bShadow) {
        if (m_Shadow.pTexture) {
            RwTextureDestroy(m_Shadow.pTexture);
            m_Shadow.pTexture = nullptr;
        }
        m_bShadow = false;
        neonStage = 0;
        neonType = eNeonTypes::ON_TYPE_DEF;
    }
}
void CVehicle::ApplyTexture(const char* szTexture, const char* szNew)
{
    if (IsRetextured(szTexture))
    {
        //RemoveTexture(szTexture);
    }

    uint8_t bID = 255;
    /*for (uint8_t i = 0; i < MAX_REPLACED_TEXTURES; i++)
    {
        if (m_bReplaceTextureStatus[i] == false)
        {
            bID = i;
            break;
        }
    }

    if (bID == 255)
    {
        return;
    }

    m_bReplaceTextureStatus[bID] = true;
    strcpy(&(m_szReplacedTextures[bID].szOld[0]), szTexture);
    m_szReplacedTextures[bID].pTexture = (RwTexture*)LoadTextureFromDB("samp", szNew);

    m_bReplacedTexture = true;*/
}
bool CVehicle::IsRetextured(const char* szOldTexture)
{
    /* for (size_t i = 0; i < MAX_REPLACED_TEXTURES; i++)
     {
         if (m_bReplaceTextureStatus[i])
         {
             if (!strcmp(m_szReplacedTextures[i].szOld, szOldTexture))
             {
                 return true;
             }
         }
     }*/
    return false;
}
// 0.3.7
bool CVehicle::IsRCVehicle()
{
    if (m_pVehicle)
    {
        if (GamePool_Vehicle_GetAt(m_dwGTAId))
        {
            int iModel = m_pVehicle->m_nModelIndex;
            if (iModel == 441
                || iModel == 464
                || iModel == 465
                || iModel == 594
                || iModel == 501
                || iModel == 564) {
                return true;
            }
        }
    }

    return false;
}
// 0.3.7
float CVehicle::GetHealth()
{
    if (m_pVehicle) {
        return m_pVehicle->fHealth;
    }

    return 0.0f;
}
// 0.3.7
void CVehicle::SetHealth(float fHealth)
{
    if (m_pVehicle) {
        m_pVehicle->fHealth = fHealth;
    }
}
// 0.3.7
int CVehicle::GetVehicleSubtype()
{
    if (m_pVehicle) {
        return ::GetVehicleSubtype(m_pVehicle);
    }

    return 0;
}
// 0.3.7
void CVehicle::AddComponent(int iComponentID)
{
    if (!m_pVehicle || !GamePool_Vehicle_GetAt(m_dwGTAId)) return;
    if (GetVehicleSubtype() != VEHICLE_SUBTYPE_CAR) return;

    if (!CStreaming::TryLoadModel(iComponentID))
        throw std::runtime_error("Model not loaded");

    if (!ScriptCommand(&is_component_available, iComponentID)) {
        return;
    }

    uint32_t dwRet;
    ScriptCommand(&add_car_component, m_dwGTAId, iComponentID, &dwRet);
}
// 0.3.7
void CVehicle::SetPaintJob(uint8_t bytePaintJobID)
{
    if (m_pVehicle && GamePool_Vehicle_GetAt(m_dwGTAId))
    {
        if (GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR)
        {
            if (bytePaintJobID <= 3) {
                if (m_dwGTAId) ScriptCommand(&change_car_skin, m_dwGTAId, bytePaintJobID);
            }
        }
    }
}
// 0.3.7
void CVehicle::SetColor(uint8_t byteColor1, uint8_t byteColor2)
{
    if (m_pVehicle && GamePool_Vehicle_GetAt(m_dwGTAId))
    {
        m_pVehicle->m_nPrimaryColor = byteColor1;
        m_pVehicle->m_nSecondaryColor = byteColor2;
    }

    m_byteColor1 = byteColor1;
    m_byteColor2 = byteColor2;
    m_bHaveColor = true;
}
// 0.3.7
void CVehicle::DetachTrailer()
{
    bInProcessDetachTrailer = true;
    if (m_pTrailer)
    {
        if (m_dwGTAId && GamePool_Vehicle_GetAt(m_dwGTAId))
        {
            if (m_pTrailer->m_pVehicle)
                ScriptCommand(&detach_trailer_from_cab, m_pTrailer->m_dwGTAId, m_dwGTAId);
        }
    }
}
// 0.3.7
void CVehicle::SetTrailer(CVehicle* pTrailer)
{
    m_pTrailer = pTrailer;
}
// 0.3.7
void CVehicle::LinkToInterior(int iInterior)
{
    if(m_pVehicle)
        m_pVehicle->SetInterior(iInterior);
}

void CVehicle::SetDamageStatus(uint32_t dwPanelDamageStatus, uint32_t dwDoorDamageStatus, uint8_t byteLightDamageStatus)
{

}

void CVehicle::SetTireDamageStatus(uint8_t byteTireDamageStatus)
{

}
// 0.3.7
void CVehicle::RemoveEveryoneFromVehicle()
{
    if (!m_pVehicle) return;
    if (!GamePool_Vehicle_GetAt(m_dwGTAId)) return;

    if (!m_pVehicle) return;
    if(!m_dwGTAId)return;
    if (!GamePool_Vehicle_GetAt(m_dwGTAId)) return;

    if (m_pVehicle->pDriver)
    {
        m_pVehicle->pDriver->RemoveFromVehicle();
    }

    for (int i = 0; i < 7; i++)
    {
        if (m_pVehicle->m_apPassengers[i] != nullptr)
        {
            m_pVehicle->m_apPassengers[i]->RemoveFromVehicle();
        }
    }
}
// 0.3.7
CVehicle* CVehicle::GetTrailer()
{
    if (m_pVehicle)
    {
        CVehicle* pTrailer = m_pVehicle->m_pTrailer;
        if (pTrailer)
        {
            if (pNetGame)
            {
                CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
                if (pVehiclePool) {
                    VEHICLEID TrailerID = pVehiclePool->FindIDFromGtaPtr(pTrailer);
                    if (TrailerID != INVALID_VEHICLE_ID)
                    {
                        return pVehiclePool->GetAt(TrailerID);
                    }
                }
            }
        }
    }

    return nullptr;
}
// 0.3.7
CVehicle* CVehicle::GetTractor()
{
    if (!m_pVehicle) return nullptr;

    int iSubType = ::GetVehicleSubtype(m_pVehicle);
    if (iSubType == VEHICLE_SUBTYPE_BIKE
        || iSubType == VEHICLE_SUBTYPE_BOAT
        || iSubType == VEHICLE_SUBTYPE_TRAIN
        || iSubType == VEHICLE_SUBTYPE_PUSHBIKE)
    {
        return nullptr;
    }

    CVehicle* pTractor = m_pVehicle->m_pTowingVehicle;
    if (pTractor)
    {
        if (pNetGame)
        {
            CVehiclePool* pVehiclePool = pNetGame->GetVehiclePool();
            if (pVehiclePool)
            {
                VEHICLEID TrailerID = pVehiclePool->FindIDFromGtaPtr(pTractor);
                if (TrailerID != INVALID_VEHICLE_ID)
                {
                    return pVehiclePool->GetAt(TrailerID);
                }
            }
        }
    }

    return nullptr;
}
// 0.3.7
float CVehicle::GetTrainSpeed()
{
    //if (m_pVehicle) {
    //	return m_pVehicle->fTrainSpeed;
    //}

    return 0.0f;
}
// 0.3.7 (dword -> word)
uint16_t CVehicle::GetHydraThrusters()
{
    //if (m_pVehicle) {
    //	return m_pVehicle->wHydraThrusters;
    //}

    return 0.0f;
}

bool CVehicle::IsLandingGearNotUp()
{
    //if(m_pVehicle)
    //return m_pVehicle->fPlaneLandingGear != 0.0f;

    return 0;
}

float CVehicle::GetBikeLean()
{
    //if (m_pVehicle && GamePool_Vehicle_GetAt(m_dwGTAId))
    //return m_pVehicle->fBikeLean;

    return 0.0f;
}
// 0.3.7
bool CVehicle::IsATrainPart()
{
    if (m_pVehicle)
    {
        int iModelIndex = m_pVehicle->m_nModelIndex;
        if (iModelIndex == 538 || iModelIndex == 570 || iModelIndex == 537 ||
            iModelIndex == 569 || iModelIndex == 449) {
            return true;
        }
    }

    return false;
}
// 0.3.7
bool CVehicle::VerifyInstance()
{
    return GamePool_Vehicle_GetAt(m_dwGTAId) != nullptr;
}
// 0.3.7
void CVehicle::SetDoorState(int state)
{
    if (state)
    {
        m_pVehicle->m_nDoorLock = (eCarLock)2;
        m_bDoorsLocked = true;
    }
    else
    {
        m_pVehicle->m_nDoorLock =(eCarLock) 0;
        m_bDoorsLocked = false;
    }
}

void CVehicle::SetComponentOpenState(int iDoor, int iComponent, float fDoorOpenRatio)
{
    if (GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR)
    {
        ScriptCommand(&set_car_door_rotation_to, m_dwGTAId, iDoor, iComponent, fDoorOpenRatio);

    }
}

void CVehicle::OpenWindow(uint8_t component)
{
    if (m_pVehicle) {

    }
}

void CVehicle::CloseWindow(uint8_t component)
{
    if (m_pVehicle) {

    }
}
// 0.3.7
void CVehicle::AttachTrailer()
{
    if (m_pTrailer) {
        ScriptCommand(&put_trailer_on_cab, m_pTrailer->m_dwGTAId, m_dwGTAId);
    }
}
// 0.3.7
void CVehicle::RemoveComponent(int iComponentID)
{
    if (!m_pVehicle || !GamePool_Vehicle_GetAt(m_dwGTAId))
        return;

    ScriptCommand(&remove_component, m_dwGTAId, iComponentID);
}
// 0.3.7
void CVehicle::SetZAngle(float fAngle)
{
    if (GamePool_Vehicle_GetAt(m_dwGTAId)) {
        ScriptCommand(&set_car_z_angle, m_dwGTAId, fAngle);
    }
}
// 0.3.7
bool CVehicle::IsDriverLocalPlayer()
{
    if (m_pVehicle && m_pVehicle->pDriver == GamePool_FindPlayerPed())
        return true;

    return false;
}
// 0.3.7
void CVehicle::SetInvulnerable(bool bInv)
{
    if (m_pVehicle && GamePool_Vehicle_GetAt(m_dwGTAId))
    {
        if (bInv)
        {
            ScriptCommand(&set_car_immunities, m_dwGTAId, 1, 1, 1, 1, 1);
            ScriptCommand(&toggle_car_tires_vulnerable, m_dwGTAId, 0);
            m_bIsInvulnerable = true;
        }
        else
        {
            ScriptCommand(&set_car_immunities, m_dwGTAId, 0, 0, 0, 0, 0);
            ScriptCommand(&toggle_car_tires_vulnerable, m_dwGTAId, 1);
            m_bIsInvulnerable = false;
        }
    }
}
// 0.3.7
bool CVehicle::HasSunk()
{
    if (m_pVehicle) {
        return ScriptCommand(&has_car_sunk, m_dwGTAId);
    }

    return false;
}
// 0.3.7
bool CVehicle::HasADriver()
{
    if (m_pVehicle && GamePool_Vehicle_GetAt(m_dwGTAId))
    {
        if (m_pVehicle->pDriver)
        {
            if (m_pVehicle->pDriver->IsInVehicle() &&
                m_pVehicle->pDriver->m_nPedType == (ePedType)0)
                return true;
        }
    }

    return false;
}

void CVehicle::ApplyEngineState(int iState)
{
    if (!m_pVehicle) return;

    m_iEngineState = iState;

    m_pVehicle->m_nVehicleFlags.bEngineOn = iState;
}

void CVehicle::ApplyLightState(int iState)
{
    if (!m_pVehicle) return;
    //if(iState > 1)
    //	return;

    m_iLightState = iState;
    m_pVehicle->m_nVehicleFlags.bLightsOn = iState;
}
// 0.3.7
void CVehicle::ProcessMarkers()
{
    if(!m_pVehicle || !GamePool_Vehicle_GetAt(m_dwGTAId))
        return;

    if(m_byteObjectiveVehicle)
    {
        if(!m_bSpecialMarkerEnabled)
        {
            if(m_dwMarkerID)
            {
                pGame->DisableMarker(m_dwMarkerID);
                m_dwMarkerID = 0;
            }

            ScriptCommand(&tie_marker_to_car, m_dwGTAId, 1, 3, &m_dwMarkerID);
            ScriptCommand(&set_marker_color, m_dwMarkerID, 1006);
            ScriptCommand(&show_on_radar, m_dwMarkerID, 3);
            m_bSpecialMarkerEnabled = true;
        }

        return;
    }

    if(m_byteObjectiveVehicle && m_bSpecialMarkerEnabled)
    {
        if(m_dwMarkerID)
        {
            pGame->DisableMarker(m_dwMarkerID);
            m_dwMarkerID = 0;

            m_bSpecialMarkerEnabled = false;
        }
    }

    if(m_pVehicle->GetDistanceFromLocalPlayerPed() < 200.0f && GetTractor() == NULL &&
       !IsOccupied())
    {
        if(!m_dwMarkerID)
        {
            ScriptCommand(&tie_marker_to_car, m_dwGTAId, 1, 2, &m_dwMarkerID);
            ScriptCommand(&set_marker_color, m_dwMarkerID, 1004);
        }
    }

    else if(IsOccupied() || GetTractor() != NULL ||
            m_pVehicle->GetDistanceFromLocalPlayerPed() >= 200.0f)
    {
        if(m_dwMarkerID)
        {
            pGame->DisableMarker(m_dwMarkerID);
            m_dwMarkerID = 0;
        }
    }
}
// 0.3.7
bool CVehicle::IsOccupied()
{
    if (m_pVehicle)
    {
        if (m_pVehicle &&
            (m_pVehicle->m_apPassengers[0] ||
             m_pVehicle->m_apPassengers[1] ||
             m_pVehicle->m_apPassengers[2] ||
             m_pVehicle->m_apPassengers[3] ||
             m_pVehicle->m_apPassengers[4] ||
             m_pVehicle->m_apPassengers[5] ||
             m_pVehicle->m_apPassengers[6])) {
            return true;
        }
    }

    return false;
}

bool CVehicle::IsATowTruck()
{
    if(!m_pVehicle) return false;
    return m_pVehicle->m_nModelIndex == 525;
}

bool CVehicle::IsATrailer()
{
    if(!m_pVehicle) return false;

    int nModel = m_pVehicle->m_nModelIndex;
    return (nModel == 435 ||
            nModel == 450 ||
            nModel == 584 ||
            nModel == 591 ||
            nModel == 606 ||
            nModel == 607 ||
            nModel == 608 ||
            nModel == 610 ||
            nModel == 611
    );
}

void CVehicle::UpdateDamageStatus(uint32_t dwPanelDamage, uint32_t dwDoorDamage, uint8_t byteLightDamage)
{
    if(m_pVehicle)
    {
        if(GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR)
        {
            if(!dwPanelDamage && !dwDoorDamage && !byteLightDamage)
            {
                if(GetPanelDamageStatus() || GetDoorDamageStatus() || GetLightDamageStatus())
                {
                    // CAutoMobile::Fix
                    (( void (*)(CVehicle*))(g_libGTASA + (VER_x32 ? 0x55D5C0 + 1 : 0x67DF0C)))(m_pVehicle);
                    return;
                }
            }

            SetDoorDamageStatus(dwDoorDamage);
            SetPanelDamageStatus(dwPanelDamage);
            SetLightDamageStatus(byteLightDamage);

            // CAutomobile::SetupDamageAfterLoad
            (( void (*)(CVehicle*))(g_libGTASA + (VER_x32 ? 0x55D886 + 1 : 0x67E368)))(m_pVehicle);
        }
    }
}

void CVehicle::SetPanelDamageStatus(uint32_t dwPanelStatus)
{
    //if(m_pVehicle && GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR)
    //m_pVehicle->dwPanelStatus = dwPanelStatus;
}

uint32_t CVehicle::GetPanelDamageStatus()
{
    //if(m_pVehicle && GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR)
    //return m_pVehicle->dwPanelStatus;

    return 0;
}

void CVehicle::SetDoorDamageStatus(uint32_t dwDoorStatus)
{
    /*if(m_pVehicle && GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR)
    {
        m_pVehicle->byteDoorStatus[0] = dwDoorStatus & 7;
        m_pVehicle->byteDoorStatus[1] = (dwDoorStatus >> 8) & 7;
        m_pVehicle->byteDoorStatus[2] = (dwDoorStatus >> 16) & 7;
        m_pVehicle->byteDoorStatus[3] = (dwDoorStatus >> 24) & 7;
    }*/
}

uint32_t CVehicle::GetDoorDamageStatus()
{
    /*if(m_pVehicle && GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR)
    {
        uint32_t dwDoorStatus = (m_pVehicle->byteDoorStatus[0] | (m_pVehicle->byteDoorStatus[1] << 8) | (m_pVehicle->byteDoorStatus[2] << 16) | (m_pVehicle->byteDoorStatus[3] << 24));
        return dwDoorStatus;
    }*/

    return 0;
}

void CVehicle::SetLightDamageStatus(uint8_t byteLightStatus)
{
    //if(m_pVehicle && GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR)
    //m_pVehicle->dwLightStatus = (uint32_t)byteLightStatus;
}

uint8_t CVehicle::GetLightDamageStatus()
{
    //if(m_pVehicle && GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR)
    //return (uint8_t)m_pVehicle->dwLightStatus;

    return 0;
}

void CVehicle::SetWheelPoppedStatus(uint8_t byteWheelStatus)
{
    /*if(m_pVehicle)
    {
        if(GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR)
        {
            m_pVehicle->byteWheelStatus[3] = byteWheelStatus & 1;
            m_pVehicle->byteWheelStatus[2] = (byteWheelStatus >> 1) & 1;
            m_pVehicle->byteWheelStatus[1] = (byteWheelStatus >> 2) & 1;
            m_pVehicle->byteWheelStatus[0] = (byteWheelStatus >> 3) & 1;
        }
        else if(GetVehicleSubtype() == VEHICLE_SUBTYPE_BIKE)
        {
            m_pVehicle->byteBikeWheelStatus[1] = byteWheelStatus & 1;
            m_pVehicle->byteBikeWheelStatus[0] = (byteWheelStatus >> 1) & 1;
        }
    }*/
}

uint8_t CVehicle::GetWheelPoppedStatus()
{
    /*if(m_pVehicle)
    {
        if(GetVehicleSubtype() == VEHICLE_SUBTYPE_CAR)
        {
            uint8_t byteWheelStatus = (m_pVehicle->byteWheelStatus[3] | (m_pVehicle->byteWheelStatus[2] << 1) | (m_pVehicle->byteWheelStatus[1] << 2) | (m_pVehicle->byteWheelStatus[0] << 3));
            return byteWheelStatus;
        }
        else if(GetVehicleSubtype() == VEHICLE_SUBTYPE_BIKE)
        {
            uint8_t byteWheelStatus = (m_pVehicle->byteBikeWheelStatus[1] | (m_pVehicle->byteBikeWheelStatus[0] << 1));
            return byteWheelStatus;
        }
    }*/

    return 0;
}

bool CVehicle::SirenEnabled()
{
    if(!m_pVehicle)
        return m_pVehicle->m_nVehicleFlags.bSirenOrAlarm;

    return false;
}

void CVehicle::EnableSiren(bool bState)
{
    if(!m_pVehicle) return;

    m_pVehicle->m_nVehicleFlags.bSirenOrAlarm = (int)bState;
}

void CVehicle::UpdateColor()
{
    if(m_pVehicle)
    {
        if(m_bHaveColor)
        {
            if(!m_bHasBeenDriven)
            {
                if(m_pVehicle->m_nPrimaryColor != m_byteColor1 || m_pVehicle->m_nSecondaryColor != m_byteColor2)
                {
                    m_pVehicle->m_nPrimaryColor = m_byteColor1;
                    m_pVehicle->m_nSecondaryColor = m_byteColor2;
                }
            }
        }
    }
}

bool CVehicle::UpdateLastDrivenTime()
{
    if(m_pVehicle)
    {
        if(m_pVehicle->pDriver)
        {
            m_bHasBeenDriven = true;
            m_dwTimeSinceLastDriven = GetTickCount();
            return true;
        }
    }

    return false;
}

void CVehicle::SetCustomShadow(uint8_t r, uint8_t g, uint8_t b, float fSizeX, float fSizeY, const char* szTex)
{
    if (fSizeX == 0.0f || fSizeY == 0.0f)
    {
        m_bShadow = false;

        if (m_Shadow.pTexture)
        {
            RwTextureDestroy(m_Shadow.pTexture);
            m_Shadow.pTexture = nullptr;
        }

        return;
    }

    m_bShadow = true;

    m_Shadow.r = r;
    m_Shadow.g = g;
    m_Shadow.b = b;
    m_Shadow.fSizeX = fSizeX;
    m_Shadow.fSizeY = fSizeY;
    m_Shadow.pTexture = CUtil::LoadTextureFromDB(OBFUSCATE("samp"), szTex);
}
static inline void HSVtoRGB(float h, float s, float v, uint8_t& outR, uint8_t& outG, uint8_t& outB)
{
    // h: 0..1, s: 0..1, v: 0..1
    float r = v, g = v, b = v;

    if (s > 0.0f)
    {
        h = fmodf(h, 1.0f);
        if (h < 0.0f) h += 1.0f;

        float hf = h * 6.0f;
        int i = (int)floorf(hf);
        float f = hf - (float)i;

        float p = v * (1.0f - s);
        float q = v * (1.0f - s * f);
        float t = v * (1.0f - s * (1.0f - f));

        switch (i % 6)
        {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            case 5: r = v; g = p; b = q; break;
        }
    }

    auto clamp255 = [](float x) -> uint8_t {
        if (x < 0.0f) x = 0.0f;
        if (x > 1.0f) x = 1.0f;
        return (uint8_t)(x * 255.0f);
    };

    outR = clamp255(r);
    outG = clamp255(g);
    outB = clamp255(b);
}
void CVehicle::SetNeonMode(uint8_t mode, uint8_t color1, uint8_t color2, uint8_t speed)
{
    if (!m_dwGTAId) return;

    NeonState& st = g_neonByGtaId[m_dwGTAId];
    st.mode  = mode;
    st.c1    = color1;
    st.c2    = color2;
    st.speed = speed;
    st.t0    = CTimer::m_snTimeInMillisecondsNonClipped;

    // preset режим: сбрасываем кастомный RGB
    st.custom = 0;
    st.brightness = 100;
    NeonPresetRGB(st.c1, st.r, st.g, st.b);

    Log("Neon Set gtaId=%u mode=%u c1=%u c2=%u speed=%u",
        (unsigned)m_dwGTAId, (unsigned)mode, (unsigned)color1, (unsigned)color2, (unsigned)speed);
}


void CVehicle::SetNeonCustom(uint8_t effect, uint8_t r, uint8_t g, uint8_t b, uint8_t brightness, uint8_t speed)
{
    if (!m_dwGTAId) return;

    NeonState& st = g_neonByGtaId[m_dwGTAId];
    st.mode       = effect;      // один из eNeonMode (STATIC/BREATHE/STROBE/...)
    st.speed      = speed;
    st.custom     = 1;
    st.r          = r;
    st.g          = g;
    st.b          = b;
    st.brightness = brightness;
    st.t0         = CTimer::m_snTimeInMillisecondsNonClipped;

    Log("Neon Custom gtaId=%u effect=%u rgb=%u,%u,%u br=%u speed=%u",
        (unsigned)m_dwGTAId, (unsigned)effect, (unsigned)r, (unsigned)g, (unsigned)b,
        (unsigned)brightness, (unsigned)speed);
}


void CVehicle::NeonRender()
{
    if(!m_bShadow) return;
    if(!m_pVehicle) return;
    if(!CVehicleNeon::pNeonTex) return;
    // ---- режимы неона (управляется RPC) ----
    uint8_t rr = 255, gg = 255, bb = 255;

    NeonState* pst = nullptr;
    auto it = g_neonByGtaId.find(m_dwGTAId);
    if (it != g_neonByGtaId.end())
        pst = &it->second;
    else
        pst = &g_neonByGtaId[m_dwGTAId]; // создадим состояние, если его ещё нет

    NeonState& st = *pst;

    if (st.mode == NEON_OFF)
        return;

    const uint32_t nowMs = CTimer::m_snTimeInMillisecondsNonClipped;

    auto speedFromPreset = [&](uint8_t p, float baseMs) -> float {
        // preset 0..3 -> x1, x0.75, x0.5, x0.33 (быстрее)
        switch (p & 3) {
            case 1: return baseMs * 0.75f;
            case 2: return baseMs * 0.50f;
            case 3: return baseMs * 0.33f;
            default:return baseMs;
        }
    };


    auto getBaseRGB = [&](uint8_t& r, uint8_t& g, uint8_t& b) {
        if (st.custom) { r = st.r; g = st.g; b = st.b; }
        else { NeonPresetRGB(st.c1, r, g, b); }
    };

    auto applyBrightness = [&](uint8_t& r, uint8_t& g, uint8_t& b, float k) {
        float br = st.custom ? ((float)st.brightness / 100.0f) : 1.0f;
        float m = br * k;
        if (m < 0.0f) m = 0.0f;
        if (m > 1.0f) m = 1.0f;
        r = (uint8_t)((float)r * m);
        g = (uint8_t)((float)g * m);
        b = (uint8_t)((float)b * m);
    };

    switch (st.mode)
    {
        default:
        case NEON_STATIC:
        {
            getBaseRGB(rr, gg, bb);
            applyBrightness(rr, gg, bb, 1.0f);
            break;
        }
        case NEON_RAINBOW:
        {
            const float periodMs = speedFromPreset(st.speed, 3000.0f);
            float t = (float)(nowMs % (uint32_t)periodMs) / periodMs;
            HSVtoRGB(t, 1.0f, 1.0f, rr, gg, bb);
            break;
        }
        case NEON_BREATHE:
        {
            getBaseRGB(rr, gg, bb);
            const float periodMs = speedFromPreset(st.speed, 1600.0f);
            float phase = (float)(nowMs % (uint32_t)periodMs) / periodMs; // 0..1
            float k = 0.20f + 0.80f * (0.5f - 0.5f * cosf(phase * 6.2831853f));
            applyBrightness(rr, gg, bb, k);
            break;
        }
        case NEON_PULSE:
        {
            getBaseRGB(rr, gg, bb);
            const float periodMs = speedFromPreset(st.speed, 900.0f);
            float phase = (float)(nowMs % (uint32_t)periodMs) / periodMs;
            float k = 0.35f + 0.65f * (0.5f + 0.5f * sinf(phase * 6.2831853f));
            applyBrightness(rr, gg, bb, k);
            break;
        }
        case NEON_STROBE:
        {
            getBaseRGB(rr, gg, bb);
            const float periodMs = speedFromPreset(st.speed, 220.0f);
            bool on = ((nowMs / (uint32_t)periodMs) % 2) == 0;
            if (!on) { rr = gg = bb = 0; }
            else { applyBrightness(rr, gg, bb, 1.0f); }
            break;
        }
        case NEON_POLICE:
        {
            const float periodMs = speedFromPreset(st.speed, 260.0f);
            bool red = ((nowMs / (uint32_t)periodMs) % 2) == 0;
            if (red) { rr = 255; gg = 30;  bb = 30;  }
            else     { rr = 30;  gg = 120; bb = 255; }
            break;
        }
        case NEON_DUALFADE:
        {
            uint8_t r1,g1,b1, r2,g2,b2;
            NeonPresetRGB(st.c1, r1,g1,b1);
            NeonPresetRGB(st.c2, r2,g2,b2);
            const float periodMs = speedFromPreset(st.speed, 2400.0f);
            float phase = (float)(nowMs % (uint32_t)periodMs) / periodMs; // 0..1
            // плавно туда-обратно
            float k = 0.5f - 0.5f * cosf(phase * 6.2831853f); // 0..1
            rr = (uint8_t)(r1 + (r2 - r1) * k);
            gg = (uint8_t)(g1 + (g2 - g1) * k);
            bb = (uint8_t)(b1 + (b2 - b1) * k);
            break;
        }
        case NEON_RANDOM:
        {
            // раз в N мс выбираем новый hue
            const float stepMs = speedFromPreset(st.speed, 900.0f);
            uint32_t step = (uint32_t)stepMs;
            if (step < 120) step = 120;
            if (((nowMs - st.t0) / step) != ((nowMs - st.t0 - 16) / step))
            {
                st.rndHue = (uint8_t)((st.rndHue + 37) & 0xFF); // детерминированно
            }
            float h = (float)st.rndHue / 255.0f;
            HSVtoRGB(h, 1.0f, 1.0f, rr, gg, bb);
            break;
        }
    }
    // ---------------------------------------

    CVehicleModelInfo* pModelInfoStart =
            static_cast<CVehicleModelInfo*>(CModelInfo::GetModelInfo(m_pVehicle->m_nModelIndex));

    CVector* m_avDummyPos = pModelInfoStart->m_pVehicleStruct->m_avDummyPos;

    float v5 = m_avDummyPos->x;
    float v6 = m_avDummyPos->y;

    CVector a3{}, v17{}, v19{};

    v19 = m_pVehicle->TransformFromObjectSpace(a3);

    v17.y = -0.5f - v6;
    a3 = m_pVehicle->TransformFromObjectSpace(v17);

    float v8  = v19.x;
    float v9  = v19.y;
    float v10 = a3.x;
    float v11 = a3.y;

    v17 = {};
    v17.x = v5 + 0.2f;
    a3 = m_pVehicle->TransformFromObjectSpace(v17);
    a3.z = v19.z;

    float v12 = a3.x - v19.x;
    float v13 = a3.y - v19.y;

    a3.x = v19.x;
    a3.y = v19.y;

    CShadows::StoreShadowToBeRendered(
            2,
            CVehicleNeon::pNeonTex,
            &a3,
            (v10 - v8) * 1.8f,
            (v11 - v9) * 1.8f,
            v12 + v12,
            v13 + v13,
            0xFFFFFF9C,
            rr, gg, bb,          // <-- вместо neonColor.r/g/b
            4.0f,
            0,
            1.0f,
            0,
            1
    );
}

void CVehicle::NeonProcess() {
    if(neonType == eNeonTypes::ON_TYPE_DEF) return;
    if(neonType == eNeonTypes::ON_TYPE_RAINBOW){
        neonColor.RainbowCycle(CTimer::m_snTimeInMillisecondsNonClipped);
    }
}

void CVehicle::SetWheelAlignment(int iWheel, float angle)
{
    if (!m_pVehicle || !m_dwGTAId)
    {
        return;
    }

    if (GetVehicleSubtype() != VEHICLE_SUBTYPE_CAR)
    {
        return;
    }

    if (iWheel == 0)
    {
        m_bWheelAlignmentX = true;
        m_fWheelAlignmentX = (M_PI / 180.0f) * angle;
    }
    else
    {
        m_bWheelAlignmentY = true;
        m_fWheelAlignmentY = (M_PI / 180.0f) * angle;
    }
}

void CVehicle::SetWheelOffset(int iWheel, float offset)
{
    if (GetVehicleSubtype() != VEHICLE_SUBTYPE_CAR)
    {
        return;
    }

    if (iWheel == 0)
    {
        m_bWheelOffsetX = true;
        m_fNewOffsetX = offset;
        m_bWasWheelOffsetProcessedX = false;
    }
    else
    {
        m_bWheelOffsetY = true;
        m_fNewOffsetY = offset;
        m_bWasWheelOffsetProcessedY = false;
    }

    m_uiLastProcessedWheelOffset = GetTickCount();
}

void CVehicle::SetWheelWidth(float fValue)
{
    if (fValue == 20.0f)
    {
        m_bWheelWidth = false;
        return;
    }
    m_bWheelWidth = true;
    m_fWheelWidth = fValue;
}

RwMatrix* RwMatrixMultiplyByVector(CVector* out, RwMatrix* a2, CVector* in);

void CVehicle::ProcessWheelOffset(RwFrame* pFrame, bool bLeft, float fValue, int iID)
{
    CVector vecOffset;
    vecOffset.x = 0.0f - fValue;
    vecOffset.y = 0.0f;
    vecOffset.z = 0.0f;
    if (bLeft)
    {
        vecOffset.x *= -1.0f;
    }

    CVector vecOut;
    RwMatrixMultiplyByVector(&vecOut, &(m_vInitialWheelMatrix[iID]), &vecOffset);

    pFrame->modelling.pos.x = vecOut.x;
    pFrame->modelling.pos.y = vecOut.y;
    pFrame->modelling.pos.z = vecOut.z;
}

void CVehicle::ProcessWheelsOffset()
{
    if (GetTickCount() - m_uiLastProcessedWheelOffset <= 30)
    {
        return;
    }

    if (!m_bWasWheelOffsetProcessedX)
    {
        if (m_bWheelOffsetX)
        {
#if !VER_x32
            RwFrame* pWheelLF = ((RwFrame * (*)(RpClump*, const char*))(g_libGTASA + 0x0045BE78))(m_pVehicle->m_pRwClump, "wheel_lf_dummy"); // GetFrameFromname
            RwFrame* pWheelRF = ((RwFrame * (*)(RpClump*, const char*))(g_libGTASA + 0x0045BE78))(m_pVehicle->m_pRwClump, "wheel_rf_dummy"); // GetFrameFromname
#else
            RwFrame* pWheelLF = ((RwFrame * (*)(RpClump*, const char*))(g_libGTASA + 0x003856F4 + 1))(m_pVehicle->m_pRwClump, "wheel_lf_dummy"); // GetFrameFromname
			RwFrame* pWheelRF = ((RwFrame * (*)(RpClump*, const char*))(g_libGTASA + 0x003856F4 + 1))(m_pVehicle->m_pRwClump, "wheel_rf_dummy"); // GetFrameFromname
#endif
            m_fWheelOffsetX = m_fNewOffsetX;

            ProcessWheelOffset(pWheelLF, true, m_fWheelOffsetX, 0);
            ProcessWheelOffset(pWheelRF, false, m_fWheelOffsetX, 1);

        }
        m_bWasWheelOffsetProcessedX = true;
    }
    if (!m_bWasWheelOffsetProcessedY)
    {
        if (m_bWheelOffsetY)
        {
#if !VER_x32
            RwFrame* pWheelRB = ((RwFrame * (*)(RpClump*, const char*))(g_libGTASA + 0x0045BE78))(m_pVehicle->m_pRwClump, "wheel_rb_dummy"); // GetFrameFromname
            RwFrame* pWheelLB = ((RwFrame * (*)(RpClump*, const char*))(g_libGTASA + 0x0045BE78))(m_pVehicle->m_pRwClump, "wheel_lb_dummy"); // GetFrameFromname
#else
            RwFrame* pWheelRB = ((RwFrame * (*)(RpClump*, const char*))(g_libGTASA + 0x003856F4 + 1))(m_pVehicle->m_pRwClump, "wheel_rb_dummy"); // GetFrameFromname
			RwFrame* pWheelLB = ((RwFrame * (*)(RpClump*, const char*))(g_libGTASA + 0x003856F4 + 1))(m_pVehicle->m_pRwClump, "wheel_lb_dummy"); // GetFrameFromname
#endif

            m_fWheelOffsetY = m_fNewOffsetY;
            ProcessWheelOffset(pWheelRB, false, m_fWheelOffsetY, 2);
            ProcessWheelOffset(pWheelLB, true, m_fWheelOffsetY, 3);
        }
        m_bWasWheelOffsetProcessedY = true;
    }
}

// from CVehicle.cpp
#include "game/renderer/Coronas.h"

void CVehicle::RenderDriverAndPassengers() {
    if(IsRCVehicleModelID())
        return;

    if (pDriver && pDriver->m_nPedState == PEDSTATE_DRIVING) {
        CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x4A6964 + 1 : 0x59D3B8), pDriver);
        // pDriver->Render();
    }

    for (auto& passenger : m_apPassengers) {
        if (passenger && passenger->m_nPedState == PEDSTATE_DRIVING) {
            CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x4A6964 + 1 : 0x59D3B8), passenger);
            // passenger->Render();
        }
    }
}

void CVehicle::SetDriver(CPed* driver) {
    CEntity::ChangeEntityReference(pDriver, driver);

    ApplyTurnForceToOccupantOnEntry(driver);
}

bool CVehicle::AddPassenger(CPed* passenger) {
    ApplyTurnForceToOccupantOnEntry(passenger);

    // Now, find a seat and place them into it
    const auto seats = GetMaxPassengerSeats();

    for(auto & emptySeat : m_apPassengers) {
        emptySeat = passenger;
        CEntity::RegisterReference(emptySeat);
        m_nNumPassengers++;
        return false;
    }

    // No empty seats
    return false;
}

bool CVehicle::AddPassenger(CPed* passenger, uint8 seatIdx) {
    if (m_nVehicleFlags.bIsBus) {
        return AddPassenger(passenger);
    }

    // Check if seat is valid
    if (seatIdx >= m_nMaxPassengers) {
        return false;
    }

    // Check if anyone is already in that seat
    if (m_apPassengers[seatIdx]) {
        return false;
    }

    // Place passenger into seat, and add ref
    m_apPassengers[seatIdx] = passenger;
    CEntity::RegisterReference(m_apPassengers[seatIdx]);
    m_nNumPassengers++;

    return true;
}

void CVehicle::ApplyTurnForceToOccupantOnEntry(CPed* passenger) {
    // Apply some turn force
    switch (m_nVehicleType) {
        case VEHICLE_TYPE_BIKE: {
            ApplyTurnForce(
                    GetUp() * passenger->m_fMass / -50.f,
                    GetForward() / -10.f // Behind the bike
            );
            break;
        }
        default: {
            ApplyTurnForce(
                    CVector{ .0f, .0f, passenger->m_fMass / -5.f },
                    CVector{ CVector2D{passenger->GetPosition() - GetPosition()}, 0.f }
            );
            break;
        }
    }
}

int CVehicle::GetPassengerIndex(const CPed* passenger) {
    for(int i = 0; i <  std::size(m_apPassengers); i++) {
        if(passenger == m_apPassengers[i])
            return i;
    }
    return -1;
}

void CVehicle::AddVehicleUpgrade(int32 modelId) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x0058C66C + 1 : 0x6AFF4C), this, modelId);
}

void CVehicle::RemoveVehicleUpgrade(int32 upgradeModelIndex) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x58CC2C + 1 : 0x6B0718), this, upgradeModelIndex);
}

// 0x6D3000
void CVehicle::SetGettingInFlags(uint8 doorId) {
    m_nGettingInFlags |= doorId;
}

// 0x6D3020
void CVehicle::SetGettingOutFlags(uint8 doorId) {
    m_nGettingOutFlags |= doorId;
}

// 0x6D3040
void CVehicle::ClearGettingInFlags(uint8 doorId) {
    m_nGettingInFlags &= ~doorId;
}

// 0x6D3060
void CVehicle::ClearGettingOutFlags(uint8 doorId) {
    m_nGettingOutFlags &= ~doorId;
}

// ----------------------------------- hooks

void RenderDriverAndPassengers_hook(CVehicle *thiz)
{
    thiz->RenderDriverAndPassengers();
}

void SetDriver_hook(CVehicle *thiz, CPed *pPed)
{
    thiz->SetDriver(pPed);
}

bool CVehicle__GetVehicleLightsStatus_hook(CVehicle *pVehicle)
{
    return pVehicle->GetLightsStatus();
}

static void SanitizePlate(char* out, size_t outSize, const char* in)
{
    if (!out || outSize == 0) return;
    out[0] = '\0';
    if (!in) return;

    // GTA SA обычно адекватно держит 8 символов
    size_t j = 0;
    for (size_t i = 0; in[i] && j + 1 < outSize; ++i)
    {
        unsigned char c = (unsigned char)in[i];

        // только латиница/цифры/пробел/-
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == ' ' || c == '-')
        {
            // upper-case
            if (c >= 'a' && c <= 'z') c = (unsigned char)(c - 32);
            out[j++] = (char)c;
        }

        // жёсткий лимит 8 (если хочешь 7 — поменяй)
        if (j >= 8) break;
    }

    out[j] = '\0';
}

void CVehicle::SetNumberPlate(const char* plate)
{
    if (!m_pVehicle || !m_dwGTAId) return;
    if (!GamePool_Vehicle_GetAt(m_dwGTAId)) return;

    char tmp[16];
    SanitizePlate(tmp, sizeof(tmp), plate);

    if (!tmp[0]) return;

    // сохраняем у себя
    memset(m_szPlateText, 0, sizeof(m_szPlateText));
    strncpy(m_szPlateText, tmp, sizeof(m_szPlateText) - 1);

    // применяем в игре
    ScriptCommand(&set_car_numberplate, m_dwGTAId, m_szPlateText);
}


void (*CVehicle__DoVehicleLights)(CVehicle* thiz, CMatrix *matVehicle, uint32 nLightFlags);
void CVehicle__DoVehicleLights_hook(CVehicle* thiz, CMatrix *matVehicle, uint32 nLightFlags)
{
    uint8_t old = thiz->m_nVehicleFlags.bEngineOn;
    thiz->m_nVehicleFlags.bEngineOn = 1;
    CVehicle__DoVehicleLights(thiz, matVehicle, nLightFlags);
    thiz->m_nVehicleFlags.bEngineOn = old;
}

bool CVehicle__DoTailLightEffect(CVehicle* thisVehicle, int32_t lightId, CMatrix* matVehicle, int isRight, int forcedOff, uint32_t nLightFlags, int lightsOn) {
    //if(!pNetGame){return false;}

    CVehicle* pVehicle = pNetGame->GetVehiclePool()->FindVehicle(thisVehicle);
    if (!pVehicle) {
        Log(OBFUSCATE("No find vehicle id!"));
        return false;
    }

    if (pVehicle)
        pVehicle->ProcessTurnLight(isRight, lightId);

    constexpr int REVERSE_LIGHT_OFFSET = 5;

    auto pModelInfoStart = CModelInfo::GetVehicleModelInfo(thisVehicle->m_nModelIndex);

    CVector* m_avDummyPos = pModelInfoStart->m_pVehicleStruct->m_avDummyPos;

    auto v = CVector(m_avDummyPos[1]);

    if (!isRight)
        v.x = -v.x;

    uint8_t alpha = (thisVehicle->m_fBreakPedal > 0) ? 200 : 96;
    if (thisVehicle->GetLightsStatus() || (thisVehicle->m_fBreakPedal > 0 && thisVehicle->pDriver)) {
        CCoronas::RegisterCorona(
                (uintptr) &thisVehicle->m_placement.m_vPosn.y + 2 * lightId + isRight,
                thisVehicle,
                100, 0, 0, alpha,
                &v,
                1.65f,
                /*TheCamera.LODDistMultiplier*/ 70.f,
                eCoronaType::CORONATYPE_HEADLIGHT,
                eCoronaFlareType::FLARETYPE_NONE,
                true,
                false,
                0,
                0.0f,
                false,
                0,
                0,
                15.0f,
                false,
                false
        );
    }

    if(thisVehicle->m_nCurrentGear == 0 && thisVehicle->m_pHandlingData->m_transmissionData.m_fCurrentSpeed < -0.01) {
        CCoronas::RegisterCorona(
                (uintptr) &thisVehicle->m_placement.m_vPosn.y + 2 * lightId + isRight + REVERSE_LIGHT_OFFSET,
                thisVehicle,
                255, 255, 255, 200,
                &v,
                0.7f,
                /*TheCamera.LODDistMultiplier*/ 70.f,
                eCoronaType::CORONATYPE_HEADLIGHT,
                eCoronaFlareType::FLARETYPE_NONE,
                false,
                false,
                0,
                0.0f,
                false,
                0,
                0,
                15.0f,
                false,
                false
        );
    }
    return true;
}

void CVehicle::InjectHooks() {
    // var
    CHook::Write(g_libGTASA + (VER_x32 ? 0x675F10 : 0x849EA8), &CVehicle::m_aSpecialColModel);

    CHook::Redirect("_ZN8CVehicle25RenderDriverAndPassengersEv", &RenderDriverAndPassengers_hook);
    CHook::Redirect("_ZN8CVehicle9SetDriverEP4CPed", &SetDriver_hook);

    CHook::Redirect("_ZN8CVehicle17DoTailLightEffectEiR7CMatrixhhjh", &CVehicle__DoTailLightEffect);
    CHook::InlineHook("_ZN8CVehicle15DoVehicleLightsER7CMatrixj", CVehicle__DoVehicleLights_hook, &CVehicle__DoVehicleLights);
    CHook::Redirect("_ZN8CVehicle22GetVehicleLightsStatusEv", &CVehicle__GetVehicleLightsStatus_hook);
}

bool CVehicle::IsRCVehicleModelID() {
    switch (m_nModelIndex) {
        case 441:
        case 464:
        case 465:
        case 594:
        case 501:
        case 564:
            return true;

        default:
            break;
    }
    return false;
}

#include "../renderer/Coronas.h"
#include "../../java/CSpeedometr.h"

void CVehicle::ProcessTurnLight(int isRight, int lightId) {
    if (m_iTurnState == eTurnState::TURN_OFF) return;

    auto *pModelInfoStart = static_cast<CVehicleModelInfo *>(CModelInfo::GetModelInfo(
            this->m_pVehicle->m_nModelIndex));
    CVector * m_avDummyPos = pModelInfoStart->m_pVehicleStruct->m_avDummyPos;

    CVector vecFront;
    vecFront.x = m_avDummyPos[0].x;
    vecFront.y = m_avDummyPos[0].y;
    vecFront.z = m_avDummyPos[0].z;

    CVector vecRear;
    vecRear.x = m_avDummyPos[1].x;
    vecRear.y = m_avDummyPos[1].y;
    vecRear.z = m_avDummyPos[1].z;

    if (!isRight) {
        vecFront.x = -vecFront.x;
        vecRear.x = -vecRear.x;
    }

    uint32_t lastTick = GetTickCount();

    if (lastTick - m_iTickTurnLight >= 400) {
        m_iTickTurnLight = lastTick;
        m_bIsTurnLight = !m_bIsTurnLight;

        CPed *pDriver = pNetGame->GetPlayerPool()->GetLocalPlayer()->GetPlayerPed()->m_pPed;

        if (pDriver->bInVehicle) {
            if (pDriver->pVehicle) {
                if (pDriver->pVehicle == this->m_pVehicle) {
                    CSpeedometr::updateTurn(m_bIsTurnLight);
                }
            }
        }
    }

    if (m_bIsTurnLight) {

        if(m_iTurnState == eTurnState::TURN_LEFT && !isRight|| m_iTurnState == eTurnState::TURN_RIGHT && isRight || m_iTurnState == eTurnState::TURN_ALL ) {
            uintptr_t v19 = reinterpret_cast<uintptr_t>(this->m_pVehicle) + 4;
            uintptr_t v20 = *(uintptr_t *)(this->m_pVehicle + 20);
            uintptr_t v21 = reinterpret_cast<uintptr_t>(this->m_pVehicle) + 4;
            if ( v20 )
                v21 = v20 + 48;
            int v22 = 1;
            if ( isRight )
                v22 = 2;
            CCoronas::RegisterCorona(
                    v22 + 2 * lightId + v21 + 4,
                    this->m_pVehicle,
                    255,
                    180,
                    0,
                    170,
                    &vecFront,
                    0.5f,
                    70.0f,
                    eCoronaType::CORONATYPE_HEADLIGHT,
                    eCoronaFlareType::FLARETYPE_NONE,
                    0,
                    0,
                    0,
                    0.0f,
                    0,
                    0.0f,
                    0,
                    80.0f,
                    0,
                    0
            );

            uintptr_t v23 = *(uintptr_t *)(reinterpret_cast<uintptr_t>(this->m_pVehicle) + 0x14);
            uintptr_t v24 = 55;
            if ( v23 )
                v19 = v23 + 48;
            if ( isRight )
                v24 = 56;
            CCoronas::RegisterCorona(
                    v24 + 2 * lightId + v19 + 4,
                    this->m_pVehicle,
                    255,
                    180,
                    0,
                    170,
                    &vecRear,
                    0.5f,
                    70.0f,
                    eCoronaType::CORONATYPE_HEADLIGHT,
                    eCoronaFlareType::FLARETYPE_NONE,
                    0,
                    0,
                    0,
                    0.0f,
                    0,
                    0.0f,
                    0,
                    80.0f,
                    0,
                    0
            );
            CVehicle::ProcessShadowLight(this->m_pVehicle, vecFront, 0, v24);
        }

    }
}

void CVehicle::SetVinyl(uint8_t vinylId)
{
    if (!m_pVehicle) return;

    auto& st = g_vinylByGtaId[m_dwGTAId];
    st.id = vinylId;

    if (vinylId == 0) return;

    // гарантируем загрузку и берём
    RwTexture* tex = CVehicleVinyl::GetOrLoad(vinylId);
    if (!tex)
    {
        Log("SetVinyl id=%d FAILED (texture not loaded)", (int)vinylId);
        return;
    }

    RpClump* clump = m_pVehicle->m_pRwClump;
    if (!clump)
    {
        Log("SetVinyl id=%d FAILED (no clump)", (int)vinylId);
        return;
    }

    ApplyRemapToClump(clump, tex);

    Log("SetVinyl OK gtaId=%u vinyl=%d name=%s",
        (unsigned)m_dwGTAId, (int)vinylId,
        CVehicleVinyl::Name(vinylId) ? CVehicleVinyl::Name(vinylId) : "null");
}



void CVehicle::ProcessShadowLight(CVehicle* vehicle, CVector coords, int isRight, int id) {
    if(!CShadows::gpShadowHeadLightsTex2) return;

    CMatrix* matrix = vehicle->m_matrix;

    CVector worldPos = matrix->m_pos +
                       (matrix->m_forward * (coords.y * 1.3f)) +
                       (matrix->m_right * coords.x) +
                       (matrix->m_up * coords.z);

    if (!isRight) {
        worldPos -= matrix->m_forward * (coords.y * 2.6f);
    }

    float forwardX = matrix->m_forward.x;
    float forwardY = matrix->m_forward.y;

    float length2D = sqrtf(forwardX * forwardX + forwardY * forwardY);
    if (length2D > 0.0f) {
        forwardX /= length2D;
        forwardY /= length2D;
    }

    if (!isRight) {
        forwardX = -forwardX;
        forwardY = -forwardY;
    }

    float size = fabsf(coords.x);
    float shadowSizeX = size * forwardX * 0.7f;
    float shadowSizeY = size * forwardY * 0.7f;

    auto shadowId = static_cast<uint32_t>(id + isRight + 0x18 + (reinterpret_cast<uintptr_t>(vehicle) & 0xFFFF));

    CShadows::StoreStaticShadow(
            (uintptr_t )vehicle->m_pVehicle + id + isRight + 0x18,
            eShadowType::SHADOW_ADDITIVE,
            CShadows::gpShadowHeadLightsTex2,
            &worldPos,
            shadowSizeX,
            shadowSizeY,
            size * forwardY,
            -(size * forwardX),
            50, 127, 90, 0,
            6.0f, 1.0f, 70.0f, 0, 0.05f
    );
}