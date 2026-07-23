//
// Created by viski on 10.04.2025.
//

#ifndef MULTIRIUM_PATHFIND_H
#define MULTIRIUM_PATHFIND_H

#include "main.h"
#include "NodeAddress.h"
#include "game/core/CompressedVector.h"

enum ePathType : uint8 {
    PATH_TYPE_VEH, /// Cars, boats, race tracks
    PATH_TYPE_PED, /// Peds

    PATH_TYPE_ALL /// @notsa
};

class CPathFind {
public:
    static void InjectHooks();

    static void DoPathSearch(uint64_t ThePaths, ePathType pathType, CVector originPos, CNodeAddress originAddrAddrHint,
                      CVector targetPos, CNodeAddress *outResultNodes, int16 &outNodesCount,
                      int32 maxNodesToFind, float *outDistance, float maxSearchDistance,
                      CNodeAddress *targetNodeAddrHint, float maxSearchDepth, bool sameLaneOnly,
                      CNodeAddress forbiddenNodeAddr, bool bAllowWaterNodeTransitions,
                      bool forBoats);
};

class CPathNode {
public:
    CPathNode        *m_next, *m_prev;
    CompressedVector m_vPos;
    int16            m_totalDistFromOrigin;
    int16            m_wBaseLinkId;
    uint16           m_wAreaId;
    uint16           m_wNodeId;
    uint8            m_nPathWidth;
    uint8            m_nFloodFill;

    // byte 0
    uint32 m_nNumLinks : 4;
    uint32 m_onDeadEnd : 1;
    uint32 m_isSwitchedOff : 1;
    uint32 m_bRoadBlocks : 1;
    uint32 m_bWaterNode : 1;

    // byte 1
    uint32 m_isOriginallyOnDeadEnd : 1;
    uint32 unk1 : 1;
    uint32 m_bDontWander : 1;
    uint32 unk2 : 1;
    uint32 m_bNotHighway : 1;
    uint32 m_bHighway : 1;
    uint32 unk3 : 1;
    uint32 unk4 : 1;

    // byte 2
    uint32 m_nSpawnProbability : 4;
    uint32 m_nBehaviourType : 4; // 1 - roadblock 2 - parking node
    // byte 3
public:
    static void InjectHooks();

    CVector GetPosition() const {
        return UncompressLargeVector(m_vPos);
    }

    CNodeAddress GetAddress() const {
        return { m_wAreaId, m_wNodeId };
    }

    friend bool operator==(const CPathNode& lhs, const CPathNode& rhs) { return lhs.GetAddress() == rhs.GetAddress(); }
    friend bool operator!=(const CPathNode& lhs, const CPathNode& rhs) { return !(lhs == rhs); }

    bool HasToBeSwitchedOff() const {
        switch (m_nBehaviourType) {
            case 1:
            case 2:
                return false;
            default:
                return true;
        }
    }
};
VALIDATE_SIZE(CPathNode, (VER_x32 ? 0x1C : 0x28));

class CCarPathLinkAddress {
public:
    CCarPathLinkAddress(size_t area, size_t nodeId) :
            m_wCarPathLinkId{ (uint16)nodeId },
            m_wAreaId{ (uint16)area }
    {
    }

    CCarPathLinkAddress() {
        m_wCarPathLinkId = -1;
        m_wAreaId = -1;
    }

    operator CNodeAddress() const { return CNodeAddress{ m_wAreaId, m_wCarPathLinkId }; }

public:
    uint16 m_wCarPathLinkId : 10;
    uint16 m_wAreaId : 6;

    inline bool IsValid() {
        return *reinterpret_cast<uint16*>(this) != 0xFFFF;
    }
};
VALIDATE_SIZE(CCarPathLinkAddress, 0x2);

#endif //MULTIRIUM_PATHFIND_H
