//
// Created by viski on 01.05.2025.
//

#include "VisibilityPlugins.h"
#include "vendor/armhook/patch.h"
#include "Camera.h"
#include "game/modelinfo/ModelInfo.h"
#include "java/CPauseMenu.h"
#include "World.h"
#include "game.h"

CLinkList<CVisibilityPlugins::AlphaObjectInfo> CVisibilityPlugins::m_alphaList{};
CLinkList<CVisibilityPlugins::AlphaObjectInfo> CVisibilityPlugins::m_alphaBoatAtomicList{};
CLinkList<CVisibilityPlugins::AlphaObjectInfo> CVisibilityPlugins::m_alphaEntityList{};
CLinkList<CVisibilityPlugins::AlphaObjectInfo> CVisibilityPlugins::m_alphaUnderwaterEntityList{};
CLinkList<CVisibilityPlugins::AlphaObjectInfo> CVisibilityPlugins::m_alphaReallyDrawLastList{};
CLinkList<CPed*> CVisibilityPlugins::ms_weaponPedsForPC{};

RwInt32 CVisibilityPlugins::ms_atomicPluginOffset;
RwInt32 CVisibilityPlugins::ms_framePluginOffset;
RwInt32 CVisibilityPlugins::ms_clumpPluginOffset;

RwCamera* CVisibilityPlugins::ms_pCamera;
RwV3d* CVisibilityPlugins::ms_pCameraPosn;

float CVisibilityPlugins::ms_bigVehicleLod0Dist;
float CVisibilityPlugins::ms_vehicleLod0Dist;
float CVisibilityPlugins::ms_vehicleLod0RenderMultiPassDist;
float CVisibilityPlugins::ms_cullCompsDist;
float CVisibilityPlugins::ms_cullBigCompsDist;
float CVisibilityPlugins::ms_vehicleLod1Dist;
float CVisibilityPlugins::ms_pedFadeDist;

#define ATOMICPLG(atomic, var) \
    (RWPLUGINOFFSET(tAtomicVisibilityPlugin, atomic, ms_atomicPluginOffset)->var)

#define ATOMICPLGCONST(atomic, var) \
    (RWPLUGINOFFSETCONST(tAtomicVisibilityPlugin, atomic, ms_atomicPluginOffset)->var)

void* CVisibilityPlugins::AtomicConstructor(void *atom, RwInt32 offset, RwInt32 size)
{
    ATOMICPLG(atom, m_modelId) = -1;
    ATOMICPLG(atom, m_flags) = 0;
    return atom;
}

void* CVisibilityPlugins::AtomicCopyConstructor(void *pDestAtom, const void *pSrcAtom, RwInt32 offset, RwInt32 size)
{
    *(uint16_t *)((char *)pDestAtom + CVisibilityPlugins::ms_atomicPluginOffset) = *(uint16_t *)((char *)pSrcAtom
                                                                                           + CVisibilityPlugins::ms_atomicPluginOffset);
    *(uint16_t *)((char *)pDestAtom + CVisibilityPlugins::ms_atomicPluginOffset + 2) = *(uint16_t *)((char *)pSrcAtom
                                                                                               + CVisibilityPlugins::ms_atomicPluginOffset
                                                                                               + 2);
    return pDestAtom;
}

void* CVisibilityPlugins::AtomicDestructor(void *atom, RwInt32 offset, RwInt32 size)
{
    return atom;
}

uint64_t CVisibilityPlugins::CalculateFadingAtomicAlpha(CBaseModelInfo *pModelInfo, CEntity *pEntity, float dist)
{
    CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

    float v6; // s3
    float v7; // s4

    unsigned int v3 = 22.0;
    float m_lodDistance = pModelInfo->m_lodDistance;
    float v5 = fminf(
            m_lodDistance * TheCamera.m_fLODDistMultiplier,
            /*CRenderer::ms_fFarClipPlane*/*(float*)(g_libGTASA + 0xBCF8E0) + pModelInfo->m_pColModel->GetBoundingSphere().m_fRadius);
    if ( pEntity->m_pLod )
    {
        v6 = /*FadeDistMult*/*(float*)(g_libGTASA + 0x88636C) * 22.0;
    }
    else
    {
        v3 = 1125515264;
        v7 = fminf(m_lodDistance, v5);
        v6 = /*FadeDistMult*/*(float*)(g_libGTASA + 0x88636C) * 22.0;
        if ( v7 > 150.0 )
            v6 = (float)(v7 / 15.0) + 10.0;
        if ( (*((uint8_t *)&pEntity->m_nFlags + 1) & 1) != 0 )
            v5 = v5 * /*CRenderer::ms_lowLodDistScale*/*(float*)(g_libGTASA + 0x886368);
    }
    //LOBYTE(v3) = pModelInfo->m_alpha;
    *((uint8_t*)&v3) = pModelInfo->m_alpha;
    return (unsigned int)(int)(float)(fminf(
            (float)((float)((float)(v5 + (float)(/*FadeDistMult*/*(float*)(g_libGTASA + 0x88636C) * 22.0)) - dist) / v6) * 0.8,
            1.0)
                                      * (float)v3);
}

void CVisibilityPlugins::ClearAtomicFlag(RpAtomic *pAtomic, uint16 flag)
{
    *(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset) &= ~flag;
}

RpAtomic* clearClumpForAllAtomicsFlagCB(RpAtomic *pAtomic, unsigned short data)
{
    *(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset) &= ~(uint16_t)data;
    return pAtomic;
}

void CVisibilityPlugins::ClearClumpForAllAtomicsFlag(RpClump *pClump, int32 id)
{
    RpClumpForAllAtomics(pClump, reinterpret_cast<RpAtomicCallBack>(clearClumpForAllAtomicsFlagCB), (void *)id);
}

RwBool CVisibilityPlugins::DefaultVisibilityCB(RpClump *pClump) {
    return true;
}

void* CVisibilityPlugins::ClumpConstructor(void *pClump, RwInt32 offset, RwInt32 size)
{
    char *v3; // x8

    v3 = (char *)pClump + CVisibilityPlugins::ms_clumpPluginOffset;
    *(uint64_t *)v3 = /*CVisibilityPlugins::DefaultVisibilityCB*/1;
    *((uint32_t *)v3 + 1) = 255;
    return pClump;
}

void* CVisibilityPlugins::ClumpCopyConstructor(void *pDestClump, const void *pSrcClump, RwInt32 offset, RwInt32 size)
{
    *(uint64_t *)((char *)pDestClump + CVisibilityPlugins::ms_clumpPluginOffset) = *(uint64_t *)((char *)pSrcClump
                                                                                             + CVisibilityPlugins::ms_clumpPluginOffset);
    return pDestClump;
}

void* CVisibilityPlugins::ClumpDestructor(void *pClump, RwInt32 offset, RwInt32 size)
{
    return pClump;
}

void* CVisibilityPlugins::FrameConstructor(void *pFrame, RwInt32 offset, RwInt32 size)
{
    *(uint64_t *)((char *)pFrame + CVisibilityPlugins::ms_framePluginOffset) = 0LL;
    return pFrame;
}

void* CVisibilityPlugins::FrameCopyConstructor(void *pDestFrame, const void *pSrcFrame, RwInt32 offset, RwInt32 size)
{
    *(uint64_t *)((char *)pDestFrame + CVisibilityPlugins::ms_framePluginOffset) = *(uint64_t *)((char *)pSrcFrame
                                                                                             + CVisibilityPlugins::ms_framePluginOffset);
    return pDestFrame;
}

void* CVisibilityPlugins::FrameDestructor(void *pFrame, RwInt32 offset, RwInt32 size)
{
    return pFrame;
}

RwBool CVisibilityPlugins::FrustumSphereCB(RpClump *pClump)
{
    auto* parent = (RwFrame *)pClump->object.parent;
    RwSphere v4 = *(RwSphere *)(*(uint64_t *)(*(uint64_t *)(&parent->object.type + CVisibilityPlugins::ms_framePluginOffset) + 48LL)
                         + 24LL);
    RwMatrix* LTM = RwFrameGetLTM(parent);
    RwV3dTransformPoints(&v4.center, &v4.center, 1, LTM);
    return RwCameraFrustumTestSphere(CVisibilityPlugins::ms_pCamera, &v4) != rwSPHEREOUTSIDE;
}

int32 CVisibilityPlugins::GetAtomicId(RpAtomic *pAtomic)
{
    return *(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset);
}

int32 CVisibilityPlugins::GetClumpAlpha(RpClump *pClump)
{
    return *(uint32_t *)(&pClump->object.privateFlags + CVisibilityPlugins::ms_clumpPluginOffset + 1);
}

CClumpModelInfo* CVisibilityPlugins::GetClumpModelInfo(RpClump *pClump)
{
    return *(CClumpModelInfo **)((char *)pClump->object.parent + CVisibilityPlugins::ms_framePluginOffset);
}

float CVisibilityPlugins::GetDistanceSquaredFromCamera(RwV3d *pVec)
{
    float dx = pVec->x - CVisibilityPlugins::ms_pCameraPosn->x;
    float dy = pVec->y - CVisibilityPlugins::ms_pCameraPosn->y;
    float dz = pVec->z - CVisibilityPlugins::ms_pCameraPosn->z;

    return dx * dx + dy * dy + dz * dz;
}

float CVisibilityPlugins::GetDotProductWithCameraVector(RwMatrix *pMatrix, RwMatrix *pCarMatrix, uint32_t atomicId)
{
    float y; // s0
    float x; // s4
    RwReal *p_z; // x8
    float result; // s0

    float v3 = pMatrix->pos.x - CVisibilityPlugins::ms_pCameraPosn->x;
    float v4 = pMatrix->pos.y - CVisibilityPlugins::ms_pCameraPosn->y;
    float v5 = pMatrix->pos.z - CVisibilityPlugins::ms_pCameraPosn->z;
    if ( (atomicId & 0x30) != 0 )
    {
        x = pCarMatrix->up.x;
        y = pCarMatrix->up.y;
        p_z = &pCarMatrix->up.z;
        LABEL_5:
        result = (float)((float)(v4 * y) + (float)(v3 * x)) + (float)(v5 * *p_z);
        goto LABEL_6;
    }
    if ( (atomicId & 0xC) != 0 )
    {
        x = pCarMatrix->right.x;
        y = pCarMatrix->right.y;
        p_z = &pCarMatrix->right.z;
        goto LABEL_5;
    }
    result = 0.0;
    LABEL_6:
    float v10 = -result;
    if ( (atomicId & 0x24) != 0 )
        result = -result;
    if ( (atomicId & 0x8000) != 0 )
    {
        v10 = 0.25;
        if ( (atomicId & 0x300) == 0 )
            v10 = 2.5;
        result = result
                 + (float)(v10
                           * (float)((float)((float)(v4 * pCarMatrix->at.y) + (float)(v3 * pCarMatrix->at.x))
                                     + (float)(v5 * pCarMatrix->at.z)));
    }
    if ( (atomicId & 0x300) != 0 )
    {
        if ( (atomicId & 0x100) != 0 )
        {
            v10 = -(float)((float)((float)(v4 * pCarMatrix->up.y) + (float)(v3 * pCarMatrix->up.x))
                           + (float)(v5 * pCarMatrix->up.z));
        }
        else if ( (atomicId & 0x200) != 0 )
        {
            v10 = (float)((float)(v4 * pCarMatrix->up.y) + (float)(v3 * pCarMatrix->up.x)) + (float)(v5 * pCarMatrix->up.z);
        }
        if ( v10 < 0.0 && result < 0.0 )
            result = result + v10;
        if ( result > 0.0 && v10 > 0.0 )
            return v10 + result;
    }
    return result;
}

uint64_t CVisibilityPlugins::GetFrameHierarchyId(RwFrame *pFrame)
{
    return *(uint64_t *)(&pFrame->object.type + CVisibilityPlugins::ms_framePluginOffset);
}

CBaseModelInfo* CVisibilityPlugins::GetModelInfo(RpAtomic *pAtomic)
{

    uint64_t v1 = *(uint16_t *)(&pAtomic->object.object.type + CVisibilityPlugins::ms_atomicPluginOffset);
    if ( v1 == -1 )
        return nullptr;
    else
        return CModelInfo::ms_modelInfoPtrs[v1];
}

int32 CVisibilityPlugins::GetModelInfoIndex(RpAtomic *pAtomic)
{
    return *(uint16_t *)(&pAtomic->object.object.type + CVisibilityPlugins::ms_atomicPluginOffset);
}

uint16 CVisibilityPlugins::GetUserValue(RpAtomic *pAtomic)
{
    return *(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset);
}

void CVisibilityPlugins::InitAlphaAtomicList()
{
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pNext; // x9

    while ( 1 )
    {
        m_pNext = CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext;
        if ( CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext == &CVisibilityPlugins::m_alphaList.m_lastLink )
            break;
        CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext->m_pNext->m_pPrev = CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext->m_pPrev;
        m_pNext->m_pPrev->m_pNext = m_pNext->m_pNext;
        m_pNext->m_pNext = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext;
        CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->m_pPrev = m_pNext;
        m_pNext->m_pPrev = &CVisibilityPlugins::m_alphaList.m_firstFreeLink;
        CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext = m_pNext;
    }
}

void CVisibilityPlugins::InitAlphaEntityList()
{
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pNext; // x9
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v1; // x9
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v2; // x9
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v3; // x9

    while ( 1 )
    {
        m_pNext = CVisibilityPlugins::m_alphaEntityList.m_firstLink.m_pNext;
        if ( CVisibilityPlugins::m_alphaEntityList.m_firstLink.m_pNext == &CVisibilityPlugins::m_alphaEntityList.m_lastLink )
            break;
        CVisibilityPlugins::m_alphaEntityList.m_firstLink.m_pNext->m_pNext->m_pPrev = CVisibilityPlugins::m_alphaEntityList.m_firstLink.m_pNext->m_pPrev;
        m_pNext->m_pPrev->m_pNext = m_pNext->m_pNext;
        m_pNext->m_pNext = CVisibilityPlugins::m_alphaEntityList.m_firstFreeLink.m_pNext;
        CVisibilityPlugins::m_alphaEntityList.m_firstFreeLink.m_pNext->m_pPrev = m_pNext;
        m_pNext->m_pPrev = &CVisibilityPlugins::m_alphaEntityList.m_firstFreeLink;
        CVisibilityPlugins::m_alphaEntityList.m_firstFreeLink.m_pNext = m_pNext;
    }
    while ( 1 )
    {
        v1 = CVisibilityPlugins::m_alphaBoatAtomicList.m_firstLink.m_pNext;
        if ( CVisibilityPlugins::m_alphaBoatAtomicList.m_firstLink.m_pNext == &CVisibilityPlugins::m_alphaBoatAtomicList.m_lastLink )
            break;
        CVisibilityPlugins::m_alphaBoatAtomicList.m_firstLink.m_pNext->m_pNext->m_pPrev = CVisibilityPlugins::m_alphaBoatAtomicList.m_firstLink.m_pNext->m_pPrev;
        v1->m_pPrev->m_pNext = v1->m_pNext;
        v1->m_pNext = CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink.m_pNext;
        CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink.m_pNext->m_pPrev = v1;
        v1->m_pPrev = &CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink;
        CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink.m_pNext = v1;
    }
    while ( 1 )
    {
        v2 = CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstLink.m_pNext;
        if ( CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstLink.m_pNext == &CVisibilityPlugins::m_alphaUnderwaterEntityList.m_lastLink )
            break;
        CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstLink.m_pNext->m_pNext->m_pPrev = CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstLink.m_pNext->m_pPrev;
        v2->m_pPrev->m_pNext = v2->m_pNext;
        v2->m_pNext = CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstFreeLink.m_pNext;
        CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstFreeLink.m_pNext->m_pPrev = v2;
        v2->m_pPrev = &CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstFreeLink;
        CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstFreeLink.m_pNext = v2;
    }
    while ( 1 )
    {
        v3 = CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstLink.m_pNext;
        if ( CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstLink.m_pNext == &CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink )
            break;
        CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstLink.m_pNext->m_pNext->m_pPrev = CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstLink.m_pNext->m_pPrev;
        v3->m_pPrev->m_pNext = v3->m_pNext;
        v3->m_pNext = CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext;
        CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext->m_pPrev = v3;
        v3->m_pPrev = &CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink;
        CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext = v3;
    }
}

