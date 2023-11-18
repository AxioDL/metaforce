#include "Runtime/MP1/World/CFireFlea.hpp"

#include "Runtime/CPlayerState.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce::MP1 {
namespace {
constexpr zeus::CColor skEndFadeColor{1.f, 1.f, 0.5f, 1.f};
constexpr zeus::CColor skStartFadeColor{1.f, 0.f, 0.f, 0.f};
} // Anonymous namespace

// region Fire Flea Death Camera

zeus::CColor CFireFlea::CDeathCameraEffect::sCurrentFadeColor = zeus::skClear;

CFireFlea::CDeathCameraEffect::CDeathCameraEffect(TUniqueId uid, TAreaId areaId, std::string_view name)
: CEntity(uid, CEntityInfo(areaId, CEntity::NullConnectionList), true, name) {}

void CFireFlea::CDeathCameraEffect::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CFireFlea::CDeathCameraEffect::PreThink(float dt, CStateManager& mgr) {
  auto& filterPass = mgr.GetCameraFilterPass(5);
  u32 r5 = x34_ + x38_;
  u32 r8 = r5 + x3c_;
  u32 r31 = r8 + x40_;
  if (x44_ >= x34_ && x44_ <= r5) {
    sCurrentFadeColor += zeus::CColor::lerp(skStartFadeColor, skEndFadeColor, x34_ - x44_);
    filterPass.SetFilter(EFilterType::Blend, EFilterShape::Fullscreen, 0.f, sCurrentFadeColor, CAssetId());
  } else if (x44_ >= r8 && x44_ <= r31) {
    sCurrentFadeColor += zeus::CColor::lerp(skEndFadeColor, skStartFadeColor, r8 - x44_);
    filterPass.SetFilter(EFilterType::Blend, EFilterShape::Fullscreen, 0.f, sCurrentFadeColor, CAssetId());
  } else if (x44_ > r5) {
    sCurrentFadeColor = skEndFadeColor;
    filterPass.SetFilter(EFilterType::Blend, EFilterShape::Fullscreen, 0.f, sCurrentFadeColor, CAssetId());
  }

  if (r31 == x44_) {
    filterPass.DisableFilter(0.f);
    mgr.FreeScriptObject(GetUniqueId());
    x44_ = 0;
  } else
    x44_++;

  if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::Thermal)
    filterPass.DisableFilter(0.f);
}

void CFireFlea::CDeathCameraEffect::Think(float dt, CStateManager& mgr) { sCurrentFadeColor = zeus::skClear; }

// endregion

s32 CFireFlea::sLightIdx = 0;

CFireFlea::CFireFlea(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo, float f1)
: CPatterned(ECharacter::FireFlea, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Flyer, actParms, EKnockBackVariant::Small)
, x56c_(f1)
, xd8c_pathFind(nullptr, 3, pInfo.GetPathfindingIndex(), 1.f, 1.f) {
  CMaterialFilter filter = GetMaterialFilter();
  filter.ExcludeList().Add(EMaterialTypes::Character);
  SetMaterialFilter(filter);

  GetModelData()->GetAnimationData()->SetParticleLightIdx(sLightIdx);
  ++sLightIdx;
}

void CFireFlea::Dead(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg != EStateMsg::Activate || mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::EPlayerVisor::Thermal)
    return;

  mgr.AddObject(new CDeathCameraEffect(mgr.AllocateUniqueId(), GetAreaIdAlways(), ""sv));
}

bool CFireFlea::Delay(CStateManager&, float arg) { return x330_stateMachineState.GetTime() > 0.5f; }

void CFireFlea::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CFireFlea::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  if (msg == EScriptObjectMessage::Registered) {
    x450_bodyController->Activate(mgr);
  } else if (msg == EScriptObjectMessage::InitializedInArea) {
    xd8c_pathFind.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
    xd8c_pathFind.SetPadding(50.0f);
  }
}

bool CFireFlea::InPosition(CStateManager& mgr, float dt) {
  if (x2dc_destObj == kInvalidUniqueId)
    return false;
  return (xd80_targetPos - GetTranslation()).magnitude() < 25.f;
}

bool CFireFlea::HearShot(metaforce::CStateManager& mgr, float arg) {
  x570_nearList.clear();
  mgr.BuildNearList(x570_nearList, zeus::CAABox(GetTranslation() - 10.f, GetTranslation() + 10.f),
                    CMaterialFilter::MakeInclude({EMaterialTypes::Projectile}), nullptr);

  return !x570_nearList.empty();
}

