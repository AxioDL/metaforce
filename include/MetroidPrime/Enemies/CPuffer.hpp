#ifndef _CPUFFER
#define _CPUFFER

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"

class CPatternedInfo;
class CGenDescription;

class CPuffer : public CPatterned {
public:
  CPuffer(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
          const CModelData& modelData, const CActorParameters& actorParameters,
          const CPatternedInfo& patternedInfo, float hoverSpeed, CAssetId cloudEffect,
          const CDamageInfo& cloudDamage, CAssetId cloudSteam, float f2, bool b1, bool b2, bool b3,
          const CDamageInfo& explosionDamage, ushort sfxId);

  ~CPuffer() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;
  void Death(CStateManager&, const CVector3f&, EScriptObjectState) override;

  void SetParticleEnabled(const int idx, const bool enabled) {
    x5d0_enabledParticles =
        enabled ? x5d0_enabledParticles | (1 << idx) : x5d0_enabledParticles & ~(1 << idx);
  }
  bool IsParticleEnabled(const int idx) const { return (x5d0_enabledParticles & (1 << idx)) != 0; }

private:
  CVector3f x568_face;
  TToken< CGenDescription > x574_cloudEffect;
  CDamageInfo x57c_cloudDamage;
  bool x598_24_ : 1;
  bool x598_25_ : 1;
  bool x598_26_ : 1;
  ushort x59a_;
  CDamageInfo x59c_explosionDamage;
  float x5b8_;
  CAssetId x5bc_cloudSteam;
  CVector3f x5c0_move;
  TUniqueId x5cc_;
  int x5d0_enabledParticles;
  rstl::reserved_vector< CVector3f, 14 > x5d4_gasLocators;

  void UpdateJets(CStateManager&);
};

#endif // _CPUFFER
