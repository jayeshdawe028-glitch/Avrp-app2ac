//
// Created by viski on 12.05.2025.
//

#ifndef MULTIRIUM_SKIDMARK_H
#define MULTIRIUM_SKIDMARK_H

#include "../main.h"

enum class eSkidmarkType : uint32 {
    DEFAULT = 0,
    SANDY,
    MUDDY,
    BLOODY,
};

enum class eSkidmarkState : uint8 {
    INACTIVE,
    JUST_UPDATED,
    DISAPPEARING // Set after `JUST_UPDATED`. See `CSkidmark::Update`.
};

constexpr auto SKIDMARK_NUM_PARTS{ 16u };


class CSkidmark {

};


#endif //MULTIRIUM_SKIDMARK_H
