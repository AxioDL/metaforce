#pragma once

#include "CGunWeapon.hpp"
#include "Graphics/Shaders/CAABoxShader.hpp"

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
  bool x274_24_loaded : 1;
  bool x274_25_clipWipeActive : 1;
  bool x274_26_veinsAlphaActive : 1;
  bool x274_27_phazonVeinsIdx : 1;
  float x278_fireTime = 1.f / 3.f;
  mutable CAABoxShader m_aaboxShaderScale = {true};
  mutable CAABoxShader m_aaboxShaderTranslate = {true};
  void ReInitVariables();
  void DrawClipScaleCube() const;
  void DrawClipTranslateCube() const;

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

  void PreRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf);
  void PostRenderGunFx(const CStateManager& mgr, const zeus::CTransform& xf);
  void UpdateGunFx(bool shotSmoke, float dt, const CStateManager& mgr, const zeus::CTransform& xf);
  void Fire(bool underwater, float dt, EChargeState chargeState, const zeus::CTransform& xf, CStateManager& mgr,
            TUniqueId homingTarget, float chargeFactor1, float chargeFactor2);
  void Update(float dt, CStateManager& mgr);
  void Load(CStateManager& mgr, bool subtypeBasePose);
  void Unload(CStateManager& mgr);
  bool IsLoaded() const;
  void Draw(bool drawSuitArm, const CStateManager& mgr, const zeus::CTransform& xf, const CModelFlags& flags,
            const CActorLights* lights) const;
  void DrawMuzzleFx(const CStateManager& mgr) const;
};

} // namespace urde
