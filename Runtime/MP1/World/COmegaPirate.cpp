#include "Runtime/MP1/World/COmegaPirate.hpp"

#include "Runtime/Camera/CGameCamera.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/Collision/CGameCollision.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptEffect.hpp"
#include "Runtime/World/CScriptPlatform.hpp"
#include "Runtime/World/CScriptSound.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CWorld.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
namespace {
constexpr std::array<SSphereJointInfo, 1> skSphereJoints{{
    {"lockon_target_LCTR", 1.f},
}};

constexpr std::array<SOBBJointInfo, 11> skObbJoints{{
    {"Spine_2", "Collar", zeus::skOne3f},
    {"R_toe_1", "R_ankle", zeus::skOne3f},
    {"L_toe_1", "L_ankle", zeus::skOne3f},
    {"R_knee", "R_ankle", zeus::skOne3f},
    {"L_knee", "L_ankle", zeus::skOne3f},
    {"R_elbow", "R_wrist", zeus::skOne3f},
    {"L_elbow", "L_wrist", zeus::skOne3f},
    {"R_wrist", "R_index_1", zeus::skOne3f},
    {"L_wrist", "L_index_1", zeus::skOne3f},
    {"R_index_1", "R_index_3_SDK", zeus::CVector3f{2.f}},
    {"L_index_1", "L_index_3_SDK", zeus::CVector3f{2.f}},
}};
} // namespace

COmegaPirate::CFlash::CFlash(TUniqueId uid, const CEntityInfo& info, const zeus::CVector3f& pos,
                             TLockedToken<CTexture>& thermalSpot, float delay)
: CActor(uid, true, "Omega Pirate Flash", info, zeus::CTransform::Translate(pos), CModelData::CModelDataNull(), {},
         CActorParameters::None(), kInvalidUniqueId)
, xf4_delay(delay)
, m_thermalSpotBlend(EFilterType::Blend, thermalSpot)
, m_thermalSpotSubtract(EFilterType::Subtract, thermalSpot) {}

void COmegaPirate::CFlash::Accept(IVisitor& visitor) { visitor.Visit(this); }

void COmegaPirate::CFlash::Think(float dt, CStateManager& mgr) {
  CEntity::Think(dt, mgr);
  xf4_delay -= dt;
  if (xf4_delay > 0.f) {
    return;
  }

  xf8_ += dt;
  float intensity = xf8_;
  if (intensity <= 0.75f) {
    intensity /= 0.75f;
  } else {
    intensity = 1.f - (intensity - 0.75f) / 0.25f;
  }
  const CGameCamera* const camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
  const zeus::CVector3f dist = (GetTranslation() - camera->GetTranslation()).normalized();
  float dot = dist.dot(camera->GetTransform().frontVector());
  float dVar4 = 0.f;
  if (dot >= 0.f) {
    dVar4 = dot * dot;
  }
  xfc_ = dVar4 * intensity;
  if (xf8_ > 1.f) {
    mgr.FreeScriptObject(GetUniqueId());
  }
}

void COmegaPirate::CFlash::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  mgr.RenderLast(GetUniqueId());
  // if (xf0_thermalSpot == nullptr && xe8_thermalSpotToken.IsLocked() && xe8_thermalSpotToken.HasReference()) {
  //   xf0_thermalSpot = xe8_thermalSpotToken.GetObj();
  // }
}

void COmegaPirate::CFlash::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {}

void COmegaPirate::CFlash::Render(CStateManager& mgr) {
  const CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetActiveVisor(mgr);
  if (visor == CPlayerState::EPlayerVisor::Thermal) {
    return;
  }

  float alphaMul = 35.f;
  CTexturedQuadFilter* filter = nullptr;
  if (visor == CPlayerState::EPlayerVisor::XRay) {
    // CGraphics::SetBlendMode(ERglBlendMode::Subtract, ERglBlendFactor::One, ERglBlendFactor::Zero,
    // ERglLogicOp::Clear);
    filter = &m_thermalSpotSubtract;
    alphaMul = 60.f;
  } else {
    // CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One,
    // ERglLogicOp::Clear);
    filter = &m_thermalSpotBlend;
  }

  CGraphics::SetModelMatrix(zeus::CTransform());
  zeus::CColor useColor{1.f, std::max(0.f, (xfc_ * alphaMul) / 255.f)};

  const auto rightVec = CGraphics::g_GXModelView.rightVector();
  const auto upVec = CGraphics::g_GXModelView.upVector();
  const auto rvS = GetTranslation() - rightVec;
  const auto rvP = GetTranslation() + rightVec;
  const std::array<CTexturedQuadFilter::Vert, 4> verts{{
      {rvS + upVec, {0.f, 0.f}},
      {rvS - upVec, {1.f, 0.f}},
      {rvP - upVec, {1.f, 1.f}},
      {rvP + upVec, {0.f, 1.f}},
  }};
  filter->drawVerts(useColor, verts);
}

COmegaPirate::COmegaPirate(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms,
                           CElitePirateData data, CAssetId w1, CAssetId w2, CAssetId w3)
: CElitePirate(uid, name, info, xf, std::move(mData), pInfo, actParms, data)
, x9d0_(GetModelData()->GetScale())
, x9f0_(*g_SimplePool, w1, w2, w3, 0, 0)
, xb70_thermalSpot(g_SimplePool->GetObj("Thermal_Spot_2"sv)) {
  x9a4_scriptWaypointPlatforms.reserve(3);
  x9b8_scriptEffects.reserve(24);
  x9dc_scriptPlatforms.reserve(4);
  xaa0_scriptSounds.reserve(4);
  xab4_.reserve(3);
  xb7c_.resize(4, 0);

  SetMass(100000.f);

  CMaterialFilter filter = GetMaterialFilter();
  filter.ExcludeList().Add(
      CMaterialList{EMaterialTypes::Character, EMaterialTypes::CollisionActor, EMaterialTypes::Platform});
  SetMaterialFilter(filter);

  GetSearchPath()->SetPadding(20.f);
}

