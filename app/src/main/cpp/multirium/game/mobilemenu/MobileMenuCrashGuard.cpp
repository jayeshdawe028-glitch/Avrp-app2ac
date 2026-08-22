//
// Defensive native MobileMenu lifecycle guard.
//
// This file deliberately sits beside MobileMenu rather than changing gameplay
// or networking code. The native menu renderer is called from the game loop,
// so a malformed screen stack or missing menu texture must not be allowed to
// turn into a process-wide SIGSEGV.
//

#include "MobileMenu.h"
#include "../core/textures/TextureDatabaseRuntime.h"
#include "../../vendor/armhook/patch.h"

extern void Log(const char* fmt, ...);

namespace {
    using MobileMenuRenderFn = void (*)(MobileMenu*);
    using MainMenuAddAllItemsFn = void (*)(void*);

    MobileMenuRenderFn g_originalMobileMenuRender = nullptr;
    MainMenuAddAllItemsFn g_originalMainMenuAddAllItems = nullptr;

    bool IsPlausibleObject(const void* ptr)
    {
        const uintptr_t value = reinterpret_cast<uintptr_t>(ptr);
        return value > 0x10000 && (value & (alignof(void*) - 1)) == 0;
    }

    void SanitizeScreenStack(MobileMenu* menu)
    {
        if (!menu)
            return;

        // A non-zero count with no backing array is never a valid render state.
        if (menu->screenStack.numEntries != 0 && !menu->screenStack.dataPtr) {
            Log("MobileMenu guard: invalid screen stack (count=%u, data=null), clearing count",
                menu->screenStack.numEntries);
            menu->screenStack.numEntries = 0;
            return;
        }

        // The native menu should never have an unbounded screen stack. Treat an
        // obviously corrupted count as a recoverable state instead of indexing
        // arbitrary memory from the render thread.
        if (menu->screenStack.numEntries > 32) {
            Log("MobileMenu guard: corrupt screen stack count=%u, clearing stack count",
                menu->screenStack.numEntries);
            menu->screenStack.numEntries = 0;
            return;
        }

        // Remove null entries from the top. This preserves all valid lower
        // screens and prevents the native renderer from dereferencing null.
        while (menu->screenStack.numEntries > 0) {
            const unsigned int index = menu->screenStack.numEntries - 1;
            MenuScreen* top = menu->screenStack.dataPtr[index];
            if (IsPlausibleObject(top))
                break;

            Log("MobileMenu guard: invalid top screen at index=%u, skipping it",
                index);
            --menu->screenStack.numEntries;
        }
    }

    bool MenuDatabaseReady()
    {
        TextureDatabaseRuntime* db = TextureDatabaseRuntime::GetDatabase("menu");
        if (!db) {
            Log("MobileMenu guard: menu texture database unavailable; render skipped");
            return false;
        }
        return true;
    }

    bool RequiredMenuTexturesReady()
    {
        // These are the textures consumed by the native MainMenu/FlowScreen
        // path. A missing texture is a data mismatch, not a reason to crash the
        // whole game. We simply skip this render frame and retry on the next one.
        static const char* const kRequired[] = {
                "menu_mainplay",
                "menu_mainsettings",
                "menu_mainstats",
                "menu_mainresume"
        };

        for (const char* name : kRequired) {
            if (!TextureDatabaseRuntime::GetTexture(name)) {
                Log("MobileMenu guard: required texture missing: %s", name);
                return false;
            }
        }
        return true;
    }

    void MobileMenuRenderHook(MobileMenu* menu)
    {
        if (!menu || !RsGlobal) {
            Log("MobileMenu guard: render called before menu/RenderWare state is valid");
            return;
        }

        if (RsGlobal->screenWidth <= 0 || RsGlobal->screenHeight <= 0) {
            Log("MobileMenu guard: invalid screen size %d x %d; render skipped",
                RsGlobal->screenWidth, RsGlobal->screenHeight);
            return;
        }

        if (menu->InitializedForSignOut) {
            // The native renderer is not useful while the menu is being torn
            // down. Avoid entering it during this transient lifecycle state.
            return;
        }

        SanitizeScreenStack(menu);

        if (menu->screenStack.numEntries == 0 && !IsPlausibleObject(menu->pendingScreen)) {
            // Nothing valid to render. Do not enter the native renderer merely
            // because stale state says the menu exists.
            return;
        }

        if (!MenuDatabaseReady())
            return;

        if (!RequiredMenuTexturesReady())
            return;

        if (!g_originalMobileMenuRender) {
            Log("MobileMenu guard: original Render function unavailable");
            return;
        }

        g_originalMobileMenuRender(menu);
    }

    void MainMenuAddAllItemsHook(void* screen)
    {
        if (!screen) {
            Log("MobileMenu guard: MainMenuScreen::AddAllItems received null");
            return;
        }

        // Do not call the native implementation when its required textures are
        // unavailable: the original routine increments refCount immediately
        // after GetTexture() without checking for nullptr.
        if (!MenuDatabaseReady() || !RequiredMenuTexturesReady()) {
            Log("MobileMenu guard: MainMenuScreen::AddAllItems deferred due to incomplete menu data");
            return;
        }

        if (g_originalMainMenuAddAllItems)
            g_originalMainMenuAddAllItems(screen);
    }
}

void InstallMobileMenuCrashGuardHooks()
{
    static bool installed = false;
    if (installed)
        return;

    CHook::InlineHook(
            "_ZN10MobileMenu6RenderEv",
            &MobileMenuRenderHook,
            &g_originalMobileMenuRender
    );

    // The native implementation dereferences the result of GetTexture() before
    // checking it. Keep that path behind the same data validation used by the
    // renderer. If the symbol is unavailable on a build, the rest of the guard
    // remains useful and the linker/build can expose that mismatch immediately.
    CHook::InlineHook(
            "_ZN14MainMenuScreen11AddAllItemsEv",
            &MainMenuAddAllItemsHook,
            &g_originalMainMenuAddAllItems
    );

    installed = true;
    Log("MobileMenu crash guard: Render/AddAllItems hooks installed");
}
