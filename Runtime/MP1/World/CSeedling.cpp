#include "Runtime/MP1/World/CSeedling.hpp"

#include <array>

#include "Runtime/CStateManager.hpp"
#include "Runtime/World/CGameArea.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
namespace {
constexpr std::array<std::array<std::string_view, 6>, 2> skNeedleLocators{{
    {
        "A_spike1_LCTR_SDK",
        "A_spike2_LCTR_SDK",
        "A_spike3_LCTR_SDK",
        "A_spike4_LCTR_SDK",
        "A_spike5_LCTR_SDK",
        "A_spike6_LCTR_SDK",
    },
    {
        "B_spike1_LCTR_SDK",
        "B_spike2_LCTR_SDK",
        "B_spike3_LCTR_SDK",
        "B_spike4_LCTR_SDK",
        "B_spike5_LCTR_SDK",
        "B_spike6_LCTR_SDK",
    },
}};
} // Anonymous namespace

CSeedling::CSeedling(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                     CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                     CAssetId needleId, CAssetId weaponId, const CDamageInfo& dInfo1, const CDamageInfo& dInfo2,
                     float f1, float f2, float f3, float f4)
: CWallWalker(ECharacter::Seedling, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
              EMovementType::Ground, EColliderType::Zero, EBodyType::WallWalker, actParms, f1, f2,
              EKnockBackVariant::Small, f3, EWalkerType::Seedling, f4, false)
, x5d8_searchPath(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x6bc_spikeData(std::make_unique<CModelData>(CStaticRes(needleId, GetModelData()->GetScale())))
, x6c0_projectileInfo(weaponId, dInfo1)
, x6e8_deathDamage(dInfo2) {
  x6c0_projectileInfo.Token().Lock();
  CreateShadow(false);
  MakeThermalColdAndHot();
}

void CSeedling::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CSeedling::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  if (msg == EScriptObjectMessage::Activate) {
    x5d6_27_disableMove = false;
    TUniqueId id = GetWaypointForState(mgr, EScriptObjectState::Patrol, EScriptObjectMessage::Follow);
    if (id != kInvalidUniqueId)
      x2dc_destObj = id;
  } else if (msg == EScriptObjectMessage::Registered) {
    x450_bodyController->Activate(mgr);
    x704_modelBounds = GetModelData()->GetBounds();
  } else if (msg == EScriptObjectMessage::InitializedInArea) {
    x5d8_searchPath.SetArea(mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways())->GetPostConstructed()->x10bc_pathArea);
  }
}

void CSeedling::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  ++x5d4_thinkCounter;
  x5d6_26_playerObstructed = false;
  const CGameArea* area = mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways());
  CGameArea::EOcclusionState occlusionState = CGameArea::EOcclusionState::Occluded;
  if (area && area->IsPostConstructed())
    occlusionState = area->GetPostConstructed()->x10dc_occlusionState;

  if (occlusionState == CGameArea::EOcclusionState::Occluded)
    x5d6_26_playerObstructed = true;

  if (!x5d6_26_playerObstructed) {
    zeus::CVector3f playerPos = mgr.GetPlayer().GetTranslation();
    float distance = (playerPos - GetTranslation()).magnitude();
    if (distance > x5c4_playerObstructionMinDist) {
      zeus::CVector3f direction = (playerPos - GetTranslation()).normalized();
      CRayCastResult result =
          mgr.RayStaticIntersection(playerPos, direction, distance, CMaterialFilter::skPassEverything);
      if (result.IsValid())
        x5d6_26_playerObstructed = true;
    }
  }

  if (x5d6_26_playerObstructed)
    xf8_24_movable = false;

  xf8_24_movable = !x5d6_24_alignToFloor;
  CWallWalker::Think(dt, mgr);

  if (!x5d6_25_hasAlignSurface && x450_bodyController->GetPercentageFrozen() < 0.00001f && x5d6_24_alignToFloor)
    AlignToFloor(mgr, x590_colSphere.GetSphere().radius, GetTranslation() + (2.f * (dt * GetVelocity())), dt);

  if (x71c_attackCoolOff > 0.f)
    x71c_attackCoolOff -= dt;
}

