#include "MetroidPrime/Enemies/CChozoGhost.hpp"

#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/Enemies/CTeamAiMgr.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include <float.h>

const rstl::string CChozoGhost::skSpeedSwooshName = rstl::string_l("SpeedSwoosh");

CChozoGhost::CBehaveChance::CBehaveChance(CInputStream& in)
: x0_propertyCount(in.Get< uint >())
, x4_lurk(in.Get< float >())
, x8_taunt(in.Get< float >())
, xc_attack(in.Get< float >())
, x10_move(in.Get< float >())
, x14_lurkTime(in.Get< float >())
, x18_chargeAttack(x0_propertyCount > 5 ? in.Get< float >() * 0.01f : 0.5f)
, x1c_numBolts(x0_propertyCount > 6 ? in.Get< uint >() : 2) {
  const float average = 1.f / (x4_lurk + x8_taunt + xc_attack + x10_move);
  x4_lurk *= average;
  x8_taunt *= average;
  xc_attack *= average;
  x10_move *= average;
}

CChozoGhost::EBehaveType CChozoGhost::CBehaveChance::GetBehave(const EBehaveType type,
                                                               CStateManager& mgr) const {
  float lurkChance = x4_lurk;
  float tauntChance = x8_taunt;
  float attackChance = xc_attack;
  switch (type) {
  case kBT_Lurk: {
    const float delta = lurkChance / 3.f;
    lurkChance = 0.f;
    tauntChance += delta;
    attackChance += delta;
  } break;
  case kBT_Taunt: {
    const float delta = tauntChance / 3.f;
    tauntChance = 0.f;
    lurkChance += delta;
    attackChance += delta;
  } break;
  case kBT_Attack: {
    const float delta = attackChance / 3.f;
    attackChance = 0.f;
    lurkChance += delta;
    tauntChance += delta;
  } break;
  case kBT_Move: {
    const float delta = x10_move / 3.f;
    lurkChance += delta;
    tauntChance += delta;
    attackChance += delta;
  } break;
  default:
    break;
  }

  const float rnd = mgr.Random()->Float();
  EBehaveType ret = kBT_Move;
  if (rnd < lurkChance) {
    ret = kBT_Lurk;
  } else if (rnd - lurkChance < tauntChance) {
    ret = kBT_Taunt;
  } else if (rnd - lurkChance - tauntChance < attackChance) {
    ret = kBT_Attack;
  }
  return ret;
}

CChozoGhost::CChozoGhost(
    const TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CModelData& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
    const float hearingRadius, const float fadeOutDelay, const float attackDelay,
    const float freezeTime, const CAssetId wpsc1, const CDamageInfo& dInfo1, const CAssetId wpsc2,
    const CDamageInfo& dInfo2, const CBehaveChance& chance1, const CBehaveChance& chance2,
    const CBehaveChance& chance3, const ushort soundImpact, const float f1, const ushort sfxFadeIn,
    const ushort sfxFadeOut, const uint w1, const float f2, const uint w2,
    const float hurlRecoverTime, const CAssetId projectileVisor, const ushort soundProjectileVisor,
    const float f3, const float f4, const uint nearChance, const uint midChance)
: CPatterned(kC_ChozoGhost, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_Zero,
             kBT_BiPedal, actParms, kCS_Medium)
, x568_hearingRadius(hearingRadius)
, x56c_fadeOutDelay(fadeOutDelay)
, x570_attackDelay(attackDelay)
, x574_freezeTime(freezeTime)
, x578_projectileInfo1(wpsc1, dInfo1)
, x5a0_projectileInfo2(wpsc2, dInfo2)
, x5c8_behaveChance1(chance1)
, x5e8_behaveChance2(chance2)
, x608_behaveChance3(chance3)
, x628_soundImpact(soundImpact)
, x62c_(f1)
, x630_sfxFadeIn(sfxFadeIn)
, x632_sfxFadeOut(sfxFadeOut)
, x634_(f2)
, x638_hurlRecoverTime(hurlRecoverTime)
, x63c_(w2)
, x650_soundProjectileVisor(soundProjectileVisor)
, x654_(f3)
, x658_(f4)
, x65c_nearChance(nearChance)
, x660_midChance(midChance)
, x664_24_behaviorEnabled((w1 << 6) & 0x40)
, x664_25_flinch((w1 << 5) & 0x40)
, x664_26_alert(false)
, x664_27_onGround(false)
, x664_28_(false)
, x664_29_fadedIn(false)
, x664_30_fadedOut(false)
, x664_31_(false)
, x665_24_(true)
, x665_25_(false)
, x665_26_shouldSwoosh(false)
//, x665_27_playerInLeashRange(false)
, x665_28_inRange(false)
, x665_29_aggressive(false)
, x668_(0.f)
, x66c_(0.f)
, x670_(0.f)
, x674_coverPoint(kInvalidUniqueId)
, x678_floorLevel(0.f)
, x67c_attackType(-1)
, x680_behaveType(x664_24_behaviorEnabled ? kBT_Attack : kBT_None)
, x684_lurkDelay(1.f)
, x68c_boneTracking(*GetAnimationData(), rstl::string_l("Head_1"), 80.f * M_PIF / 180.f,
                    CRelAngle::FromDegrees(180.f).AsRadians(), kBTF_None)
