#pragma once

#include "EntryInfoNode.h"

class CEntryInfoList {
public:
    CEntryInfoNode* m_pNode;

public:
    static void InjectHooks();

    void Flush();
    void DeleteNode(CEntryInfoNode* pNode);
    bool IsEmpty() const noexcept { return !m_pNode; }
};
VALIDATE_SIZE(CEntryInfoList, (VER_x32 ? 0x4 : 0x8));
