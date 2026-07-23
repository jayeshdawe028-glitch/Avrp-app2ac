//
// Created by viski on 17.04.2025.
//

#include "Weapon.h"
#include "main.h"
#include "patch.h"

void Weapon::InjectHooks() {
    CHook::Write(g_libGTASA + (VER_x32 ? 0x678523 + 1 : 0x84EA70), &ms_bTakePhoto);
}
