#include "../main.h"
#include "game/core/game.h"
#include "netgame.h"
#include "../gui/gui.h"
#include "../audiostream.h"
#include "../java/CHud.h"

// voice
#include "../voice/MicroIcon.h"
#include "../voice/SpeakerList.h"
#include "../voice/Network.h"
#include "java/jniutil.h"
#include "../cross_system/CChooseSpawn.h"
#include "../cross_system/CRegistration.h"
#include "java/CSpeedometr.h"
#include "game/custom/VehicleVinyl.h"

//#define AUTH_BS "39FB2DEEDB49ACFB8D4EECE6953D2507988CCCF4410"//main
#define AUTH_BS "E02262CF28BC542486C558D4BE9EFB716592AFAF8B"

extern CGUI *pGUI;
extern CGame* pGame;
extern CJavaWrapper* g_pJavaWrapper;
extern CAudioStream* pAudioStream;
//extern CVoice* pVoice;

int iNetModeNormalOnFootSendRate = 30;
int iNetModeNormalInCarSendRate = 30;
int iNetModeFiringSendRate = 30;
int iNetModeSendMultiplier = 2;

void RegisterRPCs(RakClientInterface *pRakClient);
void UnregisterRPCs(RakClientInterface *pRakClient);
void RegisterScriptRPCs(RakClientInterface *pRakClient);
void UnregisterScriptRPCs(RakClientInterface *pRakClient);

void InstallVehicleEngineLightPatches();

// 0.3.7
unsigned char GetPacketID(Packet *p)
{
    if (p == 0) return 255;

    if ((unsigned char)p->data[0] == ID_TIMESTAMP)
        return (unsigned char)p->data[sizeof(unsigned char) + sizeof(unsigned long)];
    else
        return (unsigned char)p->data[0];
}

extern CJavaWrapper *pJavaWrapper;

CNetGame::CNetGame(const char* szHostOrIp, int iPort, const char *szPlayerName, const char* szPass)
{
    Log("CNetGame initializing..");



    //Log("Voice connect %s:%d", szHostOrIp, iPort);
    //Log("Voice connect %s:%d", szHostOrIp, iPort);
    //Log("Voice connect %s:%d", szHostOrIp, iPort);

    m_pNetSet = new NET_SETTINGS;
    memset(m_szHostName, 0, 256);
    memset(m_szHostOrIp, 0, 256);

    strcpy(m_szHostName, "x64 by Cross");
    strncpy(m_szHostOrIp, szHostOrIp, sizeof(m_szHostOrIp));
    m_iPort = iPort;

    m_pRakClient = RakNetworkFactory::GetRakClientInterface();
    InitializePools();

    GetPlayerPool()->SetLocalPlayerName(szPlayerName);

    RegisterRPCs(m_pRakClient);
    RegisterScriptRPCs(m_pRakClient);
    m_pRakClient->SetPassword(szPass);

    memset(m_dwMapIcon, 0, sizeof(m_dwMapIcon));

    pGame->EnableClock(false);
    pGame->EnableZoneNames(false);

    m_pNetSet->iDeathDropMoney = 0;
    m_pNetSet->iSpawnsAvailable = 0;
    m_pNetSet->bNameTagLOS = 0;
    m_pNetSet->byteHoldTime = true;
    m_pNetSet->bUseCJWalk = 0;
    m_pNetSet->bDisableInteriorEnterExits = 0;
    m_pNetSet->bZoneNames = false;
    m_pNetSet->bInstagib = false;
    m_pNetSet->fNameTagDrawDistance = 70.0f;
    m_pNetSet->bFriendlyFire = true;
    m_pNetSet->byteWorldTime_Hour = 12;
    m_pNetSet->byteWorldTime_Minute = 0;
    m_pNetSet->byteWeather = 1;
    m_pNetSet->fGravity = 0.008f;

    m_bNameTagStatus = true;

    m_dwLastConnectAttempt = GetTickCount();
    SetGameState(GAMESTATE_WAIT_CONNECT);
    m_bLanMode = false;

    pJavaWrapper->HideLoadingScreen();

    const char* sampVer = SAMP_VERSION;
    if(pSettings)
        sampVer = pSettings->Get().szVersion;

    //if (pGUI) pGUI->AddDebugMessage("{FFFFFF}SA-MP {B9C9BF}%s {FFFFFF}Started", sampVer);
}
// 0.3.7
CNetGame::~CNetGame()
{
    // voice
    //Network::OnRaknetDisconnect();

    m_pRakClient->Disconnect(0);
    UnregisterRPCs(m_pRakClient);
    UnregisterScriptRPCs(m_pRakClient);
    RakNetworkFactory::DestroyRakClientInterface(m_pRakClient);

    UninitializePools();

    if (m_pNetSet) {
        delete m_pNetSet;
        m_pNetSet = nullptr;
    }
}

void CNetGame::InitializePools()
{
    m_pPools = new NET_POOLS;
    m_pPools->pPlayerPool = new CPlayerPool();
    m_pPools->pVehiclePool = new CVehiclePool();
    m_pPools->pGangZonePool = new CGangZonePool();
    m_pPools->pPickupPool = new CPickupPool();
    m_pPools->pObjectPool = new CObjectPool();
    m_pPools->pTextLabelPool = new C3DTextLabelPool();
    m_pPools->pTextDrawPool = new CTextDrawPool();
    m_pPools->pActorPool = new CActorPool();
}

void CNetGame::UninitializePools()
{
    if (m_pPools->pPlayerPool) {
        delete m_pPools->pPlayerPool;
        m_pPools->pPlayerPool = nullptr;
    }

    if (m_pPools->pVehiclePool) {
        delete m_pPools->pVehiclePool;
        m_pPools->pVehiclePool = nullptr;
    }

    if (m_pPools->pGangZonePool) {
        delete m_pPools->pGangZonePool;
        m_pPools->pGangZonePool = nullptr;
    }

    if (m_pPools->pPickupPool) {
        delete m_pPools->pPickupPool;
        m_pPools->pPickupPool = nullptr;
    }

    if (m_pPools->pObjectPool) {
        delete m_pPools->pObjectPool;
        m_pPools->pObjectPool = nullptr;
    }

    if (m_pPools->pTextLabelPool) {
        delete m_pPools->pTextLabelPool;
        m_pPools->pTextLabelPool = nullptr;
    }

    if (m_pPools->pTextDrawPool) {
        delete m_pPools->pTextDrawPool;
        m_pPools->pTextDrawPool = nullptr;
    }

    if (m_pPools->pActorPool) {
        delete m_pPools->pActorPool;
        m_pPools->pActorPool = nullptr;
    }

    if (m_pPools) {
        delete m_pPools;
        m_pPools = nullptr;
    }

    if (m_pNetSet) {
        delete m_pNetSet;
        m_pNetSet = nullptr;
    }
}
bool createdCar = false;
static bool g_bGtaDialogHudHidden = false;
int last_process_cnetgame = 0;
void CNetGame::Process()
{
    // 30 fps
    if (GetTickCount() - last_process_cnetgame >= 1000 / 30) {
        last_process_cnetgame = GetTickCount();
    } else {
        return;
    }
    CSpeedometr::update();
    UpdateNetwork();

    // server checkpoints update
    /*if(pGame->m_bCheckpointsEnabled) {
        CPlayerPed *pPlayerDed = GetPlayerPool()->GetLocalPlayer()->GetPlayerPed();
        if (pPlayerDed)
        {
            ScriptCommand(&is_actor_near_point_3d,pPlayerDed->m_dwGTAId,
                          pGame->m_vecCheckpointPos.x,
                          pGame->m_vecCheckpointPos.y,
                          pGame->m_vecCheckpointPos.z,
                          pGame->m_vecCheckpointExtent.x,
                          pGame->m_vecCheckpointExtent.y,
                          pGame->m_vecCheckpointExtent.z,
                          1);
        }
    }*/

    if (m_pNetSet->byteHoldTime) {
        pGame->SetWorldTime(m_pNetSet->byteWorldTime_Hour, m_pNetSet->byteWorldTime_Minute);
    }

    //pGame->PreloadObjectsAnims();

    if (GetGameState() == GAMESTATE_CONNECTED) {
        ProcessPools();
    }
    else {
        CPlayerPed* pPlayerPed = pGame->FindPlayerPed();
        if (pPlayerPed->IsInVehicle()) {
            pPlayerPed->RemoveFromVehicleAndPutAt(314.0f, 160.0f, 39.0f);
        } else {
            pPlayerPed->m_pPed->SetPosn(314.0f, 160.0f, 39.0f);
        }

        CCamera::SetPosition(-314.0f, 160.0f, 39.0f, 0.0f, 0.0f, 0.0f);
        CCamera::LookAtPoint(-310.0f, 157.0f, 39.0f, 2);
        pGame->SetWorldWeather(m_pNetSet->byteWeather);
        pGame->DisplayHUD(false);
        CHud::toggleAll(false, true);
    }
    if (GetGameState() == GAMESTATE_WAIT_CONNECT && (GetTickCount() - m_dwLastConnectAttempt) > 3000) {
        ProcessConnecting();
    }
}

