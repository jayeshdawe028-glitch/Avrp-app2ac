//
// Created by viski on 11.05.2025.
//

#ifndef MULTIRIUM_MEMORYMGR_H
#define MULTIRIUM_MEMORYMGR_H


class CMemoryMgr {

public:
    static void Init();
    static void *Malloc(unsigned int size, unsigned int nHint);

    static void Free(void *memory);

    static void *Malloc(unsigned int size);

    static void FreeAlign(void *memory);

    static unsigned char *MallocAlign(unsigned int size, unsigned int align, unsigned int nHint);

    static void *MallocAlign(unsigned int size, unsigned int align);

    static void *MoveMemory(void *memory);

    static void InitScratchPad();

    static void Shutdown();

    static void *InternalMalloc(size_t size);

    static void *InternalRealloc(void *pMem, size_t size);
};


#endif //MULTIRIUM_MEMORYMGR_H