void COmegaPirate::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  switch (msg) {
  default:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    break;
  case EScriptObjectMessage::Activate:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    xa38_collisionActorMgr1->SetActive(mgr, true);
    xa9c_collisionActorMgr2->SetActive(mgr, true);
    GetKnockBackController().SetAutoResetImpulse(false);
    if (auto* entity = mgr.ObjectById(x990_launcherId2)) {
      entity->SetActive(true);
    }
    break;
  case EScriptObjectMessage::Deactivate:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    xa38_collisionActorMgr1->SetActive(mgr, false);
    xa9c_collisionActorMgr2->SetActive(mgr, false);
    if (auto* entity = mgr.ObjectById(x990_launcherId2)) {
      entity->SetActive(false);
    }
    break;
  case EScriptObjectMessage::Decrement:
    x9ec_ = true;
    break;
  case EScriptObjectMessage::Increment:
    SetShotAt(true, mgr);
    break;
  case EScriptObjectMessage::Open:
    xb7c_[3] -= xb7c_[3] == 0 ? 0 : 1;
    break;
  case EScriptObjectMessage::Reset:
    xb78_ = true;
    break;
  case EScriptObjectMessage::SetToMax:
    xa3c_hearPlayer = true;
    break;
  case EScriptObjectMessage::SetToZero:
    xb7c_[2] -= xb7c_[2] == 0 ? 0 : 1;
    break;
  case EScriptObjectMessage::Start:
    x3b4_speed = 1.f;
    ++xade_;
    if (xade_ < 4) {
      GetBodyController()->GetCommandMgr().DeliverCmd(CBCKnockBackCmd(zeus::skLeft, pas::ESeverity::One));
    }
    break;
  case EScriptObjectMessage::Stop:
    Destroy(mgr);
    break;
  case EScriptObjectMessage::StopAndReset:
    xb7c_[1] -= xb7c_[1] == 0 ? 0 : 1;
    break;
  case EScriptObjectMessage::UNKM18:
    xb7c_[0] -= xb7c_[0] == 0 ? 0 : 1;
    break;
  case EScriptObjectMessage::Action:
    x3b4_speed = 1.f;
    ++xade_;
    if (xade_ < 4) {
      GetBodyController()->GetCommandMgr().DeliverCmd(CBCKnockBackCmd(zeus::skRight, pas::ESeverity::One));
    }
    break;
  case EScriptObjectMessage::Alert:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    break;
  case EScriptObjectMessage::Touched:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    if (uid == x990_launcherId2 && x990_launcherId2 != kInvalidUniqueId) {
      SetShotAt(true, mgr);
    }
    if (TCastToPtr<CCollisionActor> actor = mgr.ObjectById(uid)) {
      if (TCastToPtr<CPlayer> player = mgr.ObjectById(actor->GetLastTouchedObject())) {
        if (x420_curDamageRemTime <= 0.f) {
          mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
                          CMaterialFilter::MakeInclude({EMaterialTypes::Solid}), zeus::skZero3f);
          x420_curDamageRemTime = x424_damageWaitTime;
        }
      }
    }
    break;
  case EScriptObjectMessage::Registered:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    x990_launcherId2 = mgr.AllocateUniqueId();
    CreateGrenadeLauncher(mgr, x990_launcherId2);
    SetupCollisionManager(mgr);
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Internal8);
    x402_27_noXrayModel = false;
    xa4c_initialXf = x34_transform;
    xa98_maxEnergy = HealthInfo(mgr)->GetHP();
    if (auto* actor = static_cast<CActor*>(mgr.ObjectById(GetLauncherId()))) {
      actor->RemoveMaterial(EMaterialTypes::Scannable, mgr);
    }
    if (auto* actor = static_cast<CActor*>(mgr.ObjectById(x990_launcherId2))) {
      actor->RemoveMaterial(EMaterialTypes::Scannable, mgr);
    }
    GetKnockBackController().SetAutoResetImpulse(false);
    SetupPathFindSearch();
    break;
  case EScriptObjectMessage::Deleted:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    xa38_collisionActorMgr1->Destroy(mgr);
    xa9c_collisionActorMgr2->Destroy(mgr);
    mgr.FreeScriptObject(x990_launcherId2);
    break;
  case EScriptObjectMessage::InitializedInArea:
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);

    for (const SConnection& conn : GetConnectionList()) {
      TUniqueId connId = mgr.GetIdForScript(conn.x8_objId);
      if (connId == kInvalidUniqueId || conn.x0_state != EScriptObjectState::Attack) {
        continue;
      }

      if (conn.x4_msg == EScriptObjectMessage::Activate) {
        if (TCastToPtr<CScriptEffect> effect = mgr.ObjectById(connId)) {
          x9b8_scriptEffects.emplace_back(connId, effect->GetName());
        } else if (TCastToPtr<CScriptPlatform> platform = mgr.ObjectById(connId)) {
          x9dc_scriptPlatforms.emplace_back(connId, platform->GetName());
          platform->AddMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, EMaterialTypes::Character, mgr);
          platform->RemoveMaterial(EMaterialTypes::Scannable, mgr);
          CMaterialList excludes = platform->GetMaterialFilter().GetExcludeList();
          excludes.Add({EMaterialTypes::Player, EMaterialTypes::Character, EMaterialTypes::CollisionActor});
          CMaterialList includes = GetMaterialFilter().GetIncludeList();
          platform->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(includes, excludes));
          xae4_platformVuln = *platform->GetDamageVulnerability();
          xb54_platformColor = platform->GetDrawFlags().x4_color;
        } else if (TCastToPtr<CScriptSound> sound = mgr.ObjectById(connId)) {
          xaa0_scriptSounds.emplace_back(connId, sound->GetName());
        }
      } else if (conn.x4_msg == EScriptObjectMessage::Follow) {
        if (TCastToPtr<CScriptWaypoint> waypoint = mgr.ObjectById(connId)) {
          std::vector<TUniqueId> waypointPlatformIds;
          waypointPlatformIds.reserve(3);
          for (const SConnection& waypointConn : waypoint->GetConnectionList()) {
            auto waypointConnId = mgr.GetIdForScript(waypointConn.x8_objId);
            if (TCastToPtr<CScriptPlatform> platform = mgr.ObjectById(waypointConnId)) {
              platform->AddMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
              waypointPlatformIds.push_back(waypointConnId);
            }
          }
          x9a4_scriptWaypointPlatforms.emplace_back(connId, waypointPlatformIds);
        }
      }
    }
    break;
  case EScriptObjectMessage::Damage:
    if (uid == x990_launcherId2 && x990_launcherId2 != kInvalidUniqueId) {
      GetBodyController()->GetCommandMgr().DeliverCmd(
          CBCKnockBackCmd(GetTransform().frontVector(), pas::ESeverity::Eight));
    }
    CElitePirate::AcceptScriptMsg(msg, uid, mgr);
    if (uid == xa46_ && xa7c_ == 2) {
      xa7c_ = 3;
      xa84_ = 0.f;
    }
    break;
  case EScriptObjectMessage::InvulnDamage:
    if (const TCastToConstPtr<CGameProjectile> projectile = mgr.GetObjectById(uid)) {
      if (xa4a_) {
        mgr.ApplyDamage(uid, xa46_, projectile->GetOwnerId(), projectile->GetDamageInfo(),
                        CMaterialFilter::MakeInclude({EMaterialTypes::Solid}), zeus::skZero3f);
      }
    }
    SetShotAt(true, mgr);
  }
}

bool COmegaPirate::AggressionCheck(CStateManager& mgr, float arg) {
  return x990_launcherId2 == kInvalidUniqueId && CElitePirate::AggressionCheck(mgr, arg);
}

void COmegaPirate::Attack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x402_28_isMakingBigStrike = true;
    x504_damageDur = 1.f;
  } else if (msg == EStateMsg::Deactivate) {
    x402_28_isMakingBigStrike = false;
    x504_damageDur = 0.f;
  }
  CElitePirate::Attack(mgr, msg, dt);
}

bool COmegaPirate::CodeTrigger(CStateManager&, float) { return xb78_; }

void COmegaPirate::Cover(CStateManager& mgr, EStateMsg msg, float dt) {
  CElitePirate::Cover(mgr, msg, dt);
  if (msg == EStateMsg::Activate) {
    xad4_cachedSpeed = x3b4_speed;
    xad8_cover = true;
  } else if (msg == EStateMsg::Deactivate) {
    xad8_cover = false;
  }
}

