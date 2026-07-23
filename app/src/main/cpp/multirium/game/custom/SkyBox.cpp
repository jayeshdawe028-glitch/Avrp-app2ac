//
// Created by viski on 24.04.2025.
//
#include "SkyBox.h"

#include "game/core/game.h"

#include "net/netgame.h"
#include "gui/gui.h"
#include "settings.h"
#include "game/modelinfo/ModelInfo.h"
#include "game/renderer/TimeCycle.h"

extern CGame *pGame;
extern CNetGame *pNetGame;

CObject *CSkyBox::m_pSkyObject = nullptr;

bool CSkyBox::m_bNeedRender = true;
float CSkyBox::m_fRotSpeed = 0.005f;

uint8_t pChangeTime;

RwTexture *CSkyBox::m_pTex = nullptr;

void CSkyBox::Process() {
    static CCamera &TheCamera = *reinterpret_cast<CCamera *>(g_libGTASA +
                                                             (VER_x32 ? 0x00951FA8 : 0xBBA8D0));

    if (m_pSkyObject) {
        RwMatrix matrix;

        m_pSkyObject->m_pEntity->GetMatrix(&matrix);

        matrix.pos = TheCamera.GetPosition();

        CVector axis{0.0f, 0.0f, 1.0f};
        RwMatrixRotate(&matrix, &axis, m_fRotSpeed * CTimer::ms_fTimeScale);

        m_bNeedRender = true;

        ReTexture();

        m_pSkyObject->m_pEntity->Remove();

        m_pSkyObject->m_pEntity->SetMatrix((CMatrix&)matrix);
        m_pSkyObject->m_pEntity->UpdateRW();
        m_pSkyObject->m_pEntity->UpdateRwFrame();

        m_pSkyObject->m_pEntity->Add();

        RenderEntity(m_pSkyObject->m_pEntity);

        m_bNeedRender = false;
    } else {
        if (!CModelInfo::GetModelInfo(17364)) {
            Log("Error CSkyBox::Init. No mode %d", 17364);
            assert(0);
        }
        m_pSkyObject = CreateObjectScaled(17364, 0.8f);
    }
}

CObject *CSkyBox::CreateObjectScaled(int iModel, float fScale) {
    if (!pNetGame)
        return nullptr;

    CModelInfo::GetModelInfo(iModel)->m_nRefCount = 899;

    CVector vecRot(0.f, 0.f, 0.f);
    CVector vecScale(fScale);

    auto *object = pGame->NewObject(iModel, CVector(0.0f, 0.0f, 0.0f), vecRot, 0.0f);
    CEntity* entity = object->m_pEntity;
    Log("%d model", entity->m_nModelIndex);

    entity->m_bUsesCollision = false;
    entity->Remove();

    RwMatrix matrix;
    entity->GetMatrix(&matrix);
    RwMatrixScale(&matrix, &vecScale);

    entity->SetMatrix(reinterpret_cast<CMatrix&>(matrix));
    entity->UpdateRW();
    entity->UpdateRwFrame();

    entity->Add();
    entity->m_bUsesCollision = false;

    Log("SkyBox Create : %d", iModel);
    return object;
}

void CSkyBox::ReTexture() {
    int iHours = pNetGame->m_pNetSet->byteWorldTime_Hour;

    if (pChangeTime != iHours) {
        pChangeTime = iHours;

        if (iHours >= 0 && iHours < 6 || iHours > 18) {
            SetTexture("skybox_1");
        } else if (iHours >= 6 && iHours < 8) {
            SetTexture("skybox_2");
        } else if (iHours >= 8 && iHours < 11) {
            SetTexture("skybox_3");
        } else if (iHours >= 11 && iHours <= 15) {
            SetTexture("skybox_4");
        } else if (iHours == 16) {
            SetTexture("skybox_5");
        } else if (iHours == 17) {
            SetTexture("skybox_6");
        } else if (iHours == 18) {
            SetTexture("skybox_7");
        }
    }

    auto pAtomic = m_pSkyObject->m_pEntity->m_pRwObject;

    if (!pAtomic || !pAtomic->parent)
        return;

    ((void (*)()) (g_libGTASA + (VER_x32 ? 0x5D1F98 + 1 : 0x6F6664)))(); // -- DeActivateDirectional
    ((void *(*)()) (g_libGTASA + (VER_x32 ? 0x5D204C + 1 : 0x6F6720)))(); // -- SetFullAmbient
    ((void (*)()) (g_libGTASA + (VER_x32 ? 0x5D2068 + 1 : 0x6F6738)))(); // -- SetAmbientColours

    RwFrameForAllObjects((RwFrame*)pAtomic->parent, RwFrameForAllObjectsCallback, m_pSkyObject);
}

RwObject *CSkyBox::RwFrameForAllObjectsCallback(RwObject *object, void *data) {
    if (!object || object->type != rpATOMIC || !m_pTex)
        return object;

    auto *atomic = reinterpret_cast<RpAtomic *>(object);
    if (!atomic->geometry)
        return object;

    auto &geometry = *atomic->geometry;
    auto materials = std::span(geometry.matList.materials,
                               std::min<size_t>(geometry.matList.numMaterials, 16));

    const auto &skyColor = CTimeCycle::m_CurrentColours;
    const RwRGBA rgbaColor = {
            static_cast<RwUInt8>(skyColor.m_nSkyBottomRed),
            static_cast<RwUInt8>(skyColor.m_nSkyBottomGreen),
            static_cast<RwUInt8>(skyColor.m_nSkyBottomBlue),
            150
    };

    for (auto *material: materials) {
        if (!material)
            continue;

        material->texture = m_pTex;
        material->color = rgbaColor;
    }

    return object;
}

void CSkyBox::SetTexture(const char *texName) {
    if (texName == nullptr)
        return;

    if (m_pTex) {
        RwTextureDestroy(m_pTex);
    }

    Log("SkyBox: %s", texName);
    CSkyBox::m_pTex = CUtil::LoadTextureFromDB("gta3", texName);
}

void CSkyBox::SetRotSpeed(float speed) {
    m_fRotSpeed = speed;
}

bool CSkyBox::IsNeedRender() {
    return m_bNeedRender;
}

CObject *CSkyBox::GetSkyObject() {
    return m_pSkyObject;
}