void CNetGame::UpdateNetwork()
{
    bool breakStatus = false;
    Packet *pkt = nullptr;
    unsigned char packetIdentifier;

    while (pkt = m_pRakClient->Receive())
    {
        packetIdentifier = GetPacketID(pkt);

        switch (packetIdentifier) {
            case ID_AUTH_KEY:
                Packet_AuthKey(pkt);
                break;

            case ID_CONNECTION_ATTEMPT_FAILED:
                Packet_ConnectAttemptFailed(pkt);
                break;

            case ID_NO_FREE_INCOMING_CONNECTIONS:
                Packet_NoFreeIncomingConnections(pkt);
                break;

            case ID_DISCONNECTION_NOTIFICATION:
                Packet_DisconnectionNotification(pkt);
                //SetGameState(GAMESTATE_WAIT_CONNECT);
                break;

            case ID_CONNECTION_LOST:
                Packet_ConnectionLost(pkt);
                break;

            case ID_CONNECTION_REQUEST_ACCEPTED:
                Packet_ConnectionSucceeded(pkt);
                break;

            case ID_FAILED_INITIALIZE_ENCRIPTION:
                Packet_FailedInitializeEncription(pkt);
                break;

            case ID_CONNECTION_BANNED:
                Packet_ConnectionBanned(pkt);
                SetGameState(GAMESTATE_WAIT_CONNECT);
                break;

            case ID_INVALID_PASSWORD:
                Packet_InvalidPassword(pkt);
                break;

            case ID_VEHICLE_SYNC:
                Packet_VehicleSync(pkt);
                break;

            case ID_AIM_SYNC:
                Packet_AimSync(pkt);
                break;

            case ID_BULLET_SYNC:
                Packet_BulletSync(pkt);
                break;

            case ID_PLAYER_SYNC:
                Packet_PlayerSync(pkt);
                break;

            case ID_MARKERS_SYNC:
                Packet_MarkerSync(pkt);
                break;

            case ID_UNOCCUPIED_SYNC:
                Packet_UnoccupiedSync(pkt);
                break;

            case ID_TRAILER_SYNC:
                Packet_TrailerSync(pkt);
                break;

            case ID_PASSENGER_SYNC:
                Packet_PassengerSync(pkt);
                break;

            case Network::kRaknetPacketId:
            {
                Network::OnRaknetReceive(pkt);
                break;
            }

            case PACKET_SPECIALCUSTOM:
                Packet_SpecialCustomRPC(pkt);
                break;
            case 251:
                Packet_CustomRPC(pkt);
                break;
        }

        // voice
        /*if (!Network::OnRaknetReceive(*pkt)) {
            return;
        }*/

        m_pRakClient->DeallocatePacket(pkt);
    }
}
#define RPC_TOGGLE_LOGIN 101
#define RPC_CHECK	     0x50


void CNetGame::Packet_CustomRPC(Packet *p) {

    RakNet::BitStream bs((unsigned char *) p->data, p->length, false);
    bs.IgnoreBits(8); // skip packet id

    uint32_t rpcID;
    bs.Read(rpcID);

    switch (rpcID) {
        case RPC_CHECK:
        {
            Log(OBFUSCATE("RPC_CHECK"));
            uint8_t num;
            bs.Read(num);

            if(num != 180) {
                Log(OBFUSCATE(">> use actual client version"));
            }

            uint32_t relnum;
            relnum = (((num * 2) - 17) * 17);

            RakNet::BitStream bs;
            bs.Write((uint8_t)52);
            bs.Write((uint8_t)10);
            bs.Write((uint32_t)relnum);
            GetRakClient()->Send(&bs, SYSTEM_PRIORITY, RELIABLE_SEQUENCED, 0);
            break;
        }
        case RPC_CUSTOM_VISUALS: {
            VEHICLEID vehicleId;
            uint8_t bLightsColor[3];
            int8_t bWheelAlignX;
            int8_t bWheelAlignY;
            int16_t sWheelOffsetXX;
            int16_t sWheelOffsetXY;
            uint16_t bVinyls;
            uint16_t fWheelWidth;

            uint8_t bPlateType, bLen;
            char szText[30];
            char szRegion[10];
            memset(szText, 0, 30);
            memset(szRegion, 0, 10);


            bs.Read(vehicleId);

            auto pVeh = GetVehiclePool()->GetAt(vehicleId);
            if (!pVeh)
                return;

            bs.Read(bLightsColor[0]);
            bs.Read(bLightsColor[1]);
            bs.Read(bLightsColor[2]);
            bs.Read(fWheelWidth);
            bs.Read(bWheelAlignX);
            bs.Read(bWheelAlignY);
            bs.Read(sWheelOffsetXX);
            bs.Read(sWheelOffsetXY);
            bs.Read(bVinyls);
            bs.Read(bPlateType);

            bs.Read(bLen);
            bs.Read(&szText[0], bLen);
            bs.Read(bLen);


            bs.Read(&szRegion[0], bLen);


            bs.Read(pVeh->neonColor.r);
            bs.Read(pVeh->neonColor.g);
            bs.Read(pVeh->neonColor.b);
            uint8_t neontype;
            bs.Read(neontype);
            pVeh->neonType = static_cast<CVehicle::eNeonTypes>(neontype);
            bs.Read(pVeh->m_bShadow);

            Log("VehId: %d Color Light [ %d %d %d ] Wheel size %d Wheel Aling [ %d %d ] Wheel offset [ %d %d] Vinils %d plateType %d plate [ %s %s ] neon [ %d %d %d %d %d]",
                vehicleId, bLightsColor[0], bLightsColor[1], bLightsColor[2], fWheelWidth,
                bWheelAlignX, bWheelAlignY, sWheelOffsetXX, sWheelOffsetXY, bVinyls, bPlateType,
                szText, szRegion, pVeh->neonColor.r , pVeh->neonColor.g, pVeh->neonColor.b ,pVeh->m_bShadow), neontype;

            break;
        }
        case 99: {
            Log("RPC_CHECK_CASH");
            uint8_t bLen, bLen1;
            uint16_t bVersion;
            char szText[30];
            char szText1[30];

            memset(szText, 0, 30);
            memset(szText1, 0, 30);

            bs.Read(bLen);
            if (bLen >= sizeof(szText) - 1)
                return;

            bs.Read(&szText[0], bLen);

            bs.Read(bLen1);
            if (bLen1 >= sizeof(szText1) - 1)
                return;

            bs.Read(&szText1[0], bLen1);

            bs.Read(bVersion);

            RwTexture *pCashTexture = nullptr;
            pCashTexture = (RwTexture *) CUtil::LoadTextureFromDB(szText1, szText);

            int iVersion;
            if (pCashTexture) {
                iVersion = bVersion;
                RwTextureDestroy(pCashTexture);
            } else iVersion = 0;

            RakNet::BitStream bsParams;

            bsParams.Write((uint8_t) 251);
            bsParams.Write((uint32_t) 99);

            bsParams.Write(iVersion);

            m_pRakClient->Send(&bsParams, SYSTEM_PRIORITY, RELIABLE, 0);

//			bsParams.Write(iVersion);
//			m_pRakClient->RPC(&RPC_CustomHash, &bsParams, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
            break;
        }
        case 0x187: {
            uint16_t vehld;
            bs.Read(vehld);

            char str[255];
            uint8_t len;
            bs.Read(len);
            bs.Read(&str[0], len);
            str[len] = 0;

            char str2[255];
            uint8_t len2;
            bs.Read(len2);
            bs.Read(&str2[0], len2);
            str2[len2] = 0;

            CVehicle* pVeh = GetVehiclePool()->GetAt(vehld);
            if (pVeh) pVeh->ApplyTexture(str, str2);

            break;
        }

        case RPC_SET_VINYL:
        {
            VEHICLEID vehId;
            uint16_t vinylId16;

            bs.Read(vehId);
            bs.Read(vinylId16);

            CVehicle* pVeh = GetVehiclePool()->GetAt(vehId);
            if (!pVeh) return;

            uint8_t vinylId = (uint8_t)vinylId16;

            if (vinylId > CVehicleVinyl::MaxId()) {
                Log("RPC_SET_VINYL invalid id=%d max=%d", (int)vinylId, (int)CVehicleVinyl::MaxId());
                break;
            }

            // сразу гарантируем загрузку
            CVehicleVinyl::EnsureLoaded(vinylId);

            pVeh->SetVinyl(vinylId);
            Log("RPC_SET_VINYL veh=%d vinyl=%d (%s)", vehId, vinylId, CVehicleVinyl::Name(vinylId) ? CVehicleVinyl::Name(vinylId) : "null");
            break;
        }

        case RPC_CUSTOM_SET_FUEL:
        {
            float fuel;
            float mileage;
            bs.Read(fuel);
            bs.Read(mileage);

            CSpeedometr::fFuel = fuel;
            CSpeedometr::iMilliage = mileage;
            CSpeedometr::update();
            break;
        }
    }
}

