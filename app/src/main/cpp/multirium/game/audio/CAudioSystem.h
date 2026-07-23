//
// Created by viski on 10.04.2025.
//

#ifndef X64_CAUDIOSYSTEM_H
#define X64_CAUDIOSYSTEM_H


class CAudioSystem {

public:
    static void LoadEngineSoundConfig(uintptr_t configBaseAddress);

    static void injectHooks();
};


#endif //X64_CAUDIOSYSTEM_H