void CVisibilityPlugins::Initialise() {
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v0; // x0
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v1; // x9
    uint64_t v2; // x10
    uint64_t v3; // x12
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pStore; // x14
    CLink<CVisibilityPlugins::AlphaObjectInfo> **v5; // x15
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v6; // x0
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v7; // x9
    uint64_t v8; // x10
    uint64_t v9; // x12
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v10; // x14
    CLink<CVisibilityPlugins::AlphaObjectInfo> **v11; // x15
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v12; // x0
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v13; // x14
    int v14; // w9
    uint64_t v15; // x10
    uint64_t v16; // x12
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v17; // x15
    CLink<CVisibilityPlugins::AlphaObjectInfo> **v18; // x16
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v19; // x0
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v20; // x9
    uint64_t v21; // x10
    uint64_t v22; // x12
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v23; // x14
    CLink<CVisibilityPlugins::AlphaObjectInfo> **v24; // x15
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v25; // x0
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v26; // x9
    uint64_t v27; // x10
    uint64_t v28; // x12
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v29; // x14
    CLink<CVisibilityPlugins::AlphaObjectInfo> **v30; // x15
    CLink<CPed *> *v31; // x0
    CLink<CPed *> *v32; // x9
    uint64_t v33; // x10
    uint64_t v34; // x12
    CLink<CPed *> *v35; // x14
    CLink<CPed *> **v36; // x15

    v0 = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)operator new[](0x320uLL);
    v0[18].m_pNext = v0 + 19;
    v1 = v0 + 18;
    CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext = &CVisibilityPlugins::m_alphaList.m_lastLink;
    CVisibilityPlugins::m_alphaList.m_lastFreeLink.m_pPrev = v0 + 19;
    v0[19].m_pNext = &CVisibilityPlugins::m_alphaList.m_lastFreeLink;
    v2 = 18LL;
    CVisibilityPlugins::m_alphaList.m_pStore = v0;
    CVisibilityPlugins::m_alphaList.m_lastLink.m_pPrev = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList;
    v0[19].m_pPrev = &CVisibilityPlugins::m_alphaList.m_firstFreeLink;
    v0[19].m_pPrev = v0 + 18;
    v3 = 712LL;
    v0[18].m_pPrev = &CVisibilityPlugins::m_alphaList.m_firstFreeLink;
    CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext = v0 + 18;
    do
    {
        m_pStore = CVisibilityPlugins::m_alphaList.m_pStore;
        --v2;
        v5 = (CLink<CVisibilityPlugins::AlphaObjectInfo> **)((char *)CVisibilityPlugins::m_alphaList.m_pStore + v3);
        *v5 = v1;
        v3 -= 40LL;
        CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->m_pPrev = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)(v5 - 4);
        v1 = &m_pStore[v2];
        *(v5 - 1) = &CVisibilityPlugins::m_alphaList.m_firstFreeLink;
        CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)(v5 - 4);
    }
    while ( (uint32_t)v3 != -8 );
    CVisibilityPlugins::m_alphaList.m_firstLink.item.m_distance = 0.0;
    CVisibilityPlugins::m_alphaList.m_lastLink.item.m_distance = 100000000.0;
    v6 = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)operator new[](0x320uLL);
    v6[18].m_pNext = v6 + 19;
    v7 = v6 + 18;
    CVisibilityPlugins::m_alphaBoatAtomicList.m_firstLink.m_pNext = &CVisibilityPlugins::m_alphaBoatAtomicList.m_lastLink;
    CVisibilityPlugins::m_alphaBoatAtomicList.m_lastFreeLink.m_pPrev = v6 + 19;
    v6[19].m_pNext = &CVisibilityPlugins::m_alphaBoatAtomicList.m_lastFreeLink;
    v8 = 18LL;
    CVisibilityPlugins::m_alphaBoatAtomicList.m_pStore = v6;
    CVisibilityPlugins::m_alphaBoatAtomicList.m_lastLink.m_pPrev = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaBoatAtomicList;
    v6[19].m_pPrev = &CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink;
    v6[19].m_pPrev = v6 + 18;
    v9 = 712LL;
    v6[18].m_pPrev = &CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink;
    CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink.m_pNext = v6 + 18;
    do
    {
        v10 = CVisibilityPlugins::m_alphaBoatAtomicList.m_pStore;
        --v8;
        v11 = (CLink<CVisibilityPlugins::AlphaObjectInfo> **)((char *)CVisibilityPlugins::m_alphaBoatAtomicList.m_pStore + v9);
        *v11 = v7;
        v9 -= 40LL;
        CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink.m_pNext->m_pPrev = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)(v11 - 4);
        v7 = &v10[v8];
        *(v11 - 1) = &CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink;
        CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink.m_pNext = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)(v11 - 4);
    }
    while ( (uint32_t)v9 != -8 );
    CVisibilityPlugins::m_alphaBoatAtomicList.m_firstLink.item.m_distance = 0.0;
    CVisibilityPlugins::m_alphaBoatAtomicList.m_lastLink.item.m_distance = 100000000.0;
    v12 = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)operator new[](0x1F40uLL);
    CVisibilityPlugins::m_alphaEntityList.m_firstLink.m_pNext = &CVisibilityPlugins::m_alphaEntityList.m_lastLink;
    v12[199].m_pNext = &CVisibilityPlugins::m_alphaEntityList.m_lastFreeLink;
    CVisibilityPlugins::m_alphaEntityList.m_lastFreeLink.m_pPrev = v12 + 199;
    v12[198].m_pNext = v12 + 199;
    v13 = v12 + 198;
    v14 = -198;
    v15 = 198LL;
    CVisibilityPlugins::m_alphaEntityList.m_pStore = v12;
    CVisibilityPlugins::m_alphaEntityList.m_lastLink.m_pPrev = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaEntityList;
    v12[199].m_pPrev = &CVisibilityPlugins::m_alphaEntityList.m_firstFreeLink;
    v12[199].m_pPrev = v12 + 198;
    v16 = 7912LL;
    v12[198].m_pPrev = &CVisibilityPlugins::m_alphaEntityList.m_firstFreeLink;
    CVisibilityPlugins::m_alphaEntityList.m_firstFreeLink.m_pNext = v12 + 198;
    do
    {
        v17 = CVisibilityPlugins::m_alphaEntityList.m_pStore;
        --v15;
        ++v14;
        v18 = (CLink<CVisibilityPlugins::AlphaObjectInfo> **)((char *)CVisibilityPlugins::m_alphaEntityList.m_pStore + v16);
        *v18 = v13;
        v16 -= 40LL;
        CVisibilityPlugins::m_alphaEntityList.m_firstFreeLink.m_pNext->m_pPrev = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)(v18 - 4);
        v13 = &v17[v15];
        *(v18 - 1) = &CVisibilityPlugins::m_alphaEntityList.m_firstFreeLink;
        CVisibilityPlugins::m_alphaEntityList.m_firstFreeLink.m_pNext = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)(v18 - 4);
    }
    while ( v14 );
    CVisibilityPlugins::m_alphaEntityList.m_firstLink.item.m_distance = 0.0;
    CVisibilityPlugins::m_alphaEntityList.m_lastLink.item.m_distance = 100000000.0;
    v19 = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)operator new[](0xFA0uLL);
    v19[98].m_pNext = v19 + 99;
    v20 = v19 + 98;
    CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstLink.m_pNext = &CVisibilityPlugins::m_alphaUnderwaterEntityList.m_lastLink;
    CVisibilityPlugins::m_alphaUnderwaterEntityList.m_lastFreeLink.m_pPrev = v19 + 99;
    v19[99].m_pNext = &CVisibilityPlugins::m_alphaUnderwaterEntityList.m_lastFreeLink;
    v21 = 98LL;
    CVisibilityPlugins::m_alphaUnderwaterEntityList.m_pStore = v19;
    CVisibilityPlugins::m_alphaUnderwaterEntityList.m_lastLink.m_pPrev = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaUnderwaterEntityList;
    v19[99].m_pPrev = &CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstFreeLink;
    v19[99].m_pPrev = v19 + 98;
    v22 = 3912LL;
    v19[98].m_pPrev = &CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstFreeLink;
    CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstFreeLink.m_pNext = v19 + 98;
    do
    {
        v23 = CVisibilityPlugins::m_alphaUnderwaterEntityList.m_pStore;
        --v21;
        v24 = (CLink<CVisibilityPlugins::AlphaObjectInfo> **)((char *)CVisibilityPlugins::m_alphaUnderwaterEntityList.m_pStore
                                                              + v22);
        *v24 = v20;
        v22 -= 40LL;
        CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstFreeLink.m_pNext->m_pPrev = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)(v24 - 4);
        v20 = &v23[v21];
        *(v24 - 1) = &CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstFreeLink;
        CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstFreeLink.m_pNext = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)(v24 - 4);
    }
    while ( (uint32_t)v22 != -8 );
    CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstLink.item.m_distance = 0.0;
    CVisibilityPlugins::m_alphaUnderwaterEntityList.m_lastLink.item.m_distance = 100000000.0;
    v25 = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)operator new[](0x7D0uLL);
    v25[48].m_pNext = v25 + 49;
    v26 = v25 + 48;
    CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstLink.m_pNext = &CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink;
    CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastFreeLink.m_pPrev = v25 + 49;
    v25[49].m_pNext = &CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastFreeLink;
    v27 = 48LL;
    CVisibilityPlugins::m_alphaReallyDrawLastList.m_pStore = v25;
    CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink.m_pPrev = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaReallyDrawLastList;
    v25[49].m_pPrev = &CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink;
    v25[49].m_pPrev = v25 + 48;
    v28 = 1912LL;
    v25[48].m_pPrev = &CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink;
    CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext = v25 + 48;
    do
    {
        v29 = CVisibilityPlugins::m_alphaReallyDrawLastList.m_pStore;
        --v27;
        v30 = (CLink<CVisibilityPlugins::AlphaObjectInfo> **)((char *)CVisibilityPlugins::m_alphaReallyDrawLastList.m_pStore
                                                              + v28);
        *v30 = v26;
        v28 -= 40LL;
        CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext->m_pPrev = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)(v30 - 4);
        v26 = &v29[v27];
        *(v30 - 1) = &CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink;
        CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext = (CLink<CVisibilityPlugins::AlphaObjectInfo> *)(v30 - 4);
    }
    while ( (uint32_t)v28 != -8 );
    CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstLink.item.m_distance = 0.0;
    CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink.item.m_distance = 100000000.0;
    v31 = (CLink<CPed *> *)operator new[](0x960uLL);
    v31[98].m_pNext = v31 + 99;
    v32 = v31 + 98;
    CVisibilityPlugins::ms_weaponPedsForPC.m_firstLink.m_pNext = &CVisibilityPlugins::ms_weaponPedsForPC.m_lastLink;
    CVisibilityPlugins::ms_weaponPedsForPC.m_lastFreeLink.m_pPrev = v31 + 99;
    v31[99].m_pNext = &CVisibilityPlugins::ms_weaponPedsForPC.m_lastFreeLink;
    v33 = 98LL;
    CVisibilityPlugins::ms_weaponPedsForPC.m_pStore = v31;
    CVisibilityPlugins::ms_weaponPedsForPC.m_lastLink.m_pPrev = (CLink<CPed *> *)&CVisibilityPlugins::ms_weaponPedsForPC;
    v31[99].m_pPrev = &CVisibilityPlugins::ms_weaponPedsForPC.m_firstFreeLink;
    v31[99].m_pPrev = v31 + 98;
    v34 = 2344LL;
    v31[98].m_pPrev = &CVisibilityPlugins::ms_weaponPedsForPC.m_firstFreeLink;
    CVisibilityPlugins::ms_weaponPedsForPC.m_firstFreeLink.m_pNext = v31 + 98;
    do
    {
        v35 = CVisibilityPlugins::ms_weaponPedsForPC.m_pStore;
        --v33;
        v36 = (CLink<CPed *> **)((char *)CVisibilityPlugins::ms_weaponPedsForPC.m_pStore + v34);
        *v36 = v32;
        v34 -= 24LL;
        CVisibilityPlugins::ms_weaponPedsForPC.m_firstFreeLink.m_pNext->m_pPrev = (CLink<CPed *> *)(v36 - 2);
        v32 = &v35[v33];
        *(v36 - 1) = &CVisibilityPlugins::ms_weaponPedsForPC.m_firstFreeLink;
        CVisibilityPlugins::ms_weaponPedsForPC.m_firstFreeLink.m_pNext = (CLink<CPed *> *)(v36 - 2);
    }
    while ( (uint32_t)v34 != -8 );
}

bool CVisibilityPlugins::InsertAtomicIntoBoatSortedList(RpAtomic *pAtomic, float dist)
{
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *p_m_lastLink; // x8
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pNext; // x10
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pPrev; // x11
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v5; // x12
    bool result; // x0
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v7; // x8

    p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaBoatAtomicList.m_lastLink;
    if ( CVisibilityPlugins::m_alphaBoatAtomicList.m_firstLink.m_pNext != &CVisibilityPlugins::m_alphaBoatAtomicList.m_lastLink )
    {
        p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaBoatAtomicList.m_firstLink.m_pNext;
        while ( p_m_lastLink->m_firstLink.item.m_distance < dist )
        {
            p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)p_m_lastLink->m_firstLink.m_pNext;
            if ( p_m_lastLink == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaBoatAtomicList.m_lastLink )
            {
                p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaBoatAtomicList.m_lastLink;
                break;
            }
        }
    }
    m_pNext = CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink.m_pNext;
    if ( CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink.m_pNext == &CVisibilityPlugins::m_alphaBoatAtomicList.m_lastFreeLink )
        return 0LL;
    m_pPrev = CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink.m_pNext->m_pPrev;
    v5 = CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink.m_pNext->m_pNext;
    CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink.m_pNext->item.m_distance = dist;
    m_pNext->item.m_pCallback = pAtomic;
    m_pNext->item.RenderFn = reinterpret_cast<RenderFunction>(CVisibilityPlugins::RenderAtomic);
    v5->m_pPrev = m_pPrev;
    result = m_pNext != 0LL;
    m_pNext->m_pPrev->m_pNext = m_pNext->m_pNext;
    v7 = p_m_lastLink->m_firstLink.m_pPrev;
    m_pNext->m_pNext = v7->m_pNext;
    v7->m_pNext->m_pPrev = m_pNext;
    m_pNext->m_pPrev = v7;
    v7->m_pNext = m_pNext;
    return result;
}