bool COmegaPirate::CoverBlown(CStateManager&, float) {
  if (x9b4_) {
    x9b4_ = false;
    xb5c_ = 0.f;
    return true;
  }
  return false;
}

void COmegaPirate::Dizzy(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    xa44_ = true;
  } else if (msg == EStateMsg::Update) {
    GetBodyController()->GetCommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::EReactionType::Two));
  } else if (msg == EStateMsg::Deactivate) {
    GetBodyController()->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
  }
}

void COmegaPirate::DoubleSnap(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    SendScriptMsgs(EScriptObjectState::MaxReached, mgr, EScriptObjectMessage::None);
    SetShotAt(false, mgr);
    SetState(CElitePirate::EState::Zero);
    xa44_ = false;
    xa4a_ = false;
    xa88_ = false;
    xa8c_ = 3.f;
    for (auto& entry : x9dc_scriptPlatforms) {
      if (auto* platform = static_cast<CScriptPlatform*>(mgr.ObjectById(entry.first))) {
        platform->SetActive(true);
        platform->SetDamageVulnerability(xae4_platformVuln);
        platform->AddMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
        platform->SetDisableXRayAlpha(false);
        platform->SetXRayFog(true);
      }
    }
    xb64_ = 17.f;
    AddMaterial(EMaterialTypes::Scannable, mgr);
  } else if (msg == EStateMsg::Update) {
    if (GetState() == CElitePirate::EState::Zero) {
      if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::Step) {
        SetState(CElitePirate::EState::Two);
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Backward, pas::EStepType::BreakDodge));
      }
    } else if (GetState() == CElitePirate::EState::Two &&
               GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Step) {
      SetState(CElitePirate::EState::Over);
    }
  } else if (msg == EStateMsg::Deactivate) {
    if (auto* launcher = static_cast<CGrenadeLauncher*>(mgr.ObjectById(GetLauncherId()))) {
      launcher->SetFollowPlayer(true);
    }
    if (auto* launcher = static_cast<CGrenadeLauncher*>(mgr.ObjectById(x990_launcherId2))) {
      launcher->SetFollowPlayer(true);
    }
  }
}

void COmegaPirate::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  switch (type) {
  case EUserEventType::EggLay:
    if (x990_launcherId2 != kInvalidUniqueId) {
      if (auto* entity = mgr.ObjectById(x990_launcherId2)) {
        mgr.SendScriptMsg(entity, GetUniqueId(), EScriptObjectMessage::Action);
      }
    }
    break;
  case EUserEventType::FadeIn:
    x9a1_ = true;
    break;
  case EUserEventType::FadeOut:
    if (x994_ != 2 && x9a1_) {
      x994_ = 1;
      xa30_ = 3;
    }
    break;
  case EUserEventType::ObjectPickUp:
    xab4_.clear();
    xac8_ = 0;
    ++xacc_;
    if (xac4_ == 0) {
      sub_8028cbec(2, mgr);
    } else if (xac4_ == 1) {
      sub_8028cbec(1, mgr);
      sub_8028cbec(1, mgr);
    } else if (xac4_ == 2) {
      sub_8028cbec(2, mgr);
      sub_8028cbec(1, mgr);
    } else if (xac4_ == 3) {
      sub_8028cbec(1, mgr);
      sub_8028cbec(1, mgr);
      sub_8028cbec(1, mgr);
    }
    SendScriptMsgs(EScriptObjectState::Arrived, mgr, EScriptObjectMessage::None);
    break;
  case EUserEventType::Projectile:
  case EUserEventType::DamageOn:
  case EUserEventType::DamageOff:
  case EUserEventType::ScreenShake:
  case EUserEventType::BeginAction:
  case EUserEventType::BecomeShootThrough:
  default:
    CElitePirate::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void COmegaPirate::Enraged(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    SetState(EState::Zero);
  } else if (msg == EStateMsg::Update) {
    if (GetState() == CElitePirate::EState::Zero) {
      if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::Taunt) {
        SetState(CElitePirate::EState::Two);
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCTauntCmd(pas::ETauntType::Zero));
      }
    } else if (GetState() == CElitePirate::EState::Two &&
               GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Taunt) {
      SetState(CElitePirate::EState::Over);
    }
  } else if (msg == EStateMsg::Deactivate) {
    xadf_ = true;
    xae0_ = true;
  }
}

void COmegaPirate::Explode(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    SetState(EState::Zero);
    xad0_ = false;
  } else if (msg == EStateMsg::Update) {
    if (GetState() == CElitePirate::EState::Zero) {
      if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::Step) {
        SetState(CElitePirate::EState::Two);
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Forward, pas::EStepType::Dodge));
      }
    } else if (GetState() == CElitePirate::EState::Two &&
               GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Step) {
      SetState(CElitePirate::EState::Over);
    }
  } else if (msg == EStateMsg::Deactivate) {
    GetBodyController()->SetLocomotionType(xa40_);
  }
}

void COmegaPirate::Faint(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    GetBodyController()->GetCommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::EReactionType::Zero));
    xa44_ = true;
    xa4a_ = true;
    if (xa7c_ == 2) {
      xa8c_ = 0.333f;
    }
    for (const auto& entry : x9dc_scriptPlatforms) {
      if (auto* platform = static_cast<CScriptPlatform*>(mgr.ObjectById(entry.first))) {
        platform->SetActive(true);
      }
    }
  } else if (msg == EStateMsg::Update) {
    if (xb4c_ < 4 && x9c8_ == 0 && xb58_ >= 2.5f) {
      float alpha = std::max(xb50_, 1.f);
      float invAlpha = 1.f - alpha;
      size_t idx = 0;
      for (const auto& entry : x9dc_scriptPlatforms) {
        if (auto* platform = static_cast<CScriptPlatform*>(mgr.ObjectById(entry.first))) {
          if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay) {
            if (xb4c_ > idx) {
              CModelFlags flags{5, 0, 3, zeus::skWhite};
              flags.addColor = zeus::skBlack;
              platform->SetDrawFlags(flags);
            } else if (xb4c_ == idx) {
              if (!xb6e_) {
                SendScriptMsgs(EScriptObjectState::Entered, mgr, EScriptObjectMessage::None);
                xb6e_ = true;
              }
              CModelFlags flags{5, 0, 3, zeus::skWhite};
              flags.addColor = zeus::CColor{invAlpha, alpha};
              platform->SetDrawFlags(flags);
            }
          } else {
            CModelFlags flags{5, 0, 3, zeus::skWhite};
            flags.addColor = zeus::CColor{1.f, 0.f};
            platform->SetDrawFlags(flags);
          }
        }
        ++idx;
      }
      if (xb50_ > 1.f) {
        ++xb4c_;
        xb50_ = 0.f;
        xb58_ = 0.f;
        xb6e_ = false;
      }
      xb50_ += dt;
    }
    xb58_ += dt;
    GetBodyController()->GetCommandMgr().DeliverCmd(CBCLoopReactionCmd(pas::EReactionType::Zero));
  } else if (msg == EStateMsg::Deactivate) {
    GetBodyController()->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
    if (xb58_ >= 2.5f) {
      ++xb4c_;
    }
  }
}

