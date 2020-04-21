#pragma once

#include <memory>

#include "Runtime/Graphics/Shaders/CAABoxShader.hpp"
#include "Runtime/Weapon/CGunWeapon.hpp"

namespace urde {

class CPhazonBeam final : public CGunWeapon {
  TCachedToken<CModel> x21c_phazonVeins;
  std::unique_ptr<CModelData> x224_phazonVeinsData;
  TCachedToken<CGenDescription> x228_phazon2nd1;
  std::unique_ptr<CElementGen> x234_chargeFxGen;
  zeus::CAABox x238_aaBoxScale;
  zeus::CAABox x250_aaBoxTranslate;
  float x268_clipWipeScale = 0.f;
  float x26c_clipWipeTranslate = 0.f;
  float x270_indirectAlpha = 1.f;
  bool x274_24_loaded : 1 = false;
  bool x274_25_clipWipeActive : 1 = true;
  bool x274_26_veinsAlphaActive : 1 = false;
  bool x274_27_phazonVeinsIdx : 1 = false;
  float x278_fireTime = 1.f / 3.f;
  CAABoxShader m_aaboxShaderScale{true};
  CAABoxShader m_aaboxShaderTranslate{true};
  void ReInitVariables();
  void DrawClipScaleCube();
  void DrawClipTranslateCube();

public:
  CPhazonBeam(CAssetId characterId, EWeaponType type, TUniqueId playerId, EMaterialTypes playerMaterial,
              const zeus::CVector3f& scale);
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);
  bool IsFiring() const { return x278_fireTime < 1.f / 6.f; }
  void SetClipWipeActive(bool b) { x274_25_clipWipeActive = b; }
  void SetVeinsAlphaActive(bool b) { x274_26_veinsAlphaActive = b; }
  void StopBeam(CStateManager& mgr, bool b1);
  void UpdateBeam(float dt, const zeus::CTransform& targetXf, const zeus::CVector3f& localBeamPos, CStateManager& mgr);
  void CreateBeam(CStateManager& mgr);

  void PreRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf) override;
  void PostRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf) override;
  void UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr, const zeus::CTransform& xf) override;
  void Fire(bool underwater, float dt, EChargeState chargeState, const zeus::CTransform& xf, CStateManager& mgr,
            TUniqueId homingTarget, float chargeFactor1, float chargeFactor2) override;
  void Update(float dt, CStateManager& mgr) override;
  void Load(CStateManager& mgr, bool subtypeBasePose) override;
  void Unload(CStateManager& mgr) override;
  bool IsLoaded() const override;
  void Draw(bool drawSuitArm, const CStateManager& mgr, const zeus::CTransform& xf, const CModelFlags& flags,
            const CActorLights* lights) override;
  void DrawMuzzleFx(const CStateManager& mgr) const override;
};

} // namespace urde
