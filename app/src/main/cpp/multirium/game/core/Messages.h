//
// Created by viski on 03.06.2025.
//

#ifndef MULTIRIUM_MESSAGES_H
#define MULTIRIUM_MESSAGES_H

#include "../main.h"

struct CPreviousMessage {
    uint8_t TextLabel[8];
    GxtChar *pMessage;

    uint32_t NumberToInsert;

    uint32_t NumberToInsert2;
    uint32_t NumberToInsert3;

    uint32_t NumberToInsert4;
    uint32_t NumberToInsert5;

    uint32_t NumberToInsert6;
    GxtChar *pShortMessage;
 };
VALIDATE_SIZE(CPreviousMessage, (VER_x32 ? 0x28 : 0x30));

class CMessages {
public:
    static CPreviousMessage PreviousBriefs[20];
    static void InjectHooks();
};


#endif //MULTIRIUM_MESSAGES_H