void COmegaPirate::Growth(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    x9c8_ = 2;
    xad0_ = false;
    RemoveMaterial(EMaterialTypes::RadarObject, EMaterialTypes::Scannable, mgr);
    xb6c_ = false;
    xb6d_ = false;
    ProcessSoundEvent(0xb27, 1.f, 0, 0.1f, 1000.f, 0.16f, 1.f, zeus::skZero3f, GetTranslation(), mgr.GetNextAreaId(),
                      mgr, false);
  } else if (msg == EStateMsg::Update) {
    if (xb68_ == 0) {
      if (x330_stateMachineState.GetTime() > 0.3f * xb64_ && !xb6c_) {
        SendScriptMsgs(EScriptObjectState::Exited, mgr, EScriptObjectMessage::None);
        xb6c_ = true;
      }
      if (x330_stateMachineState.GetTime() > 0.6f * xb64_ && !xb6d_) {
        SendScriptMsgs(EScriptObjectState::Exited, mgr, EScriptObjectMessage::None);
        xb6d_ = true;
      }
    } else if (x330_stateMachineState.GetTime() > 0.5f * xb64_ && !xb6c_) {
      SendScriptMsgs(EScriptObjectState::Exited, mgr, EScriptObjectMessage::None);
      xb6c_ = true;
    }
  } else if (msg == EStateMsg::Deactivate) {
    TeleportToFurthestPlatform(mgr);
    xad0_ = true;
    AddMaterial(EMaterialTypes::RadarObject, mgr);
    ProcessSoundEvent(0xb28, 1.f, 0, 0.1f, 1000.f, 0.16f, 1.f, zeus::skZero3f, GetTranslation(), mgr.GetNextAreaId(),
                      mgr, false);
  }
}

void COmegaPirate::JumpBack(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    SetShotAt(false, mgr);
    SetState(CElitePirate::EState::Two);
    xade_ = 0;
    xadf_ = false;
    xae0_ = false;
    xb68_ = 0;
    xa40_ = GetBodyController()->GetLocomotionType();
    GetBodyController()->SetLocomotionType(pas::ELocomotionType::Internal5);
    GetBodyController()->GetCommandMgr().DeliverCmd(
        CBCKnockBackCmd(GetTransform().frontVector(), pas::ESeverity::Five));
    for (const auto& entry : x9dc_scriptPlatforms) {
      if (auto* platform = static_cast<CScriptPlatform*>(mgr.ObjectById(entry.first))) {
        platform->SetActive(false);
      }
    }
  } else if (msg == EStateMsg::Update) {
    if (GetState() == CElitePirate::EState::Two &&
        GetBodyController()->GetCurrentStateId() != pas::EAnimationState::KnockBack) {
      SetState(CElitePirate::EState::Over);
    }
  }
}

bool COmegaPirate::Landed(CStateManager& mgr, float arg) { return (xb4c_ & 0xe7) == 0; }

zeus::CVector3f COmegaPirate::GetOrbitPosition(const CStateManager& mgr) const {
  if (x990_launcherId2 != kInvalidUniqueId &&
      mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Thermal) {
    if (const auto* actor = static_cast<const CActor*>(mgr.GetObjectById(x990_launcherId2))) {
      return GetLockOnPosition(actor);
    }
  }
  return CElitePirate::GetOrbitPosition(mgr);
}

bool COmegaPirate::HearPlayer(CStateManager& mgr, float arg) { return xa3c_hearPlayer; }

void COmegaPirate::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    xad4_cachedSpeed = x3b4_speed;
    x3b4_speed = 1.4f * xad4_cachedSpeed;
  } else if (msg == EStateMsg::Update) {
    if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::KnockBack) {
      x3b4_speed = xad4_cachedSpeed;
    } else if (xad4_cachedSpeed == x3b4_speed) {
      x3b4_speed = 1.4f * xad4_cachedSpeed;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x3b4_speed = xad4_cachedSpeed;
  }
  CElitePirate::PathFind(mgr, msg, dt);
}

void COmegaPirate::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  CElitePirate::PreRender(mgr, frustum);
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::XRay) {
    xb4_drawFlags = CModelFlags{1, 0, 3, zeus::CColor{xa80_}};
  }
}

void COmegaPirate::Render(CStateManager& mgr) {
  auto* mData = GetModelData();
  auto* animData = mData->GetAnimationData();

  CGraphics::SetModelMatrix(GetTransform() * zeus::CTransform::Scale(mData->GetScale()));

  if (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::XRay && xa2c_ > 0.f) {
    const CModelFlags flags{5, 0, 3, zeus::CColor{1.f, xa2c_}};
    animData->Render(x9f0_, flags, std::nullopt, nullptr);
  }
  if (x9a0_) {
    bool isXRay = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay;
    if (isXRay) {
      g_Renderer->SetWorldFog(ERglFogMode::None, 0.f, 1.f, zeus::skBlack);
      const CModelFlags flags{5, 0, 1, zeus::CColor{1.f, 0.2f}};
      auto& model = *animData->GetModelData().GetObj();
      animData->Render(model, flags, std::nullopt, nullptr);
    }
    CPatterned::Render(mgr);
    if (isXRay) {
      mgr.SetupFogForArea(GetAreaIdAlways());
    }
  }
}

void COmegaPirate::Retreat(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    SetShotAt(false, mgr);
    SetState(CElitePirate::EState::Zero);
    SendScriptMsgs(EScriptObjectState::Inside, mgr, EScriptObjectMessage::None);
    xad0_ = false;
    xa44_ = false;
    xa4a_ = false;
    xb5c_ = 0.f;
    xb60_ = 0.f;
    xb64_ = 5.f;
    ++xb68_;
  } else if (msg == EStateMsg::Update) {
    if (GetState() == CElitePirate::EState::Zero) {
      if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::Step) {
        SetState(CElitePirate::EState::Two);
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Forward, pas::EStepType::BreakDodge));
      }
    } else if (GetState() == CElitePirate::EState::Two &&
               GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Step) {
      SetState(CElitePirate::EState::Over);
    }
  }
}

void COmegaPirate::Run(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    xad4_cachedSpeed = x3b4_speed;
    x3b4_speed = 1.4f * xad4_cachedSpeed;
  } else if (msg == EStateMsg::Update) {
    if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::KnockBack) {
      x3b4_speed = xad4_cachedSpeed;
    } else if (xad4_cachedSpeed == x3b4_speed) {
      x3b4_speed = 1.4f * xad4_cachedSpeed;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x3b4_speed = xad4_cachedSpeed;
  }
  CElitePirate::Run(mgr, msg, dt);
}

bool COmegaPirate::ShotAt(CStateManager& mgr, float arg) { return CElitePirate::ShotAt(mgr, arg); }

bool COmegaPirate::ShouldCallForBackup(CStateManager& mgr, float arg) {
  if (CElitePirate::ShouldCallForBackup(mgr, arg)) {
    return ShouldCallForBackupFromLauncher(mgr, x990_launcherId2);
  }
  return false;
}

bool COmegaPirate::ShouldFire(CStateManager& mgr, float arg) {
  if (CElitePirate::ShouldFire(mgr, arg)) {
    return ShouldFireFromLauncher(mgr, x990_launcherId2);
  }
  return false;
}

bool COmegaPirate::ShouldMove(CStateManager& mgr, float arg) { return xb64_ < x330_stateMachineState.GetTime(); }

