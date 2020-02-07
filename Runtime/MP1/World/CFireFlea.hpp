#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/World/CPathFindSearch.hpp"
#include "Runtime/World/CPatterned.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CVector3f.hpp>

namespace urde::MP1 {
class CFireFlea : public CPatterned {
  class CDeathCameraEffect : public CEntity {
    u32 x34_ = 13;
    u32 x38_ = 5;
    u32 x3c_ = 60;
    u32 x40_ = 190;
    u32 x44_ = 0;

  public:
    static const zeus::CColor skStartFadeColor;
    static const zeus::CColor skEndFadeColor;
    static zeus::CColor sCurrentFadeColor;
    CDeathCameraEffect(TUniqueId, TAreaId, std::string_view);

    void Accept(IVisitor&) override;
    void PreThink(float, CStateManager&) override;
    void Think(float, CStateManager&) override;
  };
  float x568_ = 1.f;
  float x56c_;
  rstl::reserved_vector<TUniqueId, 1024> x570_nearList;
  zeus::CVector3f xd74_;
  zeus::CVector3f xd80_targetPos;
  CPathFindSearch xd8c_pathFind;

  static s32 sLightIdx;
  zeus::CVector3f FindSafeRoute(CStateManager& mgr, const zeus::CVector3f& forward);
  bool CheckNearWater(const CStateManager&, const zeus::CVector3f& dir);

public:
  DEFINE_PATTERNED(FireFlea)

  CFireFlea(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
            const CActorParameters&, const CPatternedInfo&, float);

  void Accept(IVisitor&) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Dead(CStateManager&, EStateMsg msg, float dt) override;
  bool Delay(CStateManager&, float arg) override;
  bool InPosition(CStateManager& mgr, float dt) override;
  bool HearShot(CStateManager&, float) override;
  void TargetPatrol(CStateManager&, EStateMsg, float) override;
  void Patrol(CStateManager&, EStateMsg, float) override;
  void Flee(CStateManager&, EStateMsg, float) override;
  CPathFindSearch* GetSearchPath() override { return &xd8c_pathFind; }
};
} // namespace urde::MP1