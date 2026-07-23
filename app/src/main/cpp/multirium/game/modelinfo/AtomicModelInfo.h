//
// Created by viski on 12.05.2025.
//

#pragma once

#include "BaseModelInfo.h"

struct tVehicleComponentFlag {
    const char* m_ucName;
    uint32      m_nFlag;
};


class CAtomicModelInfo : public CBaseModelInfo {
public:
    CAtomicModelInfo() : CBaseModelInfo() {}
public:
  //  static void InjectHooks();

    // vtable overrides
    //static CAtomicModelInfo* AsAtomicModelInfoPtr();
 //   ModelInfoType GetModelType() override;
   // void Init() override;
  //  void DeleteRwObject() override;
  //  uint32 GetRwModelType() const override { return rpATOMIC; }
  //  RwObject* CreateInstance() override;
  //  RwObject* CreateInstance(RwMatrix* matrix) override;

    // vtable added methods
  //  virtual void SetAtomic(RpAtomic* atomic);

    // class methods
   // struct RpAtomic* GetAtomicFromDistance(float distance);
  //  void SetupVehicleUpgradeFlags(const char* name);
};


void SetAtomicModelInfoFlags(CAtomicModelInfo* modelInfo, uint32 dwFlags);

VALIDATE_SIZE(CAtomicModelInfo, (VER_x32 ? 0x38 : 0x48));