void COmegaPirate::Shuffle(CStateManager& mgr, EStateMsg msg, float dt) {}

void COmegaPirate::Skid(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    SetState(EState::Zero);
  } else if (msg == EStateMsg::Update) {
    if (GetState() == CElitePirate::EState::Zero) {
      if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::Step) {
        SetState(CElitePirate::EState::Two);
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Forward, pas::EStepType::Normal));
      }
    } else if (GetState() == CElitePirate::EState::Two &&
               GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Step) {
      SetState(CElitePirate::EState::Over);
    }
  }
}

void COmegaPirate::Suck(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    SetState(CElitePirate::EState::Zero);
    xa7c_ = 3;
    xa88_ = true;
  } else if (msg == EStateMsg::Update) {
    if (GetState() == CElitePirate::EState::Zero) {
      if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::Step) {
        SetState(CElitePirate::EState::Two);
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCStepCmd(pas::EStepDirection::Backward, pas::EStepType::Normal));
        GetBodyController()->SetLocomotionType(pas::ELocomotionType::Relaxed);
      }
    } else if (GetState() == CElitePirate::EState::Two &&
               GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Step) {
      SetState(CElitePirate::EState::Over);
    }
  } else if (msg == EStateMsg::Deactivate) {
    for (const auto& entry : x9dc_scriptPlatforms) {
      if (auto* platform = static_cast<CScriptPlatform*>(mgr.ObjectById(entry.first))) {
        platform->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty());
        platform->RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
        platform->SetDisableXRayAlpha(true);
        CModelFlags flags{5, 0, 3, zeus::CColor{1.f, 0.f}};
        platform->SetDrawFlags(flags);
        platform->SetXRayFog(false);
      }
    }
    xb50_ = 0.f;
    xb58_ = 2.5f;
    xb4c_ = 0;
  }
}

void COmegaPirate::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    xad4_cachedSpeed = x3b4_speed;
    x3b4_speed = 1.4f * xad4_cachedSpeed;
  } else if (msg == EStateMsg::Update) {
    if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::KnockBack) {
      x3b4_speed = xad4_cachedSpeed;
    } else if (xad4_cachedSpeed == x3b4_speed) {
      x3b4_speed = 1.4f * xad4_cachedSpeed;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x3b4_speed = xad4_cachedSpeed;
  }
  CElitePirate::TargetPatrol(mgr, msg, dt);
}

void COmegaPirate::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  SetAlert(true);
  CElitePirate::Think(dt, mgr);

  {
    float maxHealth = xa98_maxEnergy;
    CHealthInfo* healthInfo = HealthInfo(mgr);
    if (healthInfo->GetHP() > 0.2f * maxHealth) {
      if (healthInfo->GetHP() > 0.7f * maxHealth) {
        if (xacc_ > 4) {
          xac4_ = 1;
        }
      } else {
        xac4_ = 2;
      }
    } else {
      xac4_ = 3;
    }
  }

  UpdateActorTransform(mgr, x990_launcherId2, "grenadeLauncher2_LCTR"sv);

  sub_8028f6f0(mgr, dt);
  sub_8028d690(mgr, dt);
  sub_8028cd04(mgr, dt);
  if ((!x9a1_ || xa4a_) && mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay && xa44_) {
    AddMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
    if (x9c8_ == 4) {
      xa38_collisionActorMgr1->SetActive(mgr, false);
      xa9c_collisionActorMgr2->SetActive(mgr, false);
    } else {
      xa38_collisionActorMgr1->SetActive(mgr, true);
      xa9c_collisionActorMgr2->SetActive(mgr, true);
      if (auto* entity = mgr.ObjectById(xa48_)) {
        entity->SetActive(false);
      }
    }
  } else {
    RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
    xa38_collisionActorMgr1->SetActive(mgr, false);
    if (x9a1_) {
      xa9c_collisionActorMgr2->SetActive(mgr, true);
      if (auto* entity = mgr.ObjectById(xa48_)) {
        entity->SetActive(true);
      }
    } else {
      xa9c_collisionActorMgr2->SetActive(mgr, false);
      if (auto* entity = mgr.ObjectById(xa48_)) {
        entity->SetActive(false);
      }
    }
  }

  sub_8028d7e4(mgr, dt);
  xa38_collisionActorMgr1->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  xa9c_collisionActorMgr2->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);

  if (auto* entity = static_cast<CActor*>(mgr.ObjectById(xa46_))) {
    float hp = GetHealthInfo(mgr)->GetHP();
    *HealthInfo(mgr) = *entity->GetHealthInfo(mgr);
    float hpChange = hp - GetHealthInfo(mgr)->GetHP();
    xb5c_ += hpChange;
    xb60_ += hpChange;
  }

  if (GetHealthInfo(mgr)->GetHP() > 0.f) {
    if (xb5c_ <= 100.f) {
      if (xb60_ > 20.f) {
        GetBodyController()->GetCommandMgr().DeliverCmd(
            CBCAdditiveReactionCmd(pas::EAdditiveReactionType::One, 1.f, false));
        xb60_ = 0.f;
      }
    } else {
      x9b4_ = true;
    }
  } else {
    Destroy(mgr);
  }

  sub_8028c704(mgr, dt);

  for (auto& entry : x9dc_scriptPlatforms) {
    auto* platform = static_cast<CScriptPlatform*>(mgr.ObjectById(entry.first));
    if ((!xb78_ && !xb79_) || xa4a_) {
      platform->RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
    } else {
      platform->AddMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
    }
  }

  {
    const CPlayerState& playerState = *mgr.GetPlayerState();
    CPlayer& player = mgr.GetPlayer();
    if (GetCollisionActorManager().GetActive() && playerState.IsFiringComboBeam() &&
        playerState.GetCurrentBeam() == CPlayerState::EBeamId::Wave && xad8_cover) {
      AddMaterial(EMaterialTypes::Target, mgr);
      player.ResetAimTargetPrediction(GetUniqueId());
      for (auto& entry : x9dc_scriptPlatforms) {
        if (auto* platform = static_cast<CScriptPlatform*>(mgr.ObjectById(entry.first))) {
          platform->RemoveMaterial(EMaterialTypes::Target, mgr);
        }
      }
      player.GetPlayerGun()->GetAuxWeapon().SetNewTarget(GetUniqueId(), mgr);
    } else if (!xa4a_) {
      RemoveMaterial(EMaterialTypes::Target, mgr);
      for (auto& entry : x9dc_scriptPlatforms) {
        if (auto* platform = static_cast<CScriptPlatform*>(mgr.ObjectById(entry.first))) {
          platform->AddMaterial(EMaterialTypes::Target, mgr);
        }
      }
      CAuxWeapon& weapon = player.GetPlayerGun()->GetAuxWeapon();
      if (weapon.HasTarget(mgr) == GetUniqueId()) {
        if (player.ValidateOrbitTargetId(player.GetOrbitTargetId(), mgr) == CPlayer::EOrbitValidationResult::OK) {
          weapon.SetNewTarget(player.GetOrbitTargetId(), mgr);
        } else {
          weapon.SetNewTarget(kInvalidUniqueId, mgr);
        }
      }
    }
  }

  if (auto* launcher = static_cast<CGrenadeLauncher*>(mgr.ObjectById(GetLauncherId()))) {
    launcher->SetFollowPlayer(xadf_);
  }
  if (auto* launcher = static_cast<CGrenadeLauncher*>(mgr.ObjectById(x990_launcherId2))) {
    launcher->SetFollowPlayer(xae0_);
  }

  if (x9ec_) {
    x9ec_ = false;
    x330_stateMachineState.SetState(mgr, *this, GetStateMachine(), "JumpBack"sv);
  }

  if (xb68_ > 1) {
    DoUserAnimEvent(mgr, CInt32POINode{}, EUserEventType::ObjectPickUp, dt);
    xb68_ = 0;
  }

  if (xb8c_ > 0.f) {
    const zeus::CAABox& box = GetBoundingBox();
    CGameCollision::AvoidStaticCollisionWithinRadius(mgr, *this, 8, dt, 1.f, 1.5f * (box.max.x() - box.min.x()),
                                                     10000.f, 0.25f);
    xb8c_ = 0.f;
  }
  xb8c_ += dt;
}