bool CVisibilityPlugins::InsertAtomicIntoReallyDrawLastList(RpAtomic *pAtomic, float dist)
{
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *p_m_lastLink; // x8
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pNext; // x10
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pPrev; // x11
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v5; // x12
    bool result; // x0
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v7; // x8

    p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink;
    if ( CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstLink.m_pNext != &CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink )
    {
        p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstLink.m_pNext;
        while ( p_m_lastLink->m_firstLink.item.m_distance < dist )
        {
            p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)p_m_lastLink->m_firstLink.m_pNext;
            if ( p_m_lastLink == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink )
            {
                p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink;
                break;
            }
        }
    }
    m_pNext = CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext;
    if ( CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext == &CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastFreeLink )
        return 0LL;
    m_pPrev = CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext->m_pPrev;
    v5 = CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext->m_pNext;
    CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext->item.m_distance = dist;
    m_pNext->item.m_pCallback = pAtomic;
    m_pNext->item.RenderFn = reinterpret_cast<RenderFunction>(CVisibilityPlugins::RenderAtomic);
    v5->m_pPrev = m_pPrev;
    result = m_pNext != 0LL;
    m_pNext->m_pPrev->m_pNext = m_pNext->m_pNext;
    v7 = p_m_lastLink->m_firstLink.m_pPrev;
    m_pNext->m_pNext = v7->m_pNext;
    v7->m_pNext->m_pPrev = m_pNext;
    m_pNext->m_pPrev = v7;
    v7->m_pNext = m_pNext;
    return result;
}

bool CVisibilityPlugins::InsertAtomicIntoSortedList(RpAtomic *pAtomic, float dist)
{
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *p_m_lastLink; // x8
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pNext; // x10
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pPrev; // x11
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v5; // x12
    bool result; // x0
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v7; // x8

    p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink;
    if ( CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext != &CVisibilityPlugins::m_alphaList.m_lastLink )
    {
        p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext;
        while ( p_m_lastLink->m_firstLink.item.m_distance < dist )
        {
            p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)p_m_lastLink->m_firstLink.m_pNext;
            if ( p_m_lastLink == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink )
            {
                p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink;
                break;
            }
        }
    }
    m_pNext = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext;
    if ( CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext == &CVisibilityPlugins::m_alphaList.m_lastFreeLink )
        return 0LL;
    m_pPrev = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->m_pPrev;
    v5 = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->m_pNext;
    CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->item.m_distance = dist;
    m_pNext->item.m_pCallback = pAtomic;
    m_pNext->item.RenderFn = reinterpret_cast<RenderFunction>(CVisibilityPlugins::RenderAtomic);
    v5->m_pPrev = m_pPrev;
    result = m_pNext != 0LL;
    m_pNext->m_pPrev->m_pNext = m_pNext->m_pNext;
    v7 = p_m_lastLink->m_firstLink.m_pPrev;
    m_pNext->m_pNext = v7->m_pNext;
    v7->m_pNext->m_pPrev = m_pNext;
    m_pNext->m_pPrev = v7;
    v7->m_pNext = m_pNext;
    return result;
}

bool CVisibilityPlugins::InsertEntityIntoReallyDrawLastList(CEntity *pEntity, float dist)
{
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *p_m_lastLink; // x8
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pNext; // x10
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pPrev; // x11
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v5; // x12
    bool result; // x0
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v7; // x8

    p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink;
    if ( CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstLink.m_pNext != &CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink )
    {
        p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstLink.m_pNext;
        while ( p_m_lastLink->m_firstLink.item.m_distance < dist )
        {
            p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)p_m_lastLink->m_firstLink.m_pNext;
            if ( p_m_lastLink == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink )
            {
                p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink;
                break;
            }
        }
    }
    m_pNext = CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext;
    if ( CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext == &CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastFreeLink )
        return 0LL;
    m_pPrev = CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext->m_pPrev;
    v5 = CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext->m_pNext;
    CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext->item.m_distance = dist;
    m_pNext->item.m_pCallback = pEntity;
    m_pNext->item.RenderFn = reinterpret_cast<RenderFunction>(CVisibilityPlugins::RenderEntity);
    v5->m_pPrev = m_pPrev;
    result = m_pNext != 0LL;
    m_pNext->m_pPrev->m_pNext = m_pNext->m_pNext;
    v7 = p_m_lastLink->m_firstLink.m_pPrev;
    m_pNext->m_pNext = v7->m_pNext;
    v7->m_pNext->m_pPrev = m_pNext;
    m_pNext->m_pPrev = v7;
    v7->m_pNext = m_pNext;
    return result;
}

bool CVisibilityPlugins::InsertEntityIntoSortedList(CEntity *pEntity, float dist)
{
    CHook::CallFunction<bool>("_ZN18CVisibilityPlugins26InsertEntityIntoSortedListEP7CEntityf", pEntity, dist);
    /*int m_nModelIndex; // w8
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *p_m_lastLink; // x8
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pNext; // x10
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pPrev; // x11
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v6; // x12
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v7; // x8
    bool result; // x0
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *v9; // x9
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *v10; // x8
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *v11; // x10
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v12; // x11
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v13; // x12
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v14; // x8

    m_nModelIndex = pEntity->m_nModelIndex;
    if ( m_nModelIndex == *(uint16_t*)(g_libGTASA + 0x8862D0) || m_nModelIndex == *(uint16_t*)(g_libGTASA + 0x8862D2) )
    {
        p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink;
        if ( CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstLink.m_pNext != &CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink )
        {
            p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstLink.m_pNext;
            while ( p_m_lastLink->m_firstLink.item.m_distance < dist )
            {
                p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)p_m_lastLink->m_firstLink.m_pNext;
                if ( p_m_lastLink == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink )
                {
                    p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink;
                    break;
                }
            }
        }
        m_pNext = CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext;
        if ( CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext != &CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastFreeLink )
        {
            m_pPrev = CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext->m_pPrev;
            v6 = CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext->m_pNext;
            CVisibilityPlugins::m_alphaReallyDrawLastList.m_firstFreeLink.m_pNext->item.m_distance = dist;
            m_pNext->item.m_pCallback = pEntity;
            m_pNext->item.RenderFn = reinterpret_cast<RenderFunction>(CVisibilityPlugins::RenderEntity);
            v6->m_pPrev = m_pPrev;
            m_pNext->m_pPrev->m_pNext = m_pNext->m_pNext;
            v7 = p_m_lastLink->m_firstLink.m_pPrev;
            m_pNext->m_pNext = v7->m_pNext;
            v7->m_pNext->m_pPrev = m_pNext;
            m_pNext->m_pPrev = v7;
            v7->m_pNext = m_pNext;
            if ( m_pNext )
                return 1LL;
        }
    }
    v9 = &CVisibilityPlugins::m_alphaUnderwaterEntityList;
    if ( (*((uint8_t *)&pEntity->m_nFlags + 2) & 0x10) == 0 )
        v9 = &CVisibilityPlugins::m_alphaEntityList;
    v10 = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)v9->m_firstLink.m_pNext;
    if ( v10 != (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&v9->m_lastLink )
    {
        while ( v10->m_firstLink.item.m_distance < dist )
        {
            v10 = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)v10->m_firstLink.m_pNext;
            if ( v10 == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&v9->m_lastLink )
            {
                v10 = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&v9->m_lastLink;
                break;
            }
        }
    }
    v11 = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)v9->m_firstFreeLink.m_pNext;
    if ( v11 == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&v9->m_lastFreeLink )
        return 0LL;
    v12 = v11->m_firstLink.m_pPrev;
    v13 = v11->m_firstLink.m_pNext;
    v11->m_firstLink.item.m_distance = dist;
    v11->m_firstLink.item.m_pCallback = pEntity;
    v11->m_firstLink.item.RenderFn = reinterpret_cast<RenderFunction>(CVisibilityPlugins::RenderEntity);
    v13->m_pPrev = v12;
    result = v11 != 0LL;
    v11->m_firstLink.m_pPrev->m_pNext = v11->m_firstLink.m_pNext;
    v14 = v10->m_firstLink.m_pPrev;
    v11->m_firstLink.m_pNext = v14->m_pNext;
    v14->m_pNext->m_pPrev = &v11->m_firstLink;
    v11->m_firstLink.m_pPrev = v14;
    v14->m_pNext = &v11->m_firstLink;
    return result;*/
}

bool CVisibilityPlugins::InsertEntityIntoUnderwaterList(CEntity *pEntity, float dist)
{
    CHook::CallFunction<bool>("_ZN18CVisibilityPlugins30InsertEntityIntoUnderwaterListEP7CEntityf", pEntity, dist);
   /* CLinkList<CVisibilityPlugins::AlphaObjectInfo> *p_m_lastLink; // x8
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pNext; // x10
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pPrev; // x11
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v5; // x12
    bool result; // x0
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v7; // x8

    p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaUnderwaterEntityList.m_lastLink;
    if ( CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstLink.m_pNext != &CVisibilityPlugins::m_alphaUnderwaterEntityList.m_lastLink )
    {
        p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstLink.m_pNext;
        while ( p_m_lastLink->m_firstLink.item.m_distance < dist )
        {
            p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)p_m_lastLink->m_firstLink.m_pNext;
            if ( p_m_lastLink == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaUnderwaterEntityList.m_lastLink )
            {
                p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaUnderwaterEntityList.m_lastLink;
                break;
            }
        }
    }
    m_pNext = CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstFreeLink.m_pNext;
    if ( CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstFreeLink.m_pNext == &CVisibilityPlugins::m_alphaUnderwaterEntityList.m_lastFreeLink )
        return 0LL;
    m_pPrev = CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstFreeLink.m_pNext->m_pPrev;
    v5 = CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstFreeLink.m_pNext->m_pNext;
    CVisibilityPlugins::m_alphaUnderwaterEntityList.m_firstFreeLink.m_pNext->item.m_distance = dist;
    m_pNext->item.m_pCallback = pEntity;
    m_pNext->item.RenderFn = reinterpret_cast<RenderFunction>(CVisibilityPlugins::RenderEntity);
    v5->m_pPrev = m_pPrev;
    result = m_pNext != 0LL;
    m_pNext->m_pPrev->m_pNext = m_pNext->m_pNext;
    v7 = p_m_lastLink->m_firstLink.m_pPrev;
    m_pNext->m_pNext = v7->m_pNext;
    v7->m_pNext->m_pPrev = m_pNext;
    m_pNext->m_pPrev = v7;
    v7->m_pNext = m_pNext;
    return result;*/
}

bool CVisibilityPlugins::InsertObjectIntoSortedList(void *pObj, float dist, RenderFunction fn)
{
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *p_m_lastLink; // x8
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pNext; // x10
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pPrev; // x9
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v6; // x11
    bool result; // x0
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v8; // x8

    p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaEntityList.m_lastLink;
    if ( CVisibilityPlugins::m_alphaEntityList.m_firstLink.m_pNext != &CVisibilityPlugins::m_alphaEntityList.m_lastLink )
    {
        p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaEntityList.m_firstLink.m_pNext;
        while ( p_m_lastLink->m_firstLink.item.m_distance < dist )
        {
            p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)p_m_lastLink->m_firstLink.m_pNext;
            if ( p_m_lastLink == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaEntityList.m_lastLink )
            {
                p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaEntityList.m_lastLink;
                break;
            }
        }
    }
    m_pNext = CVisibilityPlugins::m_alphaEntityList.m_firstFreeLink.m_pNext;
    if ( CVisibilityPlugins::m_alphaEntityList.m_firstFreeLink.m_pNext == &CVisibilityPlugins::m_alphaEntityList.m_lastFreeLink )
        return 0LL;
    m_pPrev = CVisibilityPlugins::m_alphaEntityList.m_firstFreeLink.m_pNext->m_pPrev;
    v6 = CVisibilityPlugins::m_alphaEntityList.m_firstFreeLink.m_pNext->m_pNext;
    CVisibilityPlugins::m_alphaEntityList.m_firstFreeLink.m_pNext->item.m_pCallback = pObj;
    m_pNext->item.RenderFn = fn;
    m_pNext->item.m_distance = dist;
    v6->m_pPrev = m_pPrev;
    result = m_pNext != 0LL;
    m_pNext->m_pPrev->m_pNext = m_pNext->m_pNext;
    v8 = p_m_lastLink->m_firstLink.m_pPrev;
    m_pNext->m_pNext = v8->m_pNext;
    v8->m_pNext->m_pPrev = m_pNext;
    m_pNext->m_pPrev = v8;
    v8->m_pNext = m_pNext;
    return result;
}

RwBool CVisibilityPlugins::IsAtomicVisible(RpAtomic *pAtomic)
{
    if ( (pAtomic->interpolator.flags & 2) != 0 )
        _rpAtomicResyncInterpolatedSphere(pAtomic);
    RwSphere boundingSphere = pAtomic->boundingSphere;
    RwV3dTransformPoints(
            &boundingSphere.center,
            &boundingSphere.center,
            1,
            (const RwMatrix *)((char *)pAtomic->object.object.parent + 32));
    return RwCameraFrustumTestSphere(CVisibilityPlugins::ms_pCamera, &boundingSphere) != rwSPHEREOUTSIDE;
}

RwBool CVisibilityPlugins::IsClumpVisible(RpClump *pClump)
{
    return (*(uint64_t (**)(void))(&pClump->object.type + CVisibilityPlugins::ms_clumpPluginOffset))();
}

RwBool CVisibilityPlugins::PluginAttach()
{
    CVisibilityPlugins::ms_atomicPluginOffset = RpAtomicRegisterPlugin(
            8,
            0x253F200u,
            CVisibilityPlugins::AtomicConstructor,
            CVisibilityPlugins::AtomicDestructor,
            CVisibilityPlugins::AtomicCopyConstructor);
    CVisibilityPlugins::ms_framePluginOffset = RwFrameRegisterPlugin(
            8,
            0x253F202u,
            CVisibilityPlugins::FrameConstructor,
            CVisibilityPlugins::FrameDestructor,
            CVisibilityPlugins::FrameCopyConstructor);
    CVisibilityPlugins::ms_clumpPluginOffset = RpClumpRegisterPlugin(
            16,
            0x253F201u,
            CVisibilityPlugins::ClumpConstructor,
            CVisibilityPlugins::ClumpDestructor,
            CVisibilityPlugins::ClumpCopyConstructor);
    return CVisibilityPlugins::ms_clumpPluginOffset != -1 && CVisibilityPlugins::ms_atomicPluginOffset != -1;
}

void CVisibilityPlugins::RenderAlphaAtomics()
{
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *i; // x20

    for ( i = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaList.m_lastLink.m_pPrev;
          i != &CVisibilityPlugins::m_alphaList;
          i = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)i->m_firstLink.m_pPrev )
    {
        i->m_firstLink.item.RenderFn(i->m_firstLink.item.m_pCallback, i->m_firstLink.item.m_distance);
    }
}

