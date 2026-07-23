#include "../main.h"
#include "game.h"
#include "../net/netgame.h"

#include "../fakerw/RenderWare.h"
#include "Streaming.h"
#include "game/modelinfo/ModelInfo.h"
#include "game/custom/MaterialText.h"

#include <cmath>

extern CGame* pGame;
extern CNetGame* pNetGame;

CObject::CObject(int iModel, CVector vecPos, CVector vecRot, float fDrawDistance, uint8_t bAttached)
{
    if(!CModelInfo::GetModelInfo(iModel))
        iModel = 18631;

    m_bAttachedType = 0;
    m_usAttachedVehicle = 0xFFFF;

    m_pEntity 			= 0;
    m_dwGTAId 			= 0;

    ScriptCommand(&create_object, iModel, vecPos.x, vecPos.y, vecPos.z, &m_dwGTAId);
    if(!m_dwGTAId) return;

    ScriptCommand(&put_object_at, m_dwGTAId, vecPos.x, vecPos.y, vecPos.z);

    m_pEntity = GamePool_Object_GetAt(m_dwGTAId);

    if(!m_pEntity) return;

    m_byteMoving = 0;
    m_fMoveSpeed = 0.0;
    m_bNeedRotate = false;

    m_iModel = iModel;

    InstantRotate(vecRot.x, vecRot.y, vecRot.z);

    for (int i = 0; i < 16; i++)
    {
        m_MaterialTexture[i] = 0;
        m_dwMaterialColor[i] = 0xFFFFFFFF;
    }
    m_bHasMaterial = false;
    for(auto & pTex : m_MaterialTextTexture)
    {
        pTex = 0;
    }
    m_bHasMaterialText = false;
}

CObject::~CObject()
{
    m_pEntity = GamePool_Object_GetAt(m_dwGTAId);
    if(m_pEntity)
        ScriptCommand(&destroy_object, m_dwGTAId);

    for(int i = 0; i < 16; i++)
    {
        m_MaterialTexture[i] = 0;
    }

    m_bHasMaterial = false;

    for(auto & pTex : m_MaterialTextTexture)
    {
        pTex = 0;
    }

    m_bHasMaterialText = false;
}

