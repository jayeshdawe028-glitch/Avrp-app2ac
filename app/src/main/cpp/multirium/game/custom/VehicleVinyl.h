#pragma once

#include <cstdint>
#include "game/fakerw/rwcore.h"

class CVehicleVinyl
{
public:
    // В диапазоне 0..35 (как у нас в списке). Можно оставить 256 чтобы не ломать интерфейс.
    static RwTexture* s_remap[256];

    static void Initialise();

    // грузит по id если ещё не грузили
    static void EnsureLoaded(uint8_t id);

    static RwTexture* Get(uint8_t id) { return s_remap[id]; }
    static RwTexture* GetOrLoad(uint8_t id) { EnsureLoaded(id); return s_remap[id]; }

    // Имя текстуры по id (чтобы сервер/клиент одинаково знали)
    static const char* Name(uint8_t id);

    // Максимальный id винила в списке
    static uint8_t MaxId();
};
