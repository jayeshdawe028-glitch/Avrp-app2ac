#include "main.h"
#include "settings.h"
#include "vendor/inih/cpp/INIReader.h"

#include "vendor/SimpleIni/SimpleIni.h"
#include "game/core/game.h"

#include <cstdio>
#include <cstring>
#include <sys/stat.h>

extern CGame *pGame;
stSettings CSettings::m_Settings;

// ------------------------------------------------------------
// Helpers
// ------------------------------------------------------------

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
    // /a/b/c/settings.ini -> ensure /a/b/c exists
    char tmp[512];
    strncpy(tmp, filePath, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';

    char* slash = strrchr(tmp, '/');
    if (!slash) return;
    *slash = '\0';

    // mkdir -p
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

    // ВАЖНО: ты читаешь client:name (а не nickname), поэтому пишем name=
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

// ------------------------------------------------------------
// CSettings internals
// ------------------------------------------------------------

static void ApplyDefaults(stSettings& s)
{
    // client
    snprintf(s.szNickName, sizeof(s.szNickName), "__android_%d%d", rand() % 1000, rand() % 1000);
    s.szPassword[0] = '\0';
    strncpy(s.szVersion, "0.3.7", MAX_SETTINGS_STRING);
    s.szVersion[MAX_SETTINGS_STRING] = '\0';
    s.bAutoAim = false;

    // debug
    s.bDebug = false;
    s.bOnline = true;

    // gui
    strncpy(s.szFont, "arial.ttf", sizeof(s.szFont) - 1);
    s.szFont[sizeof(s.szFont) - 1] = '\0';
    s.fFontSize = 30.0f;
    s.iFontOutline = 2;

    // chat
    s.fChatPosX = 325.0f;
    s.fChatPosY = 25.0f;
    s.fChatSizeX = 1150.0f;
    s.fChatSizeY = 220.0f;
    s.iChatMaxMessages = 6;

    // nametags
    s.fHealthBarWidth = 100.0f;
    s.fHealthBarHeight = 10.0f;

    // scoreboard
    s.fScoreBoardSizeX = 846.0f;
    s.fScoreBoardSizeY = 614.0f;

    // passenger
    s.bPassengerUseTexture = true;
    s.fPassengerTextureSize = 30.0f;
    s.fPassengerTextureX = 120.0f;
    s.fPassengerTextureY = 430.0f;

    s.iDialog = true;

    s.bVoiceChatEnable = true;
    s.iVoiceChatKey = 66;
    s.fVoiceChatSize = 30.0f;
    s.fVoiceChatPosX = 1520.0f;
    s.fVoiceChatPosY = 480.0f;

    s.iAndroidKeyboard = false;
    s.iFirstPerson = true;
    s.iCutout = false;
    s.iFPSCounter = false;
    s.iFPSCount = 60;
    s.iHPArmourText = false;
    s.iOutfitGuns = false;
    s.iPCMoney = false;
    s.iRadarRect = false;
    s.iSkyBox = false;
    s.iSnow = false;
}

static void LoadFromReader(stSettings& s, INIReader& reader)
{
    // client
    size_t length = 0;
    char defNick[64];
    snprintf(defNick, sizeof(defNick), "__android_%d%d", rand() % 1000, rand() % 1000);

    length = reader.Get("client", "name", defNick).copy(s.szNickName, 24);
    s.szNickName[length] = '\0';

    length = reader.Get("client", "password", "").copy(s.szPassword, MAX_SETTINGS_STRING);
    s.szPassword[length] = '\0';

    length = reader.Get("client", "version", "0.3.7").copy(s.szVersion, MAX_SETTINGS_STRING);
    s.szVersion[length] = '\0';

    s.bAutoAim = reader.GetBoolean("client", "autoaim", false);

    // debug
    s.bDebug = reader.GetBoolean("debug", "debug", false);
    s.bOnline = reader.GetBoolean("debug", "online", true);

    // gui
    length = reader.Get("gui", "Font", "arial.ttf").copy(s.szFont, MAX_SETTINGS_STRING);
    s.szFont[length] = '\0';
    s.fFontSize = reader.GetReal("gui", "FontSize", 30.0f);
    s.iFontOutline = reader.GetInteger("gui", "FontOutline", 2);

    // chat
    s.fChatPosX = reader.GetReal("gui", "ChatPosX", 325.0f);
    s.fChatPosY = reader.GetReal("gui", "ChatPosY", 25.0f);
    s.fChatSizeX = reader.GetReal("gui", "ChatSizeX", 1150.0f);
    s.fChatSizeY = reader.GetReal("gui", "ChatSizeY", 220.0f);
    s.iChatMaxMessages = reader.GetInteger("gui", "ChatMaxMessages", 6);

    // nametags
    s.fHealthBarWidth = reader.GetReal("gui", "HealthBarWidth", 100.0f);
    s.fHealthBarHeight = reader.GetReal("gui", "HealthBarHeight", 10.0f);

    // scoreboard
    s.fScoreBoardSizeX = reader.GetReal("gui", "ScoreBoardSizeX", 846.0f);
    s.fScoreBoardSizeY = reader.GetReal("gui", "ScoreBoardSizeY", 614.0f);

    // passenger
    s.bPassengerUseTexture = reader.GetBoolean("gui", "PassengerUseTexture", true);
    s.fPassengerTextureSize = reader.GetReal("gui", "PassengerTextureSize", 30.0f);
    s.fPassengerTextureX = reader.GetReal("gui", "PassengerTexturePosX", 120.0f);
    s.fPassengerTextureY = reader.GetReal("gui", "PassengerTexturePosY", 430.0f);

    s.iDialog = reader.GetBoolean("gui", "Dialog", true);

    s.bVoiceChatEnable = reader.GetBoolean("gui", "VoiceChatEnable", true);
    s.iVoiceChatKey = reader.GetInteger("gui", "VoiceChatKey", 66);
    s.fVoiceChatSize = reader.GetReal("gui", "VoiceChatSize", 30.0f);
    s.fVoiceChatPosX = reader.GetReal("gui", "VoiceChatPosX", 1520.0f);
    s.fVoiceChatPosY = reader.GetReal("gui", "VoiceChatPosY", 480.0f);

    s.iAndroidKeyboard = reader.GetBoolean("gui", "androidkeyboard", false);
    s.iFirstPerson = reader.GetBoolean("gui", "firstperson", true);
    s.iCutout = reader.GetBoolean("gui", "cutout", false);
    s.iFPSCounter = reader.GetBoolean("gui", "fps", false);
    s.iFPSCount = reader.GetInteger("gui", "FPSLimit", 60);
    s.iHPArmourText = reader.GetBoolean("gui", "hparmourtext", false);
    s.iOutfitGuns = reader.GetBoolean("gui", "outfitguns", false);
    s.iPCMoney = reader.GetBoolean("gui", "pcmoney", false);
    s.iRadarRect = reader.GetBoolean("gui", "radarrect", false);
    s.iSkyBox = reader.GetBoolean("gui", "skybox", false);
    s.iSnow = reader.GetBoolean("gui", "snow", false);
}

// ------------------------------------------------------------
// CSettings
// ------------------------------------------------------------

CSettings::CSettings()
{
    Log("Loading settings..");

    char path[0x7F];
    snprintf(path, sizeof(path), "%sSAMP/settings.ini", g_pszStorage);

    // Всегда ставим дефолты в память первым делом
    ApplyDefaults(m_Settings);

    // Если файла нет — создаём
    if (!FileExists(path))
    {
        Log("settings.ini missing -> creating default: %s", path);
        WriteDefaultSettingsIni(path);
    }

    // Пытаемся прочитать
    INIReader reader(path);
    if (reader.ParseError() < 0)
    {
        Log("Error: can't load %s (ParseError=%d) -> rewrite defaults", path, reader.ParseError());

        // Перезапишем дефолтом и попробуем снова
        WriteDefaultSettingsIni(path);

        INIReader reader2(path);
        if (reader2.ParseError() < 0)
        {
            // Всё равно плохо — остаёмся на дефолтах
            Log("Error: still can't load settings.ini -> using memory defaults");
            Log("Settings loaded (defaults).");
            return;
        }

        LoadFromReader(m_Settings, reader2);
        Log("Settings loaded.");
        return;
    }

    LoadFromReader(m_Settings, reader);
    Log("Settings loaded.");
}

CSettings::~CSettings()
{
}
