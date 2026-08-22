//
// Created by x1y2z on 11.01.2023.
//

#include "TextureDatabaseRuntime.h"
#include "../vendor/armhook/patch.h"

extern void Log(const char* fmt, ...);

TextureDatabaseRuntime* TextureDatabaseRuntime::Load(const char *withName, bool fullyLoad, TextureDatabaseFormat forcedFormat)
{
    Log("========== Texture Load ==========");
    Log("Name: %s", withName ? withName : "<null>");
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
    Log("TextureDB::Register db=%p", (void*)toRegister);
    if (!g_libGTASA || !toRegister) {
        Log("TextureDB::Register BLOCKED invalid state");
        return;
    }
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x1E9B48 + 1 : 0x2865D8), toRegister);
}

void TextureDatabaseRuntime::Unregister(TextureDatabaseRuntime *toUnregister) {
    Log("TextureDB::Unregister db=%p", (void*)toUnregister);
    if (!g_libGTASA || !toUnregister) {
        Log("TextureDB::Unregister BLOCKED invalid state");
        return;
    }
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x1E9C00 + 1 : 0x2866A4), toUnregister);
}

__attribute__((noinline)) RwTexture* TextureDatabaseRuntime::GetTexture(const char *name) {
    Log("TextureDB::GetTexture ENTER name=%s g_libGTASA=%p", name ? name : "<null>", (void*)g_libGTASA);

    if (!g_libGTASA || !name) {
        Log("TextureDB::GetTexture BLOCKED invalid state");
        return nullptr;
    }

    RwTexture* texture = ((RwTexture*(*)(const char*))(g_libGTASA + (VER_x32 ? 0x001E9C64 + 1 : 0x286718)))(name);

    Log("TextureDB::GetTexture RETURN name=%s texture=%p", name, (void*)texture);
    return texture;
}

void TextureDatabaseRuntime::UpdateStreaming(float deltaTime, bool flush) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x1E9518 + 1 : 0x285DBC), deltaTime, flush);
}

TextureDatabaseRuntime* TextureDatabaseRuntime::GetDatabase(const char *dbName) {
    Log("TextureDB::GetDatabase name=%s g_libGTASA=%p", dbName ? dbName : "<null>", (void*)g_libGTASA);
    if (!g_libGTASA || !dbName) {
        Log("TextureDB::GetDatabase BLOCKED invalid state");
        return nullptr;
    }

    TextureDatabaseRuntime* db = CHook::CallFunction<TextureDatabaseRuntime*>(g_libGTASA + (VER_x32 ? 0x1EAC0C + 1 : 0x287AF4), dbName);
    Log("TextureDB::GetDatabase RETURN name=%s db=%p", dbName, (void*)db);
    return db;
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