void COmegaPirate::WallDetach(CStateManager& mgr, EStateMsg msg, float dt) {
  if (msg == EStateMsg::Activate) {
    SetState(CElitePirate::EState::Zero);
  } else if (msg == EStateMsg::Update) {
    if (GetState() == CElitePirate::EState::Zero) {
      if (GetBodyController()->GetCurrentStateId() == pas::EAnimationState::Step) {
        SetState(CElitePirate::EState::Two);
      } else {
        GetBodyController()->GetCommandMgr().DeliverCmd(CBCStepCmd(pas::EStepDirection::Left, pas::EStepType::Dodge));
      }
    } else if (GetState() == CElitePirate::EState::Two &&
               GetBodyController()->GetCurrentStateId() != pas::EAnimationState::Step) {
      SetState(CElitePirate::EState::Over);
    }
  } else if (msg == EStateMsg::Deactivate) {
    mgr.SetBossParams(GetUniqueId(), xa98_maxEnergy, 89);
    xb79_ = true;
  }
}

void COmegaPirate::WallHang(CStateManager& mgr, EStateMsg msg, float dt) {}

void COmegaPirate::SetupHealthInfo(CStateManager& mgr) {
  CElitePirate::SetupHealthInfo(mgr);
  SetupLauncherHealthInfo(mgr, x990_launcherId2);
}

void COmegaPirate::SetLaunchersActive(CStateManager& mgr, bool val) {
  CElitePirate::SetLaunchersActive(mgr, val);
  SetLauncherActive(mgr, val, x990_launcherId2);
}

void COmegaPirate::CreateFlash(CStateManager& mgr, float arg) {
  mgr.AddObject(new CFlash(mgr.AllocateUniqueId(), CEntityInfo{GetAreaIdAlways(), CEntity::NullConnectionList},
                           GetRenderBounds().center(), xb70_thermalSpot, arg));
}

void COmegaPirate::SetupCollisionManager(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> sphereJoints;
  sphereJoints.reserve(skSphereJoints.size());
  AddSphereCollisionList(skSphereJoints.data(), skSphereJoints.size(), sphereJoints);
  xa38_collisionActorMgr1 =
      std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), sphereJoints, true);
  SetupCollisionActorInfo1(xa38_collisionActorMgr1, mgr);
  xa46_ = xa38_collisionActorMgr1->GetCollisionDescFromIndex(0).GetCollisionActorId();
  if (auto* actor = static_cast<CActor*>(mgr.ObjectById(xa46_))) {
    *actor->HealthInfo(mgr) = *HealthInfo(mgr);
  }

  std::vector<CJointCollisionDescription> obbJoints;
  obbJoints.reserve(skObbJoints.size());
  AddOBBAutoSizeCollisionList(skObbJoints.data(), skObbJoints.size(), obbJoints);
  xa9c_collisionActorMgr2 =
      std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), obbJoints, true);
  SetupCollisionActorInfo2(xa9c_collisionActorMgr2, mgr);
  xa48_ = xa9c_collisionActorMgr2->GetCollisionDescFromIndex(0).GetCollisionActorId();
}

void COmegaPirate::AddSphereCollisionList(const SSphereJointInfo* joints, size_t count,
                                          std::vector<CJointCollisionDescription>& outJoints) const {
  const CAnimData* animData = GetModelData()->GetAnimationData();
  for (size_t i = 0; i < count; ++i) {
    const auto& joint = joints[i];
    const CSegId seg = animData->GetLocatorSegId(joint.name);
    if (seg.IsInvalid()) {
      continue;
    }
    outJoints.emplace_back(CJointCollisionDescription::SphereCollision(seg, joint.radius, joint.name, 0.001f));
  }
}

void COmegaPirate::AddOBBAutoSizeCollisionList(const SOBBJointInfo* joints, size_t count,
                                               std::vector<CJointCollisionDescription>& outJoints) const {
  const CAnimData* animData = GetModelData()->GetAnimationData();
  for (size_t i = 0; i < count; ++i) {
    const auto& joint = joints[i];
    const CSegId from = animData->GetLocatorSegId(joint.from);
    const CSegId to = animData->GetLocatorSegId(joint.to);
    if (to.IsInvalid() || from.IsInvalid()) {
      continue;
    }
    outJoints.emplace_back(CJointCollisionDescription::OBBAutoSizeCollision(
        from, to, joint.bounds, CJointCollisionDescription::EOrientationType::One,
        "Omega_Pirate_OBB_"s + std::to_string(i), 0.001f));
  }
}

void COmegaPirate::SetupCollisionActorInfo1(const std::unique_ptr<CCollisionActorManager>& actMgr, CStateManager& mgr) {
  for (size_t i = 0; i < actMgr->GetNumCollisionActors(); ++i) {
    const auto& colDesc = actMgr->GetCollisionDescFromIndex(i);
    const TUniqueId uid = colDesc.GetCollisionActorId();
    if (TCastToPtr<CCollisionActor> act = mgr.ObjectById(uid)) {
      act->AddMaterial(EMaterialTypes::ScanPassthrough, EMaterialTypes::CameraPassthrough, EMaterialTypes::AIJoint,
                       EMaterialTypes::Immovable, mgr);
      const CMaterialFilter& selfFilter = GetMaterialFilter();
      const CMaterialFilter& actFilter = act->GetMaterialFilter();
      CMaterialFilter filter =
          CMaterialFilter::MakeIncludeExclude(selfFilter.GetIncludeList(), selfFilter.GetExcludeList());
      filter.IncludeList().Add(actFilter.GetIncludeList());
      filter.ExcludeList().Add(actFilter.GetExcludeList());
      filter.ExcludeList().Add(EMaterialTypes::Platform); // ?
      act->SetMaterialFilter(filter);
      act->RemoveMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
    }
  }
}

