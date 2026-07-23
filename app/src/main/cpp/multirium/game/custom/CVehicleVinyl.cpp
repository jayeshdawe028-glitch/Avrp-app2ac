#include "VehicleVinyl.h"
#include "util/CUtil.h"
#include <cstdio>
#include <cstring>

RwTexture* CVehicleVinyl::s_remap[256]{};

// --------- Список винилов с твоих скринов ---------
// id 0 = нет
// 1 = remapbody
// 2 = remapbody01
// 3..31 = remapbody1..remapbody30
// 32..35 = remaptoner..remaptoner3
static const char* kVinylNames[] =
        {
                nullptr,       // 0

                "cross",   // 1
                "remapbody01", // 2

                "remapbody1",  // 3
                "remapbody2",  // 4
                "remapbody3",  // 5
                "remapbody4",  // 6
                "remapbody5",  // 7
                "remapbody6",  // 8
                "remapbody7",  // 9
                "remapbody8",  // 10
                "remapbody9",  // 11
                "remapbody10", // 12
                "remapbody11", // 13
                "remapbody12", // 14
                "remapbody13", // 15
                "remapbody14", // 16
                "remapbody15", // 17
                "remapbody16", // 18
                "remapbody17", // 19
                "remapbody18", // 20
                "remapbody19", // 21
                "remapbody20", // 22
                "remapbody21", // 23
                "remapbody22", // 24
                "remapbody23", // 25
                "remapbody24", // 26
                "remapbody25", // 27
                "remapbody26", // 28
                "remapbody27", // 29
                 "elegant",// у тебя на скрине нет — пропускаем специально
                "vinyl_6", // 30
                "remapbody30", // 31

                "remaptoner",  // 32
                "remaptoner1", // 33
                "vinyl_19", // 34
                "vinyal_19", // 35
        };

static constexpr uint8_t kMaxVinylId =
        (uint8_t)(sizeof(kVinylNames) / sizeof(kVinylNames[0]) - 1);

static RwTexture* TryLoad(const char* db, const char* name)
{
    // В твоём проекте CUtil::LoadTextureFromDB возвращает RwTexture*
    return (RwTexture*)CUtil::LoadTextureFromDB(db, name);
}

uint8_t CVehicleVinyl::MaxId()
{
    return kMaxVinylId;
}

const char* CVehicleVinyl::Name(uint8_t id)
{
    if (id == 0 || id > kMaxVinylId) return nullptr;
    return kVinylNames[id];
}

void CVehicleVinyl::Initialise()
{
    Log("CVehicleVinyl::Initialise max=%d", (int)kMaxVinylId);

    // можно чуть прогреть популярные
    EnsureLoaded(1);
    EnsureLoaded(2);
    EnsureLoaded(3);
    EnsureLoaded(12);
}

void CVehicleVinyl::EnsureLoaded(uint8_t id)
{
    if (id == 0) return;
    if (id > kMaxVinylId) {
        Log("EnsureLoaded: invalid id=%d (max=%d)", (int)id, (int)kMaxVinylId);
        return;
    }

    if (s_remap[id]) return;

    const char* texName = kVinylNames[id];
    if (!texName) return;

    // 1) основной путь: texdb "samp"
    s_remap[id] = TryLoad("samp", texName);

    // 2) иногда у людей лежит в "txd"
    if (!s_remap[id])
        s_remap[id] = TryLoad("txd", texName);

    Log("EnsureLoaded vinyl id=%d name=%s => tex=%p", (int)id, texName, s_remap[id]);
}
