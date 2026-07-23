//
// Created by viski on 24.04.2025.
//
#ifndef X64_CSKYBOX_H
#define X64_CSKYBOX_H

#include "main.h"

class CObject;

class CSkyBox {
public:
    enum eAxis
    {
        X = 0,
        Y,
        Z
    };

	static void Process();

	static void SetRotSpeed(float mFRotSpeed);
	static void SetTexture(const char *texName);

	static bool IsNeedRender();

	static CObject *GetSkyObject();
    static CObject* m_pSkyObject;

private:
	static CObject* CreateObjectScaled(int iModel, float fScale);

	static void ReTexture();

	static RwObject* RwFrameForAllObjectsCallback(RwObject* object, void* data);

private:
    static RwTexture* m_pTex;

	static bool m_bNeedRender;

	static float m_fRotSpeed;
};

#endif //X64_CSKYBOX_H