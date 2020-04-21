#pragma once

#include "Runtime/World/CPathFindSearch.hpp"
#include "Runtime/World/CPatterned.hpp"

#include <zeus/CTransform.hpp>

namespace urde::MP1 {
class CTryclops : public CPatterned {

  static const CDamageVulnerability skVulnerabilities;
  CPathFindSearch x568_pathFindSearch;
  zeus::CTransform x64c_;
  float x67c_;
  float x680_;
  float x684_;
  float x688_launchSpeed;
  float x68c_ = 0.f;
  u32 x690_ = 0;
  TUniqueId x694_bombId = kInvalidUniqueId;
  TUniqueId x696_ = kInvalidUniqueId;
  bool x698_24_ : 1 = false;
  bool x698_25_ : 1 = false;
  bool x698_26_ : 1 = false;
  bool x698_27_dizzy : 1 = false;
  bool sub8025dbd0(CStateManager&) { return false; }
  void LaunchPlayer(CStateManager& mgr, const zeus::CTransform& xf, float);
  void DragBomb(CStateManager& mgr, const zeus::CTransform& xf);
  void ApplySeparation(CStateManager&);
  void GrabBomb(CStateManager& mgr);
  void DragPlayer(CStateManager& mgr, const zeus::CVector3f& locOrig);
  bool InRangeToLocator(const zeus::CVector3f& vec, float) const;
  bool sub80260180(const zeus::CVector3f&, const zeus::CVector3f&, const zeus::CAABox&, CStateManager&);
  void SuckPlayer(CStateManager& mgr, float);
  void AttractPlayer(CStateManager& mgr, const zeus::CVector3f& dest, float);
  void AttractBomb(CStateManager& mgr, float);

public:
  DEFINE_PATTERNED(Tryclops)
  CTryclops(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
            const CPatternedInfo&, const CActorParameters&, float, float, float, float);

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  const CDamageVulnerability* GetDamageVulnerability() const override {
    if (x698_26_)
      return CAi::GetDamageVulnerability();

    return &skVulnerabilities;
  }

  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                     const CDamageInfo&) const override {
    if (x698_26_)
      return CAi::GetDamageVulnerability();

    return &skVulnerabilities;
  }

  void DoUserAnimEvent(CStateManager&, const CInt32POINode&, EUserEventType, float) override;
  void Death(CStateManager&, const zeus::CVector3f&, EScriptObjectState) override;
  bool IsListening() const override { return true; }
  void Patrol(CStateManager&, EStateMsg, float) override;
  void PathFind(CStateManager&, EStateMsg, float) override;
  void SelectTarget(CStateManager&, EStateMsg, float) override;
  void TargetPatrol(CStateManager&, EStateMsg, float) override;
  void TargetPlayer(CStateManager&, EStateMsg, float) override;
  void TargetCover(CStateManager&, EStateMsg, float) override;
  void Attack(CStateManager&, EStateMsg, float) override;
  void JumpBack(CStateManager&, EStateMsg, float) override;
  void Shuffle(CStateManager&, EStateMsg, float) override;
  void TurnAround(CStateManager&, EStateMsg, float) override;
  void Crouch(CStateManager&, EStateMsg, float) override;
  void GetUp(CStateManager&, EStateMsg, float) override;
  void Suck(CStateManager&, EStateMsg, float) override;
  void Cover(CStateManager&, EStateMsg, float) override;
  void Approach(CStateManager&, EStateMsg, float) override;
  void PathFindEx(CStateManager&, EStateMsg, float) override;
  void Dizzy(CStateManager&, EStateMsg, float) override;
  bool InAttackPosition(CStateManager&, float) override;
  bool InRange(CStateManager&, float) override;
  bool InMaxRange(CStateManager&, float) override;
  bool InDetectionRange(CStateManager&, float) override;
  bool SpotPlayer(CStateManager&, float) override;
  bool InPosition(CStateManager&, float) override;
  bool HearShot(CStateManager&, float) override;
  bool CoverBlown(CStateManager&, float) override;
  bool Inside(CStateManager&, float) override;
  bool ShouldRetreat(CStateManager&, float) override;
  bool IsDizzy(CStateManager&, float) override;
  CPathFindSearch* GetSearchPath() override { return &x568_pathFindSearch; }
};
} // namespace urde::MP1