, x6c4_teamMgr(kInvalidUniqueId)
, x6c8_spaceWarpTime(0.f)
, x6cc_spaceWarpPosition(CVector3f::Zero())
, x6d8_(1) {
  x578_projectileInfo1.Token().Lock();
  x5a0_projectileInfo2.Token().Lock();

  const CPASAnimParmData jumpAnimParms(pas::kAS_Jump, CPASAnimParm::FromEnum(3),
                                       CPASAnimParm::FromEnum(0));
  x668_ = GetModelScale().GetZ() * GetAnimationDistance(jumpAnimParms);
  const CPASAnimParmData slideAnimParms(pas::kAS_Slide, CPASAnimParm::FromEnum(1),
                                        CPASAnimParm::FromReal32(90.f));
  x66c_ = GetModelScale().GetY() * GetAnimationDistance(slideAnimParms);
  const CPASAnimParmData meleeAnimParms(pas::kAS_MeleeAttack, CPASAnimParm::FromEnum(2),
                                        CPASAnimParm::FromEnum(1));
  x670_ = GetModelScale().GetZ() * GetAnimationDistance(meleeAnimParms);

  if (projectileVisor != kInvalidAssetId) {
    x640_projectileVisor = gpSimplePool->GetObj(SObjectTag('PART', projectileVisor));
  }

  x460_knockBackController.SetEnableBurn(false);
  x460_knockBackController.SetEnableLaggedBurnDeath(false);
  x460_knockBackController.SetEnableShock(false);
  x460_knockBackController.SetEnableFreeze(false);
  SetDrawShadow(false);
  MakeThermalColdAndHot();
}

ENTITY_ACCEPT_IMPL(CChozoGhost)

void CChozoGhost::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_InitializedInArea:
    if (GetActive()) {
      AddToTeam(mgr);
    }
    break;
  case kSM_Activate:
    AddToTeam(mgr);
    break;
  case kSM_Alert:
    if (!x664_26_alert) {
      x664_26_alert = true;
      SetWasHit(true);
    }
    break;
  case kSM_Action:
    if (x664_25_flinch) {
      x665_29_aggressive = true;
    }
    break;
  case kSM_Deactivate:
  case kSM_Deleted:
    RemoveFromTeam(mgr);
    break;
  case kSM_Falling:
  case kSM_Jumped:
    if (!GetVerticalMovement()) {
      x150_momentum = CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass());
    }
    break;
  default:
    break;
  }
}

void CChozoGhost::Touch(CActor& act, CStateManager& mgr) {
  if (IsVisibleEnough(mgr)) {
    if (CPlayer* player = TCastToPtr< CPlayer >(act)) {
      if (x420_curDamageRemTime <= 0.f) {
        mgr.ApplyDamage(
            GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
        x420_curDamageRemTime = x424_damageWaitTime;
      }
    }
  }
  CPatterned::Touch(act, mgr);
}

bool CChozoGhost::CanBeShot(const CStateManager& mgr, int w1) { return IsVisibleEnough(mgr); }

EWeaponCollisionResponseTypes CChozoGhost::GetCollisionResponseType(const CVector3f&,
                                                                    const CVector3f&,
                                                                    const CWeaponMode&, int) const {
  return kWCR_ChozoGhost;
}

const CDamageVulnerability* CChozoGhost::GetDamageVulnerability() const {
  if (x665_24_) {
    return &CDamageVulnerability::PassThroughVulnerability();
  }

  return CAi::GetDamageVulnerability();
}

uchar CChozoGhost::GetModelAlphau8(const CStateManager& mgr) const {
  uchar ret = 255;
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_XRay || !IsAlive()) {
    ret = x42c_color.GetAlphau8();
  }

  return ret & 0xFF;
}