// 0.3.7
void CNetGame::ShutdownForGameModeRestart()
{
    // voice
    SpeakerList::Hide();
    MicroIcon::Hide();
    Network::OnRaknetDisconnect();

    for (PLAYERID playerId = 0; playerId < MAX_PLAYERS; playerId++)
    {
        CRemotePlayer* pRemotePlayer = GetPlayerPool()->GetAt(playerId);
        if (pRemotePlayer) {
            pRemotePlayer->SetTeam(NO_TEAM);
            pRemotePlayer->ResetAllSyncAttributes();
        }
    }

    GetPlayerPool()->GetLocalPlayer()->ResetAllSyncAttributes();
    GetPlayerPool()->GetLocalPlayer()->ToggleSpectating(false);
    GameResetStats();

    if (pAudioStream) { //add new
        pAudioStream->Stop(true);
    }

    SetGameState(GAMESTATE_RESTARTING);
    GetPlayerPool()->DeactivateAll();
    GetPlayerPool()->Process();
    ResetVehiclePool();
    ResetActorPool();
    ResetTextDrawPool();
    ResetGangZonePool();
    Reset3DTextLabelPool();
    ResetPickupPool();
    ResetObjectPool();

    m_pNetSet->bDisableInteriorEnterExits = false;
    m_pNetSet->fNameTagDrawDistance = 70.0f;
    m_pNetSet->byteWorldTime_Hour = 12;
    m_pNetSet->byteWorldTime_Minute = 0;
    m_pNetSet->byteWeather = 1;
    m_pNetSet->byteHoldTime = 1;
    m_pNetSet->bNameTagLOS = true;
    m_pNetSet->bUseCJWalk = false;
    pGame->ToggleCJWalk(false);
    m_pNetSet->fGravity = 0.008f;
    m_pNetSet->iDeathDropMoney = 0;

    pGame->m_bCheckpointsEnabled = false;
    pGame->m_bRaceCheckpointsEnabled = false;

    pGame->FindPlayerPed()->m_pPed->SetInterior(0, true);
    pGame->ResetLocalMoney();
    pGame->FindPlayerPed()->SetDead();
    pGame->FindPlayerPed()->SetArmour(0.0f);
    pGame->EnableZoneNames(false);
    m_pNetSet->bZoneNames = false;
    GameResetRadarColors();
    pGame->SetGravity(m_pNetSet->fGravity);
    pGame->SetWantedLevel(0);
    pGame->EnableClock(false);

    // voice
    //SpeakerList::Hide();
    //MicroIcon::Hide();
    //Network::OnRaknetDisconnect();

    if (pGUI) pGUI->AddDebugMessage("The server is restarting..");
}

int iVehiclePoolProcessFlag = 0;
void CNetGame::ProcessPools()
{
    if (GetPlayerPool())
        GetPlayerPool()->Process();

    if(GetVehiclePool())
        GetVehiclePool()->Process();

    if (GetPickupPool())
        GetPickupPool()->Process();

    if (GetObjectPool())
        GetObjectPool()->Process();
}
// 0.3.7
void CNetGame::ProcessLoadingScreen()
{
    /*CPlayerPed* pPlayerPed = pGame->FindPlayerPed();
    if (pPlayerPed->IsInVehicle()) {
        pPlayerPed->RemoveFromVehicleAndPutAt(314.0f, 160.0f, 39.0f);
    } else {
        pPlayerPed->m_pPed->SetPosn(314.0f, 160.0f, 39.0f);
    }

    CCamera::SetPosition(-314.0f, 160.0f, 39.0f, 0.0f, 0.0f, 0.0f);
    CCamera::LookAtPoint(-310.0f, 157.0f, 39.0f, 2);
    pGame->SetWorldWeather(4);
    pGame->DisplayHUD(false);
    CHud::toggleAll(false, true);*/
    //CCamera *pCamera = pGame->GetCamera();

    //CCamera::Restore();
}
// 0.3.7
void CNetGame::ProcessConnecting()
{
    if (pGUI) pGUI->AddDebugMessage("x64 by Cross Connected the game");

    m_pRakClient->Connect(m_szHostOrIp, m_iPort, 0, 0, 2);
    Network::OnRaknetConnect(m_szHostOrIp, (uint32_t)m_iPort);
    // voice fix voice not connect when restart
    Network::OnRaknetConnect(m_szHostOrIp, m_iPort);

    m_dwLastConnectAttempt = GetTickCount();
    SetGameState(GAMESTATE_CONNECTING);
}
// 0.3.7
void gen_auth_key(char buf[260], char* auth_in);
void CNetGame::Packet_AuthKey(Packet *pkt)
{
    uint8_t byteAuthLen;
    char szAuth[260], szAuthKey[269];

    RakNet::BitStream bsAuth((unsigned char*)pkt->data, pkt->length, false);

    if (GetGameState() < GAMESTATE_WAIT_CONNECT || GetGameState() > GAMESTATE_AWAIT_JOIN) return;

    bsAuth.IgnoreBits(8);
    bsAuth.Read(byteAuthLen);
    bsAuth.Read(szAuth, byteAuthLen);
    szAuth[byteAuthLen] = '\0';

    gen_auth_key(szAuthKey, szAuth);
    uint8_t byteAuthKeyLen = strlen(szAuthKey);

    RakNet::BitStream bsKey;
    bsKey.Write((uint8_t)ID_AUTH_KEY);
    bsKey.Write(byteAuthKeyLen);
    bsKey.Write(szAuthKey, byteAuthKeyLen);
    m_pRakClient->Send(&bsKey, SYSTEM_PRIORITY, RELIABLE, 0);
}
// 0.3.7
void CNetGame::Packet_ConnectAttemptFailed(Packet *pkt)
{
    if (pGUI) pGUI->AddDebugMessage("The server didn't respond. Retrying..");
    if (pAudioStream) { //add new
        pAudioStream->Stop(true);
    }
    SpeakerList::Hide(); //add new
    MicroIcon::Hide();
    SetGameState(GAMESTATE_WAIT_CONNECT);

    //SpeakerList::Hide();
    //MicroIcon::Hide();
}
// 0.3.7
void CNetGame::Packet_NoFreeIncomingConnections(Packet *pkt)
{
    if(pGUI) pGUI->AddDebugMessage("The server is full. Retrying...");
    SpeakerList::Hide(); //addnew
    MicroIcon::Hide();
    SetGameState(GAMESTATE_WAIT_CONNECT);

    //SpeakerList::Hide();
    //MicroIcon::Hide();
}
// 0.3.7
void CNetGame::Packet_DisconnectionNotification(Packet *pkt)
{
    if (pGUI) pGUI->AddDebugMessage("Server closed the connection.");
    if (pAudioStream) {
        pAudioStream->Stop(true);
    }
    m_pRakClient->Disconnect(2000);

    SpeakerList::Hide();
    MicroIcon::Hide();
}
#include "game/custom/SkyBox.h"
#include "CAuthrization.h"
#include "CCustomNotify.h"
#include "CGtaDialog.h"
#include "CMinerIndicator.h"