RpAtomic* CVisibilityPlugins::RenderAtomicWithAlphaCB(RpAtomic *pAtomic, void *pAlpha)
{
    if ( (pAtomic->object.object.flags & 4) != 0 )
    {
        CHook::CallFunction<void>("_Z23emu_EnableAlphaModulatef", (float)*(int *)pAlpha / 255.0);
        //emu_EnableAlphaModulate((float)*(int *)pAlpha / 255.0);
        AtomicDefaultRenderCallBack(pAtomic);
        CHook::CallFunction<void>("_Z24emu_DisableAlphaModulatev");
        // emu_DisableAlphaModulate();
    }
    return pAtomic;
}

void CVisibilityPlugins::RenderAtomic(RpAtomic *pObj, float dist)
{
    AtomicDefaultRenderCallBack(pObj);
}

void CVisibilityPlugins::RenderBoatAlphaAtomics()
{
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *i; // x20

    RwRenderStateSet(rwRENDERSTATECULLMODE, RWRSTATE(rwCULLMODECULLNONE));
    for ( i = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaBoatAtomicList.m_lastLink.m_pPrev;
          i != &CVisibilityPlugins::m_alphaBoatAtomicList;
          i = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)i->m_firstLink.m_pPrev )
    {
        i->m_firstLink.item.RenderFn(i->m_firstLink.item.m_pCallback, i->m_firstLink.item.m_distance);
    }
    RwRenderStateSet(rwRENDERSTATECULLMODE, RWRSTATE(rwCULLMODECULLBACK));
}

void CVisibilityPlugins::RenderFadingAtomic(CBaseModelInfo *pModelInfo, RpAtomic *pAtomic, int32 alpha)
{
    if ( (pModelInfo->m_nFlags & 4) != 0 )
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDONE));
    //emu_EnableAlphaModulate((float)alpha / 255.0);
    CHook::CallFunction<void>("_Z23emu_EnableAlphaModulatef", (float)alpha / 255.0);
    AtomicDefaultRenderCallBack(pAtomic);
    //emu_DisableAlphaModulate();
    CHook::CallFunction<void>("_Z24emu_DisableAlphaModulatev");
    if ( (pModelInfo->m_nFlags & 4) != 0 )
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDINVSRCALPHA));
}

void CVisibilityPlugins::RenderEntity(CEntity *pObj, float dist)
{
    CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));
    CHook::CallFunction<void>("_ZN18CVisibilityPlugins12RenderEntityEPvf", pObj, dist);
/*
    void *v5; // x1
    float m_lodDistance; // s2
    float v7; // s1
    CEntity::CFlags m_nFlags; // x8
    float v9; // s2
    float v10; // s3
    float v11; // s8
    char v12; // w0
    RpAtomic *m_pRwObject; // x21
    char v14; // w20
    CEntity::CFlags v15; // x8
    int pData; // [xsp+Ch] [xbp-34h] BYREF

    if ( !pObj->m_pRwObject )
        return;
    CBaseModelInfo* v4 = CModelInfo::ms_modelInfoPtrs[pObj->m_nModelIndex];
    if ( (v4->m_nFlags & 8) != 0 )
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, RWRSTATE(FALSE));
    if ( (*((uint8_t *)&pObj->m_nFlags + 1) & 0x80) == 0 )
    {
        if ( CGame::currArea || (v4->m_nFlags & 8) != 0 )
            RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, RWRSTATE(0));
        else
            RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, RWRSTATE(100u));
        //CRenderer::RenderOneNonRoad(pObj);
        CHook::CallFunction<void>("_ZN9CRenderer16RenderOneNonRoadEP7CEntity", pObj);
        if ( (v4->m_nFlags & 8) == 0 )
            return;
        LABEL_31:
        RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, RWRSTATE(TRUE));
        return;
    }
    RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, RWRSTATE(0));
    m_lodDistance = v4->m_lodDistance;
    v7 = fminf(
            m_lodDistance * TheCamera.m_fLODDistMultiplier,
            CRenderer::ms_fFarClipPlane + v4->m_pColModel->GetBoundingSphere().m_fRadius);
    if ( pObj->m_pLod )
    {
        m_nFlags = pObj->m_nFlags;
        v9 = FadeDistMult * 22.0;
    }
    else
    {
        v10 = fminf(m_lodDistance, v7);
        v9 = FadeDistMult * 22.0;
        if ( v10 > 150.0 )
            v9 = (float)(v10 / 15.0) + 10.0;
        m_nFlags = pObj->m_nFlags;
        if ( (*(_WORD *)&m_nFlags & 0x100) != 0 )
            v7 = v7 * CRenderer::ms_lowLodDistScale;
    }
    v11 = fminf((float)((float)((float)((float)(FadeDistMult * 22.0) + v7) - dist) / v9) * 0.8, 1.0) * (float)v4->m_alpha;
    pObj->m_nFlags = (CEntity::CFlags)(*(unsigned __int64 *)&m_nFlags | 0x2000);
    if ( (*(_DWORD *)&m_nFlags & 0x8000000) == 0 )
        RwRenderStateSet(rwRENDERSTATECULLMODE, (char *)&dword_0 + 1);
    v12 = (*((__int64 (__fastcall **)(CEntity *))pObj->_vptr$CPlaceable + 20))(pObj);
    m_pRwObject = (RpAtomic_0 *)pObj->m_pRwObject;
    v14 = v12;
    if ( m_pRwObject->object.object.type == 1 )
    {
        if ( (v4->m_flags & 4) != 0 )
            RwRenderStateSet(rwRENDERSTATEDESTBLEND, (char *)&dword_0 + 2);
        emu_EnableAlphaModulate((float)(int)v11 / 255.0);
        AtomicDefaultRenderCallBack(m_pRwObject);
        emu_DisableAlphaModulate();
        if ( (v4->m_flags & 4) == 0 )
            goto LABEL_28;
        LABEL_27:
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, &byte_6);
        goto LABEL_28;
    }
    pData = (int)v11;
    if ( (v4->m_flags & 4) != 0 )
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, (char *)&dword_0 + 2);
    RpClumpForAllAtomics((RpClump_0 *)m_pRwObject, CVisibilityPlugins::RenderAtomicWithAlphaCB, &pData);
    if ( (v4->m_flags & 4) != 0 )
        goto LABEL_27;
    LABEL_28:
    (*((void (__fastcall **)(CEntity *, _QWORD))pObj->_vptr$CPlaceable + 21))(pObj, v14 & 1);
    v15 = pObj->m_nFlags;
    pObj->m_nFlags = (CEntity::CFlags)(*(_QWORD *)&v15 & 0xFFFFFFFFFFFFDFFFLL);
    if ( (*(_DWORD *)&v15 & 0x8000000) == 0 )
        RwRenderStateSet(rwRENDERSTATECULLMODE, (char *)&dword_0 + 2);
    if ( (v4->m_flags & 8) != 0 )
        goto LABEL_31;*/
}

void CVisibilityPlugins::RenderFadingClump(CBaseModelInfo *pModelInfo, RpClump *pClump, int32 alpha)
{
    int32 pData = alpha;
    if ( (pModelInfo->m_nFlags & 4) != 0 )
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDONE));
    RpClumpForAllAtomics(pClump, CVisibilityPlugins::RenderAtomicWithAlphaCB, &pData);
    if ( (pModelInfo->m_nFlags & 4) != 0 )
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDINVSRCALPHA));
}

RpAtomic* CVisibilityPlugins::RenderFadingClumpCB(RpAtomic *pAtomic)
{
    int v2 = *(uint32_t *)(&pAtomic->clump->object.privateFlags + CVisibilityPlugins::ms_clumpPluginOffset + 1);
    if ( v2 == 255 )
    {
        AtomicDefaultRenderCallBack(pAtomic);
    }
    else
    {
        //emu_EnableAlphaModulate((float)v2 / 255.0);
        CHook::CallFunction<void>("_Z23emu_EnableAlphaModulatef", (float)v2 / 255.0);
        AtomicDefaultRenderCallBack(pAtomic);
        //emu_DisableAlphaModulate();
        CHook::CallFunction<void>("_Z24emu_DisableAlphaModulatev");
    }
    return pAtomic;
}

void CVisibilityPlugins::RenderFadingEntities()
{
    CHook::CallFunction<void>("_ZN18CVisibilityPlugins20RenderFadingEntitiesEv");
    /*CLinkList<CVisibilityPlugins::AlphaObjectInfo> *i; // x20
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *j; // x20
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *k; // x20
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *m; // x20

    if ( CPauseMenu::Settings[0].value == 3 )
    {
        CHook::CallFunction<void>("_Z18_rpSkipBlendRenderb", 1);
        CHook::CallFunction<void>("_Z21_rpSkipNonblendRenderb", 0);
        //_rpSkipBlendRender(1);
        //_rpSkipNonblendRender(0);
        for ( i = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaEntityList.m_lastLink.m_pPrev;
              i != &CVisibilityPlugins::m_alphaEntityList;
              i = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)i->m_firstLink.m_pPrev )
        {
            i->m_firstLink.item.RenderFn(i->m_firstLink.item.m_pCallback, i->m_firstLink.item.m_distance);
        }
        CHook::CallFunction<void>("_Z18_rpSkipBlendRenderb", 0);
        CHook::CallFunction<void>("_Z21_rpSkipNonblendRenderb", 1);
       // _rpSkipBlendRender(0);
       // _rpSkipNonblendRender(1);
        for ( j = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaEntityList.m_lastLink.m_pPrev;
              j != &CVisibilityPlugins::m_alphaEntityList;
              j = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)j->m_firstLink.m_pPrev )
        {
            j->m_firstLink.item.RenderFn(j->m_firstLink.item.m_pCallback, j->m_firstLink.item.m_distance);
        }
        //_rpSkipNonblendRender(0);
        CHook::CallFunction<void>("_Z21_rpSkipNonblendRenderb", 0);
    }
    else
    {
        for ( k = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaEntityList.m_lastLink.m_pPrev;
              k != &CVisibilityPlugins::m_alphaEntityList;
              k = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)k->m_firstLink.m_pPrev )
        {
            k->m_firstLink.item.RenderFn(k->m_firstLink.item.m_pCallback, k->m_firstLink.item.m_distance);
        }
    }
    RwRenderStateSet(rwRENDERSTATECULLMODE, RWRSTATE(rwCULLMODECULLNONE));
    for ( m = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaBoatAtomicList.m_lastLink.m_pPrev;
          m != &CVisibilityPlugins::m_alphaBoatAtomicList;
          m = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)m->m_firstLink.m_pPrev )
    {
        m->m_firstLink.item.RenderFn(m->m_firstLink.item.m_pCallback, m->m_firstLink.item.m_distance);
    }
    RwRenderStateSet(rwRENDERSTATECULLMODE, RWRSTATE(rwCULLMODECULLBACK));*/
}

void CVisibilityPlugins::RenderFadingUnderwaterEntities()
{
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *i; // x20

    for ( i = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaUnderwaterEntityList.m_lastLink.m_pPrev;
          i != &CVisibilityPlugins::m_alphaUnderwaterEntityList;
          i = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)i->m_firstLink.m_pPrev )
    {
        i->m_firstLink.item.RenderFn(i->m_firstLink.item.m_pCallback, i->m_firstLink.item.m_distance);
    }
}

RpAtomic* CVisibilityPlugins::RenderHeliRotorAlphaCB(RpAtomic *pAtomic)
{
    RwFrame *parent; // x20
    RwMatrix *LTM; // x21
    RwMatrix *v4; // x0
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *p_m_lastLink; // x8
    float v6; // s0
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pNext; // x10
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pPrev; // x11
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v9; // x12
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v10; // x8

    if ( gVehicleDistanceFromCamera < CVisibilityPlugins::ms_bigVehicleLod0Dist )
    {
        parent = (RwFrame *)pAtomic->clump->object.parent;
        LTM = RwFrameGetLTM((RwFrame *)pAtomic->object.object.parent);
        v4 = RwFrameGetLTM(parent);
        p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink;
        v6 = gVehicleDistanceFromCamera
             + (float)((float)((float)((float)((float)(LTM->pos.y - CVisibilityPlugins::ms_pCameraPosn->y) * v4->at.y)
                                       + (float)((float)(LTM->pos.x - CVisibilityPlugins::ms_pCameraPosn->x) * v4->at.x))
                               + (float)((float)(LTM->pos.z - CVisibilityPlugins::ms_pCameraPosn->z) * v4->at.z))
                       * 20.0);
        if ( CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext != &CVisibilityPlugins::m_alphaList.m_lastLink )
        {
            p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext;
            while ( p_m_lastLink->m_firstLink.item.m_distance < v6 )
            {
                p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)p_m_lastLink->m_firstLink.m_pNext;
                if ( p_m_lastLink == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink )
                {
                    p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink;
                    break;
                }
            }
        }
        m_pNext = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext;
        if ( CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext == &CVisibilityPlugins::m_alphaList.m_lastFreeLink )
            goto LABEL_9;
        m_pPrev = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->m_pPrev;
        v9 = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->m_pNext;
        CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->item.m_distance = v6;
        m_pNext->item.m_pCallback = pAtomic;
        m_pNext->item.RenderFn = (CVisibilityPlugins::RenderFunction)CVisibilityPlugins::RenderAtomic;
        v9->m_pPrev = m_pPrev;
        m_pNext->m_pPrev->m_pNext = m_pNext->m_pNext;
        v10 = p_m_lastLink->m_firstLink.m_pPrev;
        m_pNext->m_pNext = v10->m_pNext;
        v10->m_pNext->m_pPrev = m_pNext;
        m_pNext->m_pPrev = v10;
        v10->m_pNext = m_pNext;
        if ( !m_pNext )
            LABEL_9:
            AtomicDefaultRenderCallBack(pAtomic);
    }
    return pAtomic;
}

