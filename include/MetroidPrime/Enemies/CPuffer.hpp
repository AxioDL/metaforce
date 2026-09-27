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
    mEnabledParticles =
        enabled ? mEnabledParticles | (1 << idx) : mEnabledParticles & ~(1 << idx);
  }
  bool IsParticleEnabled(const int idx) const { return (mEnabledParticles & (1 << idx)) != 0; }

private:
  CVector3f mFace;
  TToken< CGenDescription > mCloudEffect;
  CDamageInfo mCloudDamage;
  bool x598_24_ : 1;
  bool x598_25_ : 1;
  bool x598_26_ : 1;
  ushort x59a_;
  CDamageInfo mExplosionDamage;
  float x5b8_;
  CAssetId mCloudSteam;
  CVector3f mMove;
  TUniqueId x5cc_;
  int mEnabledParticles;
  rstl::reserved_vector< CVector3f, 14 > mGasLocators;

  void UpdateJets(CStateManager&);
};

#endif // _CPUFFER