// 0.3.7
void CNetGame::Packet_ConnectionSucceeded(Packet *pkt)
{
    RakNet::BitStream bsSuccAuth(pkt->data, pkt->length, true);
    PLAYERID MyPlayerID;
    unsigned int uiChallenge;
    int iVersion = 4057;
    uint8_t byteMod = 1;

    bsSuccAuth.IgnoreBits(8);	// packetId
    bsSuccAuth.IgnoreBits(32);	// binaryAddress
    bsSuccAuth.IgnoreBits(16);	// port

    bsSuccAuth.Read(MyPlayerID);
    bsSuccAuth.Read(uiChallenge);
    uiChallenge ^= iVersion;

    if (pGUI) pGUI->AddDebugMessage("Connected. Join the game...");

    SetGameState(GAMESTATE_AWAIT_JOIN);

    uint8_t byteNameLen = strlen(GetPlayerPool()->GetLocalPlayerName());
    uint8_t byteAuthBSLen = strlen(AUTH_BS);
    uint8_t byteClientVerLen = strlen(SAMP_VERSION);
    if(pSettings)
        byteClientVerLen = strlen(pSettings->Get().szVersion);

    RakNet::BitStream bsSend;
    bsSend.Write(iVersion);
    bsSend.Write(byteMod);
    bsSend.Write(byteNameLen);
    bsSend.Write(GetPlayerPool()->GetLocalPlayerName(), byteNameLen);
    bsSend.Write(uiChallenge);
    bsSend.Write(byteAuthBSLen);
    bsSend.Write(AUTH_BS, byteAuthBSLen);
    bsSend.Write(byteClientVerLen);
    //bsSend.Write(SAMP_VERSION, byteClientVerLen);
    if(pSettings)
        bsSend.Write(pSettings->Get().szVersion, byteClientVerLen);
    else
        bsSend.Write(SAMP_VERSION, byteClientVerLen);
    Network::OnRaknetRpc(Network::kRaknetConnectRcpId, bsSend);
    m_pRakClient->RPC(&RPC_ClientJoin, &bsSend, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, nullptr);

    // voice
    SpeakerList::Hide();
    MicroIcon::Hide();
}
// 0.3.7
void CNetGame::Packet_FailedInitializeEncription(Packet *pkt)
{
    if (pGUI) pGUI->AddDebugMessage("Failed to initialize encryption.");
}
// 0.3.7
void CNetGame::Packet_ConnectionBanned(Packet *pkt)
{
    if (pGUI) pGUI->AddDebugMessage("You are banned from this server.");
}
// 0.3.7
void CNetGame::Packet_InvalidPassword(Packet *pkt)
{
    if (pGUI) pGUI->AddDebugMessage("Wrong server password.");
    m_pRakClient->Disconnect(0);
}
// 0.3.7
void CNetGame::Packet_ConnectionLost(Packet *pkt)
{
    if (m_pRakClient) {
        m_pRakClient->Disconnect(0);
    }

    //if (pGUI) pGUI->AddDebugMessage("Lost connection to the server. Reconnecting..");
    ShutdownForGameModeRestart();

    CPlayerPool *pPlayerPool = GetPlayerPool();
    if (pPlayerPool) {
        for (PLAYERID playerId = 0; playerId < MAX_PLAYERS; playerId++) {
            if (pPlayerPool->GetSlotState(playerId) == true) {
                pPlayerPool->Delete(playerId, 0);
            }
        }
    }

    if (pAudioStream) {
        pAudioStream->Stop(true);
    }

    SetGameState(GAMESTATE_WAIT_CONNECT);

    SpeakerList::Hide();
    MicroIcon::Hide();
}
// 0.3.7
void CNetGame::Packet_PlayerSync(Packet *pkt)
{
    RakNet::BitStream bsData(pkt->data, pkt->length, false);
    ONFOOT_SYNC_DATA ofSync;
    uint32_t dwTime = 0;
    uint8_t bytePacketId;
    PLAYERID playerId;

    bool bHasLR, bHasUD;

    if (GetGameState() != GAMESTATE_CONNECTED) return;

    memset(&ofSync, 0, sizeof(ONFOOT_SYNC_DATA));

    bsData.Read(bytePacketId);
    bsData.Read(playerId);

    bsData.Read(bHasLR);
    if (bHasLR) {
        bsData.Read(ofSync.lrAnalog);
    }

    bsData.Read(bHasUD);
    if (bHasUD) {
        bsData.Read(ofSync.udAnalog);
    }

    bsData.Read(ofSync.wKeys);
    bsData.Read((char*)&ofSync.vecPos, sizeof(CVector));
    float w, x, y, z;
    bsData.ReadNormQuat(w, x, y, z);
    ofSync.quat.Set(x, y, z, w);

    uint8_t byteHealthArmour;
    uint8_t byteArmTemp = 0, byteHlTemp = 0;

    bsData.Read(byteHealthArmour);
    byteArmTemp = (byteHealthArmour & 0x0F);
    byteHlTemp = (byteHealthArmour >> 4);

    if (byteArmTemp == 0xF) ofSync.byteArmour = 100;
    else if (byteArmTemp == 0) ofSync.byteArmour = 0;
    else ofSync.byteArmour = byteArmTemp * 7;

    if (byteHlTemp == 0xF) ofSync.byteHealth = 100;
    else if (byteHlTemp == 0) ofSync.byteHealth = 0;
    else ofSync.byteHealth = byteHlTemp * 7;

    uint8_t byteCurrentWeapon = 0;
    bsData.Read(byteCurrentWeapon);
    ofSync.byteCurrentWeapon ^= (byteCurrentWeapon ^ ofSync.byteCurrentWeapon) & 0x3F;

    bsData.Read(ofSync.byteSpecialAction);
    bsData.ReadVector(ofSync.vecMoveSpeed.x, ofSync.vecMoveSpeed.y, ofSync.vecMoveSpeed.z);

    bool bHasVehicleSurfingInfo;
    bsData.Read(bHasVehicleSurfingInfo);
    if (bHasVehicleSurfingInfo)
    {
        bsData.Read(ofSync.wSurfID);
        bsData.Read(ofSync.vecSurfOffsets.x);
        bsData.Read(ofSync.vecSurfOffsets.y);
        bsData.Read(ofSync.vecSurfOffsets.z);
    }
    else
        ofSync.wSurfID = INVALID_VEHICLE_ID;

    bool bHasAnimation;
    bsData.Read(bHasAnimation);
    if (bHasAnimation) {
        bsData.Read(ofSync.dwAnimation);
    }
    else {
        ofSync.dwAnimation = 0x80000000;
    }

    CRemotePlayer *pRemotePlayer = GetPlayerPool()->GetAt(playerId);
    if (pRemotePlayer) {
        pRemotePlayer->StoreOnFootFullSyncData(&ofSync, 0);
    }
}
// 0.3.7
void CNetGame::Packet_VehicleSync(Packet* pkt)
{
    RakNet::BitStream bsData(pkt->data, pkt->length, false);
    uint32_t dwTime = 0;
    uint8_t bytePacketId;
    INCAR_SYNC_DATA icSync;
    PLAYERID PlayerID;

    if (GetGameState() != GAMESTATE_CONNECTED) return;

    memset(&icSync, 0, sizeof(INCAR_SYNC_DATA));

    bsData.Read(bytePacketId);
    bsData.Read(PlayerID);
    bsData.Read(icSync.VehicleID);
    bsData.Read(icSync.lrAnalog);
    bsData.Read(icSync.udAnalog);
    bsData.Read(icSync.wKeys);

    float w, x, y, z;
    bsData.ReadNormQuat(w, x, y, z);
    icSync.quat.Set(x, y, z, w);

    bsData.Read((char*)& icSync.vecPos, sizeof(CVector));
    bsData.ReadVector(icSync.vecMoveSpeed.x, icSync.vecMoveSpeed.y, icSync.vecMoveSpeed.z);

    // car health
    uint16_t wTempVehicleHealth;
    bsData.Read(wTempVehicleHealth);
    icSync.fCarHealth = (float)wTempVehicleHealth;

    // health/armour
    uint8_t byteHealthArmour;
    uint8_t byteArmTemp = 0, byteHlTemp = 0;

    bsData.Read(byteHealthArmour);
    byteArmTemp = (byteHealthArmour & 0x0F);
    byteHlTemp = (byteHealthArmour >> 4);

    if (byteArmTemp == 0xF) icSync.bytePlayerArmour = 100;
    else if (byteArmTemp == 0) icSync.bytePlayerArmour = 0;
    else icSync.bytePlayerArmour = byteArmTemp * 7;

    if (byteHlTemp == 0xF) icSync.bytePlayerHealth = 100;
    else if (byteHlTemp == 0) icSync.bytePlayerHealth = 0;
    else icSync.bytePlayerHealth = byteHlTemp * 7;

    // current weapon
    uint8_t byteTempWeapon;
    bsData.Read(byteTempWeapon);
    icSync.byteCurrentWeapon ^= (byteTempWeapon ^ icSync.byteCurrentWeapon) & 0x3F;

    bool bCheck;

    // siren
    bsData.Read(bCheck);
    if (bCheck) icSync.byteSirenOn = 1;
    // landinggear
    bsData.Read(bCheck);
    if (bCheck) icSync.byteLandingGearState = 1;
    // train speed
    bsData.Read(bCheck);
    if(bCheck) bsData.Read(icSync.HydraThrustAngle);
    // triler id
    bsData.Read(bCheck);
    if (bCheck) bsData.Read(icSync.TrailerID);

    CRemotePlayer* pRemotePlayer = GetPlayerPool()->GetAt(PlayerID);
    if (pRemotePlayer) {
        pRemotePlayer->StoreInCarFullSyncData(&icSync, 0);
    }
}
// 0.3.7
void CNetGame::Packet_AimSync(Packet* pkt)
{
    RakNet::BitStream bsData(pkt->data, pkt->length, false);

    uint8_t bytePacketId;
    PLAYERID PlayerId;
    AIM_SYNC_DATA aimSync;
    bsData.Read(bytePacketId);
    bsData.Read(PlayerId);
    bsData.Read((char*)& aimSync, sizeof(AIM_SYNC_DATA));

    CRemotePlayer* pPlayer = m_pPools->pPlayerPool->GetAt(PlayerId);
    if (pPlayer)
        pPlayer->StoreAimFullSyncData(&aimSync);
}