RpAtomic* CVisibilityPlugins::RenderHeliTailRotorAlphaCB(RpAtomic *pAtomic)
{
    RwFrame *parent; // x20
    RwMatrix *LTM; // x21
    RwMatrix *v4; // x0
    float v5; // s1
    float v6; // s3
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *p_m_lastLink; // x8
    float v8; // s0
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pNext; // x10
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pPrev; // x11
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v11; // x12
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v12; // x8

    if ( gVehicleDistanceFromCamera < CVisibilityPlugins::ms_vehicleLod0Dist )
    {
        parent = (RwFrame *)pAtomic->clump->object.parent;
        LTM = RwFrameGetLTM((RwFrame *)pAtomic->object.object.parent);
        v4 = RwFrameGetLTM(parent);
        v5 = LTM->pos.y - CVisibilityPlugins::ms_pCameraPosn->y;
        v6 = LTM->pos.z - CVisibilityPlugins::ms_pCameraPosn->z;
        p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink;
        v8 = gVehicleDistanceFromCamera
             + (float)((float)-(float)((float)((float)(v5 * v4->right.y)
                                               + (float)((float)(LTM->pos.x - CVisibilityPlugins::ms_pCameraPosn->x)
                                                         * v4->right.x))
                                       + (float)(v6 * v4->right.z))
                       - (float)((float)((float)(v5 * v4->up.y)
                                         + (float)((float)(LTM->pos.x - CVisibilityPlugins::ms_pCameraPosn->x) * v4->up.x))
                                 + (float)(v6 * v4->up.z)));
        if ( CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext != &CVisibilityPlugins::m_alphaList.m_lastLink )
        {
            p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext;
            while ( p_m_lastLink->m_firstLink.item.m_distance < v8 )
            {
                p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)p_m_lastLink->m_firstLink.m_pNext;
                if ( p_m_lastLink == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink )
                {
                    p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink;
                    break;
                }
            }
        }
        m_pNext = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext;
        if ( CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext == &CVisibilityPlugins::m_alphaList.m_lastFreeLink )
            goto LABEL_9;
        m_pPrev = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->m_pPrev;
        v11 = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->m_pNext;
        CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->item.m_distance = v8;
        m_pNext->item.m_pCallback = pAtomic;
        m_pNext->item.RenderFn = (CVisibilityPlugins::RenderFunction)CVisibilityPlugins::RenderAtomic;
        v11->m_pPrev = m_pPrev;
        m_pNext->m_pPrev->m_pNext = m_pNext->m_pNext;
        v12 = p_m_lastLink->m_firstLink.m_pPrev;
        m_pNext->m_pNext = v12->m_pNext;
        v12->m_pNext->m_pPrev = m_pNext;
        m_pNext->m_pPrev = v12;
        v12->m_pNext = m_pNext;
        if ( !m_pNext )
            LABEL_9:
            AtomicDefaultRenderCallBack(pAtomic);
    }
    return pAtomic;
}

RpAtomic* CVisibilityPlugins::RenderObjNormalAtomic(RpAtomic *pAtomic)
{
    /*uint64_t v4; // d2
    RwV3d v6; // [xsp+0h] [xbp-20h] BYREF

    RwMatrix* LTM = RwFrameGetLTM((RwFrame *)pAtomic->object.object.parent);
    v6.x = LTM->pos.x - CVisibilityPlugins::ms_pCameraPosn->x;
    v6.y = LTM->pos.y - CVisibilityPlugins::ms_pCameraPosn->y;
    v6.z = LTM->pos.z - CVisibilityPlugins::ms_pCameraPosn->z;
    float v3 = RwV3dLength(&v6);
    if ( v3 <= 8.0
         || (v4 = vmul_f32(*(float32x2_t *)&v6.y, *(float32x2_t *)&LTM->up.y).n64_u64[0],
            (float)((float)(*(float *)&v4 + (float)(v6.x * LTM->up.x)) + *((float *)&v4 + 1)) >= (float)(v3 * -0.3)) )
    {
        AtomicDefaultRenderCallBack(pAtomic);
    }
    return pAtomic;*/
    RwV3d v6;

    RwMatrix* LTM = RwFrameGetLTM(reinterpret_cast<RwFrame*>(pAtomic->object.object.parent));
    if (!LTM || !CVisibilityPlugins::ms_pCameraPosn)
        return pAtomic;

    v6.x = LTM->pos.x - CVisibilityPlugins::ms_pCameraPosn->x;
    v6.y = LTM->pos.y - CVisibilityPlugins::ms_pCameraPosn->y;
    v6.z = LTM->pos.z - CVisibilityPlugins::ms_pCameraPosn->z;

    float dist = RwV3dLength(&v6);
    if (dist <= 8.0f) {
        AtomicDefaultRenderCallBack(pAtomic);
    } else {
        float dot = v6.x * LTM->up.x + v6.y * LTM->up.y + v6.z * LTM->up.z;
        if (dot >= (dist * -0.3f)) {
            AtomicDefaultRenderCallBack(pAtomic);
        }
    }

    return pAtomic;
}

void CVisibilityPlugins::RenderOrderedList(CLinkList<CVisibilityPlugins::AlphaObjectInfo> *list)
{
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *i; // x20

    for ( i = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)list->m_lastLink.m_pPrev;
          i != list;
          i = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)i->m_firstLink.m_pPrev )
    {
        i->m_firstLink.item.RenderFn(i->m_firstLink.item.m_pCallback, i->m_firstLink.item.m_distance);
    }
}

RpAtomic* CVisibilityPlugins::RenderPedCB(RpAtomic *pAtomic)
{
    /*RpClump *clump; // x20
    RwMatrix *LTM; // x0
    float32x2_t v4; // d1
    uint64_t v5; // d1
    int v6; // w8

    clump = pAtomic->clump;
    LTM = RwFrameGetLTM((RwFrame *)clump->object.parent);
    v4.n64_u64[0] = vsub_f32(*(float32x2_t *)&LTM->pos.y, *(float32x2_t *)&CVisibilityPlugins::ms_pCameraPosn->y).n64_u64[0];
    v5 = vmul_f32(v4, v4).n64_u64[0];
    if ( (float)((float)(*(float *)&v5
                         + (float)((float)(LTM->pos.x - CVisibilityPlugins::ms_pCameraPosn->x)
                                   * (float)(LTM->pos.x - CVisibilityPlugins::ms_pCameraPosn->x)))
                 + *((float *)&v5 + 1)) < CVisibilityPlugins::ms_pedLodDist )
    {
        v6 = *(uint32_t *)(&clump->object.privateFlags + CVisibilityPlugins::ms_clumpPluginOffset + 1);
        if ( v6 == 255 )
        {
            AtomicDefaultRenderCallBack(pAtomic);
        }
        else
        {
            //emu_EnableAlphaModulate((float)v6 / 255.0);
            CHook::CallFunction<void>("_Z23emu_EnableAlphaModulatef", (float)v6 / 255.0);
            AtomicDefaultRenderCallBack(pAtomic);
            //emu_DisableAlphaModulate();
            CHook::CallFunction<void>("_Z24emu_DisableAlphaModulatev");
        }
    }
    return pAtomic;*/

    RpClump* clump = pAtomic->clump;
    if (!clump)
        return pAtomic;

    RwFrame* frame = static_cast<RwFrame*>(clump->object.parent);
    RwMatrix* ltm = RwFrameGetLTM(frame);

    float dx = ltm->pos.x - CVisibilityPlugins::ms_pCameraPosn->x;
    float dy = ltm->pos.y - CVisibilityPlugins::ms_pCameraPosn->y;
    float dz = ltm->pos.z - CVisibilityPlugins::ms_pCameraPosn->z;

    float distSq = dx * dx + dy * dy + dz * dz;

    if (distSq < CVisibilityPlugins::ms_pedLodDist * CVisibilityPlugins::ms_pedLodDist) {
        uint8_t alpha = *(uint8_t*)((uintptr_t)&clump->object.privateFlags + CVisibilityPlugins::ms_clumpPluginOffset + 1);

        if (alpha == 255) {
            AtomicDefaultRenderCallBack(pAtomic);
        } else {
            CHook::CallFunction<void>("_Z23emu_EnableAlphaModulatef", static_cast<float>(alpha) / 255.0f);

            AtomicDefaultRenderCallBack(pAtomic);

            CHook::CallFunction<void>("_Z24emu_DisableAlphaModulatev");
        }
    }

    return pAtomic;
}

RpMaterial* CVisibilityPlugins::SetTextureCB(RpMaterial* material, void* texture)
{
    RpMaterialSetTexture(material, reinterpret_cast<RwTexture*>(texture));
    return material;
}

RpAtomic* CVisibilityPlugins::RenderPlayerCB(RpAtomic *pAtomic)
{
    if ( CWorld::Players[0].m_pSkinTexture )
        RpGeometryForAllMaterials(pAtomic->geometry, SetTextureCB, CWorld::Players[0].m_pSkinTexture);
    AtomicDefaultRenderCallBack(pAtomic);
    return pAtomic;
}

void CVisibilityPlugins::RenderReallyDrawLastObjects()
{
 /*   CLinkList<CVisibilityPlugins::AlphaObjectInfo> *i; // x20

    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,     RWRSTATE(NULL));
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,       RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,      RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATESRCBLEND,          RWRSTATE(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,         RWRSTATE(rwBLENDINVSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEFOGENABLE,         RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATECULLMODE,          RWRSTATE(rwCULLMODECULLNONE));
    //SetAmbientColours();
    //DeActivateDirectional();
    ((void(*)())(g_libGTASA + (VER_x32 ? 0x5D2068 + 1 : 0x6F6738)))(); // -- SetAmbientColours
    ((void(*)())(g_libGTASA + (VER_x32 ? 0x5D1F98 + 1 : 0x6F6664)))(); // -- DeActivateDirectional
    for ( i = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaReallyDrawLastList.m_lastLink.m_pPrev;
          i != &CVisibilityPlugins::m_alphaReallyDrawLastList;
          i = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)i->m_firstLink.m_pPrev )
    {
        i->m_firstLink.item.RenderFn(i->m_firstLink.item.m_pCallback, i->m_firstLink.item.m_distance);
    }
    RwRenderStateSet(rwRENDERSTATEFOGENABLE, RWRSTATE(FALSE));*/
    CHook::CallFunction<void>("_ZN18CVisibilityPlugins27RenderReallyDrawLastObjectsEv");
}

RpAtomic* CVisibilityPlugins::RenderTrainHiDetailAlphaCB(RpAtomic* pAtomic)
{
    RwUInt8 *v2; // x8
    uint16_t v3; // w9
    uint16_t v4; // w11
    uint16_t v5; // w9
    RwFrame *parent; // x21
    RwMatrix *LTM; // x22
    uint32 v8; // w20
    RwMatrix *v9; // x0
    float DotProductWithCameraVector; // s0
    float v11; // s0
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *p_m_lastLink; // x8
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pNext; // x10
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pPrev; // x11
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v15; // x12
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v16; // x8

    if ( gVehicleDistanceFromCamera < CVisibilityPlugins::ms_bigVehicleLod0Dist )
    {
        v2 = &pAtomic->object.object.type + CVisibilityPlugins::ms_atomicPluginOffset;
        v3 = *((uint16_t *)v2 + 1);
        v4 = v3 | 0x2000;
        v5 = v3 & 0xDFFF;
        if ( CVisibilityPlugins::ms_vehicleLod0RenderMultiPassDist <= gVehicleDistanceFromCamera )
            v5 = v4;
        *((uint16_t *)v2 + 1) = v5;
        parent = (RwFrame *)pAtomic->clump->object.parent;
        LTM = RwFrameGetLTM((RwFrame *)pAtomic->object.object.parent);
        v8 = *(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset);
        v9 = RwFrameGetLTM(parent);
        DotProductWithCameraVector = CVisibilityPlugins::GetDotProductWithCameraVector(LTM, v9, v8);
        if ( gVehicleDistanceFromCamera <= CVisibilityPlugins::ms_cullCompsDist
             || DotProductWithCameraVector <= 0.0
             || (v8 & 0x400) != 0
             || gAngleWithHorizontal >= 0.2
             || (v8 & 0x80) == 0
                && (float)(DotProductWithCameraVector * DotProductWithCameraVector) <= (float)(gVehicleDistanceFromCamera * 0.1) )
        {
            if ( (v8 & 0x40) != 0 )
            {
                p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink;
                if ( CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext != &CVisibilityPlugins::m_alphaList.m_lastLink )
                {
                    p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext;
                    while ( p_m_lastLink->m_firstLink.item.m_distance < gVehicleDistanceFromCamera )
                    {
                        p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)p_m_lastLink->m_firstLink.m_pNext;
                        if ( p_m_lastLink == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink )
                        {
                            p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink;
                            break;
                        }
                    }
                }
                m_pNext = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext;
                if ( CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext == &CVisibilityPlugins::m_alphaList.m_lastFreeLink )
                    goto LABEL_26;
                m_pPrev = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->m_pPrev;
                v15 = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->m_pNext;
                CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->item.m_distance = gVehicleDistanceFromCamera;
            }
            else
            {
                v11 = DotProductWithCameraVector + gVehicleDistanceFromCamera;
                p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink;
                if ( CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext != &CVisibilityPlugins::m_alphaList.m_lastLink )
                {
                    p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext;
                    while ( p_m_lastLink->m_firstLink.item.m_distance < v11 )
                    {
                        p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)p_m_lastLink->m_firstLink.m_pNext;
                        if ( p_m_lastLink == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink )
                        {
                            p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink;
                            break;
                        }
                    }
                }
                m_pNext = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext;
                if ( CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext == &CVisibilityPlugins::m_alphaList.m_lastFreeLink )
                    goto LABEL_26;
                m_pPrev = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->m_pPrev;
                v15 = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->m_pNext;
                CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->item.m_distance = v11;
            }
            m_pNext->item.m_pCallback = pAtomic;
            m_pNext->item.RenderFn = (CVisibilityPlugins::RenderFunction)CVisibilityPlugins::RenderAtomic;
            v15->m_pPrev = m_pPrev;
            m_pNext->m_pPrev->m_pNext = m_pNext->m_pNext;
            v16 = p_m_lastLink->m_firstLink.m_pPrev;
            m_pNext->m_pNext = v16->m_pNext;
            v16->m_pNext->m_pPrev = m_pNext;
            m_pNext->m_pPrev = v16;
            v16->m_pNext = m_pNext;
            if ( !m_pNext )
                LABEL_26:
                AtomicDefaultRenderCallBack(pAtomic);
        }
    }
    return pAtomic;
}

RpAtomic* CVisibilityPlugins::RenderTrainHiDetailCB(RpAtomic* pAtomic)
{
    uint32 v6; // w20
    RwFrame *parent; // x21
    RwMatrix *LTM; // x22
    RwMatrix *v9; // x0
    float DotProductWithCameraVector; // s0

    if ( gVehicleDistanceFromCamera < CVisibilityPlugins::ms_bigVehicleLod0Dist )
    {
        RwUInt8* v2 = &pAtomic->object.object.type + CVisibilityPlugins::ms_atomicPluginOffset;
        uint16_t v3 = *((uint16_t *)v2 + 1);
        uint16_t v4 = v3 & 0xDFFF;
        uint16_t v5 = v3 | 0x2000;
        if ( CVisibilityPlugins::ms_vehicleLod0RenderMultiPassDist > gVehicleDistanceFromCamera )
            v5 = v4;
        *((uint16_t *)v2 + 1) = v5;
        if ( gVehicleDistanceFromCamera <= CVisibilityPlugins::ms_cullCompsDist
             || (v6 = *(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset),
                (v6 & 0x400) != 0)
        || gAngleWithHorizontal >= 0.2
           || (parent = (RwFrame *)pAtomic->clump->object.parent,
               LTM = RwFrameGetLTM((RwFrame *)pAtomic->object.object.parent),
               v9 = RwFrameGetLTM(parent),
               DotProductWithCameraVector = CVisibilityPlugins::GetDotProductWithCameraVector(LTM, v9, v6),
                DotProductWithCameraVector <= 0.0)
           || (v6 & 0x80) == 0
              && (float)(DotProductWithCameraVector * DotProductWithCameraVector) <= (float)(gVehicleDistanceFromCamera * 0.1) )
        {
            AtomicDefaultRenderCallBack(pAtomic);
        }
    }
    return pAtomic;
}

