#pragma once

#include "World/CPatterned.hpp"

namespace urde::MP1 {
class CBabygothData {
  float x0_;
  float x4_;
  CAssetId x8_;
  CDamageInfo xc_;
  CDamageInfo x28_;
  CAssetId x44_;
  CAssetId x48_;
  CDamageInfo x4c_;
  CDamageVulnerability x68_;
  CDamageVulnerability xd0_;
  CAssetId x138_;
  CAssetId x13c_;
  float x140_;
  s16 x144_;
  CAssetId x148_;
  CAssetId x14c_;
  CAssetId x150_;
  CAssetId x154_;
  s16 x158_;
  s16 x15a_;
  s16 x15c_;
  float x160_;
  float x164_;
  float x168_;
  CAssetId x16c_;
  s16 x170_;
  CAssetId x174_;

public:
  CBabygothData(CInputStream&);
  CAssetId GetShellModelResId() const;
  void GetFireballDamage() const;
  CAssetId GetFireballResID() const;
  const CDamageVulnerability& GetShellDamageVulnerability() const;
  void GetSHellHitPoints() const;
  void GetFireballAttackVariance() const;
  void GetFireballAttackTime() const;
  void GetFireBreathDamage() const;
  CAssetId GetFireBreathResId() const;
};

class CBabygoth : public CPatterned {
public:
  DEFINE_PATTERNED(Babygoth)
  CBabygoth(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
            const CPatternedInfo&, const CActorParameters&, const CBabygothData&);
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr);
};

} // namespace urde::MP1