void CObject::Process(float fElapsedTime)
{
    if (m_bAttachedType == 1)
    {
        CVehicle* pVehicle = pNetGame->GetVehiclePool()->GetAt(m_usAttachedVehicle);
        if (pVehicle)
        {
            if (pVehicle->IsAdded())
            {
                if (pVehicle->m_pVehicle->IsAdded()) {
                    ProcessAttachToVehicle(pVehicle);
                }
            }
        }
    }
    m_pEntity = GamePool_Object_GetAt(m_dwGTAId);
    if (!m_pEntity) return;
    if (!(m_pEntity->m_matrix)) return;
    if (m_byteMoving & 1)
    {
        CVector vecSpeed = { 0.0f, 0.0f, 0.0f };
        RwMatrix matEnt;
        m_pEntity->GetMatrix(&matEnt);
        float distance = fElapsedTime * m_fMoveSpeed;
        float remaining = DistanceRemaining(&matEnt);
        uint32_t dwThisTick = GetTickCount();

        float posX = matEnt.pos.x;
        float posY = matEnt.pos.y;
        float posZ = matEnt.pos.z;

        float f1 = ((float)(dwThisTick - m_dwMoveTick)) * 0.001f * m_fMoveSpeed;
        float f2 = m_fDistanceToTargetPoint - remaining;

        if (distance >= remaining)
        {
            m_pEntity->SetVelocity(vecSpeed);
            m_pEntity->SetTurnSpeed(vecSpeed);
            matEnt.pos.x = m_matTarget.pos.x;
            matEnt.pos.y = m_matTarget.pos.y;
            matEnt.pos.z = m_matTarget.pos.z;
            if (m_bNeedRotate) {
                m_quatTarget.GetMatrix(reinterpret_cast<RwMatrix *>(&matEnt));
            }
            m_pEntity->SetMatrix((CMatrix&)matEnt);
            m_pEntity->UpdateRW();
            m_pEntity->UpdateRwFrame();

            m_pEntity->Add();
            StopMoving();
            return;
        }

        if (fElapsedTime <= 0.0f)
            return;

        float delta = 1.0f / (remaining / distance);
        matEnt.pos.x += ((m_matTarget.pos.x - matEnt.pos.x) * delta);
        matEnt.pos.y += ((m_matTarget.pos.y - matEnt.pos.y) * delta);
        matEnt.pos.z += ((m_matTarget.pos.z - matEnt.pos.z) * delta);

        distance = remaining / m_fDistanceToTargetPoint;
        float slerpDelta = 1.0f - distance;

        delta = 1.0f / fElapsedTime;
        vecSpeed.x = (matEnt.pos.x - posX) * delta * 0.02f;
        vecSpeed.y = (matEnt.pos.y - posY) * delta * 0.02f;
        vecSpeed.z = (matEnt.pos.z - posZ) * delta * 0.02f;

        if (FloatOffset(f1, f2) > 0.1f)
        {
            if (f1 > f2)
            {
                delta = (f1 - f2) * 0.1f + 1.0f;
                vecSpeed.x *= delta;
                vecSpeed.y *= delta;
                vecSpeed.z *= delta;
            }

            if (f2 > f1)
            {
                delta = 1.0f - (f2 - f1) * 0.1f;
                vecSpeed.x *= delta;
                vecSpeed.y *= delta;
                vecSpeed.z *= delta;
            }
        }

        m_pEntity->SetVelocity(vecSpeed);
        ApplyMoveSpeed();

        if (m_bNeedRotate)
        {
            float fx, fy, fz;
            GetRotation(&fx, &fy, &fz);
            distance = m_vecRotationTarget.z - distance * m_vecSubRotationTarget.z;
            vecSpeed.x = 0.0f;
            vecSpeed.y = 0.0f;
            vecSpeed.z = subAngle(remaining, distance) * 0.01f;
            if (vecSpeed.z <= 0.001f)
            {
                if (vecSpeed.z < -0.001f)
                    vecSpeed.z = -0.001f;
            }
            else
            {
                vecSpeed.z = 0.001f;
            }

            m_pEntity->SetTurnSpeed(vecSpeed);
            matEnt = m_pEntity->GetMatrix().ToRwMatrix();
            CQuaternion quat;
            quat.Slerp(&m_quatStart, &m_quatTarget, slerpDelta);
            quat.Normalize();
            quat.GetMatrix(reinterpret_cast<RwMatrix *>(&matEnt));
        }
        else
        {
            matEnt = m_pEntity->GetMatrix().ToRwMatrix();
        }
        // CPhysical::Remove
        ((void (*)(CEntity*))(*(uintptr_t*)(*(uintptr*)(m_pEntity) + (VER_x32 ? 0x10 : 0x10 * 2) )))(m_pEntity);

        m_pEntity->SetMatrix((CMatrix&)matEnt);
        m_pEntity->UpdateRW();
        m_pEntity->UpdateRwFrame();

        m_pEntity->Add();
    }
}

void CObject::InstantRotate(float x, float y, float z)
{
    if (m_pEntity)
    {
        ScriptCommand(&set_object_rotation, m_dwGTAId, x, y, z);
    }
}

// 0.3.7
void CObject::GetRotation(float* pfX, float* pfY, float* pfZ)
{
    if (!m_pEntity) return;

    m_pEntity->m_matrix->ConvertToEulerAngles(pfX, pfY, pfZ, 21);

    *pfX = *pfX * 57.295776 * -1.0;
    *pfY = *pfY * 57.295776 * -1.0;
    *pfZ = *pfZ * 57.295776 * -1.0;
}
// 0.3.7
void CObject::ApplyMoveSpeed()
{
    if(m_pEntity)
    {
        float fTimeStep	= *(float*)(g_libGTASA + (VER_x32 ? 0x0096B500 : 0x00BDC580));

        RwMatrix mat;
        mat = m_pEntity->GetMatrix().ToRwMatrix();
        mat.pos.x += fTimeStep * m_pEntity->GetMoveSpeed().x;
        mat.pos.y += fTimeStep * m_pEntity->GetMoveSpeed().y;
        mat.pos.z += fTimeStep * m_pEntity->GetMoveSpeed().z;

        // CPhysical::Remove
        ((void (*)(CEntity*))(*(uintptr_t*)(*(uintptr*)(m_pEntity) + (VER_x32 ? 0x10 : 0x10 * 2) )))(m_pEntity);

        m_pEntity->SetMatrix((CMatrix&)mat);
        m_pEntity->UpdateRW();
        m_pEntity->UpdateRwFrame();

        m_pEntity->Add();
    }
}
// 0.3.7
float CObject::DistanceRemaining(RwMatrix* matPos)
{
    float	fSX, fSY, fSZ;
    fSX = (matPos->pos.x - m_matTarget.pos.x) * (matPos->pos.x - m_matTarget.pos.x);
    fSY = (matPos->pos.y - m_matTarget.pos.y) * (matPos->pos.y - m_matTarget.pos.y);
    fSZ = (matPos->pos.z - m_matTarget.pos.z) * (matPos->pos.z - m_matTarget.pos.z);
    return (float)sqrt(fSX + fSY + fSZ);
}

