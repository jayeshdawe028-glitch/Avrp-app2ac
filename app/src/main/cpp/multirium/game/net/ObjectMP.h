#pragma once

#include "game/core/core/Quaternion.h"
#include "game/entity/Physical.h"

#define MATERIAL_TYPE_MATERIAL	1
#define MATERIAL_TYPE_TEXT		2

class CObject
{
public:
    CObject(int iModel, CVector vecPos, CVector vecRot, float fDrawDistance, uint8_t unk);
    virtual ~CObject();

    void Process(float fElapsedTime);

    void GetRotation(float* pfX, float* pfY, float* pfZ);
    void ApplyMoveSpeed();
    float DistanceRemaining(RwMatrix* matPos);

    void SetMaterial(int iModel, int iIndex, char* txdname, char* texturename, uint32_t dwColor);
    void SetMaterialText(int iMaterialIndex, uint8_t byteMaterialSize, const char *szFontName, uint8_t byteFontSize, uint8_t byteFontBold, uint32_t dwFontColor, uint32_t dwBackgroundColor, uint8_t byteAlign, const char *szText);

    void MoveTo(float fX, float fY, float fZ, float fSpeed, float fRotX, float fRotY, float fRotZ);
    void StopMoving();

    void SetPos(float x, float y, float z);

    /* attach */
    CVector     m_vecAttachedOffset;
    CVector     m_vecAttachedRotation;
    uint16_t    m_usAttachedVehicle;
    uint8_t     m_bAttachedType;
    void AttachToVehicle(uint16_t usVehID, CVector* pVecOffset, CVector* pVecRot);
    void ProcessAttachToVehicle(CVehicle* pVehicle);
    /* attach */

    int	m_MaterialTextIndex;
    uintptr_t	m_MaterialTexture[16];
    uint32_t	m_dwMaterialColor[16];
    uintptr_t	m_MaterialTextTexture[16];
    int			m_iMaterialType[16];
    bool		m_bHasMaterial;
    bool		m_bHasMaterialText;
    /* materialText */
    char* m_szMaterialText[16];
    int			m_iMaterialSize[16];
    int			m_iMaterialFontSize[16];
    uint32_t	m_dwMaterialFontColor[16];
    uint32_t	m_dwMaterialBackColor[16];
    int			m_iMaterialTextAlign[16];

    CPhysical		*m_pEntity;
    uint8_t		m_byteMoving;
    uint32_t		m_dwGTAId;

    void InstantRotate(float x, float y, float z);

    int			m_iModel;
private:
    RwMatrix	m_Matrix;
    CVector		m_vecRotation;

    float		m_fMoveSpeed;
    bool		m_bNeedRotate;
    RwMatrix	m_matTarget;
    CQuaternion m_quatStart;
    CQuaternion m_quatTarget;
    CVector		m_vecRotationTarget;
    CVector		m_vecSubRotationTarget;
    float		m_fDistanceToTargetPoint;
    uint32_t	m_dwMoveTick;
};