void COmegaPirate::SetupCollisionActorInfo2(const std::unique_ptr<CCollisionActorManager>& actMgr, CStateManager& mgr) {
  for (size_t i = 0; i < actMgr->GetNumCollisionActors(); ++i) {
    const auto& colDesc = actMgr->GetCollisionDescFromIndex(i);
    const TUniqueId uid = colDesc.GetCollisionActorId();
    if (TCastToPtr<CCollisionActor> act = mgr.ObjectById(uid)) {
      act->AddMaterial(EMaterialTypes::ScanPassthrough, EMaterialTypes::CameraPassthrough, EMaterialTypes::AIJoint,
                       EMaterialTypes::Immovable, mgr);
      const CMaterialFilter& selfFilter = GetMaterialFilter();
      const CMaterialFilter& actFilter = act->GetMaterialFilter();
      CMaterialFilter filter =
          CMaterialFilter::MakeIncludeExclude(selfFilter.GetIncludeList(), selfFilter.GetExcludeList());
      filter.IncludeList().Add(actFilter.GetIncludeList());
      filter.IncludeList().Add(EMaterialTypes::Player); // ?
      filter.ExcludeList().Add(actFilter.GetExcludeList());
      filter.ExcludeList().Remove(EMaterialTypes::Player); // ?
      filter.ExcludeList().Add(EMaterialTypes::Platform);  // ?
      act->SetMaterialFilter(filter);
      act->RemoveMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
      act->SetDamageVulnerability(CDamageVulnerability::ReflectVulnerabilty());
    }
  }
}

u8 COmegaPirate::sub_8028bfac() const {
  std::array<u8, 4> arr{0, 0, 0, 0};
  for (const auto i : xab4_) {
    ++arr[i];
  }
  u8 ret = 0;
  for (size_t i = 0; i < arr.size(); ++i) {
    if (xb7c_[i] != 0 || arr[i] != 0) {
      ++ret;
    }
  }
  return ret;
}

void COmegaPirate::sub_8028cbec(u32 arg, CStateManager& mgr) {
  int i = mgr.GetActiveRandom()->Next() % 4;
  u32 v = 3 - (xab4_.size() + sub_8028c230());
  u32 ct = std::min(arg, v);

  if (sub_8028bfac() < 2) {
    for (u32 n = 0; n < ct; ++n) {
      xab4_.push_back(i);
    }
  } else {
    sub_8028c840(ct, mgr);
  }
}

u8 COmegaPirate::sub_8028c230() const { return xb7c_[0] + xb7c_[1] + xb7c_[2] + xb7c_[3]; }

void COmegaPirate::sub_8028c840(u32 arg, CStateManager& mgr) {
  std::array<u8, 4> arr{0, 0, 0, 0};
  for (const auto i : xab4_) {
    ++arr[i];
  }
  std::vector<u8> vec;
  for (size_t i = 0; i < arr.size(); ++i) {
    if (xb7c_[i] != 0 || arr[i] != 0) {
      vec.push_back(i);
    }
  }
  if (vec.empty()) {
    sub_8028cbec(arg, mgr);
  } else {
    s32 rand = mgr.GetActiveRandom()->Next();
    int sz = vec.size(); // might be wrong?
    int val = vec[rand - (rand / sz) * sz];
    u32 v = 3 - (xab4_.size() + sub_8028c230());
    u32 ct = std::min(arg, v);
    for (u32 n = 0; n < ct; ++n) {
      xab4_.push_back(val);
    }
  }
}

void COmegaPirate::TeleportToFurthestPlatform(CStateManager& mgr) {
  size_t waypointIdx = 0;
  float maxDist = 0.f;
  zeus::CVector3f pos;
  for (size_t i = 0; i < x9a4_scriptWaypointPlatforms.size(); ++i) {
    const auto& entry = x9a4_scriptWaypointPlatforms[i];
    if (TCastToConstPtr<CScriptWaypoint> waypoint = mgr.GetObjectById(entry.first)) {
      auto waypointPos = waypoint->GetTranslation();
      float dist = (mgr.GetPlayer().GetTranslation() - waypointPos).magnitude();
      if (dist > maxDist && waypoint->GetUniqueId() != xada_) {
        waypointIdx = i;
        maxDist = dist;
        pos = waypointPos;
      }
    }
  }
  SetTranslation(FindGround(pos, mgr));

  auto waypointId = x9a4_scriptWaypointPlatforms[waypointIdx].first;
  xada_ = waypointId;
  if (TCastToPtr<CScriptWaypoint> waypoint = mgr.ObjectById(waypointId)) {
    waypoint->SendScriptMsgs(EScriptObjectState::Arrived, mgr, EScriptObjectMessage::None);
  }

  const zeus::CVector2f distXY = (mgr.GetPlayer().GetTranslation().toVec2f() - GetTranslation().toVec2f()).normalized();
  const zeus::CVector2f frontVecXY = GetTransform().frontVector().toVec2f().normalized();
  const zeus::CQuaternion quat =
      zeus::CQuaternion::shortestRotationArc(zeus::CVector3f{frontVecXY, 0.f}, zeus::CVector3f{distXY, 0.f});
  SetTransform(zeus::CTransform{GetTransform().basis * zeus::CMatrix3f{quat}, GetTranslation()});
}

zeus::CVector3f COmegaPirate::FindGround(const zeus::CVector3f& pos, CStateManager& mgr) const {
  auto result = mgr.RayStaticIntersection(pos, zeus::skDown, 30.f, CMaterialFilter::MakeInclude(EMaterialTypes::Solid));
  if (result.IsValid()) {
    return result.GetPoint();
  }
  return pos;
}

void COmegaPirate::sub_8028f6f0(CStateManager& mgr, float dt) {
  if (x994_ == 1) {
    x99c_ = 1.f - std::min(x998_, 1.25f) / 1.25f;
    x42c_color.a() = x99c_;
    if (x998_ > 1.25f) {
      x994_ = 2;
      x9a1_ = false;
      x998_ = 0.f;
    }
    x998_ += dt;
    x9a0_ = true;
  } else if (x994_ == 2) {
    x99c_ = 0.f;
    if (x998_ > 1.5f && x9a1_) {
      CreateFlash(mgr, 0.f);
      x994_ = 3;
      x998_ = 0.f;
    }
    x998_ += dt;
    x9a0_ = false;
  } else if (x994_ == 3) {
    x99c_ = std::min(x998_, 1.f) / 1.25f;
    if (x998_ > 1.f) {
      x994_ = 0;
      x998_ = 0.f;
    }
    x998_ += dt;
    x9a0_ = true;
  } else {
    x99c_ = 1.f;
    x9a0_ = true;
  }

  float alpha = x99c_;
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay) {
    alpha = 0.f;
    x99c_ = 1.f;
    x9a0_ = true;
  }
  x42c_color.a() = x99c_;

  if (alpha >= 1.f) {
    if (auto* launcher = static_cast<CGrenadeLauncher*>(mgr.ObjectById(GetLauncherId()))) {
      launcher->SetVisible(true);
      launcher->SetColor(zeus::CColor{0.f});
    }
    if (auto* launcher = static_cast<CGrenadeLauncher*>(mgr.ObjectById(x990_launcherId2))) {
      launcher->SetVisible(true);
      launcher->SetColor(zeus::CColor{0.f});
    }
  } else {
    if (auto* launcher = static_cast<CGrenadeLauncher*>(mgr.ObjectById(GetLauncherId()))) {
      launcher->SetColor(zeus::CColor{1.f, alpha});
      launcher->SetVisible(alpha != 0.f);
    }
    if (auto* launcher = static_cast<CGrenadeLauncher*>(mgr.ObjectById(x990_launcherId2))) {
      launcher->SetColor(zeus::CColor{1.f, alpha});
      launcher->SetVisible(alpha != 0.f);
    }
  }
}