// TODO: Alpha register allocation and the model-data reload before bone tracking.
void CChozoGhost::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  const bool xray = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay;
  x402_29_drawParticles = !xray;
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Thermal) {
    SetCalculateLighting(false);
    ActorLights()->BuildConstantAmbientLighting(CColor::White());
  } else {
    SetCalculateLighting(true);
  }
  CColor color = x42c_color;
  const uchar alpha = GetModelAlphau8(mgr);
  if (alpha < 255 || color.GetRedu8() != 0) {
    if (color.GetRedu8() != 0) {
      const uchar value = CMath::Max(0, 255 - 2 * color.GetRedu8());
      color.SetRed(static_cast< uchar >(255));
      color.SetGreen(value);
      color.SetBlue(value);
    } else {
      color = CColor::White();
    }
    SetModelFlags(CModelFlags::AlphaBlended(
        CColor(color.GetRedu8(), color.GetGreenu8(), color.GetBlueu8(), alpha)));
  } else {
    SetModelFlags(CModelFlags::Normal());
  }
  CActor::PreRender(mgr, frustum);
  x68c_boneTracking.PreRender(mgr, *ModelData()->AnimationData(), GetTransform(), GetModelScale(),
                              *BodyCtrl());
}

void CChozoGhost::Render(const CStateManager& mgr) const {
  if (x6c8_spaceWarpTime > 0.f) {
    mgr.DrawSpaceWarp(x6cc_spaceWarpPosition,
                      CMath::FastSinR(M_PIF * x6c8_spaceWarpTime / x56c_fadeOutDelay));
  }
  const bool xray = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay;
  const TAreaId areaId = GetCurrentAreaId();
  if (xray) {
    CElementGen::SetSubtractBlend(true);
    CElementGen::SetMoveRedToAlphaBuffer(true);
    CGraphics::SetFog(kRFM_PerspLin, 0.f, 75.f, CColor::Black());
    GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirst();
    mgr.SetupFogForArea3XRange(areaId);
  }
  CPatterned::Render(mgr);
  if (xray) {
    CGraphics::SetFog(kRFM_PerspLin, 0.f, 75.f, CColor::Black());
    GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnLast();
    mgr.SetupFogForArea(areaId);
    CElementGen::SetSubtractBlend(false);
    CElementGen::SetMoveRedToAlphaBuffer(false);
  }
}

bool CChozoGhost::IsVisibleEnough(const CStateManager& mgr) const {
  return GetModelAlphau8(mgr) > 31;
}

CVector3f CChozoGhost::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                                 const CVector3f& aimPos) const {
  return GetTranslation();
}

void CChozoGhost::AddToTeam(CStateManager& mgr) {
  if (x6c4_teamMgr == kInvalidUniqueId) {
    x6c4_teamMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
  }

  if (x6c4_teamMgr == kInvalidUniqueId) {
    return;
  }

  if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x6c4_teamMgr))) {
    teamMgr->AssignTeamAiRole(*this, CTeamAiRole::kTAR_Projectile, CTeamAiRole::kTAR_Unknown,
                              CTeamAiRole::kTAR_Invalid);
  }
}

void CChozoGhost::RemoveFromTeam(CStateManager& mgr) {
  if (x6c4_teamMgr == kInvalidUniqueId) {
    return;
  }

  CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x6c4_teamMgr));
  if (teamMgr && teamMgr->IsPartOfTeam(GetUniqueId())) {
    teamMgr->RemoveTeamAiRole(GetUniqueId());
    x6c4_teamMgr = kInvalidUniqueId;
  }
}

void CChozoGhost::FloatToLevel(const float f1, const float dt) {
  CVector3f translation = GetTranslation();
  const float floatAmt = ((f1 - translation[kDZ]) * 4.f);
  translation[kDZ] += floatAmt * dt;
  SetTranslation(translation);
}

bool CChozoGhost::IsOnGround() const { return x664_27_onGround; }

