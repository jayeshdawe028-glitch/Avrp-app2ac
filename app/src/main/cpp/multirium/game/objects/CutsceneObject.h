//
// Created by viski on 12.05.2025.
//

#ifndef MULTIRIUM_CUTSCENEOBJECT_H
#define MULTIRIUM_CUTSCENEOBJECT_H

#include "game/objects/Object.h"

class CCutsceneObject : public CObjectGta {
public:
    union {
        RwFrame* m_pAttachToFrame;
        uint32   m_nAttachBone; // this one if m_pAttachmentObject != 0
    };
    CObjectGta* m_pAttachmentObject;
    CVector  m_vWorldPosition;
    CVector  m_vForce;

public:
    static constexpr int32 NUM_CUTSCENE_VEHS = 6;
    static char* (&ms_sCutsceneVehNames)[NUM_CUTSCENE_VEHS];

public:
    static void InjectHooks();

    CCutsceneObject();

    void SetModelIndex(uint32 index) override;
    //void ProcessControl() override;
    //void PreRender() override;
    //bool SetupLighting() override;
    //void RemoveLighting(bool bRemove) override;

    void SetupCarPipeAtomicsForClump(unsigned int modelId, RpClump *clump);
};
VALIDATE_SIZE(CCutsceneObject, (VER_x32 ? 0x1A4 : 0x220));

#endif //MULTIRIUM_CUTSCENEOBJECT_H