void CFireFlea::TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    CPatterned::Patrol(mgr, msg, arg);
    UpdateDest(mgr);
    xd80_targetPos = x2e0_destPos;
  } else if (msg == EStateMsg::Update) {
    if (auto* pathFind = GetSearchPath()) {
      if (pathFind->GetResult() != CPathFindSearch::EResult::Success) {
        zeus::CVector3f closestPoint = zeus::skZero3f;
        if (pathFind->FindClosestReachablePoint(GetTranslation(), closestPoint) == CPathFindSearch::EResult::Success) {
          zeus::CVector3f delta = FindSafeRoute(mgr, x45c_steeringBehaviors.Arrival(*this, xd80_targetPos, 5.f));
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(delta, {}, 1.f));
        }
      } else {
        PathFind(mgr, msg, arg);
      }
    } else {
      x450_bodyController->GetCommandMgr().DeliverCmd(
          CBCLocomotionCmd(x45c_steeringBehaviors.Arrival(*this, xd80_targetPos, 5.f), {}, 1.f));
    }
  }
}

zeus::CVector3f CFireFlea::FindSafeRoute(CStateManager& mgr, const zeus::CVector3f& forward) const {
  const float mag = forward.magnitude();
  if (mag <= 0.f) {
    return {};
  }

  const CRayCastResult res = mgr.RayStaticIntersection(GetTranslation(), forward.normalized(), 1.f,
                                                       CMaterialFilter::MakeInclude({EMaterialTypes::Solid}));
  if (res.IsInvalid() && !CheckNearWater(mgr, forward.normalized())) {
    return forward;
  }

  const zeus::CVector3f right = forward.normalized().cross(zeus::skUp).normalized();
  const CRayCastResult res1 =
      mgr.RayStaticIntersection(GetTranslation(), right, 1.f, CMaterialFilter::MakeInclude({EMaterialTypes::Solid}));
  if (res1.IsInvalid()) {
    return mag * right;
  }

  const zeus::CVector3f left = -right;
  const CRayCastResult res2 =
      mgr.RayStaticIntersection(GetTranslation(), left, 1.f, CMaterialFilter::MakeInclude({EMaterialTypes::Solid}));
  if (res2.IsInvalid()) {
    return mag * left;
  }

  const zeus::CVector3f up = right.cross(forward.normalized());
  const CRayCastResult res3 =
      mgr.RayStaticIntersection(GetTranslation(), up, 1.f, CMaterialFilter::MakeInclude({EMaterialTypes::Solid}));
  if (res3.IsInvalid()) {
    return mag * up;
  }

  const zeus::CVector3f down = -up;
  const CRayCastResult res4 =
      mgr.RayStaticIntersection(GetTranslation(), down, 1.f, CMaterialFilter::MakeInclude({EMaterialTypes::Solid}));
  if (res4.IsInvalid()) {
    return mag * down;
  }

  return -forward;
}

bool CFireFlea::CheckNearWater(const CStateManager& mgr, const zeus::CVector3f& dir) const {
  EntityList nearList;
  mgr.BuildNearList(nearList, GetTranslation(), dir, 2.f, CMaterialFilter::skPassEverything, nullptr);

  for (const auto& id : nearList) {
    if (TCastToConstPtr<CScriptWater>(mgr.GetObjectById(id)))
      return true;
  }

  return false;
}

void CFireFlea::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  if (!zeus::close_enough(x310_moveVec, {}))
    x310_moveVec.normalize();

  x310_moveVec = FindSafeRoute(mgr, x310_moveVec);
  CPatterned::Patrol(mgr, msg, arg);
  if (x2d8_patrolState == EPatrolState::Done)
    mgr.FreeScriptObject(GetUniqueId());
}

void CFireFlea::Flee(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
  } else if (msg == EStateMsg::Update) {
    if (x570_nearList.empty()) {
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(FindSafeRoute(mgr, xd74_), {}, 1.f));
    } else {
      for (TUniqueId id : x570_nearList) {
        if (const CActor* act = static_cast<const CActor*>(mgr.GetObjectById(id))) {
          zeus::CVector3f fleeDirection = x45c_steeringBehaviors.Flee(*this, act->GetTranslation());
          x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(FindSafeRoute(mgr, fleeDirection), {}, 1.f));
        }
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
  }
}
} // namespace metaforce::MP1