void CChozoGhost::FindBestAnchor(CStateManager& mgr) {
  float bestScore = FLT_MAX;
  x665_27_playerInLeashRange = false;
  CScriptCoverPoint* target = nullptr;
  CObjectList& waypoints = mgr.ObjectListById(kOL_AiWaypoint);
  const int random = mgr.Random()->Next() % 100;
  const int range =
      random < x65c_nearChance ? 0 : (random < x65c_nearChance + x660_midChance ? 1 : 2);
  float nearWeight = 10.f * x658_;
  float midWeight = nearWeight;
  float farWeight = nearWeight;
  switch (range) {
  case 0:
    farWeight *= 10.f;
    midWeight *= 5.f;
    break;
  case 1:
    nearWeight *= 10.f;
    farWeight *= 5.f;
    break;
  case 2:
    nearWeight *= 10.f;
    midWeight *= 5.f;
    break;
  }
  for (int i = waypoints.GetFirstObjectIndex(); i != -1; i = waypoints.GetNextObjectIndex(i)) {
    if (CScriptCoverPoint* cover = TCastToPtr< CScriptCoverPoint >(waypoints[i])) {
      if (cover->GetActive() && !cover->GetInUse(kInvalidUniqueId) &&
          cover->GetCurrentAreaId() == GetCurrentAreaId()) {
        const float distance = (cover->GetTranslation() - GetTranslation()).Magnitude();
        if (!(distance < 2.f * x66c_)) {
          float score = CMath::Max(0.f, x654_ - distance);
          CVector3f delta = cover->GetTranslation() - mgr.GetPlayer()->GetTranslation();
          const float playerDistance = delta.Magnitude();
          if (!(playerDistance < x2fc_minAttackRange)) {
            if (CMath::AbsF(delta.GetZ()) / playerDistance > 0.2f) {
              score += (20.f * x658_) * (CMath::AbsF(delta.GetZ()) / playerDistance - 0.2f);
            }
            if (playerDistance < x654_) {
              score += nearWeight;
              if (score < bestScore) {
                delta *= 1.f / playerDistance;
                score +=
                    (10.f * x658_) *
                    (1.f - CVector3f::Dot(mgr.GetPlayer()->GetTransform().GetForward(), delta));
              }
            } else if (playerDistance < x658_) {
              score += midWeight;
              if (score < bestScore) {
                delta *= 1.f / playerDistance;
                score +=
                    (10.f * x658_) *
                    (1.f - CVector3f::Dot(mgr.GetPlayer()->GetTransform().GetForward(), delta));
              }
            } else {
              score += farWeight;
            }
            if (score < bestScore) {
              score += x658_ * mgr.Random()->Float();
              if (score < bestScore) {
                bestScore = score;
                target = cover;
                x665_27_playerInLeashRange = playerDistance > x3c8_leashRadius;
              }
            }
          }
        }
      }
    }
  }
  if (target) {
    x2dc_destObj = target->GetUniqueId();
    SetDestPos(target->GetTranslation());
    ReleaseCoverPoint(mgr, x674_coverPoint);
    SetCoverPoint(target, x674_coverPoint);
  } else if (mgr.GetPlayer()->GetCurrentAreaId() == GetCurrentAreaId()) {
    x2dc_destObj = mgr.GetPlayer()->GetUniqueId();
    CVector3f destPos =
        mgr.GetPlayer()->GetTranslation() -
        x654_ * (mgr.GetPlayer()->GetTranslation() - GetTranslation()).AsNormalized();
    const CRayCastResult result = mgr.RayStaticIntersection(
        destPos, CVector3f::Down(), 8.f, CMaterialFilter::MakeInclude(CMaterialList(kMT_Floor)));
    if (result.IsValid()) {
      destPos = result.GetPoint();
    }
    SetDestPos(destPos);
  } else {
    x2dc_destObj = kInvalidUniqueId;
    x2e0_destPos = GetTranslation();
  }
}

const CChozoGhost::CBehaveChance& CChozoGhost::ChooseBehaveChanceRange(CStateManager& mgr) const {
  const float dist = (GetTranslation() - mgr.GetPlayer()->GetTranslation()).Magnitude();
  if (dist < x654_) {
    return x5c8_behaveChance1;
  }
  if (dist < x658_) {
    return x5e8_behaveChance2;
  }

  return x608_behaveChance3;
}

