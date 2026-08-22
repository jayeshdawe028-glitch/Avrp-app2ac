//
// Created by x1y2z on 11.01.2023.
//

#include "TextureDatabaseRuntime.h"
#include "../vendor/armhook/patch.h"
#include <atomic>
#include <cstring>

extern void Log(const char* fmt, ...);
extern void InstallMobileMenuCrashGuardHooks();

namespace {
    // Keep the texture fix narrow: MobileMenu is the first native consumer
    // currently reaching GetTexture before its database is valid.
    std::atomic<bool> g_menuDbPreparing{false};
    std::atomic<bool> g_menuDbReady{false};

    using MobileMenuLoadFn = void(*)();
    MobileMenuLoadFn g_originalMobileMenuLoad = nullptr;

    bool IsRenderWareReady()
    {
        return g_libGTASA > 0x1000 && RsGlobal != nullptr;
    }

    bool EnsureMenuTextureDatabase()
    {
        if (g_menuDbReady.load(std::memory_order_acquire))
            return true;

        if (!IsRenderWareReady()) {
            Log("Texture DB: menu bootstrap deferred; RenderWare is not ready");
            return false;
        }

        bool expected = false;
        if (!g_menuDbPreparing.compare_exchange_strong(
                expected, true, std::memory_order_acq_rel)) {
            // Another thread/frame is doing the load. Do not enter the native
            // texture path concurrently with that load.
            return false;
        }

        bool ready = false;
        TextureDatabaseRuntime* menu = TextureDatabaseRuntime::GetDatabase("menu");

        if (!menu) {
            Log("Texture DB: loading required 'menu' database");
            menu = TextureDatabaseRuntime::Load(
                    "menu",
                    true,
                    TextureDatabaseFormat::DF_DXT
            );
        }

        if (menu) {
            TextureDatabaseRuntime* registeredMenu =
                    TextureDatabaseRuntime::GetDatabase("menu");
            if (!registeredMenu) {
                Log("Texture DB: registering loaded 'menu' database (%p)", (void*)menu);
                TextureDatabaseRuntime::Register(menu);
                registeredMenu = TextureDatabaseRuntime::GetDatabase("menu");
            }

            ready = registeredMenu != nullptr;
            Log("Texture DB: menu ready=%d ptr=%p", ready ? 1 : 0,
                (void*)registeredMenu);
        } else {
            Log("Texture DB: required 'menu' database could not be loaded");
        }

        if (ready)
            g_menuDbReady.store(true, std::memory_order_release);

        g_menuDbPreparing.store(false, std::memory_order_release);
        return ready;
    }

    void MobileMenuLoadHook()
    {
        Log("MobileMenu::Load: validating texture DB lifecycle");

        if (!EnsureMenuTextureDatabase()) {
            Log("MobileMenu::Load deferred: 'menu' texture database unavailable");
            return;
        }

        if (g_originalMobileMenuLoad) {
            g_originalMobileMenuLoad();
        } else {
            Log("MobileMenu::Load deferred: original function unavailable");
        }
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
    if (!toRegister) {
        Log("Texture DB: Register skipped for null database");
        return;
    }
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x1E9B48 + 1 : 0x2865D8), toRegister);
}

void TextureDatabaseRuntime::Unregister(TextureDatabaseRuntime *toUnregister) {
    if (!toUnregister)
        return;
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x1E9C00 + 1 : 0x2866A4), toUnregister);
}

RwTexture* TextureDatabaseRuntime::GetTexture(const char *name) {
    // Keep this wrapper low-level. Global database bootstrap from GetTexture()
    // caused re-entrancy and startup ordering problems; the owning menu path is
    // guarded at its entry point instead.
    RwTexture* texture = ((RwTexture*(*)(const char*))(g_libGTASA + (VER_x32 ? 0x001E9C64 + 1 : 0x286718)))(name);
    if (!texture)
        Log("TextureDatabaseRuntime::GetTexture returned null: %s", name ? name : "<null>");

    return texture;
}

void TextureDatabaseRuntime::UpdateStreaming(float deltaTime, bool flush) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x1E9518 + 1 : 0x285DBC), deltaTime, flush);
}

TextureDatabaseRuntime* TextureDatabaseRuntime::GetDatabase(const char *dbName) {
    if (!dbName || !*dbName)
        return nullptr;
    return CHook::CallFunction<TextureDatabaseRuntime*>(g_libGTASA + (VER_x32 ? 0x1EAC0C + 1 : 0x287AF4), dbName);
}

void TextureDatabaseRuntime::InjectHooks() {
    // CHook::Write(g_libGTASA + (VER_x32 ? 0x675FCC : 0x84A020), &detailTextures);
    CHook::Write(g_libGTASA + (VER_x32 ? 0x676720 : 0x84AEB0), &storedTexels);
    // CHook::Write(g_libGTASA + (VER_x32 ? 0x679768 : 0x850EE8), &loaded);
    // CHook::Write(g_libGTASA + (VER_x32 ? 0x679E70 : 0x851CF8), &registered);

    static bool mobileMenuHookInstalled = false;
    if (!mobileMenuHookInstalled)
    {
        CHook::InlineHook(
            "_ZN10MobileMenu4LoadEv",
            &MobileMenuLoadHook,
            &g_originalMobileMenuLoad
        );
        mobileMenuHookInstalled = true;
        Log("MobileMenu::Load lifecycle hook installed");
    }

    // Install the broader defensive menu lifecycle once the native hook table
    // is being initialized. The guard itself is isolated in MobileMenu.cpp so
    // the texture subsystem does not own menu rendering logic.
    InstallMobileMenuCrashGuardHooks();
}

RwBool TextureAnnihilate(RwTexture *texture) {
    return CHook::CallFunction<RwBool>(g_libGTASA + (VER_x32 ? 0x1DB33C + 1 : 0x273A1C), texture);
}
