#pragma once

#include "World/CPatterned.hpp"
#include "World/CPathFindSearch.hpp"

namespace urde::MP1 {
class CTryclops : public CPatterned {

  static const CDamageVulnerability skVulnerabilities;
  CPathFindSearch x568_pathFindSearch;
  zeus::CTransform x64c_;
  float x67c_;
  float x680_;
  float x684_;
  float x688_;
  float x68c_ = 0.f;
  u32 x690_ = 0;
  TUniqueId x694_bombId = kInvalidUniqueId;
  TUniqueId x696_ = kInvalidUniqueId;
  bool x698_24_ : 1;
  bool x698_25_ : 1;
  bool x698_26_ : 1;
  bool x698_27_dizzy : 1;
  bool sub8025dbd0(CStateManager&) { return false; }
  void LaunchPlayer(CStateManager& mgr, const zeus::CTransform& xf, float);
  void DragBomb(CStateManager& mgr, const zeus::CTransform& xf);
  void ApplySeparation(CStateManager&);
  void GrabBomb(CStateManager& mgr);
  void DragPlayer(CStateManager& mgr, const zeus::CVector3f& locOrig);
  bool sub802600c8(const zeus::CVector3f&, float);
  bool sub80260180(const zeus::CVector3f&, const zeus::CVector3f&, const zeus::CAABox&, CStateManager&);
  void SuckPlayer(CStateManager& mgr, float);
  void AttractPlayer(CStateManager& mgr, const zeus::CVector3f& dest, float);

public:
  DEFINE_PATTERNED(Tryclops)
  CTryclops(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
            const CPatternedInfo&, const CActorParameters&, float, float, float, float);

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
  void Think(float, CStateManager&);
  const CDamageVulnerability* GetDamageVulnerability() const {
    if (x698_26_)
      return CAi::GetDamageVulnerability();

    return &skVulnerabilities;
  }

  const CDamageVulnerability* GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                     const CDamageInfo&) const {
    if (x698_26_)
      return CAi::GetDamageVulnerability();

    return &skVulnerabilities;
  }

  void DoUserAnimEvent(CStateManager&, const CInt32POINode&, EUserEventType, float);
  void Death(CStateManager&, const zeus::CVector3f&, EScriptObjectState);
  bool IsListening() const { return true; }
  void Patrol(CStateManager&, EStateMsg, float);
  void PathFind(CStateManager&, EStateMsg, float);
  void SelectTarget(CStateManager&, EStateMsg, float);
  void TargetPatrol(CStateManager&, EStateMsg, float);
  void TargetPlayer(CStateManager&, EStateMsg, float);
  void TargetCover(CStateManager&, EStateMsg, float);
  void Attack(CStateManager&, EStateMsg, float);
  void JumpBack(CStateManager&, EStateMsg, float);
  void Shuffle(CStateManager&, EStateMsg, float);
  void TurnAround(CStateManager&, EStateMsg, float);
  void Crouch(CStateManager&, EStateMsg, float);
  void GetUp(CStateManager&, EStateMsg, float);
  void Suck(CStateManager&, EStateMsg, float);
  void Cover(CStateManager&, EStateMsg, float);
  void Approach(CStateManager&, EStateMsg, float);
  void PathFindEx(CStateManager&, EStateMsg, float);
  void Dizzy(CStateManager&, EStateMsg, float);
  bool InAttackPosition(CStateManager&, float);
  bool InRange(CStateManager&, float);
  bool InMaxRange(CStateManager&, float);
  bool InDetectionRange(CStateManager&, float);
  bool SpotPlayer(CStateManager&, float);
  bool InPosition(CStateManager&, float);
  bool HearShot(CStateManager&, float);
  bool CoverBlown(CStateManager&, float);
  bool Inside(CStateManager&, float);
  bool ShouldRetreat(CStateManager&, float);
  bool IsDizzy(CStateManager&, float);
  CPathFindSearch* GetSearchPath() { return &x568_pathFindSearch; }
};
} // namespace urde::MP1
