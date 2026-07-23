//
// Created by viski on 28.03.2025.
//

#ifndef X64_CCHAT_H
#define X64_CCHAT_H

#include <jni.h>
#include "game/core/core/Vector2D.h"

class CChat {
public:
    static jobject thiz;
    static bool bIsShow;

    static void AddChatMessage(const char *msg, uint32_t textcolor);

    static bool bIsShowChat;

    static void ToggleChat(bool toggle);
};

#endif //X64_CCHAT_H
