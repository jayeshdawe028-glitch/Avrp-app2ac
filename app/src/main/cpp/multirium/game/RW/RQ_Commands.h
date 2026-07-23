//
// Created by viski on 10.04.2025.
//

#pragma once

#include "ES2VertexBuffer.h"

class CRQ_Commands {
public:
    static void rqVertexBufferSelect(uintptr_t **qData);

public:
    static void InjectHooks();

    static void rqTextureMipMode(int **a1);
};
