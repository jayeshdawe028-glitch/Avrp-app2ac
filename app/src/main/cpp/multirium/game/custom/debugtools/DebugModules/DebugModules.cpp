#include <fstream>

#include "DebugModules.h"
#include "../game/core/Timer.h"
#include "../vendor/imgui/imgui.h"

#include "modules/TimeCycleDebugModule.h"

std::vector<std::unique_ptr<DebugModule>> DebugModules::m_Modules{};
ImGuiContext* DebugModules::m_ImCtx = nullptr;
time_t DebugModules::m_LastSerializationTimeMs = 0;

void DebugModules::PreRenderUpdate() {
    for (auto& module : m_Modules) {
        module->Update();
    }

    // Handle DebugModule serialization
    const auto now = CTimer::GetTimeInMS();
    if ((now - m_LastSerializationTimeMs) >= MODULE_SERIALIZATION_INTERVAL_MS) {
        m_LastSerializationTimeMs = now;
    }
}

void DebugModules::Render2D() {
    ImGui::BeginMainMenuBar();
    for (auto& module : m_Modules) {
        module->RenderMenuEntry();
    }
    ImGui::EndMainMenuBar();

    for (auto& module : m_Modules) {
        module->RenderWindow();
    }
}

void DebugModules::Render3D() {
    for (auto& module : m_Modules) {
        module->Render3D();
    }
}

void DebugModules::CreateModules(ImGuiContext* ctx) {
    m_ImCtx = ctx;

    Add<TimeCycleDebugModule>(); // Visualization + Extra
}