void CSeedling::Render(CStateManager& mgr) {
  if (x400_25_alive && x6bc_spikeData) {
    const size_t index = x722_24_renderOnlyClusterA ? 0 : size_t(x722_25_curNeedleCluster);
    CModelFlags flags;
    flags.x2_flags = 3;
    flags.x4_color = zeus::skWhite;

    for (const std::string_view sv : skNeedleLocators[index]) {
      x6bc_spikeData->Render(mgr, GetLctrTransform(sv), x90_actorLights.get(), flags);
    }
  }

  CWallWalker::Render(mgr);
}

void CSeedling::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  if (type == EUserEventType::Projectile)
    LaunchNeedles(mgr);
  else if (type == EUserEventType::BeginAction)
    x722_24_renderOnlyClusterA = true;
  else
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

std::optional<zeus::CAABox> CSeedling::GetTouchBounds() const {
  return x704_modelBounds.getTransformedAABox(GetTransform());
}

void CSeedling::Touch(CActor& act, CStateManager& mgr) {
  if (x400_25_alive) {
    if (TCastToPtr<CPlayer> pl = act)
      MassiveDeath(mgr);
  }

  CPatterned::Touch(act, mgr);
}

void CSeedling::Patrol(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    x5d6_24_alignToFloor = true;
    x150_momentum.zeroOut();
    x5d6_25_hasAlignSurface = false;
    xf8_24_movable = false;

    TUniqueId id = (x720_prevObj != kInvalidUniqueId
                        ? x720_prevObj
                        : GetWaypointForState(mgr, EScriptObjectState::Patrol, EScriptObjectMessage::Follow));

    if (id != kInvalidUniqueId)
      x2dc_destObj = id;
  } else if (msg == EStateMsg::Update) {
    UpdateWPDestination(mgr);
    zeus::CVector3f upVec = GetTransform().upVector();
    x450_bodyController->GetCommandMgr().DeliverCmd(
        CBCLocomotionCmd(ProjectVectorToPlane((x2e0_destPos - GetTranslation()).normalized(), upVec), {}, 0.f));
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(
        ProjectVectorToPlane(ProjectVectorToPlane(x45c_steeringBehaviors.Seek(*this, x2e0_destPos), upVec), upVec), {},
        1.f));
    x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(1.f * GetTransform().frontVector(), {}, 0.f));
  } else if (msg == EStateMsg::Deactivate) {
    x720_prevObj = x2dc_destObj;
  }
}

void CSeedling::Active(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate)
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
  CPatterned::Patrol(mgr, msg, arg);
}

void CSeedling::Enraged(CStateManager&, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate)
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Internal8);
}

void CSeedling::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate)
    x32c_animState = EAnimState::Ready;
  else if (msg == EStateMsg::Update)
    TryCommand(mgr, pas::EAnimationState::ProjectileAttack, &CPatterned::TryProjectileAttack, 0);
  else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x71c_attackCoolOff = (x300_maxAttackRange * mgr.GetActiveRandom()->Float()) + x304_averageAttackTime;
  }
}

void CSeedling::Generate(CStateManager& mgr, EStateMsg msg, float) {
  if (msg == EStateMsg::Activate)
    x32c_animState = EAnimState::Ready;
  else if (msg == EStateMsg::Update)
    TryCommand(mgr, pas::EAnimationState::Generate, &CPatterned::TryGenerateNoXf, 0);
}

bool CSeedling::ShouldAttack(CStateManager& mgr, float) {
  if (x71c_attackCoolOff > 0.f)
    return false;

  return mgr.CanCreateProjectile(GetUniqueId(), EWeaponType::AI, 6);
}

void CSeedling::LaunchNeedles(CStateManager& mgr) {
  const auto& needleLocators = skNeedleLocators[size_t(x722_25_curNeedleCluster)];
  for (const std::string_view needle : needleLocators) {
    LaunchProjectile(GetLctrTransform(needle), mgr, int(needleLocators.size()), EProjectileAttrib::None, true, {},
                     0xFFFF, false, GetModelData()->GetScale());
  }

  x722_25_curNeedleCluster = !x722_25_curNeedleCluster;
  x722_24_renderOnlyClusterA = false;
}

void CSeedling::MassiveDeath(CStateManager& mgr) {
  if (x400_25_alive) {
    mgr.ApplyDamageToWorld(GetUniqueId(), *this, GetTranslation(), x6e8_deathDamage,
                           CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}));
    LaunchNeedles(mgr);
  }
  CPatterned::MassiveDeath(mgr);
}

} // namespace urde::MP1