void CNetGame::SendCustomPacket(uint8_t packet, uint8_t RPC, uint8_t Quantity)
{
    RakNet::BitStream bsSend;
    bsSend.Write(packet);
    bsSend.Write(RPC);
    bsSend.Write(Quantity);
    GetRakClient()->Send(&bsSend, HIGH_PRIORITY, RELIABLE, 0);
}
void CNetGame::SendCustomPacketSpawn(uint8_t packet, uint8_t RPC, uint32_t value)
{
    RakNet::BitStream bsSend;
    bsSend.Write(packet);
    bsSend.Write(RPC);
    bsSend.Write(value); // 4 байта

    GetRakClient()->Send(&bsSend, HIGH_PRIORITY, RELIABLE, 0);
}


void CNetGame::SendLoginPacket(const char password[])
{
    if(!password) return;

    uint8_t packet = PACKET_AUTH;
    uint8_t RPC = 1;

    uint8_t bytePasswordLen = (uint8_t)strlen(password);
    if(bytePasswordLen == 0) return;

    RakNet::BitStream bsSend;
    bsSend.Write(packet);
    bsSend.Write(RPC);
    bsSend.Write(bytePasswordLen);
    bsSend.Write(password, bytePasswordLen);

    GetRakClient()->Send(&bsSend, HIGH_PRIORITY, RELIABLE, 0);

    // сохраним пароль для автологина
    memset(CSettings::m_Settings.player_password, 0, sizeof(CSettings::m_Settings.player_password));
    strncpy(CSettings::m_Settings.player_password, password, sizeof(CSettings::m_Settings.player_password) - 1);

}

// 0.3.7
void CNetGame::Packet_BulletSync(Packet* pkt)
{
    RakNet::BitStream bsData(pkt->data, pkt->length, false);

    if (GetGameState() != GAMESTATE_CONNECTED) return;

    BULLET_SYNC_DATA btSync;
    uint8_t bytePacketId = 0;
    PLAYERID PlayerID = 0;

    bsData.Read(bytePacketId);
    bsData.Read(PlayerID);
    bsData.Read((char*)&btSync, sizeof(BULLET_SYNC_DATA));

    CPlayerPool* pPlayerPool = GetPlayerPool();
    CRemotePlayer* pRemotePlayer = pPlayerPool->GetAt(PlayerID);
    if (pRemotePlayer)
        pRemotePlayer->StoreBulletFullSyncData(&btSync);

}
// 0.3.7
void CNetGame::Packet_PassengerSync(Packet* pkt)
{
    RakNet::BitStream bsData(pkt->data, pkt->length, false);

    if (GetGameState() == GAMESTATE_CONNECTED)
    {
        uint8_t bytePacketId;
        PLAYERID PlayerID;
        PASSENGER_SYNC_DATA psSync;

        bsData.Read(bytePacketId);
        bsData.Read(PlayerID);
        bsData.Read((char*)& psSync, sizeof(PASSENGER_SYNC_DATA));

        CRemotePlayer* pRemotePlayer = GetPlayerPool()->GetAt(PlayerID);
        if (pRemotePlayer) {
            pRemotePlayer->StorePassengerFullSyncData(&psSync);
        }
    }
}

// 0.3.7
void CNetGame::Packet_MarkerSync(Packet *pkt)
{
    if(m_iGameState != GAMESTATE_CONNECTED) return;

    uint8_t bytePacketId;
    int	iNumberOfPlayers;
    PLAYERID playerId;
    bool bIsPlayerActive;
    short sPos[3];

    RakNet::BitStream bsMarkersSync((unsigned char *)pkt->data, pkt->length, false);
    bsMarkersSync.Read(bytePacketId);
    bsMarkersSync.Read(iNumberOfPlayers);
    if(iNumberOfPlayers)
    {
        for(int i = 0; i < iNumberOfPlayers; i++)
        {
            bsMarkersSync.Read(playerId);
            bsMarkersSync.ReadCompressed(bIsPlayerActive);

            if(bIsPlayerActive)
            {
                bsMarkersSync.Read(sPos[0]);
                bsMarkersSync.Read(sPos[1]);
                bsMarkersSync.Read(sPos[2]);
            }

            if(playerId >= 0 && playerId < MAX_PLAYERS && GetPlayerPool()->GetSlotState(playerId))
            {
                CRemotePlayer *pPlayer = GetPlayerPool()->GetAt(playerId);
                if(pPlayer)
                {
                    if(bIsPlayerActive)
                        pPlayer->ShowGlobalMarker(sPos[0], sPos[1], sPos[2]);
                    else
                        pPlayer->HideGlobalMarker();
                }
            }
        }
    }
}

