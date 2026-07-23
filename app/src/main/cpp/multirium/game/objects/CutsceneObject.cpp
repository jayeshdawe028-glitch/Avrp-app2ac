//
// Created by viski on 12.05.2025.
//

#include "CutsceneObject.h"
#include "patch.h"
#include "game/modelinfo/ModelInfo.h"

char* g_CutsceneVehNames[CCutsceneObject::NUM_CUTSCENE_VEHS];
char* (&CCutsceneObject::ms_sCutsceneVehNames)[CCutsceneObject::NUM_CUTSCENE_VEHS] = g_CutsceneVehNames;

CCutsceneObject::CCutsceneObject() : CObjectGta() {
    m_vWorldPosition.Set(0.0F, 0.0F, 0.0F);
    m_vForce.Set(0.0F, 0.0F, 0.0F);
    m_bUsesCollision = false;
    m_nStatus = eEntityStatus::STATUS_SIMPLE;
    m_nObjectType = eObjectType::OBJECT_TYPE_CUTSCENE;
    m_nAttachBone = 0;
    m_pAttachmentObject = nullptr;
    m_bLightObject = true;
    m_bStreamingDontDelete = true;

    CObjectGta::SetIsStatic(false);
    m_bBackfaceCulled = false;
    m_fContactSurfaceBrightness = 0.5F;
}

void CCutsceneObject::SetModelIndex(unsigned index) {
    CEntity::SetModelIndex(index);
    if (RwObjectGetType(m_pRwObject) == rpCLUMP) {
        //RpAnimBlendClumpInit(m_pRwClump);
        //auto* animData = RpAnimBlendClumpGetData(m_pRwClump);
       // animData->m_PedPosition = &m_vecMoveSpeed;
        //animData->m_FrameDatas->HasZVelocity = true;
        CCutsceneObject::SetupCarPipeAtomicsForClump(index, m_pRwClump);
    }
    GetModelInfo()->m_alpha = 0xFF;
}

RpAtomic* SetupCarPipeAtomicCB(RpAtomic* atomic, void* data) {
    UNUSED(data);
    CHook::CallFunction<int>("_ZN14CCarFXRenderer24CustomCarPipeAtomicSetupEP8RpAtomic");
    // CCarFXRenderer::CustomCarPipeAtomicSetup(atomic);
    return atomic;
}

void CCutsceneObject::SetupCarPipeAtomicsForClump(unsigned modelId, RpClump* clump) {
    static bool& bCarPipeAtomicsInitialized = *(bool*)0xBC4058;                              // TODO | STATICREF // = false;
    static uint32(&anHashKeys)[NUM_CUTSCENE_VEHS] = *(uint32(*)[NUM_CUTSCENE_VEHS])0xBC4040; // TODO | STATICREF

    if (!bCarPipeAtomicsInitialized) {
        bCarPipeAtomicsInitialized = true;
        for (auto i = 0; i < NUM_CUTSCENE_VEHS; ++i) {
            anHashKeys[i] = CKeyGen::GetUppercaseKey(ms_sCutsceneVehNames[i]);
        }
    }

    for (auto anHashKey : anHashKeys) {
        if (CModelInfo::GetModelInfo(modelId)->m_nKey == anHashKey) {
            RpClumpForAllAtomics(clump, SetupCarPipeAtomicCB, nullptr);
            return;
        }
    }
}

void CCutsceneObject::InjectHooks() {
    CHook::Write(g_libGTASA + 0x222dd4, &CCutsceneObject::ms_sCutsceneVehNames);
}