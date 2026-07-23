//
// Created by viski on 03.06.2025.
//

#include "Messages.h"

CPreviousMessage CMessages::PreviousBriefs[20];

void CMessages::InjectHooks() {
    //CMessages::PreviousBriefs = *(CPreviousMessage*)(g_libGTASA + 0xCA04C8);
    std::memcpy(CMessages::PreviousBriefs, reinterpret_cast<void*>(g_libGTASA + (VER_x32 ? 0xA00448 : 0xCA04C8)), sizeof(CPreviousMessage) * 20);
}
