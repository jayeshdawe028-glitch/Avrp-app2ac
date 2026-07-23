#include "RpAnimBlend.h"
#include "../vendor/armhook/patch.h"

bool RpAnimBlendPluginAttach() {
    return CHook::CallFunction<bool>(g_libGTASA + (VER_x32 ? 0x00390470 + 1 : 0x46A30C));
}

void* RtAnimBlendKeyFrameApply(void* result, void* frame) {
    return CHook::CallFunction<void*>(g_libGTASA + (VER_x32 ? 0x003902D8 + 1 : 0x46A1B8), result, frame);

    auto matrix = static_cast<RwMatrix*>(result);
    auto quat = static_cast<RtQuat*>(frame);
}

CAnimBlendClumpData* RpAnimBlendAllocateData(RpClump* clump) {
    return CHook::CallFunction<CAnimBlendClumpData*>(g_libGTASA + (VER_x32 ? 0x003902B4 + 1 : 0x46A17C), clump);
}

CAnimBlendAssociation* RpAnimBlendClumpAddAssociation(RpClump* clump, CAnimBlendAssociation* association, uint32 flags, float startTime, float blendAmount) {
    return CHook::CallFunction<CAnimBlendAssociation*>(g_libGTASA + (VER_x32 ? 0x0039093C + 1 : 0x46A9A0), clump, association, flags, startTime, blendAmount);
}

CAnimBlendAssociation* RpAnimBlendClumpExtractAssociations(RpClump* clump) {
    int *v1; // r1
    int v2; // r3
    int result; // r0

    v1 = *(int **)(clump + ClumpOffset);
    v2 = *v1;
    *v1 = 0;
    result = v2 - 4;
    *(uint32_t *)(v2 + 4) = 0;
    return reinterpret_cast<CAnimBlendAssociation *>(result);
}

void RpAnimBlendClumpFillFrameArray(RpClump* clump, AnimBlendFrameData** frameData) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x00390750 + 1 : 0x46A6F0), clump, frameData);
}

AnimBlendFrameData* RpAnimBlendClumpFindBone(RpClump* clump, uint32 id) {
    return CHook::CallFunction<AnimBlendFrameData*>(g_libGTASA + (VER_x32 ? 0x0039070C + 1 : 0x46A694), clump, id);
}

AnimBlendFrameData* RpAnimBlendClumpFindFrame(RpClump* clump, const char* name) {
    return CHook::CallFunction<AnimBlendFrameData*>(g_libGTASA + (VER_x32 ? 0x003905C4 + 1 : 0x46A4A4), clump, name);
}

AnimBlendFrameData* RpAnimBlendClumpFindFrameFromHashKey(RpClump* clump, uint32 key) {
    return CHook::CallFunction<AnimBlendFrameData*>(g_libGTASA + (VER_x32 ? 0x0039066C + 1 : 0x46A5A0), clump, key);
}

CAnimBlendAssociation* RpAnimBlendClumpGetAssociation(RpClump* clump, bool bStopFunctionConfusion, CAnimBlendHierarchy* hierarchy) {
    return CHook::CallFunction<CAnimBlendAssociation*>(g_libGTASA + (VER_x32 ? 0x00390A78 + 1 : 0x46AB7C), clump, bStopFunctionConfusion, hierarchy);
}

CAnimBlendAssociation* RpAnimBlendClumpGetAssociation(RpClump* clump, const char* name) {
    return CHook::CallFunction<CAnimBlendAssociation*>(g_libGTASA + (VER_x32 ? 0x00390A24 + 1 : 0x46AAF4), clump, name);
}

// AnimationId animId
CAnimBlendAssociation* RpAnimBlendClumpGetAssociation(RpClump* clump, uint32 animId) {
    return CHook::CallFunction<CAnimBlendAssociation*>(g_libGTASA + (VER_x32 ? 0x00390A50 + 1 : 0x46AB48), clump, animId);
}

CAnimBlendAssociation* RpAnimBlendClumpGetFirstAssociation(RpClump* clump) {
    int *v4; // r4
    int result; // r0

    v4 = *(int **)(clump + ClumpOffset);
    result = RpAnimBlendClumpIsInitialized(clump);
    if ( result )
    {
        result = *v4;
        if ( *v4 )
            result -= 4;
    }
    return reinterpret_cast<CAnimBlendAssociation *>(result);
}

CAnimBlendAssociation* RpAnimBlendClumpGetFirstAssociation(RpClump* clump, uint32 flags) {
    return CHook::CallFunction<CAnimBlendAssociation*>(g_libGTASA + (VER_x32 ? 0x00390C04 + 1 : 0x46AD68), clump, flags);
}

CAnimBlendAssociation* RpAnimBlendClumpGetMainAssociation(RpClump* clump, CAnimBlendAssociation** pp2ndAnim, float* pBlendVal2nd) {
    return CHook::CallFunction<CAnimBlendAssociation*>(g_libGTASA + (VER_x32 ? 0x00390A9C + 1 : 0x46ABB0), clump, pp2ndAnim, pBlendVal2nd);
}

