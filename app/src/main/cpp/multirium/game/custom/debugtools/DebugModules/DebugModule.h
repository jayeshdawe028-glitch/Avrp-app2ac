#pragma once

#include <imgui.h>
#include "../Utility.h" // TODO Remove this and add it individually to all places this headear is included in

class DebugModule {
public:
    virtual ~DebugModule() = default;

    //! Called once on ImGUI initialization
    virtual void OnImGuiInitialised(ImGuiContext* ctx) { /*nothing*/ }
    
    //! Called once every frame (pre-render)
    virtual void Update() { /*nothing*/ }

    //! Render 3D stuff here (lines, etc...). If rendered elsewhere they (probably) won't be visible.
    virtual void Render3D() { /*nothing*/ };

    //! Module's window should be rendered here
    virtual void RenderWindow() { /*nothing*/ }

    //! Module's entry in the main menu should be rendered here
    virtual void RenderMenuEntry() = 0;

    //! Get the ID of this module (Used for serialization)
    virtual std::string_view GetID() const { return ""; }
};