RpAtomic* CVisibilityPlugins::RenderVehicleHiDetailAlphaCB(RpAtomic *pAtomic)
{
    RwUInt8 *v2; // x8
    uint16_t v3; // w9
    uint16_t v4; // w11
    uint16_t v5; // w9
    RwFrame *parent; // x21
    RwMatrix *LTM; // x22
    uint32 v8; // w20
    RwMatrix *v9; // x0
    float DotProductWithCameraVector; // s0
    float v11; // s0
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *p_m_lastLink; // x8
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pNext; // x10
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pPrev; // x11
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v15; // x12
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v16; // x8

    if ( gVehicleDistanceFromCamera < CVisibilityPlugins::ms_vehicleLod0Dist )
    {
        v2 = &pAtomic->object.object.type + CVisibilityPlugins::ms_atomicPluginOffset;
        v3 = *((uint16_t *)v2 + 1);
        v4 = v3 | 0x2000;
        v5 = v3 & 0xDFFF;
        if ( CVisibilityPlugins::ms_vehicleLod0RenderMultiPassDist <= gVehicleDistanceFromCamera )
            v5 = v4;
        *((uint16_t *)v2 + 1) = v5;
        parent = (RwFrame *)pAtomic->clump->object.parent;
        LTM = RwFrameGetLTM((RwFrame *)pAtomic->object.object.parent);
        v8 = *(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset);
        v9 = RwFrameGetLTM(parent);
        DotProductWithCameraVector = CVisibilityPlugins::GetDotProductWithCameraVector(LTM, v9, v8);
        if ( gVehicleDistanceFromCamera <= CVisibilityPlugins::ms_cullCompsDist
             || DotProductWithCameraVector <= 0.0
             || (v8 & 0x400) != 0
             || gAngleWithHorizontal >= 0.2
             || (v8 & 0x80) == 0
                && (float)(DotProductWithCameraVector * DotProductWithCameraVector) <= (float)(gVehicleDistanceFromCamera * 0.1) )
        {
            if ( (v8 & 0x40) != 0 )
            {
                p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink;
                v11 = gVehicleDistanceFromCamera + -0.0001;
                if ( CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext != &CVisibilityPlugins::m_alphaList.m_lastLink )
                {
                    p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext;
                    while ( p_m_lastLink->m_firstLink.item.m_distance < v11 )
                    {
                        p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)p_m_lastLink->m_firstLink.m_pNext;
                        if ( p_m_lastLink == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink )
                        {
                            LABEL_20:
                            p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink;
                            break;
                        }
                    }
                }
            }
            else
            {
                v11 = DotProductWithCameraVector + gVehicleDistanceFromCamera;
                p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink;
                if ( CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext != &CVisibilityPlugins::m_alphaList.m_lastLink )
                {
                    p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext;
                    while ( p_m_lastLink->m_firstLink.item.m_distance < v11 )
                    {
                        p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)p_m_lastLink->m_firstLink.m_pNext;
                        if ( p_m_lastLink == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink )
                            goto LABEL_20;
                    }
                }
            }
            m_pNext = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext;
            if ( CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext == &CVisibilityPlugins::m_alphaList.m_lastFreeLink )
                goto LABEL_23;
            m_pPrev = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->m_pPrev;
            v15 = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->m_pNext;
            CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->item.m_distance = v11;
            m_pNext->item.m_pCallback = pAtomic;
            m_pNext->item.RenderFn = (CVisibilityPlugins::RenderFunction)CVisibilityPlugins::RenderAtomic;
            v15->m_pPrev = m_pPrev;
            m_pNext->m_pPrev->m_pNext = m_pNext->m_pNext;
            v16 = p_m_lastLink->m_firstLink.m_pPrev;
            m_pNext->m_pNext = v16->m_pNext;
            v16->m_pNext->m_pPrev = m_pNext;
            m_pNext->m_pPrev = v16;
            v16->m_pNext = m_pNext;
            if ( !m_pNext )
                LABEL_23:
                AtomicDefaultRenderCallBack(pAtomic);
        }
    }
    return pAtomic;
}

RpAtomic* CVisibilityPlugins::RenderVehicleHiDetailAlphaCB_BigVehicle(RpAtomic *pAtomic)
{
    RwUInt8 *v2; // x8
    uint16_t v3; // w9
    uint16_t v4; // w11
    uint16_t v5; // w9
    RwFrame *parent; // x21
    RwMatrix *LTM; // x22
    uint32 v8; // w20
    RwMatrix *v9; // x0
    float DotProductWithCameraVector; // s0
    float v11; // s0
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *p_m_lastLink; // x8
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pNext; // x10
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pPrev; // x11
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v15; // x12
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v16; // x8

    if ( gVehicleDistanceFromCamera < CVisibilityPlugins::ms_bigVehicleLod0Dist )
    {
        v2 = &pAtomic->object.object.type + CVisibilityPlugins::ms_atomicPluginOffset;
        v3 = *((uint16_t *)v2 + 1);
        v4 = v3 | 0x2000;
        v5 = v3 & 0xDFFF;
        if ( CVisibilityPlugins::ms_vehicleLod0RenderMultiPassDist <= gVehicleDistanceFromCamera )
            v5 = v4;
        *((uint16_t *)v2 + 1) = v5;
        parent = (RwFrame *)pAtomic->clump->object.parent;
        LTM = RwFrameGetLTM((RwFrame *)pAtomic->object.object.parent);
        v8 = *(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset);
        v9 = RwFrameGetLTM(parent);
        DotProductWithCameraVector = CVisibilityPlugins::GetDotProductWithCameraVector(LTM, v9, v8);
        if ( gVehicleDistanceFromCamera <= CVisibilityPlugins::ms_cullBigCompsDist
             || DotProductWithCameraVector <= 0.0
             || (v8 & 0x400) != 0
             || gAngleWithHorizontal >= 0.2
             || (v8 & 0x80) == 0
                && (float)(DotProductWithCameraVector * DotProductWithCameraVector) <= (float)(gVehicleDistanceFromCamera * 0.1) )
        {
            if ( (v8 & 0x40) != 0 )
            {
                p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink;
                v11 = gVehicleDistanceFromCamera + -0.0001;
                if ( CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext != &CVisibilityPlugins::m_alphaList.m_lastLink )
                {
                    p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext;
                    while ( p_m_lastLink->m_firstLink.item.m_distance < v11 )
                    {
                        p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)p_m_lastLink->m_firstLink.m_pNext;
                        if ( p_m_lastLink == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink )
                        {
                            LABEL_20:
                            p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink;
                            break;
                        }
                    }
                }
            }
            else
            {
                v11 = DotProductWithCameraVector + gVehicleDistanceFromCamera;
                p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink;
                if ( CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext != &CVisibilityPlugins::m_alphaList.m_lastLink )
                {
                    p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaList.m_firstLink.m_pNext;
                    while ( p_m_lastLink->m_firstLink.item.m_distance < v11 )
                    {
                        p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)p_m_lastLink->m_firstLink.m_pNext;
                        if ( p_m_lastLink == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaList.m_lastLink )
                            goto LABEL_20;
                    }
                }
            }
            m_pNext = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext;
            if ( CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext == &CVisibilityPlugins::m_alphaList.m_lastFreeLink )
                goto LABEL_23;
            m_pPrev = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->m_pPrev;
            v15 = CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->m_pNext;
            CVisibilityPlugins::m_alphaList.m_firstFreeLink.m_pNext->item.m_distance = v11;
            m_pNext->item.m_pCallback = pAtomic;
            m_pNext->item.RenderFn = (CVisibilityPlugins::RenderFunction)CVisibilityPlugins::RenderAtomic;
            v15->m_pPrev = m_pPrev;
            m_pNext->m_pPrev->m_pNext = m_pNext->m_pNext;
            v16 = p_m_lastLink->m_firstLink.m_pPrev;
            m_pNext->m_pNext = v16->m_pNext;
            v16->m_pNext->m_pPrev = m_pNext;
            m_pNext->m_pPrev = v16;
            v16->m_pNext = m_pNext;
            if ( !m_pNext )
                LABEL_23:
                AtomicDefaultRenderCallBack(pAtomic);
        }
    }
    return pAtomic;
}

RpAtomic * CVisibilityPlugins::RenderVehicleHiDetailAlphaCB_Boat(RpAtomic *pAtomic)
{
    RwUInt8 *v2; // x10
    uint16_t v3; // w11
    uint16_t v4; // w13
    uint16_t v5; // w11
    CLinkList<CVisibilityPlugins::AlphaObjectInfo> *p_m_lastLink; // x8
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pNext; // x10
    CLink<CVisibilityPlugins::AlphaObjectInfo> *m_pPrev; // x11
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v9; // x12
    CLink<CVisibilityPlugins::AlphaObjectInfo> *v10; // x8

    if ( gVehicleDistanceFromCamera < CVisibilityPlugins::ms_vehicleLod0Dist )
    {
        v2 = &pAtomic->object.object.type + CVisibilityPlugins::ms_atomicPluginOffset;
        v3 = *((uint16_t *)v2 + 1);
        v4 = v3 | 0x2000;
        v5 = v3 & 0xDFFF;
        if ( CVisibilityPlugins::ms_vehicleLod0RenderMultiPassDist <= gVehicleDistanceFromCamera )
            v5 = v4;
        *((uint16_t *)v2 + 1) = v5;
        if ( (*(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset) & 0x40) == 0 )
            goto LABEL_12;
        p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaBoatAtomicList.m_lastLink;
        if ( CVisibilityPlugins::m_alphaBoatAtomicList.m_firstLink.m_pNext != &CVisibilityPlugins::m_alphaBoatAtomicList.m_lastLink )
        {
            p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaBoatAtomicList.m_firstLink.m_pNext;
            while ( p_m_lastLink->m_firstLink.item.m_distance < gVehicleDistanceFromCamera )
            {
                p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)p_m_lastLink->m_firstLink.m_pNext;
                if ( p_m_lastLink == (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaBoatAtomicList.m_lastLink )
                {
                    p_m_lastLink = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)&CVisibilityPlugins::m_alphaBoatAtomicList.m_lastLink;
                    break;
                }
            }
        }
        m_pNext = CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink.m_pNext;
        if ( CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink.m_pNext == &CVisibilityPlugins::m_alphaBoatAtomicList.m_lastFreeLink )
            goto LABEL_12;
        m_pPrev = CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink.m_pNext->m_pPrev;
        v9 = CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink.m_pNext->m_pNext;
        CVisibilityPlugins::m_alphaBoatAtomicList.m_firstFreeLink.m_pNext->item.m_distance = gVehicleDistanceFromCamera;
        m_pNext->item.m_pCallback = pAtomic;
        m_pNext->item.RenderFn = (CVisibilityPlugins::RenderFunction)CVisibilityPlugins::RenderAtomic;
        v9->m_pPrev = m_pPrev;
        m_pNext->m_pPrev->m_pNext = m_pNext->m_pNext;
        v10 = p_m_lastLink->m_firstLink.m_pPrev;
        m_pNext->m_pNext = v10->m_pNext;
        v10->m_pNext->m_pPrev = m_pNext;
        m_pNext->m_pPrev = v10;
        v10->m_pNext = m_pNext;
        if ( !m_pNext )
            LABEL_12:
            AtomicDefaultRenderCallBack(pAtomic);
    }
    return pAtomic;
}

RpAtomic* CVisibilityPlugins::RenderVehicleHiDetailCB(RpAtomic *pAtomic)
{
    RwUInt8 *v2; // x9
    uint16_t v3; // w10
    uint16_t v4; // w11
    uint16_t v5; // w10
    uint32 v6; // w20
    RwFrame *parent; // x21
    RwMatrix *LTM; // x22
    RwMatrix *v9; // x0
    float DotProductWithCameraVector; // s0

    if ( gVehicleDistanceFromCamera < CVisibilityPlugins::ms_vehicleLod0Dist )
    {
        v2 = &pAtomic->object.object.type + CVisibilityPlugins::ms_atomicPluginOffset;
        v3 = *((uint16_t *)v2 + 1);
        v4 = v3 & 0xDFFF;
        v5 = v3 | 0x2000;
        if ( CVisibilityPlugins::ms_vehicleLod0RenderMultiPassDist > gVehicleDistanceFromCamera )
            v5 = v4;
        *((uint16_t *)v2 + 1) = v5;
        if ( gVehicleDistanceFromCamera <= CVisibilityPlugins::ms_cullCompsDist
             || (v6 = *(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset),
                (v6 & 0x400) != 0)
        || gAngleWithHorizontal >= 0.2
           || (parent = (RwFrame *)pAtomic->clump->object.parent,
               LTM = RwFrameGetLTM((RwFrame *)pAtomic->object.object.parent),
               v9 = RwFrameGetLTM(parent),
               DotProductWithCameraVector = CVisibilityPlugins::GetDotProductWithCameraVector(LTM, v9, v6),
                DotProductWithCameraVector <= 0.0)
           || (v6 & 0x80) == 0
              && (float)(DotProductWithCameraVector * DotProductWithCameraVector) <= (float)(gVehicleDistanceFromCamera * 0.1) )
        {
            AtomicDefaultRenderCallBack(pAtomic);
        }
    }
    return pAtomic;
}

RpAtomic* CVisibilityPlugins::RenderVehicleHiDetailCB_BigVehicle(RpAtomic *pAtomic)
{
    RwUInt8 *v2; // x9
    uint16_t v3; // w10
    uint16_t v4; // w11
    uint16_t v5; // w10
    RwFrame *parent; // x20
    RwMatrix *LTM; // x21
    RwMatrix *v8; // x0

    if ( gVehicleDistanceFromCamera < CVisibilityPlugins::ms_bigVehicleLod0Dist )
    {
        v2 = &pAtomic->object.object.type + CVisibilityPlugins::ms_atomicPluginOffset;
        v3 = *((uint16_t *)v2 + 1);
        v4 = v3 & 0xDFFF;
        v5 = v3 | 0x2000;
        if ( CVisibilityPlugins::ms_vehicleLod0RenderMultiPassDist > gVehicleDistanceFromCamera )
            v5 = v4;
        *((uint16_t *)v2 + 1) = v5;
        if ( gVehicleDistanceFromCamera <= CVisibilityPlugins::ms_cullBigCompsDist
             || (*(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset) & 0x400) != 0
             || gAngleWithHorizontal >= 0.2
             || (parent = (RwFrame *)pAtomic->clump->object.parent,
                 LTM = RwFrameGetLTM((RwFrame *)pAtomic->object.object.parent),
                 v8 = RwFrameGetLTM(parent),
                CVisibilityPlugins::GetDotProductWithCameraVector(
                        LTM,
                        v8,
                        *(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset)) <= 0.0) )
        {
            AtomicDefaultRenderCallBack(pAtomic);
        }
    }
    return pAtomic;
}