void COmegaPirate::sub_8028c704(CStateManager& mgr, float dt) {
  int idx = xac8_;
  if (idx >= xab4_.size()) {
    return;
  }

  if (xab0_ <= 0.f) {
    ++xac8_;
    int val = xab4_[idx];
    if (val == 0) {
      SendScriptMsgs(EScriptObjectState::Closed, mgr, EScriptObjectMessage::None);
      xb7c_[0]++;
    } else if (val == 1) {
      SendScriptMsgs(EScriptObjectState::Open, mgr, EScriptObjectMessage::None);
      xb7c_[1]++;
    } else if (val == 2) {
      SendScriptMsgs(EScriptObjectState::CloseIn, mgr, EScriptObjectMessage::None);
      xb7c_[2]++;
    } else if (val == 3) {
      SendScriptMsgs(EScriptObjectState::Modify, mgr, EScriptObjectMessage::None);
      xb7c_[3]++;
    }
    xab0_ = 1.5f;
  }
  xab0_ -= dt;
}

void COmegaPirate::sub_8028cd04(CStateManager& mgr, float dt) {
  if (xa7c_ == 1) {
    xa80_ = std::min(xa84_, xa90_) / xa90_;
    if (xa90_ < xa84_) {
      xa7c_ = 0;
      xa84_ = 0.f;
    }
    xa84_ += dt;
  } else if (xa7c_ == 2) {
    xa80_ = 0.f;
    if ((xa94_ < xa84_) && !xa88_) {
      xa7c_ = 1;
      xa84_ = 0.f;
    }
    xa84_ += dt;
  } else if (xa7c_ == 3) {
    xa80_ = 1.f - std::min(xa84_, xa8c_) / xa8c_;
    if (xa8c_ < xa84_) {
      xa7c_ = 2;
      xa84_ = 0.f;
    }
    xa84_ += dt;
  } else {
    xa80_ = 1.f;
  }
}

void COmegaPirate::sub_8028d7e4(CStateManager& mgr, float dt) {
  auto* modelData = GetModelData();
  zeus::CVector3f scale = modelData->GetScale();
  switch (x9c8_) {
  case 0:
  default:
    return;
  case 1:
    scale.x() = x9d0_.x() * std::min(1.f, 0.005f + (1.f - std::min(x9cc_, 0.25f) / 0.25f));
    if (x9cc_ > 0.25f) {
      x9c8_ = 3;
      x9cc_ = 0.f;
    }
    x9cc_ += dt;
    break;
  case 2:
    scale.y() = x9d0_.y() * std::min(1.f, 0.005f + (1.f - std::min(x9cc_, 0.25f) / 0.25f));
    if (x9cc_ > 0.25f) {
      x9c8_ = 1;
      x9cc_ = 0.f;
    }
    x9cc_ += dt;
    break;
  case 3:
    scale.z() = x9d0_.z() * std::min(1.f, 0.005f + (1.f - std::min(x9cc_, 0.25f) / 0.25f));
    if (x9cc_ > 0.25f) {
      x9c8_ = 4;
      x9cc_ = 0.f;
    }
    x9cc_ += dt;
    break;
  case 4:
    if (x9cc_ > 0.1f && xad0_) {
      x9c8_ = 7;
      x9cc_ = 0.f;
    }
    x9cc_ += dt;
    break;
  case 5:
    scale.x() = x9d0_.x() * std::min(1.f, 0.005f + (1.f - std::min(x9cc_, 0.25f) / 0.25f));
    if (x9cc_ > 0.25f) {
      x9c8_ = 6;
      x9cc_ = 0.f;
    }
    x9cc_ += dt;
    break;
  case 6:
    scale.y() = x9d0_.y() * std::min(1.f, 0.005f + (1.f - std::min(x9cc_, 0.25f) / 0.25f));
    if (x9cc_ > 0.25f) {
      x9c8_ = 0;
      x9cc_ = 0.f;
    }
    x9cc_ += dt;
    break;
  case 7:
    scale.z() = x9d0_.z() * std::min(1.f, 0.005f + (1.f - std::min(x9cc_, 0.25f) / 0.25f));
    if (x9cc_ > 0.25f) {
      x9c8_ = 5;
      x9cc_ = 0.f;
    }
    x9cc_ += dt;
    break;
  }

  modelData->SetScale(scale);
  if (auto* launcher = static_cast<CGrenadeLauncher*>(mgr.ObjectById(GetLauncherId()))) {
    launcher->GetModelData()->SetScale(scale);
  }
  if (auto* launcher = static_cast<CGrenadeLauncher*>(mgr.ObjectById(x990_launcherId2))) {
    launcher->GetModelData()->SetScale(scale);
  }
  for (const auto& entry : x9dc_scriptPlatforms) {
    if (auto* platform = static_cast<CScriptPlatform*>(mgr.ObjectById(entry.first))) {
      platform->GetModelData()->SetScale(scale);
    }
  }
}

void COmegaPirate::sub_8028d690(CStateManager& mgr, float dt) {
  if (xa30_ == 1) {
    xa2c_ = 1.f - std::min(xa34_, 1.f);
    if (xa34_ > 1.f) {
      xa30_ = 0;
      xa34_ = 0.f;
    }
    xa34_ += dt;
  } else if (xa30_ == 2) {
    xa2c_ = 1.f;
    if (xa34_ > 1.f) {
      xa30_ = 1;
      xa34_ = 0.f;
      CreateFlash(mgr, 0.75f);
    }
    xa34_ += dt;
  } else if (xa30_ == 3) {
    xa2c_ = std::min(xa34_, 1.f);
    if (xa34_ > 1.f) {
      xa30_ = 2;
      xa34_ = 0.f;
    }
    xa34_ += dt;
  } else {
    xa2c_ = 0.f;
  }
}

void COmegaPirate::Destroy(CStateManager& mgr) {
  RemoveEmitter();
  SetTransform(xa4c_initialXf);
  x9a1_ = true;
  xa4a_ = false;
  SendScriptMsgs(EScriptObjectState::DeathRattle, mgr, EScriptObjectMessage::None);
  SendScriptMsgs(EScriptObjectState::Dead, mgr, EScriptObjectMessage::None);
  SendScriptMsgs(EScriptObjectState::Inside, mgr, EScriptObjectMessage::None);
  for (auto& entry : x9dc_scriptPlatforms) {
    if (auto* platform = static_cast<CScriptPlatform*>(mgr.ObjectById(entry.first))) {
      platform->SetActive(false);
      platform->RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
      mgr.FreeScriptObject(entry.first);
    }
  }
  x9dc_scriptPlatforms.clear();

  if (auto* launcher = static_cast<CGrenadeLauncher*>(mgr.ObjectById(GetLauncherId()))) {
    launcher->SetActive(false);
  }
  if (auto* launcher = static_cast<CGrenadeLauncher*>(mgr.ObjectById(x990_launcherId2))) {
    launcher->SetActive(false);
  }
  SetActive(false);
  mgr.SetBossParams(kInvalidUniqueId, 0.f, 89);
  xa38_collisionActorMgr1->SetActive(mgr, false);
  xa9c_collisionActorMgr2->SetActive(mgr, false);
}
} // namespace urde::MP1