/*
void CNetGame::Packet_VoiceChannelOpenReply(Packet* pkt)
{
	Log("Packet_VoiceChannelOpenReply");

	if (!pVoice || !GetGameState() == GAMESTATE_CONNECTED) return;

	GetPlayerPool()->GetLocalPlayer()->VoiceChannelAccept();
}*/

/*
void CNetGame::Packet_VoiceData(Packet* pkt)
{
	if (!pVoice || !GetGameState() == GAMESTATE_CONNECTED) return;

	PLAYERID playerId;
	int size;
	unsigned char data[MAX_VOICE_PACKET_SIZE];
	RakNet::BitStream bsData(pkt->data, pkt->length, false);
	bsData.IgnoreBits(8);
	bsData.Read(playerId);
	bsData.Read(size);
	bsData.Read((char*)data, size);

	if (GetPlayerPool()->GetSlotState(playerId)) {
		pVoice->Push(playerId, data, size);
	}
}*/

// 0.3.7
void CNetGame::UpdatePlayerScoresAndPings()
{
    static uint32_t dwLastUpdateTick = 0;

    if (GetTickCount() - dwLastUpdateTick > 3000) {
        dwLastUpdateTick = GetTickCount();

        RakNet::BitStream bsSend;
        m_pRakClient->RPC(&RPC_UpdateScoresPingsIPs, &bsSend, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, nullptr);
    }
}

