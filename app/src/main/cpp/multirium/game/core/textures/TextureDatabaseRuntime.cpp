//
// Created by x1y2z on 11.01.2023.
//

#include "TextureDatabaseRuntime.h"
#include "../vendor/armhook/patch.h"
#include "../../RW/TxdStore.h"
#include "../../RW/VisibilityPlugins.h"
#include <atomic>

extern void Log(const char* fmt, ...);

namespace {
    std::atomic<bool> g_textureDbBootstrapping{false};
    std::atomic<bool> g_textureDbReady{false};

    bool EnsureStartupTextureDatabases()
    {
        if (g_textureDbReady.load(std::memory_order_acquire))
            return true;

        bool expected = false;
        if (!g_textureDbBootstrapping.compare_exchange_strong(
                expected, true, std::memory_order_acq_rel))
            return false;

        if (g_libGTASA <= 0x1000 || RsGlobal == nullptr) {
            Log("Texture DB bootstrap skipped: RenderWare is not ready");
            g_textureDbBootstrapping.store(false, std::memory_order_release);
            return false;
        }

        Log("Texture DB bootstrap: initializing native texture runtime");
        CTxdStore::Initialise();
        CHook::CallFunction<void>("_ZN18CVisibilityPlugins10InitialiseEv");

        struct TextureDbSpec {
            const char* name;
            TextureDatabaseFormat format;
            bool registerDb;
        };

        static const TextureDbSpec databases[] = {
            {"mobile", TextureDatabaseFormat::DF_DXT, false},
            {"txd",    TextureDatabaseFormat::DF_DXT, false},
            {"gta3",   TextureDatabaseFormat::DF_DXT, false},
            {"menu",   TextureDatabaseFormat::DF_DXT, false},
            {"samp",   TextureDatabaseFormat::DF_DXT, false},
            {"gui",    TextureDatabaseFormat::DF_DXT, false},
            {"gtasa",  TextureDatabaseFormat::DF_DXT, true},
            {"radar",  TextureDatabaseFormat::DF_ETC, true},
            {"cars",   TextureDatabaseFormat::DF_DXT, true},
            {"skins",  TextureDatabaseFormat::DF_DXT, true},
        };

        bool menuLoaded = false;
        bool sampLoaded = false;

        for (const auto& spec : databases)
        {
            TextureDatabaseRuntime* db = TextureDatabaseRuntime::Load(spec.name, false, spec.format);
            if (!db)
            {
                Log("Texture DB bootstrap: %s failed", spec.name);
                continue;
            }

            Log("Texture DB bootstrap: %s loaded (%p)", spec.name, (void*)db);

            if (strcmp(spec.name, "menu") == 0)
                menuLoaded = true;
            else if (strcmp(spec.name, "samp") == 0)
                sampLoaded = true;

            if (spec.registerDb)
                TextureDatabaseRuntime::Register(db);
        }

        const bool ready = menuLoaded && sampLoaded;
        g_textureDbReady.store(ready, std::memory_order_release);
        g_textureDbBootstrapping.store(false, std::memory_order_release);

        Log("Texture DB bootstrap finished: menu=%d samp=%d ready=%d",
            menuLoaded ? 1 : 0, sampLoaded ? 1 : 0, ready ? 1 : 0);

        return ready;
    }
}

TextureDatabaseRuntime* TextureDatabaseRuntime::Load(const char *withName, bool fullyLoad, TextureDatabaseFormat forcedFormat)
{
    Log("========== Texture Load ==========");
    Log("Name: %s", withName);
    Log("g_libGTASA = %p", (void*)g_libGTASA);

    TextureDatabaseRuntime* db =
        CHook::CallFunction<TextureDatabaseRuntime*>(
            g_libGTASA + (VER_x32 ? 0x001EA864 + 1 : 0x28771C),
            withName,
            fullyLoad,
            forcedFormat
        );

    Log("Returned Pointer = %p", (void*)db);
    Log("==================================");

    return db;
}

void TextureDatabaseRuntime::Register(TextureDatabaseRuntime *toRegister) {
    if (!toRegister)
        return;
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x1E9B48 + 1 : 0x2865D8), toRegister);
}

void TextureDatabaseRuntime::Unregister(TextureDatabaseRuntime *toUnregister) {
    if (!toUnregister)
        return;
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x1E9C00 + 1 : 0x2866A4), toUnregister);
}

RwTexture* TextureDatabaseRuntime::GetTexture(const char *name) {
    if (!name || g_libGTASA <= 0x1000 || RsGlobal == nullptr) {
        Log("TextureDatabaseRuntime::GetTexture blocked: runtime not ready");
        return nullptr;
    }

    if (!EnsureStartupTextureDatabases()) {
        Log("TextureDatabaseRuntime::GetTexture blocked until texture DBs are ready: %s", name);
        return nullptr;
    }

    RwTexture* texture = ((RwTexture*(*)(const char*))(g_libGTASA + (VER_x32 ? 0x001E9C64 + 1 : 0x286718)))(name);
    if (!texture)
        Log("TextureDatabaseRuntime::GetTexture returned null: %s", name);

    return texture;
}

void TextureDatabaseRuntime::UpdateStreaming(float deltaTime, bool flush) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x1E9518 + 1 : 0x285DBC), deltaTime, flush);
}

TextureDatabaseRuntime* TextureDatabaseRuntime::GetDatabase(const char *dbName) {
    if (!dbName || g_libGTASA <= 0x1000 || RsGlobal == nullptr)
        return nullptr;
    return CHook::CallFunction<TextureDatabaseRuntime*>(g_libGTASA + (VER_x32 ? 0x1EAC0C + 1 : 0x287AF4), dbName);
}

void TextureDatabaseRuntime::InjectHooks() {
    // CHook::Write(g_libGTASA + (VER_x32 ? 0x675FCC : 0x84A020), &detailTextures);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x676720 : 0x84AEB0), &storedTexels);
    // CHook::Write(g_libGTASA + (VER_x32 ? 0x679768 : 0x850EE8), &loaded);
    // CHook::Write(g_libGTASA + (VER_x32 ? 0x679E70 : 0x851CF8), &registered);
}

RwBool TextureAnnihilate(RwTexture *texture) {
    return CHook::CallFunction<RwBool>(g_libGTASA + (VER_x32 ? 0x1DB33C + 1 : 0x273A1C), texture);
}