RpAtomic * CVisibilityPlugins::RenderVehicleHiDetailCB_Boat(RpAtomic *pAtomic)
{
    RwUInt8 *v2; // x8
    uint16_t v3; // w9
    uint16_t v4; // w11
    uint16_t v5; // w9

    if ( gVehicleDistanceFromCamera < CVisibilityPlugins::ms_vehicleLod0Dist )
    {
        v2 = &pAtomic->object.object.type + CVisibilityPlugins::ms_atomicPluginOffset;
        v3 = *((uint16_t *)v2 + 1);
        v4 = v3 | 0x2000;
        v5 = v3 & 0xDFFF;
        if ( CVisibilityPlugins::ms_vehicleLod0RenderMultiPassDist <= gVehicleDistanceFromCamera )
            v5 = v4;
        *((uint16_t *)v2 + 1) = v5;
        AtomicDefaultRenderCallBack(pAtomic);
    }
    return pAtomic;
}

RpAtomic * CVisibilityPlugins::RenderVehicleLoDetailCB_Boat(RpAtomic *pAtomic)
{
    if ( gVehicleDistanceFromCamera >= CVisibilityPlugins::ms_vehicleLod0Dist )
    {
        *(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset) |= 0x2000u;
        int v2 = *(uint32_t *)(&pAtomic->clump->object.privateFlags + CVisibilityPlugins::ms_clumpPluginOffset + 1);
        if ( v2 == 255 )
        {
            AtomicDefaultRenderCallBack(pAtomic);
        }
        else
        {
            //emu_EnableAlphaModulate((float)v6 / 255.0);
            CHook::CallFunction<void>("_Z23emu_EnableAlphaModulatef", (float)v2 / 255.0);
            AtomicDefaultRenderCallBack(pAtomic);
            //emu_DisableAlphaModulate();
            CHook::CallFunction<void>("_Z24emu_DisableAlphaModulatev");
        }
    }
    return pAtomic;
}

RpAtomic * CVisibilityPlugins::RenderVehicleReallyLowDetailCB(RpAtomic *pAtomic)
{
    if ( gVehicleDistanceFromCamera >= CVisibilityPlugins::ms_vehicleLod0Dist )
    {
        *(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset) |= 0x2000u;
        int v2 = *(uint32_t *)(&pAtomic->clump->object.privateFlags + CVisibilityPlugins::ms_clumpPluginOffset + 1);
        if ( v2 == 255 )
        {
            AtomicDefaultRenderCallBack(pAtomic);
        }
        else
        {
            //emu_EnableAlphaModulate((float)v6 / 255.0);
            CHook::CallFunction<void>("_Z23emu_EnableAlphaModulatef", (float)v2 / 255.0);
            AtomicDefaultRenderCallBack(pAtomic);
            //emu_DisableAlphaModulate();
            CHook::CallFunction<void>("_Z24emu_DisableAlphaModulatev");
        }
    }
    return pAtomic;
}

RpAtomic* CVisibilityPlugins::RenderVehicleReallyLowDetailCB_BigVehicle(RpAtomic* pAtomic)
{
    if ( gVehicleDistanceFromCamera >= CVisibilityPlugins::ms_bigVehicleLod0Dist )
        AtomicDefaultRenderCallBack(pAtomic);
    return pAtomic;
}

RpAtomic* CVisibilityPlugins::RenderWeaponCB(RpAtomic* pAtomic)
{
    /*CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

    RwFrame *parent; // x0
    uint16_t v3; // x20
    RwMatrix *LTM; // x0
    float32x2_t v5; // d1
    uint64_t v6; // d1

    parent = (RwFrame *)pAtomic->clump->object.parent;
    v3 = *(uint64_t *)(&parent->object.type + CVisibilityPlugins::ms_framePluginOffset);
    LTM = RwFrameGetLTM(parent);
    v5.n64_u64[0] = vsub_f32(*(float32x2_t *)&LTM->pos.y, *(float32x2_t *)&CVisibilityPlugins::ms_pCameraPosn->y).n64_u64[0];
    v6 = vmul_f32(v5, v5).n64_u64[0];
    if ( (float)((float)(*(float *)&v6
                         + (float)((float)(LTM->pos.x - CVisibilityPlugins::ms_pCameraPosn->x)
                                   * (float)(LTM->pos.x - CVisibilityPlugins::ms_pCameraPosn->x)))
                 + *((float *)&v6 + 1)) < (float)((float)(*(float *)(v3 + 56) * TheCamera.m_fLODDistMultiplier)
                                                  * (float)(*(float *)(v3 + 56) * TheCamera.m_fLODDistMultiplier)) )
        AtomicDefaultRenderCallBack(pAtomic);
    return pAtomic;
*/
    CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

    RwFrame* frame = reinterpret_cast<RwFrame*>(pAtomic->clump->object.parent);
    RwMatrix* LTM = RwFrameGetLTM(frame);

    float dx = LTM->pos.x - CVisibilityPlugins::ms_pCameraPosn->x;
    float dy = LTM->pos.y - CVisibilityPlugins::ms_pCameraPosn->y;
    float dz = LTM->pos.z - CVisibilityPlugins::ms_pCameraPosn->z;

    float distSquared = dx * dx + dy * dy + dz * dz;

    void* pluginData = reinterpret_cast<uint8_t*>(&frame->object) + CVisibilityPlugins::ms_framePluginOffset;

    float lodDistance = *(float*)((uintptr_t)pluginData + 56);

    float effectiveDist = lodDistance * TheCamera.m_fLODDistMultiplier;

    if (distSquared < effectiveDist * effectiveDist) {
        AtomicDefaultRenderCallBack(pAtomic);
    }

    return pAtomic;
}

void CVisibilityPlugins::RenderWeaponPedsForPC()
{
    /*CLinkList<CPed *> *m_pPrev; // x19
    CPed *item; // x23
    RwInt32 v2; // w24
    RpHAnimHierarchy *AnimHierarchyFromSkinClump; // x25
    RwMatrix *MatrixArray; // x26
    uint64_t *p_x; // x9
    char *parent; // x24
    eWeaponType m_eWeaponType; // w27
    int8 WeaponSkill; // w0
    RwMatrix *v9; // x27
    uint64_t *v10; // x8

    m_pPrev = (CLinkList<CPed *> *)CVisibilityPlugins::ms_weaponPedsForPC.m_lastLink.m_pPrev;
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,          RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,         RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATEFOGENABLE,            RWRSTATE(TRUE));
    RwRenderStateSet(rwRENDERSTATESRCBLEND,             RWRSTATE(rwBLENDSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,            RWRSTATE(rwBLENDINVSRCALPHA));
    RwRenderStateSet(rwRENDERSTATEALPHATESTFUNCTIONREF, RWRSTATE(20));
    for ( ; m_pPrev != &CVisibilityPlugins::ms_weaponPedsForPC; m_pPrev = (CLinkList<CPed *> *)m_pPrev->m_firstLink.m_pPrev )
    {
        item = m_pPrev->m_firstLink.item;
        if ( m_pPrev->m_firstLink.item && item->m_pWeaponClump )
        {
            (*((void (__fastcall **)(CPed *))item->_vptr$CPlaceable + 20))(m_pPrev->m_firstLink.item);
            if ( item->m_WeaponSlots[item->m_nCurrentWeapon].m_eWeaponType == WEAPONTYPE_PARACHUTE )
                v2 = 3;
            else
                v2 = 24;
            AnimHierarchyFromSkinClump = GetAnimHierarchyFromSkinClump((RpClump *)item->m_pRwObject);
            MatrixArray = RpHAnimHierarchyGetMatrixArray(AnimHierarchyFromSkinClump);
            p_x = (uint64_t *)&MatrixArray[(uint64_t)RpHAnimIDGetIndex(AnimHierarchyFromSkinClump, v2)].right.x;
            parent = (char *)item->m_pWeaponClump->object.parent;
            *((uint64_t *)parent + 5) = p_x[3];
            *((uint64_t *)parent + 4) = p_x[2];
            *((uint64_t *)parent + 3) = p_x[1];
            *((uint64_t *)parent + 2) = *p_x;
            if ( item->m_WeaponSlots[item->m_nCurrentWeapon].m_eWeaponType == WEAPONTYPE_PARACHUTE )
            {
                RwMatrixTranslate(
                        (RwMatrix *)(parent + 32),
                        &CVisibilityPlugins::RenderWeaponPedsForPC(void)::parachuteOffset,
                        rwCOMBINEPRECONCAT);
                RwMatrixRotate((RwMatrix *)(parent + 32), &CPedIK::YaxisIK, 90.0, rwCOMBINEPRECONCAT);
            }
            CPed::SetGunFlashAlpha(item, 0);
            RwFrameUpdateObjects((RwFrame *)parent);
            RpClumpRender(item->m_pWeaponClump);
            m_eWeaponType = item->m_WeaponSlots[item->m_nCurrentWeapon].m_eWeaponType;
            WeaponSkill = CPed::GetWeaponSkill(item);
            if ( (CWeaponInfo::GetWeaponInfo(m_eWeaponType, WeaponSkill)->m_nFlags & 0x800) != 0 )
            {
                v9 = RpHAnimHierarchyGetMatrixArray(AnimHierarchyFromSkinClump);
                v10 = (uint64_t *)&v9[(uint64_t)RpHAnimIDGetIndex(AnimHierarchyFromSkinClump, 34)].right.x;
                *((uint64_t *)parent + 5) = v10[3];
                *((uint64_t *)parent + 4) = v10[2];
                *((uint64_t *)parent + 3) = v10[1];
                *((uint64_t *)parent + 2) = *v10;
                RwMatrixRotate(
                        (RwMatrix *)(parent + 32),
                        &CVisibilityPlugins::RenderWeaponPedsForPC(void)::axis,
                        180.0,
                        rwCOMBINEPRECONCAT);
                RwMatrixTranslate(
                        (RwMatrix *)(parent + 32),
                        &CVisibilityPlugins::RenderWeaponPedsForPC(void)::trans,
                        rwCOMBINEPRECONCAT);
                CPed::SetGunFlashAlpha(item, 1);
                RwFrameUpdateObjects((RwFrame *)parent);
                RpClumpRender(item->m_pWeaponClump);
            }
            CPed::ResetGunFlashAlpha(item);
        }
    }*/
}

RpAtomic* CVisibilityPlugins::RenderWheelAtomicCB(RpAtomic *pAtomic)
{
    AtomicDefaultRenderCallBack(pAtomic);
    return pAtomic;
}

void CVisibilityPlugins::ResetRenderFadingAtomic(CBaseModelInfo *pModelInfo)
{
    if ( (pModelInfo->m_nFlags & 4) != 0 )
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDINVSRCALPHA));
}

void CVisibilityPlugins::SetAtomicFlag(RpAtomic* pAtomic, uint16 flag)
{
    *(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset) |= flag;
}

void CVisibilityPlugins::SetAtomicId(RpAtomic* pAtomic, int32 id)
{
    *(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset) = id;
}

void CVisibilityPlugins::SetAtomicRenderCallback(RpAtomic* pAtomic, RpAtomicCallBackRender renderCB)
{
    RpAtomic *(*v2)(RpAtomic *); // x8

    v2 = AtomicDefaultRenderCallBack;
    if ( renderCB )
        v2 = renderCB;
    pAtomic->renderCallBack = v2;
}

void CVisibilityPlugins::SetClumpAlpha(RpClump *pClump, int32 alpha)
{
    *(uint32_t *)(&pClump->object.privateFlags + CVisibilityPlugins::ms_clumpPluginOffset + 1) = alpha;
}

RpAtomic* setClumpForAllAtomicsFlagCB(RpAtomic* pAtomic, unsigned short data)
{
    *(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset) |= (uint16_t)data;
    return pAtomic;
}

void CVisibilityPlugins::SetClumpForAllAtomicsFlag(RpClump* pClump, int32 id)
{
    RpClumpForAllAtomics(pClump, reinterpret_cast<RpAtomicCallBack>(setClumpForAllAtomicsFlagCB), (void *)id);
}

void CVisibilityPlugins::SetClumpModelInfo(RpClump *pClump, CClumpModelInfo *pModelInfo)
{
    /*RwBool (__fastcall *v6)(RpClump *); // x8

    *(uint64_t *)((char *)pClump->object.parent + CVisibilityPlugins::ms_framePluginOffset) = pModelInfo;
    if ( (*((uint8_t (__fastcall **)(CClumpModelInfo *))pModelInfo->_vptr$CBaseModelInfo + 5))(pModelInfo) == 6 )
    {
        int v4 = *(uint32_t *)&pModelInfo[1].m_modelName[16];
        bool v5 = v4 == 8 || v4 == 6;
        v6 = CVisibilityPlugins::VehicleVisibilityCB_BigVehicle;
        if ( !v5 )
            v6 = CVisibilityPlugins::VehicleVisibilityCB;
        *(uint64_t *)(&pClump->object.type + CVisibilityPlugins::ms_clumpPluginOffset) = v6;
    }*/
}

void CVisibilityPlugins::SetFrameHierarchyId(RwFrame* pFrame, uint64_t value)
{
    *(uint64_t *)(&pFrame->object.type + CVisibilityPlugins::ms_framePluginOffset) = value;
}

void CVisibilityPlugins::SetModelInfoIndex(RpAtomic* pAtomic, int32 index)
{
    *(uint16_t *)(&pAtomic->object.object.type + CVisibilityPlugins::ms_atomicPluginOffset) = index;
}

void CVisibilityPlugins::SetRenderWareCamera(RwCamera *pCamera)
{
    /*
    CCamera& TheCamera = *reinterpret_cast<CCamera*>(g_libGTASA + (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

    CVisibilityPlugins::ms_pCamera = pCamera;
    char* parent = (char *)pCamera->object.object.parent;
    CVisibilityPlugins::ms_cullCompsDist = (float)(TheCamera.m_fLODDistMultiplier * 20.0)
                                           * (float)(TheCamera.m_fLODDistMultiplier * 20.0);
    CVisibilityPlugins::ms_cullBigCompsDist = (float)(TheCamera.m_fLODDistMultiplier * 50.0)
                                              * (float)(TheCamera.m_fLODDistMultiplier * 50.0);
    CVisibilityPlugins::ms_vehicleLod0RenderMultiPassDist = (float)((float)(TheCamera.GenerationDistMultiplier * 45.0)
                                                                    * (float)(TheCamera.GenerationDistMultiplier * 45.0))
                                                            + (float)((float)(TheCamera.GenerationDistMultiplier * 45.0)
                                                                      * (float)(TheCamera.GenerationDistMultiplier * 45.0));
    CVisibilityPlugins::ms_pCameraPosn = (RwV3d *)(parent + 80);
    CVisibilityPlugins::ms_vehicleLod0Dist = (float)((float)(TheCamera.GenerationDistMultiplier * 70.0)
                                                     * (float)(TheCamera.GenerationDistMultiplier * 70.0))
                                             + (float)((float)(TheCamera.GenerationDistMultiplier * 70.0)
                                                       * (float)(TheCamera.GenerationDistMultiplier * 70.0));
    CVisibilityPlugins::ms_vehicleLod1Dist = (float)((float)(TheCamera.GenerationDistMultiplier * 150.0)
                                                     * (float)(TheCamera.GenerationDistMultiplier * 150.0))
                                             + (float)((float)(TheCamera.GenerationDistMultiplier * 150.0)
                                                       * (float)(TheCamera.GenerationDistMultiplier * 150.0));
    CVisibilityPlugins::ms_bigVehicleLod0Dist = CVisibilityPlugins::ms_vehicleLod1Dist;
    uint64_t v2 = 4LL * (CPauseMenu::Settings[0].value == 0);
    float v3 = *(float *)((char *)(g_libGTASA + 0x764F98) + v2);
    CVisibilityPlugins::ms_pedLodDist = (float)((float)(TheCamera.m_fLODDistMultiplier * *(float *)((char *)(g_libGTASA + 0x764F90) + v2))
                                                * (float)(TheCamera.m_fLODDistMultiplier * *(float *)((char *)(g_libGTASA + 0x764F90) + v2)))
                                        + (float)((float)(TheCamera.m_fLODDistMultiplier * *(float *)((char *)(g_libGTASA + 0x764F90) + v2))
                                                  * (float)(TheCamera.m_fLODDistMultiplier * *(float *)((char *)(g_libGTASA + 0x764F90) + v2)));
    CVisibilityPlugins::ms_pedFadeDist = (float)((float)(TheCamera.m_fLODDistMultiplier * v3)
                                                 * (float)(TheCamera.m_fLODDistMultiplier * v3))
                                         + (float)((float)(TheCamera.m_fLODDistMultiplier * v3)
                                                   * (float)(TheCamera.m_fLODDistMultiplier * v3));*/
    CHook::CallFunction<void>("_ZN18CVisibilityPlugins19SetRenderWareCameraEP8RwCamera", pCamera);
}

