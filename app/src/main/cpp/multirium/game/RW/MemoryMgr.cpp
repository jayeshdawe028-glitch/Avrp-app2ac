//
// Created by viski on 11.05.2025.
//

#include <cstdlib>
#include "MemoryMgr.h"
#include "game/core/common.h"
#include "vendor/armhook/patch.h"

void CMemoryMgr::Init() {
    // empty
}

void CMemoryMgr::Shutdown()
{
    // empty
}

void* CMemoryMgr::InternalMalloc(size_t size)
{
   /* if ( size > 2048 )
        return CMemoryHeap::Malloc(&gMainHeap, size);
    void* result = CMemoryHeap::Malloc(&gSmallHeap, size);
    if ( !result )
        return CMemoryHeap::Malloc(&gMainHeap, size);
    return result;*/
}

void* CMemoryMgr::InternalRealloc(void *pMem, size_t size)
{
    /*CMemoryHeap *v4; // x0
    void *v6; // x21

    if ( gScratchPadHeap.m_pHeap <= pMem && gScratchPadHeap.m_pHeapEnd > pMem )
    {
        v4 = &gScratchPadHeap;
        return CMemoryHeap::Realloc(v4, pMem, size);
    }
    if ( gSmallHeap.m_pHeap > pMem || gSmallHeap.m_pHeapEnd <= pMem )
    {
        if ( size >= 0x801 )
        {
            v4 = &gMainHeap;
            return CMemoryHeap::Realloc(v4, pMem, size);
        }
        v6 = CMemoryHeap::Malloc(&gSmallHeap, size);
        if ( v6 )
            goto LABEL_13;
    }
    else if ( size <= 0x800 )
    {
        v4 = &gSmallHeap;
        return CMemoryHeap::Realloc(v4, pMem, size);
    }
    v6 = CMemoryHeap::Malloc(&gMainHeap, size);
    if ( v6 )
    {
        LABEL_13:
        memcpy(v6, pMem, (uint64_t)fminf((float)size, (float)*((int *)pMem - 6)));
        free(pMem);
    }
    return v6;*/
}

void CMemoryMgr::Free(void* memory) {
    return free(memory);
}

void* CMemoryMgr::MoveMemory(void* memory) {
    //return CHook::CallFunction<void*>(g_libGTASA + (VER_x32 ? 0x005D3032 + 1 : 0x6F7C90), memory);
}

void* CMemoryMgr::Malloc(uint32 size, uint32 nHint) {
#ifdef MEMORY_MGR_USE_MEMORY_HEAP
    _LOCK_MEMORYHEAP();
    void* memory = nullptr;
    int32 iAttempts = 0;
    while (true) {
        memory = InternalMalloc(size);
        if (memory) {
            break;
        }
        if (_gfnMemCompact) {
            _UNLOCK_MEMORYHEAP();
            // sub_2644FC(_nSize, ++i);
            _gfnMemCompact(size, ++iAttempts);
            _LOCK_MEMORYHEAP();
        } else {
            _FATAL("Failed to allocate %d bytes!", size);
        }
    }
    RegisterMalloc(memory);
    CMemoryHeap::SetDebugInfo(memory, nHint);
    _UNLOCK_MEMORYHEAP();
    return memory;
#else
    return ::malloc(size);
#endif
}

void* CMemoryMgr::Malloc(uint32 size) {
    void* memory = Malloc(size, 0);
#if defined MEMORY_MGR_USE_HEAP_FLAGS
    GET_HEAP_DESC(memory)->m_Flags.NoDebugHint = true;
#endif
    return memory;
}

void CMemoryMgr::FreeAlign(void* memory) {
    Free(*((void**)memory - 1));
}

uint8* CMemoryMgr::MallocAlign(uint32 size, uint32 align, uint32 nHint) {
    auto* memory = Malloc(size + align, nHint);

    auto* result = (void*)(uintptr ((uint8*)memory + align) & ~(align - 1));
    *((void**)result - 1) = memory;
    return static_cast<uint8*>(result);
}

void* CMemoryMgr::MallocAlign(uint32 size, uint32 align) {
    void* v3 = malloc(align + size);
    *(uint64_t *)((((uint64_t)v3 + align) & -(uint64_t)align) - 8) = reinterpret_cast<uint64_t>(v3);
    return (void *)(((uint64_t)v3 + align) & -(uint64_t)align);

   /* void* memory = MallocAlign(size, align, 0);
#if defined MEMORY_MGR_USE_HEAP_FLAGS
    GET_HEAP_DESC(memory)->m_Flags.NoDebugHint = true;
#endif
    return memory;*/
}

void CMemoryMgr::InitScratchPad() {
    //return CMemoryHeap::Init((CMemoryHeap *)&qword_D28460, &PC_Scratch, 0x4000uLL, 0);
}