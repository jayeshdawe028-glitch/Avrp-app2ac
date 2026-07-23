//
// Created by viski on 10.04.2025.
//

#ifndef X64_GPS_H
#define X64_GPS_H

#include "patch.h"
#include "game/control/NodeAddress.h"
#include <dlfcn.h>
#include "game/fakerw/rwcore.h"
#include "game/fakerw/rwplcore.h"

#define GPS_LINE_R      235
#define GPS_LINE_G      212
#define GPS_LINE_B      0
#define GPS_LINE_A      255

#define MIN_MARKER_DISTANCE 30.0f
#define MARKER_FADE_START_DISTANCE 150.0f
#define MAX_PATH_SEARCH_DISTANCE 1000000.0f
#define MAX_PATH_NODES 2000
#define MAX_NODE_POINTS 2000
struct RwOpenGLVertex {
    RwV3d pos;
    float rhw;
    RwUInt32 color;
    struct {
        float u, v;
    } texCoord;
};

class GPS {
public:
    static bool only3D;
    static CVector to;
    static void injectHooks();

    static bool enabled;

    static void Set(CVector pos, bool toggle);
    static void DoPathDraw();
};

#endif //X64_GPS_H
