//
// Created by viski on 10.04.2025.
//

#include "PathFind.h"
#include "main.h"
#include "patch.h"

void CPathFind::InjectHooks() {
    Log(__FUNCTION__);
}

void CPathFind::DoPathSearch(uint64_t ThePaths, ePathType pathType,
                             CVector originPos,
                             CNodeAddress originAddrAddrHint,
                             CVector targetPos,
                             CNodeAddress* outResultNodes,
                             int16& outNodesCount,
                             int32 maxNodesToFind,
                             float* outDistance,
                             float maxSearchDistance,
                             CNodeAddress* targetNodeAddrHint,
                             float maxSearchDepth,
                             bool sameLaneOnly,
                             CNodeAddress forbiddenNodeAddr,
                             bool bAllowWaterNodeTransitions,
                             bool forBoats) {
    //outNodesCount = 0;

    CHook::CallFunction<int>("_ZN9CPathFind12DoPathSearchEh7CVector12CNodeAddressS0_PS1_PsiPffS2_fbS1_bb",
                             ThePaths, pathType, originPos, originAddrAddrHint, targetPos,
                             outResultNodes, &outNodesCount, maxNodesToFind,
                             outDistance, maxSearchDistance, targetNodeAddrHint,
                             maxSearchDepth, sameLaneOnly, forbiddenNodeAddr,
                             bAllowWaterNodeTransitions, forBoats);
}
