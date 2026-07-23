#pragma once

#include "game/fakerw/types.h"
#include "game/fakerw/color.h"
#include "game/fakerw/rwcore.h"
#include "game/fakerw/rtquat.h"
#include "game/fakerw/rphanim.h"
#include "game/fakerw/rpskin.h"

#include "game/fakerw/rpworld.h"
#include "game/fakerw/RwHelper.h"

/* skeleton.h */
typedef struct RsGlobalType RsGlobalType;
struct RsGlobalType
{
    const RwChar *appName;
    RwInt32 screenWidth;
    RwInt32 screenHeight;
    RwInt32 maxFPS;
    RwBool quit;

   // void *ps;
    //RsInputDevice keyboard;
   // RsInputDevice mouse;
   // RsInputDevice pad;
};

#define RWRSTATE(a) (reinterpret_cast<void *>(a))
#define RWRGBALONG(r,g,b,a) ((unsigned int) (((a) << 24) | ((r) << 16) | ((g) << 8) | (b)))

/* rtpng.h */
extern RwImage* (*RtPNGImageWrite)(RwImage* image, const RwChar* imageName);
extern RwImage* (*RtPNGImageRead)(const RwChar* imageName);

extern RwBool (*RwTextureDestroy)(RwTexture* texture);

extern RsGlobalType* RsGlobal;