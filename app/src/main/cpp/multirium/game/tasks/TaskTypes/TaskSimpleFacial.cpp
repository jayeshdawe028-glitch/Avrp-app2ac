//
// Created by viski on 15.05.2025.
//

#include "TaskSimpleFacial.h"
#include "patch.h"
#include "General.h"
#include "../game/peds/Ped.h"
#include "plugins/RpAnimBlendPlugin/RpAnimBlend.h"
#include "game/animation/AnimManager.h"


void CTaskSimpleFacial::InjectHooks() {
  //  CHook::Redirect("", &GetAnimId);
  //  CHook::Redirect("", &Clone);
  //  CHook::Redirect("", &GetTaskType);
  //  CHook::Redirect("", &MakeAbortable);
  //  CHook::Redirect("", &ProcessPed);
}

CTaskSimpleFacial::CTaskSimpleFacial(eFacialExpression type, int32 durMs) :
    m_Duration{ durMs },
    m_Type{ type }
{
}

// See Android, looks like **map**
AnimationId CTaskSimpleFacial::GetAnimId(eFacialExpression expression) {
    //return CHook::CallFunction<AnimationId>("_ZN17CTaskSimpleFacial9GetAnimIdEi");
    switch (expression) {
        case eFacialExpression::SURPRISEPANIC: return ANIM_ID_FACSURP;
        case eFacialExpression::SURPRISEMILD: return ANIM_ID_FACSURPM;
        case eFacialExpression::CURIOSITY:
            case eFacialExpression::HAPPINESS: return ANIM_ID_FACURIOS;
            case eFacialExpression::ANGER:
                case eFacialExpression::SADNESS: return ANIM_ID_FACANGER_0;
                case eFacialExpression::TALKING: return ANIM_ID_FACTALK;
                case eFacialExpression::CHEWING: return ANIM_ID_FACGUM;
                default: return ANIM_ID_UNDEFINED;
    }
}

bool CTaskSimpleFacial::MakeAbortable(CPed* ped, eAbortPriority priority, const CEvent* event) {
  //  return CHook::CallFunction<int>("_ZN17CTaskSimpleFacial13MakeAbortableEP4CPediPK6CEvent", ped, priority, event);
    if (const auto anim = RpAnimBlendClumpGetAssociation(ped->m_pRwClump, GetAnimId(m_Type))) {
        anim->m_fBlendDelta = -4.0f;
    }
    return true;
}

bool CTaskSimpleFacial::ProcessPed(CPed* ped) {
    const auto animId = CTaskSimpleFacial::GetAnimId(m_Type);
    const auto anim  = RpAnimBlendClumpGetAssociation(ped->m_pRwClump, animId);

    if (RpAnimBlendClumpGetAssociation(ped->m_pRwClump, ANIM_ID_FACTALK)) {
        if (animId == ANIM_ID_FACTALK) {
            if (CGeneral::GetRandomNumberInRange(0, 100) < 40) {
                anim->SetSpeed(CGeneral::GetRandomNumberInRange(0.5f, 3.0f));
            }
            return false;
        }
        if (anim) {
            anim->SetFlag(ANIMATION_FREEZE_LAST_FRAME, true);
            anim->SetBlendDelta(-4.0f);
        }
        return true;
    }

    if (!m_Timer.IsStarted()) {
        if (!anim) {
            CAnimManager::BlendAnimation(ped->m_pRwClump, ANIM_GROUP_DEFAULT, animId, 4.0f);
            m_Timer.Start(m_Duration);
            return false;
        }
        return true;
    }

    if (!anim) {
        return true;
    }

    if (!m_Timer.IsOutOfTime()) {
        return false;
    }

    anim->m_fBlendDelta = -4.0f;

    return true;
}