void CVisibilityPlugins::SetUserValue(RpAtomic* pAtomic, uint16 value)
{
    *(uint16_t *)(&pAtomic->object.object.flags + CVisibilityPlugins::ms_atomicPluginOffset) = value;
}

void CVisibilityPlugins::SetupRenderFadingAtomic(CBaseModelInfo *pModelInfo, int32 alpha)
{
    if ( (pModelInfo->m_nFlags & 4) != 0 )
        RwRenderStateSet(rwRENDERSTATEDESTBLEND, RWRSTATE(rwBLENDONE));
}

void CVisibilityPlugins::SetupVehicleVariables(RpClump *pClump)
{
   /* if ( pClump->object.type == 2 )
    {
        RwFrame* parent = (RwFrame *)pClump->object.parent;
        RwMatrix* LTM = RwFrameGetLTM(parent);
        float y = CVisibilityPlugins::ms_pCameraPosn->y;
        float z = CVisibilityPlugins::ms_pCameraPosn->z;
        gVehicleDistanceFromCamera = (float)((float)((float)(LTM->pos.y - y) * (float)(LTM->pos.y - y))
                                             + (float)((float)(LTM->pos.x - CVisibilityPlugins::ms_pCameraPosn->x)
                                                       * (float)(LTM->pos.x - CVisibilityPlugins::ms_pCameraPosn->x)))
                                     + (float)((float)(LTM->pos.z - z) * (float)(LTM->pos.z - z));
        gAngleWithHorizontal = atan2f(
                z - parent->modelling.pos.z,
                sqrtf(
                        (float)((float)(y - parent->modelling.pos.y) * (float)(y - parent->modelling.pos.y))
                        + (float)((float)(CVisibilityPlugins::ms_pCameraPosn->x - parent->modelling.pos.x)
                                  * (float)(CVisibilityPlugins::ms_pCameraPosn->x - parent->modelling.pos.x))));
    }*/
   CHook::CallFunction<void>("_ZN18CVisibilityPlugins21SetupVehicleVariablesEP7RpClump", pClump);
}

void CVisibilityPlugins::Shutdown()
{
    if ( CVisibilityPlugins::m_alphaList.m_pStore )
        operator delete[](CVisibilityPlugins::m_alphaList.m_pStore);
    CVisibilityPlugins::m_alphaList.m_pStore = 0LL;
    if ( CVisibilityPlugins::m_alphaBoatAtomicList.m_pStore )
        operator delete[](CVisibilityPlugins::m_alphaBoatAtomicList.m_pStore);
    CVisibilityPlugins::m_alphaBoatAtomicList.m_pStore = 0LL;
    if ( CVisibilityPlugins::m_alphaEntityList.m_pStore )
        operator delete[](CVisibilityPlugins::m_alphaEntityList.m_pStore);
    CVisibilityPlugins::m_alphaEntityList.m_pStore = 0LL;
    if ( CVisibilityPlugins::m_alphaUnderwaterEntityList.m_pStore )
        operator delete[](CVisibilityPlugins::m_alphaUnderwaterEntityList.m_pStore);
    CVisibilityPlugins::m_alphaUnderwaterEntityList.m_pStore = 0LL;
    if ( CVisibilityPlugins::m_alphaReallyDrawLastList.m_pStore )
        operator delete[](CVisibilityPlugins::m_alphaReallyDrawLastList.m_pStore);
    CVisibilityPlugins::m_alphaReallyDrawLastList.m_pStore = 0LL;
    if ( CVisibilityPlugins::ms_weaponPedsForPC.m_pStore )
        operator delete[](CVisibilityPlugins::ms_weaponPedsForPC.m_pStore);
    CVisibilityPlugins::ms_weaponPedsForPC.m_pStore = 0LL;
}

RwBool CVisibilityPlugins::VehicleVisibilityCB(RpClump *pClump)
{
    if (!pClump || !pClump->object.parent || !CVisibilityPlugins::ms_pCameraPosn)
        return false;

    RwFrame* frame = reinterpret_cast<RwFrame*>(pClump->object.parent);
    RwMatrix* LTM = RwFrameGetLTM(frame);
    if (!LTM)
        return false;

    float dx = LTM->pos.x - CVisibilityPlugins::ms_pCameraPosn->x;
    float dy = LTM->pos.y - CVisibilityPlugins::ms_pCameraPosn->y;
    float dz = LTM->pos.z - CVisibilityPlugins::ms_pCameraPosn->z;
    float distSquared = dx * dx + dy * dy + dz * dz;

    if (distSquared > CVisibilityPlugins::ms_vehicleLod1Dist)
        return false;

    uintptr_t pluginData = *reinterpret_cast<uintptr_t*>(
            reinterpret_cast<uint8_t*>(&frame->object.type) + CVisibilityPlugins::ms_framePluginOffset
    );

    uintptr_t modelInfoPtr = *reinterpret_cast<uintptr_t*>(pluginData + 48);
    if (!modelInfoPtr)
        return false;

    RwSphere boundingSphere = *reinterpret_cast<RwSphere*>(modelInfoPtr + 24);
    RwV3dTransformPoints(&boundingSphere.center, &boundingSphere.center, 1, LTM);
    return RwCameraFrustumTestSphere(CVisibilityPlugins::ms_pCamera, &boundingSphere) != rwSPHEREOUTSIDE;
/*
    float32x2_t v3; // d1

    RwMatrix* LTM = RwFrameGetLTM((RwFrame *)pClump->object.parent);
    v3.n64_u64[0] = vsub_f32(*(float32x2_t *)&LTM->pos.y, *(float32x2_t *)&CVisibilityPlugins::ms_pCameraPosn->y).n64_u64[0];
    uint64_t v4 = vmul_f32(v3, v3).n64_u64[0];
    if ( (float)((float)(*(float *)&v4
                         + (float)((float)(LTM->pos.x - CVisibilityPlugins::ms_pCameraPosn->x)
                                   * (float)(LTM->pos.x - CVisibilityPlugins::ms_pCameraPosn->x)))
                 + *((float *)&v4 + 1)) > CVisibilityPlugins::ms_vehicleLod1Dist )
        return 0;
    RwFrame* parent = (RwFrame *)pClump->object.parent;
    RwSphere v8 = *(RwSphere *)(*(uint64_t *)(*(uint64_t *)(&parent->object.type + CVisibilityPlugins::ms_framePluginOffset) + 48LL)
                         + 24LL);
    RwMatrix* v7 = RwFrameGetLTM(parent);
    RwV3dTransformPoints(&v8.center, &v8.center, 1, v7);
    return RwCameraFrustumTestSphere(CVisibilityPlugins::ms_pCamera, &v8) != rwSPHEREOUTSIDE;*/
}

RwBool CVisibilityPlugins::VehicleVisibilityCB_BigVehicle(RpClump *pClump)
{
    RwFrame* parent = (RwFrame *)pClump->object.parent;
    RwSphere v4 = *(RwSphere *)(*(uint64_t *)(*(uint64_t *)(&parent->object.type + CVisibilityPlugins::ms_framePluginOffset) + 48LL)
                         + 24LL);
    RwMatrix* LTM = RwFrameGetLTM(parent);
    RwV3dTransformPoints(&v4.center, &v4.center, 1, LTM);
    return RwCameraFrustumTestSphere(CVisibilityPlugins::ms_pCamera, &v4) != rwSPHEREOUTSIDE;
}

float CVisibilityPlugins::GetDistanceSquaredFromCamera(RwFrame* frame) {
    RwMatrix* transformMatrix = RwFrameGetLTM(frame);
    CVector distance;
    RwV3dSub(&distance, &transformMatrix->pos, ms_pCameraPosn);
    return distance.SquaredMagnitude();
}

void CVisibilityPlugins::RenderAlphaAtomic(RpAtomic* atomic, int32 alpha) {
    //CLinkList<CVisibilityPlugins::AlphaObjectInfo> *i; // x20
//
    //for ( i = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)CVisibilityPlugins::m_alphaList.m_lastLink.m_pPrev;
   //       i != &CVisibilityPlugins::m_alphaList;
    //      i = (CLinkList<CVisibilityPlugins::AlphaObjectInfo> *)i->m_firstLink.m_pPrev )
    //{
    //    i->m_firstLink.item.RenderFn(i->m_firstLink.item.pObj, i->m_firstLink.item.dist);
   // }
    CHook::CallFunction<void>(g_libGTASA + (VER_x32 ? 0x005D6D20 + 1 : 0x6FC1A8), atomic, alpha);
}

void CVisibilityPlugins::InjectHooks() {
    // все что юз в гтаса
    //CHook::Redirect("_ZN18CVisibilityPlugins26CalculateFadingAtomicAlphaEP14CBaseModelInfoP7CEntityf", &CVisibilityPlugins::CalculateFadingAtomicAlpha);
   // CHook::Redirect("_ZN18CVisibilityPlugins15ClearAtomicFlagEP8RpAtomict", &CVisibilityPlugins::ClearAtomicFlag);
   // CHook::Redirect("_ZN18CVisibilityPlugins27ClearClumpForAllAtomicsFlagEP7RpClumpi", &CVisibilityPlugins::ClearClumpForAllAtomicsFlag);

    // new
    //CHook::Redirect("_ZN18CVisibilityPlugins11GetAtomicIdEP8RpAtomic", &CVisibilityPlugins::GetAtomicId);
   // CHook::Redirect("_ZN18CVisibilityPlugins13GetClumpAlphaEP7RpClump", &CVisibilityPlugins::GetClumpAlpha);
   // CHook::Redirect("_ZN18CVisibilityPlugins17GetClumpModelInfoEP7RpClump", &CVisibilityPlugins::GetClumpModelInfo);
   // CHook::Redirect("_ZN18CVisibilityPlugins11GetAtomicIdEP8RpAtomic", &CVisibilityPlugins::GetAtomicId);
   // CHook::Redirect("_ZN18CVisibilityPlugins13GetClumpAlphaEP7RpClump", &CVisibilityPlugins::GetClumpAlpha);

    /*CVisibilityPlugins::ms_pedLodDist = *(float*)(g_libGTASA + (VER_x32 ? 0x00676200 : 0x84A480));
    CVisibilityPlugins::ms_pCameraPosn = *(RwV3d**)(g_libGTASA + (VER_x32 ? 0x00676530 : 0x84AAD0));

    CVisibilityPlugins::ms_pCamera = *(RwCamera**)(g_libGTASA + 0xD28618);

    CVisibilityPlugins::ms_bigVehicleLod0Dist = *(float*)(g_libGTASA + 0xD2897C);
    CVisibilityPlugins::ms_vehicleLod0Dist = *(float*)(g_libGTASA + 0xD28974);
    CVisibilityPlugins::ms_vehicleLod0RenderMultiPassDist = *(float*)(g_libGTASA + 0xD28970);
    CVisibilityPlugins::ms_cullCompsDist = *(float*)(g_libGTASA + 0xD28988);
    CVisibilityPlugins::ms_cullBigCompsDist = *(float*)(g_libGTASA + 0xD2898C);
    CVisibilityPlugins::ms_vehicleLod1Dist = *(float*)(g_libGTASA + 0xD28978);
    CVisibilityPlugins::ms_pedFadeDist = *(float*)(g_libGTASA + 0xD28984);

    CVisibilityPlugins::ms_atomicPluginOffset = *(RwInt32*)(g_libGTASA + 0x88F624);
    CVisibilityPlugins::ms_framePluginOffset = *(RwInt32*)(g_libGTASA + 0x88F628);
    CVisibilityPlugins::ms_clumpPluginOffset = *(RwInt32*)(g_libGTASA + 0x88F62C);*/

    /*  CHook::Write(g_libGTASA + (VER_x32 ? 0x00676200 : 0x84A480), &CVisibilityPlugins::ms_pedLodDist);
      CHook::Write(g_libGTASA + (VER_x32 ? 0x00676530 : 0x84AAD0), &CVisibilityPlugins::ms_pCameraPosn);

      CHook::Write(g_libGTASA + 0xD28618, &CVisibilityPlugins::ms_pCamera);

      CHook::Write(g_libGTASA + 0xD2897C, &CVisibilityPlugins::ms_bigVehicleLod0Dist);
      CHook::Write(g_libGTASA + 0xD28974, &CVisibilityPlugins::ms_vehicleLod0Dist);
      CHook::Write(g_libGTASA + 0xD28970, &CVisibilityPlugins::ms_vehicleLod0RenderMultiPassDist);
      CHook::Write(g_libGTASA + 0xD28988, &CVisibilityPlugins::ms_cullCompsDist);
      CHook::Write(g_libGTASA + 0xD2898C, &CVisibilityPlugins::ms_cullBigCompsDist);
      CHook::Write(g_libGTASA + 0xD28978, &CVisibilityPlugins::ms_vehicleLod1Dist);
      CHook::Write(g_libGTASA + 0xD28978, &CVisibilityPlugins::ms_pedFadeDist);

      CHook::Write(g_libGTASA + 0x88F624, &CVisibilityPlugins::ms_atomicPluginOffset);
      CHook::Write(g_libGTASA + 0x88F628, &CVisibilityPlugins::ms_framePluginOffset);
      CHook::Write(g_libGTASA + 0x88F62C, &CVisibilityPlugins::ms_clumpPluginOffset);*/

    /*  CHook::Write(g_libGTASA + 0x84DAF0, &CVisibilityPlugins::m_alphaEntityList);
      CHook::Write(g_libGTASA + 0xD28628, &CVisibilityPlugins::m_alphaList);
      CHook::Write(g_libGTASA + 0x84B260, &CVisibilityPlugins::m_alphaBoatAtomicList);
      CHook::Write(g_libGTASA + 0x849F90, &CVisibilityPlugins::m_alphaUnderwaterEntityList);
      CHook::Write(g_libGTASA + 0x8513C8, &CVisibilityPlugins::m_alphaReallyDrawLastList);
      CHook::Write(g_libGTASA + 0x851A08, &CVisibilityPlugins::ms_weaponPedsForPC);*/
}