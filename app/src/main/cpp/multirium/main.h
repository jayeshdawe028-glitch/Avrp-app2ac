#pragma once

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <unistd.h>
#include "log.h"
#include <jni.h>
#include <cstring>
#include "game/core/common.h"
#include "vendor/bass/bass.h"
#include "vendor/bass/bass_fx.h"
//#include "Camera.h"

extern char* g_pszStorage;

extern float gVehicleDistanceFromCamera;
extern float gAngleWithHorizontal;

#ifndef VALIDATE_SIZE
#define VALIDATE_SIZE(type, size) static_assert(sizeof(type) == size, "Invalid size for " #type);
#endif

struct tBeatInfoBeat {
    int Time;
    int Type;
};
VALIDATE_SIZE(tBeatInfoBeat, 0x8);

struct tBeatInfo {
    tBeatInfoBeat BeatWindow[20];
    int bBeatInfoPresent;
    int BeatTypeThisFrame;
    int BeatNumber;
};
VALIDATE_SIZE(tBeatInfo, 0xAC);

struct CAudioEngine // sizeof=0x2F50
{
    bool m_bPlayingMissionCompleteTrack;
    bool m_bStoppingMissionCompleteTrack;
    uint8_t m_nActiveRadioStationBeforeCutscene;
    uint8_t pad[1];
    int m_nAudioStreamClientHandle;
    tBeatInfo m_BeatInfo;
    uint8_t pad1[4];
    uintptr_t m_FrontendAudioEntity;
    uintptr_t m_ScriptAudioEntity;
    uintptr_t m_CollisionAudioEntity;
    uintptr_t* m_pGlobalWeaponAudioEntity;
    uintptr_t m_PedlessSpeechAudioEntity;
    uintptr_t m_GlobalDoorAudioEntity;
};
#if VER_x32
VALIDATE_SIZE(CAudioEngine, 0xD0);
#else
VALIDATE_SIZE(CAudioEngine, 0xE8);
#endif

//extern CCamera TheCamera;

#ifndef SAFE_DELETE
	#define SAFE_DELETE(p) { if (p) { delete (p); (p) = NULL; } }
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) ( sizeof((a)) / sizeof(*(a)) )
#endif

#define SAMP_VERSION	"0.3.7"

#include "obfuscate/str_obfuscate.hpp"

#define RAKSAMP_CLIENT
#define NETCODE_CONNCOOKIELULZ 0x6969

extern uintptr_t g_libGTASA;
extern uintptr_t g_libMultirium;

extern JavaVM* javaVM;

uint32_t GetTickCount();
void LogVoice(const char* fmt, ...);

void Log(const char* fmt, ...);

template<typename ... Args>
std::string string_format( const std::string& format, Args ... args )
{
    int size_s = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    if( size_s <= 0 ){ throw std::runtime_error( "Error during formatting." ); }
    auto size = static_cast<size_t>( size_s );
    std::unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, format.c_str(), args ... );
    return std::string( buf.get(), buf.get() + size - 1 ); // We don't want the '\0' inside
}