void CNetGame::SendChatMessage(const char* szMsg)
{
    if (GetGameState() != GAMESTATE_CONNECTED) return;

    RakNet::BitStream bsSend;
    uint8_t byteTextLen = strlen(szMsg);

    bsSend.Write(byteTextLen);
    bsSend.Write(szMsg, byteTextLen);

    m_pRakClient->RPC(&RPC_Chat, &bsSend, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
}

void CNetGame::SendChatCommand(const char* szCommand)
{
    if (GetGameState() != GAMESTATE_CONNECTED) return;

    RakNet::BitStream bsParams;
    int iStrlen = strlen(szCommand);

    bsParams.Write(iStrlen);
    bsParams.Write(szCommand, iStrlen);
    m_pRakClient->RPC(&RPC_ServerCommand, &bsParams, HIGH_PRIORITY, RELIABLE, 0, false, UNASSIGNED_NETWORK_ID, NULL);
}
// 0.3.7
void CNetGame::SetMapIcon(uint8_t byteIconID, float fPosX, float fPosY, float fPosZ, uint8_t byteType, uint32_t dwColor, uint8_t byteStyle)
{
    if (m_dwMapIcon[byteIconID] != 0) {
        DisableMapIcon(byteIconID);
    }

    m_dwMapIcon[byteIconID] = pGame->CreateRadarMarkerIcon(byteType, fPosX, fPosY, fPosZ, dwColor, byteStyle);
}
// 0.3.7
void CNetGame::DisableMapIcon(uint8_t byteIconID)
{
    ScriptCommand(&disable_marker, m_dwMapIcon[byteIconID]);
    m_dwMapIcon[byteIconID] = 0;
}
// 0.3.7
void CNetGame::ResetVehiclePool()
{
    if (m_pPools->pVehiclePool) {
        delete m_pPools->pVehiclePool;
    }

    m_pPools->pVehiclePool = new CVehiclePool();
}
// 0.3.7
void CNetGame::ResetActorPool()
{
    if (m_pPools->pActorPool) {
        delete m_pPools->pActorPool;
    }

    m_pPools->pActorPool = new CActorPool();
}
// 0.3.7
void CNetGame::ResetTextDrawPool()
{
    if (m_pPools->pTextDrawPool) {
        delete m_pPools->pTextDrawPool;
    }

    m_pPools->pTextDrawPool = new CTextDrawPool();
}
// 0.3.7
void CNetGame::ResetGangZonePool()
{
    if (m_pPools->pGangZonePool) {
        delete m_pPools->pGangZonePool;
    }

    m_pPools->pGangZonePool = new CGangZonePool();
}
// 0.3.7
void CNetGame::Reset3DTextLabelPool()
{
    if (m_pPools->pTextLabelPool) {
        delete m_pPools->pTextLabelPool;
    }

    m_pPools->pTextLabelPool = new C3DTextLabelPool();
}
// 0.3.7
void CNetGame::ResetMapIcons()
{
    for (int i = 0; i < MAX_MAP_ICONS; i++)
    {
        if (m_dwMapIcon[i]) {
            ScriptCommand(&disable_marker, m_dwMapIcon[i]);
            m_dwMapIcon[i] = 0;
        }
    }
}
// 0.3.7
void CNetGame::ResetPickupPool()
{
    if (m_pPools->pPickupPool) {
        delete m_pPools->pPickupPool;
    }

    m_pPools->pPickupPool = new CPickupPool();
}
// 0.3.7
void CNetGame::ResetObjectPool()
{
    if (m_pPools->pObjectPool) {
        delete m_pPools->pObjectPool;
    }

    m_pPools->pObjectPool = new CObjectPool();
}

// 0.3.7
void CNetGame::InitGameLogic()
{
    if (m_pNetSet->bManualVehicleEngineAndLight) {
        //InstallVehicleEngineLightPatches();
    }

    m_pNetSet->fWorldBounds[0] = 20000.0f;
    m_pNetSet->fWorldBounds[1] = -20000.0f;
    m_pNetSet->fWorldBounds[2] = 20000.0f;
    m_pNetSet->fWorldBounds[3] = -20000.0f;
}

void CNetGame::SendPrevClass()
{
    CPlayerPool* pPlayerPool = GetPlayerPool();
    if (pPlayerPool)
    {
        pPlayerPool->GetLocalPlayer()->SendPrevClass();
    }
}

void CNetGame::SendSpawn()
{
    CPlayerPool* pPlayerPool = GetPlayerPool();
    if (pPlayerPool)
    {
        pPlayerPool->GetLocalPlayer()->SendSpawn();
    }
}

void CNetGame::SendNextClass()
{
    CPlayerPool* pPlayerPool = GetPlayerPool();
    if (pPlayerPool)
    {
        pPlayerPool->GetLocalPlayer()->SendNextClass();
    }
}

void CNetGame::Packet_TrailerSync(Packet *pkt)
{
    if(GetGameState() != GAMESTATE_CONNECTED) return;

    uint8_t bytePacketId;
    PLAYERID playerId;

    TRAILER_SYNC_DATA trSync;
    memset(&trSync, 0, sizeof(TRAILER_SYNC_DATA));

    RakNet::BitStream bsTrailerSync((unsigned char *)pkt->data, pkt->length, false);
    bsTrailerSync.Read(bytePacketId);
    bsTrailerSync.Read(playerId);
    bsTrailerSync.Read((char*)&trSync, sizeof(TRAILER_SYNC_DATA));

    if(GetPlayerPool())
    {
        CRemotePlayer *pPlayer = GetPlayerPool()->GetAt(playerId);
        if(pPlayer)
            pPlayer->StoreTrailerFullSyncData(&trSync);
    }
}

static std::string Cp1251ToUtf8(const std::string& s)
{
    std::string out;
    out.reserve(s.size() * 2);

    for (unsigned char ch : s)
    {
        // ASCII
        if (ch < 0x80)
        {
            out.push_back((char)ch);
            continue;
        }

        // CP1251 Cyrillic
        uint32_t code = 0;

        if (ch == 0xA8) code = 0x0401;          // Ё
        else if (ch == 0xB8) code = 0x0451;     // ё
        else if (ch >= 0xC0 && ch <= 0xFF) code = 0x0410 + (ch - 0xC0); // А..я
        else
        {
            // остальное оставим как '?'
            out.push_back('?');
            continue;
        }

        // UTF-8 encode (U+0400..U+04FF => 2 bytes)
        out.push_back((char)(0xC0 | (code >> 6)));
        out.push_back((char)(0x80 | (code & 0x3F)));
    }

    return out;
}
static std::string ReadAnyString(RakNet::BitStream& bs)
{
    const int saved = bs.GetReadOffset();

    auto read_len_bytes = [&](uint32_t len) -> std::string {
        std::string s;
        if (len == 0) return s;
        if (len > 4096) return s;
        if (bs.GetNumberOfUnreadBits() < (int)(len * 8)) return s;
        s.resize(len);
        bs.Read((char*)s.data(), len);
        while (!s.empty() && (s.back() == '\0' || s.back() == '\n' || s.back() == '\r'))
            s.pop_back();
        return s;
    };

    // 1) uint16 len
    bs.SetReadOffset(saved);
    uint16_t len16 = 0;
    if (bs.GetNumberOfUnreadBits() >= 16 && bs.Read(len16))
    {
        std::string s = read_len_bytes((uint32_t)len16);
        if (!s.empty() || len16 == 0) return s;
    }

    // 2) uint8 len
    bs.SetReadOffset(saved);
    uint8_t len8 = 0;
    if (bs.GetNumberOfUnreadBits() >= 8 && bs.Read(len8))
    {
        std::string s = read_len_bytes((uint32_t)len8);
        if (!s.empty() || len8 == 0) return s;
    }

    // 3) uint32 len
    bs.SetReadOffset(saved);
    uint32_t len32 = 0;
    if (bs.GetNumberOfUnreadBits() >= 32 && bs.Read(len32))
    {
        std::string s = read_len_bytes(len32);
        if (!s.empty() || len32 == 0) return s;
    }

    // 4) fallback: read until '\0' or end
    bs.SetReadOffset(saved);
    std::string out;
    while (bs.GetNumberOfUnreadBits() >= 8)
    {
        char c = 0;
        bs.Read(c);
        if (c == '\0') break;
        out.push_back(c);
        if (out.size() >= 4096) break;
    }
    return out;
}
static void SkipAnyString(RakNet::BitStream& bs)
{
    const int saved = bs.GetReadOffset();

    auto skip_len_bytes = [&](uint32_t len) -> bool {
        if (len == 0) return true;
        if (len > 4096) return false;
        if (bs.GetNumberOfUnreadBits() < (int)(len * 8)) return false;
        bs.IgnoreBits((int)(len * 8)); // <-- FIX: вместо IgnoreBytes
        return true;
    };

    // 1) uint16 len
    bs.SetReadOffset(saved);
    uint16_t len16 = 0;
    if (bs.GetNumberOfUnreadBits() >= 16 && bs.Read(len16))
    {
        if (skip_len_bytes((uint32_t)len16)) return;
    }

    // 2) uint8 len
    bs.SetReadOffset(saved);
    uint8_t len8 = 0;
    if (bs.GetNumberOfUnreadBits() >= 8 && bs.Read(len8))
    {
        if (skip_len_bytes((uint32_t)len8)) return;
    }

    // 3) uint32 len
    bs.SetReadOffset(saved);
    uint32_t len32 = 0;
    if (bs.GetNumberOfUnreadBits() >= 32 && bs.Read(len32))
    {
        if (skip_len_bytes(len32)) return;
    }

    // 4) fallback: read until '\0' or end
    bs.SetReadOffset(saved);
    while (bs.GetNumberOfUnreadBits() >= 8)
    {
        char c = 0;
        bs.Read(c);
        if (c == '\0') break;
    }
}

void CNetGame::Packet_SpecialCustomRPC(Packet *p)
{
    RakNet::BitStream bs((unsigned char *)p->data, p->length, false);

    uint8_t  packetID = 0;
    uint32_t rpcID = 0;

    bs.Read(packetID);
    bs.Read(rpcID);

    Log("Packet_SpecialCustomRPC rpc=%u", (unsigned)rpcID);

    switch (rpcID)
    {
        /* ---------- Авторизация ---------- */
        case 1:
        {
            bool toggle = false, isEmail = false, isAutoAuth = false;
            bs.Read(toggle);
            bs.Read(isEmail);
            bs.Read(isAutoAuth);

            uint8_t skinId = 0;
            if (bs.GetNumberOfUnreadBits() >= 8)
                bs.Read(skinId);

            Log("AuthGUI toggle=%d email=%d auto=%d skin=%d",
                toggle ? 1 : 0, isEmail ? 1 : 0, isAutoAuth ? 1 : 0, (int)skinId);

            if (CSettings::m_Settings.szAutoLogin && isAutoAuth)
                SendLoginPacket(CSettings::m_Settings.player_password);

            if (toggle)
                CAuthrization::show(isEmail, isAutoAuth, (int)skinId);
            else
                CAuthrization::hide();

            break;
        }

            /* ---------- Регистрация (Pawn: rpcID=3, PR_BOOL toggle) ---------- */
        case 3:
        {
            // PR_BOOL на сервере почти всегда 1 байт -> читаем uint8
            uint8_t toggle = 0;
            bs.Read(toggle);

            Log("Regist toggle=%d", (int)toggle);

            if (toggle)
                CRegistration::show();
            else
                CRegistration::hide();

            break;
        }
        case 20: // RPC_UVED_NOTIFY_SHOW
        {
            uint8_t toggle = 0;
            uint32_t npcId = 0;
            uint32_t actionId = 0;

            bs.Read(toggle);
            bs.Read(npcId);
            bs.Read(actionId);
            std::string text = ReadAnyString(bs);

// если сервер шлёт CP1251 — конвертим в UTF-8
            std::string textUtf8 = Cp1251ToUtf8(text);

            Log("UvedNotify text raw='%s' utf8='%s'", text.c_str(), textUtf8.c_str());

            if (toggle)
                CCustomNotify::show(textUtf8.c_str(), (int)npcId, (int)actionId);
            else
                CCustomNotify::hide();

            break;
        }
        case RPC_SET_PLATE:
        {
            VEHICLEID vid = 0;
            uint8_t len = 0;
            char plate[33];

            bs.Read(vid);
            bs.Read(len);
            if (len > 32) break;

            bs.Read(plate, len);
            plate[len] = '\0';

            CVehiclePool* pool = GetVehiclePool();
            if (!pool) break;

            CVehicle* v = pool->GetAt(vid);
            if (!v) break;

            v->SetPlateText(plate);
            Log("RPC_SET_PLATE vid=%d plate='%s'", (int)vid, plate);
            break;
        }


        case RPC_GTA_DIALOG_SHOW: // 30
        {
            uint8_t toggle = 0;
            uint32_t dialogId = 0;
            uint8_t style = 0;

            bs.Read(toggle);
            bs.Read(dialogId);
            bs.Read(style);

            auto ReadStr8 = [](RakNet::BitStream& b) -> std::string
            {
                uint8_t len = 0;
                if (b.GetNumberOfUnreadBits() < 8) return {};
                b.Read(len);
                if (len == 0) return {};
                if (b.GetNumberOfUnreadBits() < (int)len * 8) return {};
                std::string s;
                s.resize(len);
                b.Read((char*)s.data(), len);
                while (!s.empty() && (s.back() == '\0' || s.back() == '\n' || s.back() == '\r'))
                    s.pop_back();
                return s;
            };

            std::string title = Cp1251ToUtf8(ReadStr8(bs));
            std::string text  = Cp1251ToUtf8(ReadStr8(bs));
            std::string left  = Cp1251ToUtf8(ReadStr8(bs));
            std::string right = Cp1251ToUtf8(ReadStr8(bs));

            Log("GtaDialog toggle=%d id=%u style=%d title='%s' text='%s' left='%s' right='%s'",
                (int)toggle, (unsigned)dialogId, (int)style,
                title.c_str(), text.c_str(), left.c_str(), right.c_str());

            if (toggle)
            {
                if (!g_bGtaDialogHudHidden)
                {
                    if (pGame) pGame->DisplayHUD(false);
                    CHud::toggleAll(false, true);
                    g_bGtaDialogHudHidden = true;
                }

                CGtaDialog::show((int)dialogId, title.c_str(), text.c_str(), left.c_str(), right.c_str());
            }
            else
            {
                CGtaDialog::hide();

                if (g_bGtaDialogHudHidden)
                {
                    if (pGame) pGame->DisplayHUD(true);
                    CHud::toggleAll(true, true);
                    g_bGtaDialogHudHidden = false;
                }
            }

            break;
        }
        case RPC_MINER_INDICATOR:
        {
            uint8_t show = 0, inWork = 0;
            int ore = 0, ingot = 0;

            bs.Read(show);
            bs.Read(inWork);
            bs.Read(ore);
            bs.Read(ingot);

            CMinerIndicator::Update(show != 0, inWork != 0, ore, ingot);
            break;
        }

        case RPC_HUD_MONEY:
        {
            uint32_t money = 0;
            bs.Read(money);

            CHud::iLocalMoney = (int)money;
            CHud::UpdateMoney();
            return;
        }

        case RPC_NPC_CREATE:
        {
            uint16_t npcId = 0;
            uint16_t skin  = 0;
            float x = 0.0f, y = 0.0f, z = 0.0f;
            float angle = 0.0f;
            float health = 100.0f;
            uint8_t invul = 0;

            bs.Read(npcId);
            bs.Read(skin);
            bs.Read(x);
            bs.Read(y);
            bs.Read(z);
            bs.Read(angle);
            bs.Read(health);
            bs.Read(invul);

            Log("RPC_NPC_CREATE id=%u skin=%u pos=%.3f %.3f %.3f ang=%.2f hp=%.1f inv=%u",
                (unsigned)npcId, (unsigned)skin, x, y, z, angle, health, (unsigned)invul);

            if (auto* pool = GetActorPool())
            {
                pool->Delete((uint16_t)npcId);
                pool->NewCustom((uint16_t)npcId, (int)skin, x, y, z, angle, health, invul != 0);
            }
            break;
        }

        case RPC_NPC_DELETE:
        {
            uint16_t npcId = 0;
            bs.Read(npcId);

            Log("RPC_NPC_DELETE id=%u", (unsigned)npcId);

            if (auto* pool = GetActorPool())
                pool->Delete((uint16_t)npcId);
            break;
        }

            /* ---------- Выбор спавна (твой RPC_TOGGLE_CHOOSE_SPAWN) ---------- */
        case RPC_TOGGLE_CHOOSE_SPAWN:
        {
            uint8_t toggle = 0;
            uint8_t org = 0, home = 0, family = 0, pos = 0;

            bs.Read(toggle);
            bs.Read(org);
            bs.Read(home);
            bs.Read(family);
            bs.Read(pos);

            Log("SpawnSelect toggle=%d org=%d home=%d family=%d pos=%d",
                (int)toggle, (int)org, (int)home, (int)family, (int)pos);

            if (toggle)
                CChooseSpawn::show((int)org, (int)home, (int)family, (int)pos);
            else
                CChooseSpawn::hide();

            break;
        }
            /* ---------- Неон на авто ---------- */
        case RPC_NEON_SET:
        {
            VEHICLEID vid = 0;
            uint8_t mode = 0;
            uint8_t c1 = 0;
            uint8_t c2 = 0;
            uint8_t speed = 1;

            bs.Read(vid);
            bs.Read(mode);
            bs.Read(c1);

            // опционально: второй цвет и скорость
            if (bs.GetNumberOfUnreadBits() >= 8) bs.Read(c2);
            if (bs.GetNumberOfUnreadBits() >= 8) bs.Read(speed);

            Log("RPC_NEON_SET vid=%u mode=%u c1=%u c2=%u speed=%u",
                (unsigned)vid, (unsigned)mode, (unsigned)c1, (unsigned)c2, (unsigned)speed);

            // Мы уже внутри CNetGame, поэтому pNetGame не нужен
            auto* pPool = GetVehiclePool();          // или this->GetVehiclePool()
            if (!pPool) break;

            CVehicle* pVeh = pPool->GetAt(vid);      // если у тебя другой метод — подставь (Find/Get/etc)
            if (!pVeh) break;

            pVeh->SetNeonMode(mode, c1, c2, speed);
            break;
        }

    }
}

void CNetGame::Packet_UnoccupiedSync(Packet *pkt)
{
    if(m_iGameState != GAMESTATE_CONNECTED) return;

    uint8_t bytePacketId;
    PLAYERID playerId;

    UNOCCUPIED_SYNC_DATA unocSync;
    memset(&unocSync, 0, sizeof(UNOCCUPIED_SYNC_DATA));

    RakNet::BitStream bsUnocSync((unsigned char *)pkt->data, pkt->length, false);
    bsUnocSync.Read(bytePacketId);
    bsUnocSync.Read(playerId);
    bsUnocSync.Read((char*)&unocSync,sizeof(UNOCCUPIED_SYNC_DATA));

    if(GetPlayerPool())
    {
        CRemotePlayer *pPlayer = GetPlayerPool()->GetAt(playerId);
        if(pPlayer)
            pPlayer->StoreUnoccupiedSyncData(&unocSync);
    }
}
void CActorPool::NewCustom(uint16_t actorId, int skin, float x, float y, float z,
                           float angle, float health, bool invul)
{
    if (actorId >= MAX_ACTORS)
        return;

    if (m_pActors[actorId])
        Delete((PLAYERID)actorId);

    CActor* pActor = new CActor(skin, x, y, z, angle);
    if (!pActor || !pActor->m_pPed)
    {
        if (pActor) delete pActor;
        m_pActors[actorId] = nullptr;
        m_pGtaPed[actorId] = nullptr;
        m_bActorSlotState[actorId] = false;
        return;
    }

    m_pActors[actorId] = pActor;
    m_pGtaPed[actorId] = pActor->m_pPed;
    m_bActorSlotState[actorId] = true;

    pActor->SetHealth(health);
    pActor->SetInvulnerable(invul);
}

void CNetGame::SendRegisterPacket(char *password, char *mail, uint8_t sex, uint8_t age, uint32_t skinid) {
    uint8_t packet = PACKET_AUTH;
    uint8_t RPC = 4;
    uint8_t bytePasswordLen = strlen(password);
    uint8_t byteMailLen = strlen(mail);
    RakNet::BitStream bsSend;
    bsSend.Write(packet);
    bsSend.Write(RPC);
    bsSend.Write(bytePasswordLen);
    bsSend.Write(byteMailLen);
    bsSend.Write(password, bytePasswordLen);
    bsSend.Write(mail, byteMailLen);
    bsSend.Write(sex);
    bsSend.Write(age);
    bsSend.Write(skinid);
    GetRakClient()->Send(&bsSend, HIGH_PRIORITY, RELIABLE, 0);
}

void CNetGame::SendRegisterSkinPacket(uint32_t skinId) {
    uint8_t packet = PACKET_AUTH;
    uint8_t RPC = 3;
    RakNet::BitStream bsSend;
    bsSend.Write(packet);
    bsSend.Write(RPC);
    bsSend.Write(skinId);
    GetRakClient()->Send(&bsSend, SYSTEM_PRIORITY, RELIABLE, 0);
}
void CNetGame::SendNotifyClick(uint32_t npcId, uint32_t actionId)
{
    RakNet::BitStream bs;

    bs.Write((uint8_t)253);     // Packet_SpecialCustomRPC (id пакета)
    bs.Write((uint32_t)21);     // RPC_UVED_NOTIFY_CLICK

    bs.Write((uint32_t)npcId);
    bs.Write((uint32_t)actionId);

    m_pRakClient->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0);
}

void CNetGame::SendGtaDialogClick(uint32_t dialogId, uint8_t button)
{
    RakNet::BitStream bs;

    bs.Write((uint8_t)253);              // Packet_SpecialCustomRPC
    bs.Write((uint32_t)RPC_GTA_DIALOG_CLICK);

    bs.Write((uint32_t)dialogId);
    bs.Write((uint8_t)button);

    m_pRakClient->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0);
}