void CChozoGhost::SetWarpPosition(CStateManager& mgr, const CVector3f& dir) {
  const CVector3f center = GetBoundingBox().GetCenterPoint();
  const CRayCastResult result = mgr.RayStaticIntersection(
      center + dir * 8.f, -dir, 8.f, CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
  if (result.IsValid()) {
    x6cc_spaceWarpPosition = result.GetPoint();
  } else {
    x6cc_spaceWarpPosition = center + dir;
  }
}

void CChozoGhost::InActive(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    if (!BodyCtrl()->GetIsActive()) {
      BodyCtrl()->Activate(mgr);
    }

    if (x63c_ == 3) {
      BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
      x42c_color.SetAlpha(1.f);
    } else {
      BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
      x42c_color.SetAlpha(0.f);
    }
    RemoveMaterial(kMT_Solid, mgr);
    x150_momentum = CVector3f::Zero();
    x665_24_ = true;
  } break;
  case kStateMsg_Update:
    break;
  default:
    break;
  }
}

// TODO: The equality subtraction has its operands reversed.
bool CChozoGhost::AIStage(CStateManager& mgr, float arg) {
  return static_cast< int >(arg) == x63c_;
}
void CChozoGhost::Growth(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    StateMachineState().SetDelay(x56c_fadeOutDelay);
    BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
    x3e8_alphaDelta = 1.f;
    x664_29_fadedIn = true;
    if (x56c_fadeOutDelay > 0.f) {
      x6c8_spaceWarpTime = x56c_fadeOutDelay;
      SetWarpPosition(mgr, CVector3f::Up());
    }
  } break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    x665_24_ = false;
    x68c_boneTracking.SetActive(true);
    x68c_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    break;
  }
}

void CChozoGhost::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    StateMachineState().SetDelay(x56c_fadeOutDelay);
    SetAnimationState(kAS_Ready);
    x664_27_onGround = false;
    const CRayCastResult result =
        mgr.RayStaticIntersection(GetTranslation(), CVector3f::Down(), 100.f,
                                  CMaterialFilter::MakeInclude(CMaterialList(kMT_Floor)));
    if (result.IsValid()) {
      x678_floorLevel = result.GetPoint().GetZ();
    } else {
      x678_floorLevel = mgr.GetPlayer()->GetTranslation().GetZ();
    }

    x3e8_alphaDelta = 1.f;
    x664_29_fadedIn = true;

    if (x56c_fadeOutDelay > 0.f) {
      x6c8_spaceWarpTime = x56c_fadeOutDelay;
      SetWarpPosition(mgr, CVector3f::Down());
    }
  } break;
  case kStateMsg_Update: {
    TryCommand(mgr, pas::kAS_Jump, &CPatterned::TryJump, 0);
    switch (GetAnimationState()) {
    case kAS_Repeat: {
      BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
      if (x664_27_onGround) {
        break;
      }

      if (GetTranslation().GetZ() < x678_floorLevel + x668_) {
        CVector3f newPos = GetTranslation();
        newPos.SetZ(x678_floorLevel + x668_);
        SetTranslation(newPos);
        x664_27_onGround = true;
      }
    } break;
    case kAS_Over: {
      x68c_boneTracking.SetActive(true);
      x68c_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
      FloatToLevel(x678_floorLevel, arg);
    } break;
    default:
      break;
    }
  } break;
  case kStateMsg_Deactivate: {
    SetAnimationState(kAS_NotReady);
    x665_24_ = false;
    x664_27_onGround = false;
  } break;
  }
}

