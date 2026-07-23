//
// Created by viski on 12.05.2025.
//

#ifndef MULTIRIUM_HELI_H
#define MULTIRIUM_HELI_H


#include "Automobile.h"

class CHeli : public CAutomobile {
#if VER_x32
    uint8 pad0[0x90];
#else
    uint8_t pad0[0x98];
#endif
};
//VALIDATE_SIZE(CHeli, (VER_x32 ? 0xBD8 : 0xE20));

#endif //MULTIRIUM_HELI_H
