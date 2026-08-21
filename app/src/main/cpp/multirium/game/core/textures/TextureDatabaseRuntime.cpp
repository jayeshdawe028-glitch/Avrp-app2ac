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

    void EnsureStartupTextureDatabases()
    {
        if (g_textureDbReady.load(std::memory_order_acquire))
            return;

        bool expected = false;
        if (!g_textureDbBootstrapping.compare_exchange_strong(
                expected, true, std::memory_order_acq_rel))
            return;

        if (g_libGTASA <= 0x1000 || RsGlobal == nullptr) {
            Log("Texture DB bootstrap skipped: RenderWare is not ready");
            g_textureDbBootstrapping.store(false, std::memory_order_release);
            return;
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

        bool anyLoaded = false;
        for (const auto& spec : databases)
        {
            TextureDatabaseRuntime* db = Load(spec.name, false, spec.format);
            if (!db)
            {
                Log("Texture DB bootstrap: %s failed", spec.name);
                continue;
            }

            anyLoaded = true;
            Log("Texture DB bootstrap: %s loaded (%p)", spec.name, (void*)db);

            if (spec.registerDb)
                Register(db);
        }

        if (anyLoaded)
            g_textureDbReady.store(true, std::memory_order_release);

        g_textureDbBootstrapping.store(false, std::memory_order_release);
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
    EnsureStartupTextureDatabases();

    RwTexture* texture = ((RwTexture*(*)(const char*))(g_libGTASA + (VER_x32 ? 0x001E9C64 + 1 : 0x286718)))(name);
    if (!texture)
        Log("TextureDatabaseRuntime::GetTexture returned null: %s", name ? name : "<null>");

    return texture;
}

void TextureDatabaseRuntime::UpdateStreaming(float deltaTime, bool flush) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x1E9518 + 1 : 0x285DBC), deltaTime, flush);
}

TextureDatabaseRuntime* TextureDatabaseRuntime::GetDatabase(const char *dbName) {
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
