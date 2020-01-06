#pragma once

#include <string_view>

#include "Runtime/World/CAi.hpp"

namespace urde {

class CDestroyableRock : public CAi {

  /* Used to be a CModelData */
  CStaticRes x2d8_phazonModel;
  float x324_ = 0.f;
  float x328_ = 0.f;
  float x32c_thermalMag;
  zeus::CColor x330_ = zeus::skWhite;
  bool x334_isCold = false;
  bool x335_usePhazonModel = false;
  CHealthInfo x338_healthInfo;
  bool x341_;

public:
  CDestroyableRock(TUniqueId id, bool active, std::string_view name, const CEntityInfo& info,
                   const zeus::CTransform& xf, CModelData&& modelData, float mass, const CHealthInfo& health,
                   const CDamageVulnerability& vulnerability, const CMaterialList& matList, CAssetId fsm,
                   const CActorParameters& actParams, const CStaticRes& phazonModel, s32);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override {
    CAi::AcceptScriptMsg(msg, uid, mgr);
    if (msg == EScriptObjectMessage::Registered) {
      AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
      AddMaterial(EMaterialTypes::CameraPassthrough, mgr);
    }
  }
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) override {
    x334_isCold = true;
  }
  void KnockBack(const zeus::CVector3f&, CStateManager&, const CDamageInfo& info, EKnockBackType type, bool inDeferred,
                 float magnitude) override {}

  zeus::CVector3f GetAimPosition(const CStateManager&, float) const override { return GetTranslation(); }
  zeus::CVector3f GetOrbitPosition(const CStateManager&) const override { return GetTranslation(); }
  std::optional<zeus::CAABox> GetTouchBounds() const override { return GetModelData()->GetBounds(GetTransform()); }
  bool CanRenderUnsorted(const CStateManager&) const override { return true; }
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void TakeDamage(const zeus::CVector3f&, float) override {
    x324_ = 1.f;
    x328_ = 2.f;
  }

  void Think(float dt, CStateManager& mgr) override;

  void Set_x32c(float val) { x32c_thermalMag = val; }
  void SetIsCold(bool v) { x334_isCold = v; }
  bool IsUsingPhazonModel() const { return x335_usePhazonModel; }
  void UsePhazonModel() {
    SetModelData(std::make_unique<CModelData>(x2d8_phazonModel));
    x335_usePhazonModel = true;
    /* This used to be in the constructor, since we can't store CModelData directly in the class we must set it here */
    GetModelData()->SetSortThermal(true);
  }
};

} // namespace urde