void CChozoGhost::WallDetach(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    StateMachineState().SetDelay(x56c_fadeOutDelay);
    x3e8_alphaDelta = 1.f;
    x664_29_fadedIn = false;

    if (x56c_fadeOutDelay > 0.f) {
      x6c8_spaceWarpTime = x56c_fadeOutDelay;
      SetWarpPosition(mgr, GetTransform().GetForward());
    }

    const CActor* wp = nullptr;
    const TUniqueId wpId = GetConnectedObject(mgr, kSS_Attack, kSM_Follow);
    if (wpId != kInvalidUniqueId) {
      wp = TCastToConstPtr< CActor >(mgr.GetObjectById(wpId));
    }

    if (wp) {
      SetDestPos(wp->GetTranslation());
    } else {
      SetDestPos(GetTranslation() + GetTransform().GetForward() * (2.f * x66c_));
    }

    SendScriptMsgs(kSS_Attack, mgr, kSM_Follow);
  } break;
  case kStateMsg_Update: {

  } break;
  case kStateMsg_Deactivate: {
    x68c_boneTracking.SetActive(true);
    x68c_boneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    x665_24_ = false;
    x680_behaveType = kBT_Move;
  } break;
  }
}
void CChozoGhost::Run(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    SetWasHit(false);
    KnockBackCtrl().EnableAnimReaction(kAR_KnockBack, false);
    x665_28_inRange = false;
  } break;
  case kStateMsg_Update: {
    BodyCtrl()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(x688_steeringBehaviors.Seek(*this, x2e0_destPos), CVector3f::Zero(), 1.f));
    if (!x665_26_shouldSwoosh) {
      break;
    }

    x678_floorLevel = x2e0_destPos.GetZ();
    FloatToLevel(x678_floorLevel, arg);
    AnimationData()->SetParticleEffectState(skSpeedSwooshName, true, mgr);
    x665_24_ = false;
    if (x665_28_inRange) {
      break;
    }

    const float movement = arg * x138_velocity.Magnitude();
    const float range = x66c_ + movement * 2.5f;
    // TODO: These two vector temporaries use reversed stack slots.
    const CVector3f& translation = GetTranslation();
    const CVector3f& delta = translation - x2e0_destPos;
    x665_28_inRange = delta.MagSquared() < range * range;
  } break;
  case kStateMsg_Deactivate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
    SetDestPos(mgr.GetPlayer()->GetTranslation());
    AnimationData()->SetParticleEffectState(skSpeedSwooshName, false, mgr);
    KnockBackCtrl().EnableAnimReaction(kAR_KnockBack, true);
    x665_28_inRange = false;
    break;
  }
}

bool CChozoGhost::InRange(CStateManager& mgr, float arg) { return x665_28_inRange; }

void CChozoGhost::SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    FindBestAnchor(mgr);
    break;
  }
  }
}

bool CChozoGhost::ShouldAttack(CStateManager& mgr, float arg) {
  return x680_behaveType == kBT_Attack;
}

void CChozoGhost::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    CTeamAiMgr::AddAttacker(kAT_Projectile, mgr, x6c4_teamMgr, GetUniqueId());
    SetAnimationState(kAS_Ready);
    switch (x6d8_) {
    case 1:
      x67c_attackType = 3;
      break;
    case 2:
      x67c_attackType = 4;
      break;
    case 3:
      x67c_attackType = 5;
      break;
    }
    if (x665_25_) {
      const CRayCastResult result =
          mgr.RayStaticIntersection(GetTranslation() + 0.5f * CVector3f::Up(), CVector3f::Up(),
                                    x670_, CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
      if (!result.IsValid()) {
        x67c_attackType = 2;
        KnockBackCtrl().EnableAnimReaction(kAR_KnockBack, false);
      }
    }
    x150_momentum = CVector3f::Zero();
    xfc_constantForce = CVector3f::Zero();
    break;
  }
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_MeleeAttack, &CPatterned::TryMeleeAttack, x67c_attackType);
    BodyCtrl()->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                 GetTranslation());
    if (x67c_attackType != 2) {
      FloatToLevel(x678_floorLevel, arg);
    }
    break;
  case kStateMsg_Deactivate:
    SetAnimationState(kAS_NotReady);
    x665_26_shouldSwoosh = false;
    KnockBackCtrl().EnableAnimReaction(kAR_KnockBack, true);
    CTeamAiMgr::ResetTeamAiRole(kAT_Projectile, mgr, x6c4_teamMgr, GetUniqueId(), true);
    break;
  }
}

void CChozoGhost::Land(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Update: {
    FloatToLevel(x678_floorLevel, arg);
    if (CMath::AbsF(x678_floorLevel - GetTranslation().GetZ()) < 0.05f) {
      StateMachineState().SetCodeTrigger();
    }
    break;
  }
  }
}