CAnimBlendAssociation* RpAnimBlendClumpGetMainAssociation_N(RpClump* clump, int32 n) {
    return CHook::CallFunction<CAnimBlendAssociation*>(g_libGTASA + (VER_x32 ? 0x00390B9C + 1 : 0x46ACE8), clump, n);
}

CAnimBlendAssociation* RpAnimBlendClumpGetMainPartialAssociation(RpClump* clump) {
    return CHook::CallFunction<CAnimBlendAssociation*>(g_libGTASA + (VER_x32 ? 0x00390B44 + 1 : 0x46AC80), clump);
}

CAnimBlendAssociation* RpAnimBlendClumpGetMainPartialAssociation_N(RpClump* clump, int32 n) {
    return CHook::CallFunction<CAnimBlendAssociation*>(g_libGTASA + (VER_x32 ? 0x00390BD0 + 1 : 0x46AD28), clump, n);
}

uint32 RpAnimBlendClumpGetNumAssociations(RpClump* clump) {
    return CHook::CallFunction<uint32>(g_libGTASA + (VER_x32 ? 0x00390CB0 + 1 : 0x46AE54), clump);
}

uint32 RpAnimBlendClumpGetNumNonPartialAssociations(RpClump* clump) {
    return CHook::CallFunction<uint32>(g_libGTASA + (VER_x32 ? 0x00390CF4 + 1 : 0x46AEB4), clump);
}

uint32 RpAnimBlendClumpGetNumPartialAssociations(RpClump* clump) {
    return CHook::CallFunction<uint32>(g_libGTASA + (VER_x32 ? 0x00390CCC + 1 : 0x46AE78), clump);
}

void RpAnimBlendClumpGiveAssociations(RpClump* clump, CAnimBlendAssociation* association) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x00390D3C + 1 : 0x46AF18), clump, association);
}

void RpAnimBlendClumpInit(RpClump* clump) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x003907E8 + 1 : 0x46A7D4), clump);
}

bool RpAnimBlendClumpIsInitialized(RpClump* clump) {
   return CHook::CallFunction<bool>(g_libGTASA + (VER_x32 ? 0x39091C + 1 : 0x46A974), clump);
}

void RpAnimBlendClumpPauseAllAnimations(RpClump* clump) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x00390C60 + 1 : 0x46ADF4), clump);
}

void RpAnimBlendClumpRemoveAllAssociations(RpClump* clump) {
    RpAnimBlendClumpRemoveAssociations(clump, 0);
}

void RpAnimBlendClumpRemoveAssociations(RpClump* clump, uint32 flags) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x003909EC + 1 : 0x46AA98), clump, flags);
}

void RpAnimBlendClumpSetBlendDeltas(RpClump* clump, uint32 flags, float delta) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x00390988 + 1 : 0x46AA14), clump, flags, delta);
}

void RpAnimBlendClumpUnPauseAllAnimations(RpClump* clump) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x00390C88 + 1 : 0x46AE24), clump);
}

void RpAnimBlendClumpUpdateAnimations(RpClump* clump, float step, bool onScreen) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x0038BF50 + 1 : 0x464624), clump, step, onScreen);
}

RtAnimAnimation* RpAnimBlendCreateAnimationForHierarchy(RpHAnimHierarchy* hierarchy) {
    return CHook::CallFunction<RtAnimAnimation*>(g_libGTASA + (VER_x32 ? 0x00390594 + 1 : 0x46A460), hierarchy);
}

char* RpAnimBlendFrameGetName(RwFrame* frame) {
    return CHook::CallFunction<char*>(g_libGTASA + (VER_x32 ? 0x003902A0 + 1 : 0x46A154), frame);
}

void RpAnimBlendFrameSetName(RwFrame* frame, char* name) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x003902A4 + 1 : 0x46A158), frame, name);
}

CAnimBlendAssociation* RpAnimBlendGetNextAssociation(CAnimBlendAssociation* association) {
    return CHook::CallFunction<CAnimBlendAssociation*>(g_libGTASA + (VER_x32 ? 0x00390C34 + 1 : 0x46ADAC), association);
}

CAnimBlendAssociation* RpAnimBlendGetNextAssociation(CAnimBlendAssociation* association, uint32 flags) {
    return CHook::CallFunction<CAnimBlendAssociation*>(g_libGTASA + (VER_x32 ? 0x00390C3E + 1 : 0x46ADC0), association, flags);
}

void RpAnimBlendKeyFrameInterpolate(void *pVoidOut, void *pVoidIn1, void *pVoidIn2, RwReal time, void *customData) {
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x00390440 + 1 : 0x46A2F4), pVoidOut, pVoidIn1, pVoidIn2, time, customData);
}