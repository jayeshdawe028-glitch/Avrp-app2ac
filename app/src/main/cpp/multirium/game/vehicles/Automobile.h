//
// Created by viski on 12.05.2025.
//

#ifndef MULTIRIUM_AUTOMOBILE_H
#define MULTIRIUM_AUTOMOBILE_H

#include "game/vehicles/vehicle.h"

class CAutomobile : public CVehicle {
#if VER_x32
    uint8_t pad0[0x3E8];
#else
    uint8_t pad0[0x470];
#endif
};
//VALIDATE_SIZE(CAutomobile, (VER_x32 ? 0xB48 : 0xD88));

#endif //MULTIRIUM_AUTOMOBILE_H