void CChozoGhost::Shuffle(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CBehaveChance& chance = ChooseBehaveChanceRange(mgr);
    const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, x6c4_teamMgr, GetUniqueId());
    if (role &&
        (role->GetTeamAiRole() != CTeamAiRole::kTAR_Projectile ||
         !CTeamAiMgr::CanAcceptAttacker(kAT_Projectile, mgr, x6c4_teamMgr, GetUniqueId()))) {
      x680_behaveType = kBT_Attack;
    }
    x680_behaveType = ChooseBehaveChanceRange(mgr).GetBehave(x680_behaveType, mgr);
    switch (x680_behaveType) {
    case kBT_Lurk:
      x684_lurkDelay = chance.GetLurkTime();
      break;
    case kBT_Attack:
      x665_25_ = mgr.Random()->Float() < chance.GetChargeAttack();
      x6d8_ = mgr.Random()->Next() % chance.GetNumBolts() + 1;
      break;
    default:
      break;
    }
    x664_31_ = false;
    x665_27_playerInLeashRange = false;
    break;
  }
  }
}

bool CChozoGhost::ShouldTaunt(CStateManager& mgr, float arg) {
  return x680_behaveType == kBT_Taunt;
}

void CChozoGhost::Taunt(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    SetAnimationState(kAS_Ready);
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Taunt, &CPatterned::TryTaunt, 0);
    FloatToLevel(x678_floorLevel, arg);
    break;
  case kStateMsg_Deactivate:
    SetAnimationState(kAS_NotReady);
    x665_26_shouldSwoosh = false;
    break;
  }
}

void CChozoGhost::Hurled(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    SetVerticalMovement(false);
    x664_27_onGround = false;
    x665_24_ = true;
    break;
  case kStateMsg_Update:
    x3e8_alphaDelta = 2.f;
    if (!x664_27_onGround) {
      if (GetVelocityWR().GetZ() < 0.f) {
        const CRayCastResult result =
            mgr.RayStaticIntersection(GetTranslation() + CVector3f::Up(), CVector3f::Down(), 2.f,
                                      CMaterialFilter::MakeInclude(CMaterialList(kMT_Floor)));
        if (result.IsValid() && result.GetTime() < 1.05f) {
          x664_27_onGround = true;
          x150_momentum = CVector3f::Zero();
          SetVelocityWR(CVector3f(GetVelocityWR().GetX(), GetVelocityWR().GetY(), 0.f));
          x678_floorLevel = result.GetPoint().GetZ();
          StateMachineState().SetCodeTrigger();
        }
      }
      if (!x664_27_onGround && GetStateMachineTime() > x638_hurlRecoverTime) {
        BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
        BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
        StateMachineState().SetCodeTrigger();
      }
    }
    break;
  case kStateMsg_Deactivate:
    SetVerticalMovement(true);
    x150_momentum = CVector3f::Zero();
    break;
  }
}

void CChozoGhost::Lurk(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    StateMachineState().SetDelay(x684_lurkDelay);
    break;
  case kStateMsg_Update:
    FloatToLevel(x678_floorLevel, arg);
    break;
  }
}

bool CChozoGhost::ShouldMove(CStateManager& mgr, float arg) { return x680_behaveType == kBT_Move; }

bool CChozoGhost::Leash(CStateManager& mgr, float arg) {
  return x665_27_playerInLeashRange || CPatterned::Leash(mgr, arg);
}

bool CChozoGhost::ShouldFlinch(CStateManager& mgr, float arg) { return x664_25_flinch; }

bool CChozoGhost::AggressionCheck(CStateManager& mgr, float arg) { return x665_29_aggressive; }

void CChozoGhost::Deactivate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x68c_boneTracking.SetActive(false);
    ReleaseCoverPoint(mgr, x674_coverPoint);
    SetAnimationState(kAS_Ready);
    x665_24_ = true;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Generate, &CPatterned::TryGenerateDeactivate, 1);
    if (GetAnimationState() == kAS_Repeat) {
      BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    }
    break;
  case kStateMsg_Deactivate:
    SetAnimationState(kAS_NotReady);
    break;
  }
}

void CChozoGhost::Dead(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    ReleaseCoverPoint(mgr, x674_coverPoint);
    x3e8_alphaDelta = 4.f;
    x664_30_fadedOut = false;
    x664_29_fadedIn = false;
    x68c_boneTracking.SetActive(false);
    Stop();
    break;
  case kStateMsg_Update:
    Stop();
    x150_momentum = CVector3f::Zero();
    break;
  }
}