RwTexture* LoadFromTxdSlot2(const char* szSlot, const char* szTexture)
{
    RwTexture* tex;
    if (strncmp(szSlot, "none", 5u))
    {
        uintptr_t v10 = CHook::CallFunction<int>("_ZN9CTxdStore11FindTxdSlotEPKc", szSlot);
        CHook::CallFunction<void>("_ZN9CTxdStore14PushCurrentTxdEv");
        CHook::CallFunction<void>("_ZN9CTxdStore14PushCurrentTxdEv", v10, 0);
        tex = CHook::CallFunction<RwTexture *>("_Z13RwTextureReadPKcS0_", szTexture, 0);
        CHook::CallFunction<void>("_ZN9CTxdStore13PopCurrentTxdEv");
    }

    if(!tex && strncmp(szTexture, "none", 5u))
    {
        if(!tex) tex = (RwTexture*)LoadTexture(std::string(std::string(szTexture) + "_" + szSlot).c_str());
        if(!tex) tex = (RwTexture*)LoadTexture(std::string(std::string(szSlot) + "_" + szTexture).c_str());
        if(!tex) tex = (RwTexture*)LoadTexture(szTexture);
        if(!tex)
        {
            std::string str = szTexture;
            std::transform(str.begin(), str.end(), str.begin(), ::tolower);
            tex = (RwTexture*)LoadTexture(str.c_str());
        }
        if(!tex)
        {
            std::string str = szTexture;
            std::transform(str.begin(), str.end(), str.begin(), ::toupper);
            tex = (RwTexture*)LoadTexture(std::string(str + "_" + szSlot).c_str());
        }
    }

    return tex;
}

void CObject::SetMaterial(int iModel, int iMaterialIndex, char* txdname, char* texturename, uint32_t dwColor)
{
#if DEBUG_LOG
    Log(OBFUSCATE("SetMaterial: model: %d, %s, %s"), iModel, txdname, texturename);
#endif

    if (iMaterialIndex < 16)
    {
        if (m_MaterialTexture[iMaterialIndex])
        {
            RwTextureDestroy(reinterpret_cast<RwTexture *>(m_MaterialTexture[iMaterialIndex]));
            m_MaterialTexture[iMaterialIndex] = 0;
        }
        m_dwMaterialColor[iMaterialIndex] = dwColor;
        Log("Object color: 0x%X", dwColor);
#if DEBUG_LOG
        Log(OBFUSCATE("Object color: 0x%X"), dwColor);
#endif
        m_MaterialTexture[iMaterialIndex] = reinterpret_cast<uintptr_t>(LoadFromTxdSlot2(txdname,
                                                                                        texturename));
        m_bHasMaterial = true;
    }
}

void CObject::SetMaterialText(int iMaterialIndex, uint8_t byteMaterialSize, const char *szFontName, uint8_t byteFontSize, uint8_t byteFontBold, uint32_t dwFontColor, uint32_t dwBackgroundColor, uint8_t byteAlign, const char *szText)
{
    if (iMaterialIndex < 16)
    {
        if(m_MaterialTextTexture[iMaterialIndex])
        {
            RwTextureDestroy(reinterpret_cast<RwTexture *>(m_MaterialTextTexture[iMaterialIndex]));
            m_MaterialTextTexture[iMaterialIndex] = 0;
        }
        m_MaterialTextTexture[iMaterialIndex] = reinterpret_cast<uintptr_t>(CMaterialText::Generate(
                byteMaterialSize, szFontName, byteFontSize, byteFontBold, dwFontColor, dwBackgroundColor, byteAlign,
                szText));
        m_bHasMaterialText = true;
    }
}

