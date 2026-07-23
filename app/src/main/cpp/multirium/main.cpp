#include <jni.h>
#include <pthread.h>
#include <syscall.h>
#include <cstring>
#include <type_traits>
#include <utility>
#include <android/log.h>
#include <atomic>
#include <cmath>

#include "main.h"
#include "game/core/game.h"
#include "net/netgame.h"
#include "gui/gui.h"
#include "playertags.h"
#include "audiostream.h"
#include "java/jniutil.h"
#include <dlfcn.h>
#include "StackTrace.h"

#include "game/control/multitouch.h"
#include "armhook/patch.h"
#include "util/CUtil.h"
#include "java/Loader.h"
#include "voice/Plugin.h"
#include "vendor/armhook/patch.h"
#include "vendor/str_obfuscator/str_obfuscator.hpp"
#include "settings.h"
#include "crashlytics.h"
#include "java/CHud.h"
#include "java/CChat.h"
#include "gui/CFontRenderer.h"
#include "../java/CPauseMenu.h"
#include "../game/mobilemenu/MobileMenu.h"

JavaVM* javaVM;

char* g_pszStorage = nullptr;

CGUI* pGUI = nullptr;
CGame* pGame = nullptr;

CNetGame* pNetGame = nullptr;
CPlayerTags* pPlayerTags = nullptr;
CSnapShotHelper* pSnapShotHelper = nullptr;
CAudioStream* pAudioStream = nullptr;
CJavaWrapper* pJavaWrapper = nullptr;
CSettings* pSettings = nullptr;

bool bDebug = false;
bool bGameInited = false;
bool bNetworkInited = false;

static char g_pendingNick[64] = {0};

static const float kLauncherX   = -290.6006f;
static const float kLauncherY   =  98.1770f;
static const float kLauncherZ   =  13.0531f;
static const float kLauncherRot =  80.0f;

static std::atomic<int>      g_launcherSkinId{34};
static std::atomic<bool>     g_launcherNeedApplySkin{false};
static std::atomic<int>      g_launcherApplyFrameCounter{0};

static std::atomic<bool>     g_launcherNeedSetupScene{true};
static std::atomic<int>      g_launcherSetupFrameCounter{0};

static std::atomic<uint32_t> g_launcherHoldUntilTick{0};
static std::atomic<bool>     g_launcherForceHold{true};

static std::atomic<bool>     g_inLauncherScene{false};
static std::atomic<bool>     g_launcherDidInitialTeleport{false};

static std::atomic<bool>     g_launcherRebuildDone{false};
static std::atomic<bool>     g_launcherControlsLocked{false};

static std::atomic<uint32_t> g_nextReassertControlTick{0};
static std::atomic<uint32_t> g_lastTouchResetTick{0};

static std::atomic<bool> g_launcherDidRespawn{false};
static std::atomic<int>  g_launcherRespawnFrames{0};


void Log(const char* fmt, ...);
void LogVoice(const char* fmt, ...);
uint32_t GetTickCount();





static inline CVector Vec3(float x, float y, float z) { CVector v; v.x = x; v.y = y; v.z = z; return v; }

