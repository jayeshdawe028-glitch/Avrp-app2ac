#pragma once

#include <chrono>
#include <imgui.h>
#include "DebugModule.h"

class DebugModules {
    static constexpr time_t MODULE_SERIALIZATION_INTERVAL_MS = 5000;

public:
    static void CreateModules(ImGuiContext* ctx);
    static void PreRenderUpdate();
    static void Render2D();
    static void Render3D();

private:

    template<std::derived_from<DebugModule> T>
    static void Add() {
        auto& m = m_Modules.emplace_back(std::make_unique<T>());
        m->OnImGuiInitialised(m_ImCtx);
    }

private:
    static std::vector<std::unique_ptr<DebugModule>> m_Modules;
    static ImGuiContext* m_ImCtx;
    static time_t m_LastSerializationTimeMs;
};