// 0.3.7
void CObject::MoveTo(float fX, float fY, float fZ, float fSpeed, float fRotX, float fRotY, float fRotZ)
{
    RwMatrix mat;
    mat = m_pEntity->GetMatrix().ToRwMatrix();

    if (m_byteMoving & 1) {
        this->StopMoving();
        mat.pos.x = m_matTarget.pos.x;
        mat.pos.y = m_matTarget.pos.y;
        mat.pos.z = m_matTarget.pos.z;

        if (m_bNeedRotate) {
            m_quatTarget.GetMatrix(reinterpret_cast<RwMatrix *>(&mat));
        }

        // CPhysical::Remove
        ((void (*)(CEntity*))(*(uintptr_t*)(*(uintptr*)(m_pEntity) + (VER_x32 ? 0x10 : 0x10 * 2) )))(m_pEntity);

        m_pEntity->SetMatrix((CMatrix&)mat);
        m_pEntity->UpdateRW();
        m_pEntity->UpdateRwFrame();

        m_pEntity->Add();
    }

    m_dwMoveTick = GetTickCount();
    m_fMoveSpeed = fSpeed;
    m_matTarget.pos.x = fX;
    m_matTarget.pos.y = fY;
    m_matTarget.pos.z = fZ;
    m_byteMoving |= 1;

    if (fRotX <= -999.0f || fRotY <= -999.0f || fRotZ <= -999.0f) {
        m_bNeedRotate = false;
    }
    else
    {
        m_bNeedRotate = true;

        CVector vecRot;
        RwMatrix matrix;
        this->GetRotation(&vecRot.x, &vecRot.y, &vecRot.z);
        m_vecRotationTarget.x = fixAngle(fRotX);
        m_vecRotationTarget.y = fixAngle(fRotY);
        m_vecRotationTarget.z = fixAngle(fRotZ);

        m_vecSubRotationTarget.x = subAngle(vecRot.x, fRotX);
        m_vecSubRotationTarget.y = subAngle(vecRot.y, fRotY);
        m_vecSubRotationTarget.z = subAngle(vecRot.z, fRotZ);

        this->InstantRotate(fRotX, fRotY, fRotZ);
        matrix = m_pEntity->GetMatrix().ToRwMatrix();
        m_matTarget.right = matrix.right;
        m_matTarget.at = matrix.at;
        m_matTarget.up = matrix.up;

        this->InstantRotate(vecRot.x, vecRot.y, vecRot.z);
        matrix = m_pEntity->GetMatrix().ToRwMatrix();

        m_quatStart.SetFromMatrix(&matrix);
        m_quatTarget.SetFromMatrix(&m_matTarget);
        m_quatStart.Normalize();
        m_quatTarget.Normalize();
    }

    m_fDistanceToTargetPoint = m_pEntity->GetDistanceFromPoint(m_matTarget.pos.x, m_matTarget.pos.y,
                                                               m_matTarget.pos.z);

    if (pNetGame) {
        CPlayerPool* pPlayerPool = pNetGame->GetPlayerPool();
        if (pPlayerPool) {
            pPlayerPool->GetLocalPlayer()->UpdateSurfing();
        }
    }
}
// 0.3.7
void CObject::StopMoving()
{
    CVector vec = { 0.0f, 0.0f, 0.0f };
    m_pEntity->SetVelocity(vec);
    m_pEntity->SetTurnSpeed(vec);

    m_byteMoving &= ~1;
}

void CObject::AttachToVehicle(uint16_t usVehID, CVector* pVecOffset, CVector* pVecRot)
{
    m_bAttachedType = 1;
    m_usAttachedVehicle = usVehID;
    m_vecAttachedOffset.x = pVecOffset->x;
    m_vecAttachedOffset.y = pVecOffset->y;
    m_vecAttachedOffset.z = pVecOffset->z;

    m_vecAttachedRotation.x = pVecRot->y;
    m_vecAttachedRotation.y = pVecRot->y;
    m_vecAttachedRotation.z = pVecRot->z;
}

void CObject::ProcessAttachToVehicle(CVehicle* pVehicle)
{
    if (GamePool_Object_GetAt(m_dwGTAId))
    {
        if (!ScriptCommand(&is_object_attached, m_dwGTAId))
        {
            ScriptCommand(&attach_object_to_car, m_dwGTAId, pVehicle->m_dwGTAId, m_vecAttachedOffset.x,
                          m_vecAttachedOffset.y, m_vecAttachedOffset.z, m_vecAttachedRotation.x, m_vecAttachedRotation.y, m_vecAttachedRotation.z);
        }
    }
}

void CObject::SetPos(float x, float y, float z)
{
    if (GamePool_Object_GetAt(m_dwGTAId))
    {
        ScriptCommand(&put_object_at, m_dwGTAId, x, y, z);
    }
}