static inline float Dist3(const CVector& a, const CVector& b)
{
    const float dx = a.x - b.x;
    const float dy = a.y - b.y;
    const float dz = a.z - b.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

static inline int ClampSkin(int s) { if (s < 0) return 0; if (s > 311) return 311; return s; }

template <class T>
static inline void SetPosnCompat(T* ent, const CVector& v)
{
    if (!ent) return;

    if constexpr (requires { ent->SetPosn(v.x, v.y, v.z); }) { ent->SetPosn(v.x, v.y, v.z); return; }
    if constexpr (requires { ent->SetPosn(v); })            { ent->SetPosn(v); return; }

    if constexpr (requires { ent->GetModellingMatrix(); }) {
        if (RwMatrix* mm = ent->GetModellingMatrix()) {
            RwV3d* pos = RwMatrixGetPos(mm);
            pos->x = v.x; pos->y = v.y; pos->z = v.z;
            RwMatrixUpdate(mm);
        }
    }
}

static inline void ZeroPedSpeed(CPlayerPed* p)
{
    if (!p || !p->m_pPed) return;
    p->m_pPed->m_vecMoveSpeed.x = 0.0f;
    p->m_pPed->m_vecMoveSpeed.y = 0.0f;
    p->m_pPed->m_vecMoveSpeed.z = 0.0f;

    p->m_pPed->m_vecTurnSpeed.x = 0.0f;
    p->m_pPed->m_vecTurnSpeed.y = 0.0f;
    p->m_pPed->m_vecTurnSpeed.z = 0.0f;
}



static inline void RespawnLauncherPedOnce()
{
    if (!pGame) return;
    if (g_launcherDidRespawn.load(std::memory_order_acquire)) return;

    int f = g_launcherRespawnFrames.fetch_add(1, std::memory_order_relaxed);
    if (f < 3) return;

    const int skin = ClampSkin(g_launcherSkinId.load(std::memory_order_relaxed));

    pGame->NewPlayer((uint)skin, kLauncherX, kLauncherY, kLauncherZ, kLauncherRot, true, true);

    CPlayerPed* p = pGame->FindPlayerPed();
    if (!p || !p->m_pPed) return;

    p->SetInterior(0, true);
    ZeroPedSpeed(p);
    p->SetRotation(kLauncherRot);
    p->SetTargetRotation(kLauncherRot);
    p->TogglePlayerControllable(false);

    g_launcherDidRespawn.store(true, std::memory_order_release);
}

static inline bool GetPedHeadPos(CPlayerPed* p, CVector& out)
{
    if (!p || !p->m_pPed) return false;

    CVector head{};
    p->GetBonePosition(8, &head);

    if (std::isnan(head.x) || std::isnan(head.y) || std::isnan(head.z) ||
        (std::fabs(head.x) < 0.001f && std::fabs(head.y) < 0.001f && std::fabs(head.z) < 0.001f))
    {
        RwMatrix mat = p->m_pPed->GetMatrix().ToRwMatrix();
        out = Vec3(mat.pos.x, mat.pos.y, mat.pos.z + 0.75f);
        return true;
    }

    out = head;
    return true;
}

static inline void LauncherSetupCameraToPed(CPlayerPed* pPed)
{
    if (!pPed) return;

    CVector headPos{};
    if (!GetPedHeadPos(pPed, headPos)) return;

    const float rotRad = DegToRad(kLauncherRot);
    const float fx = std::cos(rotRad);
    const float fy = std::sin(rotRad);

    const float forward = 0.40f;
    const float side    = 4.45f;
    const float up      = 0.05f;

    const float rx = -fy;
    const float ry =  fx;

    const float camX = headPos.x + fx * forward + rx * side;
    const float camY = headPos.y + fy * forward + ry * side;
    const float camZ = headPos.z + up;

    CCamera::SetPosition(-294.5006f, 98.1770f, 14.0531f, 0.0f, 0.0f, 0.0f);
    CCamera::LookAtPoint(-291.6006f, 98.1770f, 14.0531f, 2);
}

static inline bool ForceSetEntityPosByModellingMatrix(CPlayerPed* p, const CVector& target)
{
    if (!p || !p->m_pPed) return false;

    SetPosnCompat(p->m_pPed, target);

    if (RwMatrix* mm = p->m_pPed->GetModellingMatrix())
    {
        RwV3d* pos = RwMatrixGetPos(mm);
        pos->x = target.x; pos->y = target.y; pos->z = target.z;
        RwMatrixUpdate(mm);
    }

    p->m_pPed->UpdateRW();
    p->m_pPed->UpdateRwFrame();
    return true;
}

static inline void TeleportToLauncherNoFall(CPlayerPed* p, float x, float y, float z, float rotDeg)
{
    if (!p || !p->m_pPed) return;
    if (!GamePool_Ped_GetAt(p->m_dwGTAId)) return;

    const CVector target{x,y,z};

    p->SetInterior(0, true);
    ForceSetEntityPosByModellingMatrix(p, target);
    ZeroPedSpeed(p);

    if (!std::isnan(rotDeg)) {
        p->SetRotation(rotDeg);
        p->SetTargetRotation(rotDeg);
    }
}

static inline void ForcePedRebuildBySameSkin(CPlayerPed* p)
{
    if (!p || !p->m_pPed) return;
    if (!GamePool_Ped_GetAt(p->m_dwGTAId)) return;

    const int cur = (int)p->m_pPed->m_nModelIndex;

    p->SetModelIndex((uint)cur);
    p->SetModelIndex((uint)cur);

    ZeroPedSpeed(p);
    p->SetRotation(kLauncherRot);
    p->SetTargetRotation(kLauncherRot);
}

static inline void LockControlsInLauncher(CPlayerPed* p)
{
    if (!p || !p->m_pPed) return;

    const uint32_t now = GetTickCount();

    if (!g_launcherControlsLocked.load(std::memory_order_acquire))
    {
        p->TogglePlayerControllable(false);
        g_launcherControlsLocked.store(true, std::memory_order_release);
        g_nextReassertControlTick.store(now + 500, std::memory_order_release);
        g_lastTouchResetTick.store(now, std::memory_order_release);
        return;
    }

    const uint32_t nextRe = g_nextReassertControlTick.load(std::memory_order_acquire);
    if (now >= nextRe)
    {
        // p->TogglePlayerControllable(false);
        g_nextReassertControlTick.store(now + 500, std::memory_order_release);
    }
}

static inline void ResetTouchOnceAfterSetup(CPlayerPed* p)
{
    if (!p || !p->m_pPed) return;

    const uint32_t now = GetTickCount();
    const uint32_t last = g_lastTouchResetTick.load(std::memory_order_acquire);

    if (last == 0 || (now - last) > 2000)
    {
        //p->TogglePlayerControllable(false);
        g_lastTouchResetTick.store(now, std::memory_order_release);
    }
}
static void TrySetupLauncherSceneDeferred()
{
    if (bDebug) return;
    if (!g_launcherNeedSetupScene.load(std::memory_order_acquire)) return;
    if (!pGame) return;

    const int f = g_launcherSetupFrameCounter.fetch_add(1, std::memory_order_relaxed);
    if ((f % 2) != 0) return;

    CPlayerPed* pPlayer = pGame->FindPlayerPed();
    if (!pPlayer || !pPlayer->m_pPed) return;
    if (!GamePool_Ped_GetAt(pPlayer->m_dwGTAId)) return;

    if (!g_launcherDidInitialTeleport.exchange(true, std::memory_order_acq_rel)) {
        TeleportToLauncherNoFall(pPlayer, kLauncherX, kLauncherY, kLauncherZ, kLauncherRot);
    } else {
        pPlayer->SetInterior(0, true);
        ZeroPedSpeed(pPlayer);
        pPlayer->SetRotation(kLauncherRot);
        pPlayer->SetTargetRotation(kLauncherRot);
    }

    LauncherSetupCameraToPed(pPlayer);

    pGame->SetWorldWeather(1);
    pGame->SetWorldTime(1, 0);
    pGame->DisplayHUD(false);

    if (!g_launcherRebuildDone.exchange(true, std::memory_order_acq_rel)) {
        RespawnLauncherPedOnce();
    } else {
        RespawnLauncherPedOnce();
    }

    pPlayer = pGame->FindPlayerPed();
    if (pPlayer && pPlayer->m_pPed)
    {
        LauncherSetupCameraToPed(pPlayer);
        LockControlsInLauncher(pPlayer);
        ResetTouchOnceAfterSetup(pPlayer);
    }

    g_launcherNeedApplySkin.store(true, std::memory_order_release);
    g_launcherApplyFrameCounter.store(0, std::memory_order_relaxed);

    g_launcherHoldUntilTick.store(GetTickCount() + 3000, std::memory_order_release);

    if (pJavaWrapper) pJavaWrapper->HideLoadingScreen();

    bNetworkInited = true;
    g_launcherNeedSetupScene.store(false, std::memory_order_release);
    g_inLauncherScene.store(true, std::memory_order_release);
}

static bool IsLauncherHoldActive()
{
    if (!bNetworkInited || bDebug) return false;
    if (!g_launcherForceHold.load(std::memory_order_acquire)) return false;

    const uint32_t until = g_launcherHoldUntilTick.load(std::memory_order_acquire);
    if (until == 0) return true;
    return GetTickCount() < until;
}

static void LauncherHold()
{
    if (!IsLauncherHoldActive()) return;
    if (!pGame) return;

    CPlayerPed* p = pGame->FindPlayerPed();
    if (!p || !p->m_pPed) return;

    RwMatrix mat = p->m_pPed->GetMatrix().ToRwMatrix();
    CVector mpos{mat.pos.x, mat.pos.y, mat.pos.z};

    const CVector target{kLauncherX, kLauncherY, kLauncherZ};

    if (Dist3(mpos, target) > 1.5f)
    {
        TeleportToLauncherNoFall(p, kLauncherX, kLauncherY, kLauncherZ, kLauncherRot);
    }
    else
    {
        ZeroPedSpeed(p);
        p->SetRotation(kLauncherRot);
        p->SetTargetRotation(kLauncherRot);
    }

    LauncherSetupCameraToPed(p);
    LockControlsInLauncher(p);
}

static void TryApplyLauncherSkinDeferred()
{
    if (!bNetworkInited || bDebug) return;
    if (!g_launcherNeedApplySkin.load(std::memory_order_acquire)) return;
    if (!pGame) return;

    int c = g_launcherApplyFrameCounter.fetch_add(1, std::memory_order_relaxed);
    if ((c % 10) != 0) return;

    CPlayerPed* pPlayer = pGame->FindPlayerPed();
    if (!pPlayer || !pPlayer->m_pPed) return;
    if (!GamePool_Ped_GetAt(pPlayer->m_dwGTAId)) return;

    const int skin = ClampSkin(g_launcherSkinId.load(std::memory_order_relaxed));
    const int cur  = (int)pPlayer->m_pPed->m_nModelIndex;

    if (cur == skin) {
        g_launcherNeedApplySkin.store(false, std::memory_order_release);
        return;
    }

    pPlayer->SetModelIndex((uint)skin);
    pPlayer->SetModelIndex((uint)skin);

    pPlayer->SetInterior(0, true);
    ZeroPedSpeed(pPlayer);
    pPlayer->SetRotation(kLauncherRot);
    pPlayer->SetTargetRotation(kLauncherRot);

    LauncherSetupCameraToPed(pPlayer);

    LockControlsInLauncher(pPlayer);
    ResetTouchOnceAfterSetup(pPlayer);

    g_launcherNeedApplySkin.store(false, std::memory_order_release);
}

static void SetNickToSettings(const char* nick)
{
    if (!nick) nick = "";

    std::strncpy(g_pendingNick, nick, sizeof(g_pendingNick) - 1);
    g_pendingNick[sizeof(g_pendingNick) - 1] = '\0';

    if (!pSettings) return;

    auto& cfg = pSettings->Get();
    size_t cap = sizeof(cfg.szNickName);
    if (cap == 0) return;

    std::strncpy(cfg.szNickName, nick, cap - 1);
    cfg.szNickName[cap - 1] = '\0';
}

static const char* GetClientNickCStr()
{
    if (pSettings)
    {
        const char* n = pSettings->Get().szNickName;
        if (n && n[0] != '\0') return n;
    }
    return g_pendingNick;
}
#include <cstdio>
#include <cstring>
#include <sys/stat.h>
#include <errno.h>

static bool FileExists(const char* path)
{
    struct stat st{};
    return (stat(path, &st) == 0) && S_ISREG(st.st_mode);
}

static void MkdirOne(const char* path)
{
#if defined(_WIN32)
    _mkdir(path);
#else
    mkdir(path, 0777);
#endif
}

static void EnsureDirForFile(const char* filePath)
{
    // filePath: /.../SAMP/settings.ini -> ensure /.../SAMP exists
    char tmp[512];
    strncpy(tmp, filePath, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    // cut to last '/'
    char* slash = strrchr(tmp, '/');
    if (!slash) return;
    *slash = '\0';

    // create all parents
    // /a/b/c
    // naive mkdir -p
    for (char* p = tmp + 1; *p; ++p)
    {
        if (*p == '/')
        {
            *p = '\0';
            MkdirOne(tmp);
            *p = '/';
        }
    }
    MkdirOne(tmp);
}

static void WriteDefaultSettingsIni(const char* path)
{
    EnsureDirForFile(path);

    FILE* f = fopen(path, "wb");
    if (!f) return;

    // IMPORTANT:
    // ты читаешь client:name (не nickname), поэтому пишем name=
    // и всё остальное, что реально нужно.
    const char* ini =
            "[client]\n"
            "name=__android_default\n"
            "password=\n"
            "version=0.3.7\n"
            "autoaim=0\n"
            "\n"
            "[debug]\n"
            "debug=0\n"
            "online=1\n"
            "\n"
            "[gui]\n"
            "Font=arial.ttf\n"
            "FontSize=30.0\n"
            "FontOutline=2\n"
            "ChatPosX=325.0\n"
            "ChatPosY=25.0\n"
            "ChatSizeX=1150.0\n"
            "ChatSizeY=220.0\n"
            "ChatMaxMessages=6\n"
            "HealthBarWidth=100.0\n"
            "HealthBarHeight=10.0\n"
            "ScoreBoardSizeX=846.0\n"
            "ScoreBoardSizeY=614.0\n"
            "PassengerUseTexture=1\n"
            "PassengerTextureSize=30.0\n"
            "PassengerTexturePosX=120.0\n"
            "PassengerTexturePosY=430.0\n"
            "Dialog=1\n"
            "VoiceChatEnable=1\n"
            "VoiceChatKey=66\n"
            "VoiceChatSize=30.0\n"
            "VoiceChatPosX=1520.0\n"
            "VoiceChatPosY=480.0\n"
            "androidkeyboard=0\n"
            "firstperson=1\n"
            "cutout=0\n"
            "fps=0\n"
            "FPSLimit=60\n"
            "hparmourtext=0\n"
            "outfitguns=0\n"
            "pcmoney=0\n"
            "radarrect=0\n"
            "skybox=0\n"
            "snow=0\n";

    fwrite(ini, 1, strlen(ini), f);
    fclose(f);
}

void ReadSettingFile()
{
    // если уже были настройки, не плодим утечки (опционально)
    // if (pSettings) { delete pSettings; pSettings = nullptr; }

    pSettings = new CSettings();

    if (g_pendingNick[0] != '\0')
        SetNickToSettings(g_pendingNick);

    // Crashlytics user id — только если ник не пустой
    if (pSettings && pSettings->Get().szNickName[0] != '\0')
        firebase::crashlytics::SetUserId(pSettings->Get().szNickName);
}

void DoInitStuff()
{
    if (!pGame) return;
    if (bGameInited) return;

    pPlayerTags = new CPlayerTags();
    pSnapShotHelper = new CSnapShotHelper();
    pAudioStream = new CAudioStream();
    pAudioStream->Initialize();

    CHud::toggleAll(false, true);
    CChat::ToggleChat(true);

    pGame->Initialize();
    pGame->SetMaxStats();

    LogVoice(OBFUSCATE("[dbg:multirium:load] : module loading..."));
    for (const auto& loadCallback : Samp::loadCallbacks) {
        if (loadCallback) loadCallback();
    }
    Samp::loadStatus = true;
    LogVoice(OBFUSCATE("[dbg:multirium:load] : module loaded"));

    JNIEnv *env = CJavaWrapper::GetEnv();
    jmethodID methods = env->GetStaticMethodID(CLoader::clazz, "loadWidgetSettings", "()V");
    env->CallStaticVoidMethod(CLoader::clazz, methods);

    if (!bDebug) {
        bNetworkInited = false;
        g_launcherNeedSetupScene.store(true, std::memory_order_release);
        g_launcherSetupFrameCounter.store(0, std::memory_order_relaxed);
        g_launcherDidInitialTeleport.store(false, std::memory_order_release);
        g_launcherForceHold.store(true, std::memory_order_release);
        g_inLauncherScene.store(false, std::memory_order_release);
        g_launcherRebuildDone.store(false, std::memory_order_release);
        g_launcherControlsLocked.store(false, std::memory_order_release);
        g_nextReassertControlTick.store(0, std::memory_order_release);
        g_lastTouchResetTick.store(0, std::memory_order_release);
        g_launcherNeedApplySkin.store(false, std::memory_order_release);
        g_launcherDidRespawn.store(false, std::memory_order_release);
        g_launcherRespawnFrames.store(0, std::memory_order_relaxed);
    }

    bGameInited = true;
}

void MainLoop()
{
    if (!pGame || pGame->bIsGameExiting) return;

    DoInitStuff();

    TrySetupLauncherSceneDeferred();
    LauncherHold();
    TryApplyLauncherSkinDeferred();
    g_launcherNeedApplySkin.store(false, std::memory_order_release);
    if (pNetGame) pNetGame->Process();
    if (pAudioStream) pAudioStream->Process();
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_multirium_mobile_launcher_telegram_TgBindActivity_nativeGetClientNick(JNIEnv* env, jclass)
{
    const char* nick = GetClientNickCStr();
    if (!nick) nick = "";
    return env->NewStringUTF(nick);
}

extern "C"
JNIEXPORT jstring JNICALL
Java_com_multirium_mobile_launcher_MainScreen_getClientNick(JNIEnv* env, jobject)
{
    const char* nick = GetClientNickCStr();
    if (!nick) nick = "";
    return env->NewStringUTF(nick);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_launcher_MainScreen_setClientNick(JNIEnv* env, jobject, jstring jnick)
{
    const char* nick = (jnick ? env->GetStringUTFChars(jnick, nullptr) : nullptr);
    SetNickToSettings(nick);
    if (jnick && nick) env->ReleaseStringUTFChars(jnick, nick);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_launcher_telegram_TgBindActivity_nativeSetClientNick(JNIEnv* env, jclass, jstring jnick)
{
    const char* nick = (jnick ? env->GetStringUTFChars(jnick, nullptr) : nullptr);
    SetNickToSettings(nick);
    if (jnick && nick) env->ReleaseStringUTFChars(jnick, nick);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_multirium_mobile_launcher_MainScreen_setLauncherSkin(JNIEnv*, jobject, jint skinId)
{
    const int s = ClampSkin((int)skinId);
    g_launcherSkinId.store(s, std::memory_order_relaxed);

    if (bNetworkInited && !bDebug) {
        g_launcherNeedApplySkin.store(true, std::memory_order_release);
        g_launcherApplyFrameCounter.store(0, std::memory_order_relaxed);
    }
}

extern "C" {

JNIEXPORT void JNICALL Java_com_multirium_mobile_core_Samp_initializeSAMP(JNIEnv *pEnv, jobject thiz)
{
    pJavaWrapper = new CJavaWrapper(pEnv, thiz);
}

JNIEXPORT void JNICALL
Java_com_multirium_mobile_launcher_MainScreen_startConnect(JNIEnv*, jobject)
{
    g_launcherForceHold.store(false, std::memory_order_release);
    g_launcherNeedApplySkin.store(false, std::memory_order_release);
    g_launcherNeedSetupScene.store(false, std::memory_order_release);
    g_inLauncherScene.store(false, std::memory_order_release);

    bNetworkInited = false;

    g_launcherRebuildDone.store(false, std::memory_order_release);
    g_launcherControlsLocked.store(false, std::memory_order_release);
    g_nextReassertControlTick.store(0, std::memory_order_release);
    g_lastTouchResetTick.store(0, std::memory_order_release);

    if (pGame) {
        CPlayerPed* p = pGame->FindPlayerPed();
        if (p && p->m_pPed) p->TogglePlayerControllable(true);

        CCamera::SetBehindPlayer();
        pGame->DisplayHUD(true);
        pGame->EnableClock(true);
    }

    const char* nick = GetClientNickCStr();
    const char* pass = (pSettings ? pSettings->Get().szPassword : "");
    if (!nick) nick = "";
    if (!pass) pass = "";

    pNetGame = new CNetGame(OBFUSCATE("141.95.234.17"), 2045, nick, pass);
}

JNIEXPORT void JNICALL Java_com_multirium_mobile_core_Samp_onInputEnd(JNIEnv*, jobject, jbyteArray) {}
JNIEXPORT void JNICALL Java_com_multirium_mobile_core_Samp_onEventBackPressed(JNIEnv*, jobject) {}

}

void InitGui()
{
    Plugin::OnPluginLoad();
    Plugin::OnSampLoad();
    pGUI = new CGUI();
    CFontRenderer::Initialise();
}

uint32_t GetTickCount()
{
    return CTimer::m_snTimeInMillisecondsNonClipped;
}

void Log(const char* fmt, ...)
{
    char buffer[0xFF];
    static FILE* flLog = nullptr;

    if (flLog == nullptr && g_pszStorage != nullptr)
    {
        sprintf(buffer, "%s/samp_log.txt", g_pszStorage);
        flLog = fopen(buffer, "a");
    }

    memset(buffer, 0, sizeof(buffer));

    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, arg);
    va_end(arg);

    firebase::crashlytics::Log(buffer);

    if (flLog) {
        fprintf(flLog, "%s\n", buffer);
        fflush(flLog);
    }

    __android_log_write(ANDROID_LOG_INFO, "AXL", buffer);
}

void LogVoice(const char* fmt, ...)
{
    char buffer[0xFF];
    static FILE* flLog = nullptr;
    const char* pszStorage = g_pszStorage;

    if (flLog == nullptr && pszStorage != nullptr)
    {
        sprintf(buffer, "%sSAMP/%s", pszStorage, SV::kLogFileName);
        flLog = fopen(buffer, "w");
    }

    memset(buffer, 0, sizeof(buffer));

    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, arg);
    va_end(arg);

    __android_log_write(ANDROID_LOG_INFO, "AXL", buffer);

    if (flLog) {
        fprintf(flLog, "%s\n", buffer);
        fflush(flLog);
    }
}

struct sigaction act_old;
struct sigaction act1_old;
struct sigaction act2_old;
struct sigaction act3_old;

extern int g_iLastProcessedSkinCollision, g_iLastProcessedEntityCollision, g_iLastRenderedObject;
extern uintptr_t g_dwLastRetAddrCrash;

void handler(int signum, siginfo_t *info, void* contextPtr)
{
    ucontext* context = (ucontext_t*)contextPtr;
    if (act_old.sa_sigaction) act_old.sa_sigaction(signum, info, contextPtr);

    if(info->si_signo == SIGSEGV)
    {
        Log("SIGSEGV | Fault address: 0x%x", info->si_addr);
        PRINT_CRASH_STATES(context);
        CStackTrace::printBacktrace();
    }
}

void handler1(int signum, siginfo_t *info, void* contextPtr)
{
    ucontext* context = (ucontext_t*)contextPtr;
    if (act1_old.sa_sigaction) act1_old.sa_sigaction(signum, info, contextPtr);

    if(info->si_signo == SIGABRT)
    {
        Log("SIGABRT | Fault address: 0x%x", info->si_addr);
        PRINT_CRASH_STATES(context);
        CStackTrace::printBacktrace();
    }
}

void handler2(int signum, siginfo_t *info, void* contextPtr)
{
    ucontext* context = (ucontext_t*)contextPtr;
    if (act2_old.sa_sigaction) act2_old.sa_sigaction(signum, info, contextPtr);

    if(info->si_signo == SIGFPE)
    {
        Log("SIGFPE | Fault address: 0x%x", info->si_addr);
        PRINT_CRASH_STATES(context);
        CStackTrace::printBacktrace();
    }
}

void handler3(int signum, siginfo_t *info, void* contextPtr)
{
    ucontext* context = (ucontext_t*)contextPtr;
    if (act3_old.sa_sigaction) act3_old.sa_sigaction(signum, info, contextPtr);

    if(info->si_signo == SIGBUS)
    {
        Log("SIGBUS | Fault address: 0x%x", info->si_addr);
        PRINT_CRASH_STATES(context);
        CStackTrace::printBacktrace();
    }
}

void ApplyPatches();
void ApplyPatches_level0();
void InstallSpecialHooks();
void InitRenderWareFunctions();

float gVehicleDistanceFromCamera;
float gAngleWithHorizontal;

uint32_t gGameState;
CAudioEngine AudioEngine;
bool RwInitialized;

void OS_ThreadSleep(int uS) { usleep(uS); }

void InitialiseGame()
{
    CHook::CallFunction<void>("_Z20Menu_SwitchOffToGamev");
    CHook::CallFunction<void>("_Z14InitialiseGamev");
}

int SkipBankLoader;
bool isForeground;

void DoGameState() {
    gGameState = *(uint32_t*)(g_libGTASA + 0xD3D640);
    AudioEngine = *(CAudioEngine*)(g_libGTASA + 0xBB65F8);
    RwInitialized = *(bool*)(g_libGTASA + 0xD3D668);
    SkipBankLoader = *(bool*)(g_libGTASA + 0x85DCE8);
    isForeground = *(bool*)(g_libGTASA + 0xD3D66C);

    switch (gGameState) {
        case 0: case 1: case 2: case 3: case 4: case 5:
        {
            bool initSuccess = CHook::CallFunction<bool>("_ZN5CGame27InitialiseEssentialsAfterRWEv");
            if (!initSuccess) { RsGlobal->quit = true; break; }
            CHook::CallFunction<bool>("_ZN5CGame25InitialiseCoreDataAfterRWEv");
            gGameState = 6;
            SkipBankLoader = 0;
        } break;

        case 6: case 7: case 8:
            InitialiseGame();
            gGameState = 9;
            CHook::CallFunction<void>("_ZN12CAudioEngine21InitialisePostLoadingEv", &AudioEngine);
            break;

        case 9:
        {
            uint32_t currentTime = CTimer::GetCurrentTimeInCycles();
            uint32_t cyclesPerMs = CTimer::GetCyclesPerMillisecond();
            double frameTimeThreshold = cyclesPerMs * 0.001 * (1000000.0 / RsGlobal->maxFPS);

            if (RwInitialized && currentTime > frameTimeThreshold) {
                CHook::CallFunction<void>("_Z11UpdateInputv");
                CHook::CallFunction<RsEventStatus>("_Z14RsEventHandler7RsEventPv", rsIDLE, 1);
            } else {
                OS_ThreadSleep(100);
            }
        } break;

        default:
            break;
    }

    if (!isForeground) isForeground = true;
}

uintptr_t g_libGTASA = 0x00;
uintptr_t g_libMultirium = 0x00;

jint JNI_OnLoad(JavaVM* vm, void*)
{
    javaVM = vm;

    g_libGTASA = CUtil::FindLib("libGTASA.so");
    if (g_libGTASA == 0x00) return JNI_VERSION_1_6;

    g_libMultirium = CUtil::FindLib("libmultirium-client.so");
    if (g_libMultirium == 0x00) return JNI_VERSION_1_6;

    firebase::crashlytics::Initialize();

    CHook::gloslib = GlossOpen("libGTASA.so");

    InstallSpecialHooks();
    ApplyPatches_level0();
    InitRenderWareFunctions();
    MultiTouch::initialize();

    gGameState = *(uint32_t*)(g_libGTASA + 0xD3D640);
    AudioEngine = *(CAudioEngine*)(g_libGTASA + 0xBB65F8);
    RwInitialized = *(bool*)(g_libGTASA + 0xD3D668);
    SkipBankLoader = *(bool*)(g_libGTASA + 0x85DCE8);
    isForeground = *(bool*)(g_libGTASA + 0xD3D66C);

    CLoader::initJavaClasses(vm);

    pGame = new CGame();

    struct sigaction act{};
    act.sa_sigaction = handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGSEGV, &act, &act_old);

    struct sigaction act1{};
    act1.sa_sigaction = handler1;
    sigemptyset(&act1.sa_mask);
    act1.sa_flags = SA_SIGINFO;
    sigaction(SIGABRT, &act1, &act1_old);

    struct sigaction act2{};
    act2.sa_sigaction = handler2;
    sigemptyset(&act2.sa_mask);
    act2.sa_flags = SA_SIGINFO;
    sigaction(SIGFPE, &act2, &act2_old);

    struct sigaction act3{};
    act3.sa_sigaction = handler3;
    sigemptyset(&act3.sa_mask);
    act3.sa_flags = SA_SIGINFO;
    sigaction(SIGBUS, &act3, &act3_old);

    return JNI_VERSION_1_6;
}
