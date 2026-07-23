//
// Created by viski on 18.04.2025.
//

#include "main.h"

#include "EGL/egl.h"
#include "GLES2/gl2.h"
#include "GLES2/gl2ext.h"

class RQShader {
public:
    static void InjectHooks();

    static int BuildSource(int flags, char **pxlsrc, char **vtxsrc);
};