CProjectileInfo* CChozoGhost::ProjectileInfo() {
  if (x67c_attackType == 2) {
    return &x578_projectileInfo1;
  }
  return &x5a0_projectileInfo2;
}

void CChozoGhost::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                  EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile: {
    const CTransform4f locator = GetLctrTransform(node.GetLocatorName());
    const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    const CTransform4f projectileXf = CTransform4f::LookAt(locator.GetTranslation(), aimPos);
    switch (x67c_attackType) {
    case 2: {
      CGameProjectile* projectile = LaunchProjectile(
          projectileXf, mgr, 2,
          static_cast< CWeapon::EProjectileAttrib >(CWeapon::kPA_BigStrike |
                                                    CWeapon::kPA_StaticInterference),
          true, x640_projectileVisor, x650_soundProjectileVisor, false, CVector3f(1.f, 1.f, 1.f));
      if (projectile) {
        projectile->SetDamageDuration(x62c_);
        projectile->SetInterferenceDuration(x62c_);
        projectile->SetMinHomingDistance(x634_);
      }
      break;
    }
    default: {
      CGameProjectile* projectile = LaunchProjectile(
          projectileXf, mgr, 5,
          static_cast< CWeapon::EProjectileAttrib >(CWeapon::kPA_DamageFalloff |
                                                    CWeapon::kPA_StaticInterference),
          true, x640_projectileVisor, x650_soundProjectileVisor, false, CVector3f(1.f, 1.f, 1.f));
      if (projectile) {
        const float speed = ProjectileInfo()->GetProjectileSpeed();
        if (speed > 0.f) {
          projectile->SetDamageFalloffSpeed(80.f / speed);
        }
        projectile->SetDamageDuration(x62c_);
        projectile->SetInterferenceDuration(x62c_);
        projectile->SetMinHomingDistance(x634_);
      }
      break;
    }
    }
    handled = true;
    break;
  }
  case kUE_FadeIn:
    if (x664_30_fadedOut) {
      x3e8_alphaDelta = 2.f;
      CSfxManager::AddEmitter(x630_sfxFadeIn, GetTranslation(), CVector3f::Zero(), false, false,
                              CSfxManager::kMedPriority, CSfxManager::kAllAreas);
    }
    AddMaterial(kMT_Target, mgr);
    x664_30_fadedOut = false;
    x664_29_fadedIn = true;
    handled = true;
    break;
  case kUE_FadeOut:
    if (x664_29_fadedIn) {
      x3e8_alphaDelta = -2.f;
      CSfxManager::AddEmitter(x632_sfxFadeOut, GetTranslation(), CVector3f::Zero(), false, false,
                              CSfxManager::kMedPriority, CSfxManager::kAllAreas);
    }
    RemoveMaterial(kMT_Target, mgr);
    x664_29_fadedIn = false;
    x664_30_fadedOut = true;
    x665_26_shouldSwoosh = true;
    handled = true;
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
  if (type == kUE_Delete) {
    x3e8_alphaDelta = -1.f;
  }
}

void CChozoGhost::KnockBack(const CVector3f& dir, CStateManager& mgr, const CDamageInfo& info,
                            float magnitude, bool direct, const bool inDeferred) {
  if (!IsAlive()) {
    KnockBackCtrl().EnableAnimReaction(kAR_Hurled, false);
  } else if (!KnockBackCtrl().TestAvailableState(kAR_KnockBack) &&
             info.GetWeaponMode().IsCharged()) {
    KnockBackCtrl().SetAnimationStateRange(kAR_Hurled, kAR_Fall);
  }
  CPatterned::KnockBack(dir, mgr, info, magnitude, direct, inDeferred);
  KnockBackCtrl().SetAnimationStateRange(kAR_Flinch, kAR_Fall);
  if (IsAlive()) {
    if (KnockBackCtrl().GetActiveParms().x0_animState == kAR_Hurled) {
      StateMachineState().SetState(mgr, *this, GetStateMachine(), rstl::string_l("Hurled"));
    }
  } else {
    Stop();
    x150_momentum = CVector3f::Zero();
  }
}

void CChozoGhost::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  CPatterned::Think(dt, mgr);
  UpdateThermalFrozenState(false);
  x68c_boneTracking.Update(dt);
  x6c8_spaceWarpTime = CMath::Max(0.f, x6c8_spaceWarpTime - dt);
  SetTargetable(IsVisibleEnough(mgr));
}
