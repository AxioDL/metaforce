#include "MetroidPrime/Enemies/CDrone.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CFluidPlaneCPU.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameCollision.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CDroneLaser.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CTeamAiMgr.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptVisorFlare.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWater.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "Weapons/CCollisionResponseData.hpp"

#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "rstl/math.hpp"

static const char* sBeaconLocator = "Beacon_LCTR";
static const char* sRightGunLocator = "R_GUN_TOP_LCTR";
static const char* sLeftGunLocator = "L_GUN_TOP_LCTR";

CDroneLaser::~CDroneLaser() {}

CDroneLaser::CDroneLaser(TUniqueId uid, TAreaId areaId, const CTransform4f& xf, CAssetId particle)
: CActor(uid, true, rstl::string_l("DroneLaser"), CEntityInfo(areaId, NullConnectionList), xf,
         CModelData::CModelDataNull(), CMaterialList(kMT_NoStepLogic),
         CActorParameters::None().HotInThermal(true), kInvalidUniqueId)
, xe8_wallIntersection(CVector3f::Zero())
, xf4_scannerLight(kInvalidUniqueId)
, xf8_beamDesc(gpSimplePool->GetObj(SObjectTag('PART', particle)))
, x104_beamParticle(
      rs_new CElementGen(xf8_beamDesc, CElementGen::kMOT_Normal, CElementGen::kOSF_One)) {}

void CDroneLaser::SetWallIntersection(CStateManager& mgr, const CVector3f& pos,
                                      const CVector3f& direction) {
  xe8_wallIntersection = pos;
  const CVector3f beamDirection = (pos - GetTranslation()).AsNormalized();
  if (xf4_scannerLight != kInvalidUniqueId) {
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(xf4_scannerLight))) {
      light->SetTranslation(pos - 0.5f * beamDirection);
    }
  }
  x104_beamParticle->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), direction));
  x104_beamParticle->SetTranslation(pos);
}

void CDroneLaser::Think(float dt, CStateManager&) { x104_beamParticle->Update(dt); }

void CDroneLaser::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  CActor::AcceptScriptMsg(msg, sender, mgr);
  switch (msg) {
  case kSM_Activate:
    SetScannerLightActive(mgr, true);
    x104_beamParticle->SetParticleEmission(true);
    break;
  case kSM_Deactivate:
    SetScannerLightActive(mgr, false);
    x104_beamParticle->SetParticleEmission(false);
    break;
  case kSM_Registered: {
    xf4_scannerLight = mgr.AllocateUniqueId();
    const CLight light(CLight::BuildPoint(CVector3f::Zero(), CColor::Red()));
    CGameLight* const scanner = rs_new CGameLight(
        xf4_scannerLight, GetCurrentAreaId(), GetActive(), rstl::string_l("LaserScanner"),
        CTransform4f::Identity(), GetUniqueId(), light, 0, 0, 0.f);
    mgr.AddObject(*scanner);
    break;
  }
  case kSM_Deleted:
    if (xf4_scannerLight != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(xf4_scannerLight);
      xf4_scannerLight = kInvalidUniqueId;
    }
    break;
  }
}

ENTITY_ACCEPT_IMPL(CDroneLaser)

void CDroneLaser::CalculateRenderBounds() {
  CAABox bounds = CAABox::MakeMaxInvertedBox();
  const CVector3f diff = xe8_wallIntersection - GetTranslation();
  const float length = diff.Magnitude();
  bounds.AccumulateBounds(GetTranslation());
  bounds.AccumulateBounds(xe8_wallIntersection + (0.1f * length) * GetTransform().GetUp());
  bounds.AccumulateBounds(xe8_wallIntersection - (0.1f * length) * GetTransform().GetUp());
  SetRenderBounds(bounds);
}

void CDroneLaser::AddToRenderer(const CFrustumPlanes&, const CStateManager& mgr) const {
  gpRender->AddParticleGen(*x104_beamParticle);
  EnsureRendered(mgr);
}

void CDroneLaser::Render(const CStateManager&) const {
  gpRender->SetDepthReadWrite(true, true);
  RenderBeam(4, 0.01f, CColor(1.f, 0.9f, 0.9f, 1.f), true);
  gpRender->SetDepthReadWrite(true, false);
  RenderBeam(5, 0.06f, CColor(0.4f, 0.f, 0.f, 0.5f), false);
  RenderBeam(7, 0.08f, CColor(0.4f, 0.2f, 0.2f, 0.1f), false);
}

void CDroneLaser::RenderBeam(uint subdivisions, float radius, const CColor& color,
                             bool additive) const {
  if (GetActive()) {
    const CVector3f diff = xe8_wallIntersection - GetTranslation();
    const float length = diff.Magnitude();
    const CTransform4f xf(CTransform4f::LookAt(GetTranslation(), xe8_wallIntersection));
    CGraphics::DisableAllLights();
    CGX::SetNumTexGens(0);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    if (additive) {
      CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
    } else {
      CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
    }
    CGraphics::SetCullMode(kCM_None);
    gpRender->SetModelMatrix(CTransform4f::Identity());
    CGraphics::StreamBegin(kP_TriangleStrip);
    const uint beamColor = color.GetColor_u32();
    const float step = M_2PIF / subdivisions;
    float angle = 0.f;
    for (uint i = 0; i < subdivisions + 1; ++i) {
      const float cosine = CMath::FastCosR(angle);
      const float sine = CMath::FastSinR(angle);
      const CVector3f offset(cosine * radius, 0.f, sine * radius);
      CGraphics::StreamVertex(xf * offset);
      CGraphics::StreamColor(beamColor);
      CGraphics::StreamTexcoord(0.f, cosine);
      CGraphics::StreamVertex(xf * (CVector3f(0.f, length, 0.f) + offset));
      CGraphics::StreamColor(beamColor);
      CGraphics::StreamTexcoord(length, cosine);
      angle += step;
    }
    CGraphics::StreamEnd();
    CGraphics::SetCullMode(kCM_Front);
  }
}

void CDroneLaser::SetScannerLightActive(CStateManager& mgr, bool active) {
  mgr.SendScriptMsgAlways(xf4_scannerLight, GetUniqueId(), active ? kSM_Activate : kSM_Deactivate);
}

CDrone::CDrone(TUniqueId uid, const rstl::string& name, EFlavorType flavor, const CEntityInfo& info,
               const CTransform4f& xf, float f1, const CModelData& mData,
               const CPatternedInfo& pInfo, const CActorParameters& aParms, EMovementType moveType,
               EColliderType collider, EBodyType bodyType, const CDamageInfo& dInfo1, CAssetId aId1,
               const CDamageInfo& dInfo2, CAssetId aId2,
               rstl::vector< CVisorFlare::CFlareDef > flares, float f2, float f3, float f4,
               float f5, float f6, float f7, float f8, float f9, float f10, float f11, float f12,
               float f13, float f14, float f15, float f16, float f17, float f18, float f19,
               float f20, CAssetId crscId, float f21, float f22, float f23, float f24, int soundId,
               bool b1)
: CPatterned(kC_Drone, uid, name, flavor, info, xf, mData, pInfo, moveType, collider, bodyType,
             aParms, flavor == kFT_Zero ? kCS_Medium : kCS_Large)
, x568_laserParticlesId(aId1)
, x56c_collisionResponse(gpSimplePool->GetObj(SObjectTag('CRSC', crscId)))
, x578_lightId(kInvalidUniqueId)
, x57a_visorFlareId(kInvalidUniqueId)
, x57c_flares(flares)
, x58c_prevDodgeDir(pas::kSD_Left)
, x590_damageInfo(dInfo1)
, x5ac_laserDamageInfo(dInfo2)
, x5c8_(0.f)
, x5cc_(0.f)
, x5d0_(0.f)
, x5d4_(0.f)
, x5d8_(0.f)
, x5dc_(0.f)
, x5e0_(0.f)
, x5e4_(f23)
, x5e8_shieldTime(0.f)
, x5ec_turnSpeed(f1)
, x5f0_(f2)
, x5f4_(f3)
, x5f8_(f4)
, x5fc_(f5)
, x600_(f11)
, x604_(0.f)
, x608_(f6)
, x60c_(f7)
, x610_(f8)
, x614_(f9)
, x618_(f10)
, x61c_(f12)
, x620_(f20)
, x624_(0.f)
, x628_(0.f)
, x62c_(0.f)
, x630_(0.f)
, x634_(0.f)
, x638_(0.f)
, x63c_(f13)
, x640_(f14)
, x644_(0.f)
, x648_(f15)
, x64c_(f16)
, x650_(f17)
, x654_(f18)
, x658_(f19)
, x65c_(f21)
, x660_(f22)
, x664_(f24)
, x668_(0.f)
, x66c_(0.f)
, x670_(CVector3f::Zero())
, x67c_(CVector3f::Zero())
, x688_teamMgr(kInvalidUniqueId)
, x690_colSphere(CSphere(CVector3f(0.f, 0.f, 1.8f), 1.1f), GetMaterialList())
, x6b0_pathFind(nullptr, b1 ? 4 : 3, pInfo.GetPathfindingIndex(), 1.f, 2.4f)
, x794_(CAxisAngle::Identity())
, x7a0_(CVector3f::Zero())
, x7ac_lightPos(0.f, 0.f, 0.f)
, x7b8_(0.f)
, x7bc_(0.f)
, x7c0_(0.f)
, x7c4_(0.f)
, x7c8_(0)
, x7cc_laserSfx(CSfxManager::TranslateSFXID(soundId))
, x7d0_laserSfxHandle()
, x7d8_laserIds(kInvalidUniqueId)
, x7e0_lasersStart(CVector3f::Zero())
, x7fc_lasersEnd(CVector3f::Zero())
, x818_lasersTime(0.f)
, x824_activeLasers(false)
, x82c_shieldModel(rs_new CModelData(CStaticRes(aId2, CVector3f(1.f, 1.f, 1.f))))
, x830_(0)
, x832_24_(0)
, x832_27_(0)
, x834_24_waveHit(false)
, x834_25_(false)
, x834_26_(false)
, x834_27_(false)
, x834_28_(false)
, x834_29_codeTrigger(false)
, x834_30_visible(false)
, x834_31_attackOver(false)
, x835_24_(false)
, x835_25_(b1)
, x835_26_(false) {
  UpdateTouchBounds(pInfo.GetHalfExtent());
  x460_knockBackController.SetEnableShock(true);
  x460_knockBackController.EnableAnimReaction(kAR_Hurled, false);
  x460_knockBackController.SetLocomotionDuringElectrocution(true);
  MakeThermalColdAndHot();
  SetDrawShadow(x3fc_flavor != kFT_One);
}

ENTITY_ACCEPT_IMPL(CDrone)

void CDrone::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, sender, mgr);
  switch (msg) {
  case kSM_Activate:
    SetLightEnabled(mgr, true);
    SquadAdd(mgr);
    break;
  case kSM_Deactivate:
  case kSM_Deleted:
    for (int i = 0; i < x7d8_laserIds.size(); ++i) {
      if (x7d8_laserIds[i] != kInvalidUniqueId) {
        mgr.DeleteObjectRequest(x7d8_laserIds[i]);
        x7d8_laserIds[i] = kInvalidUniqueId;
      }
    }
    SquadRemove(mgr);
    mgr.PlayerState()->StaticInterference().RemoveSource(GetUniqueId());
    if (x578_lightId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(x578_lightId);
      x578_lightId = kInvalidUniqueId;
    }
    if (x57a_visorFlareId != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(x57a_visorFlareId);
      x57a_visorFlareId = kInvalidUniqueId;
    }
    if (x7d0_laserSfxHandle) {
      CSfxManager::RemoveEmitter(x7d0_laserSfxHandle);
      x7d0_laserSfxHandle = CSfxHandle();
    }
    break;
  case kSM_Registered: {
    BodyCtrl()->Activate(mgr);
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    BodyCtrl()->BodyStateInfo().SetMaximumPitch(0.f);
    x5cc_ = 0.f;
    x460_knockBackController.SetEnableFreeze(false);
    AddMaterial(kMT_AIJoint, mgr);
    x578_lightId = mgr.AllocateUniqueId();
    const CLight light(CLight::BuildPoint(CVector3f::Zero(), CColor::Red()));
    CGameLight* const laserLight = rs_new CGameLight(
        x578_lightId, GetCurrentAreaId(), GetActive(), rstl::string_l("LaserLight"),
        CTransform4f::Identity(), GetUniqueId(), light, 0, 0, 0.f);
    mgr.AddObject(*laserLight);
    break;
  }
  case kSM_Alert:
    x834_29_codeTrigger = true;
    break;
  case kSM_InitializedInArea: {
    const TAreaId area = GetCurrentAreaId();
    x6b0_pathFind.SetArea(mgr.GetWorld()->GetAreaAlways(area).GetPostConstructed()->x10bc_pathArea);
    if (x688_teamMgr == kInvalidUniqueId) {
      x688_teamMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
      if (GetActive()) {
        SquadAdd(mgr);
      }
    }
    x604_ = HealthInfo(mgr)->GetHP();
    const CVector3f& scale = GetModelData()->GetScale();
    x55c_moveScale = CVector3f(1.f / scale.GetX(), 1.f / scale.GetY(), 1.f / scale.GetZ());
    if (x835_25_) {
      SetSoundEventPitchBend(0);
    }
    break;
  }
  case kSM_OnFloor:
    if (!x835_26_ && x834_24_waveHit && !IsAlive()) {
      x835_26_ = true;
      MassiveFrozenDeath(mgr);
    }
    break;
  }
}

void CDrone::SetLightEnabled(CStateManager& mgr, bool active) {
  mgr.SendScriptMsgAlways(x578_lightId, GetUniqueId(), active ? kSM_Activate : kSM_Deactivate);
}

void CDrone::SetVisorFlareEnabled(CStateManager& mgr, bool active) {
  if (IsAlive()) {
    CScriptVisorFlare* flare = TCastToPtr< CScriptVisorFlare >(mgr.ObjectById(x57a_visorFlareId));
    if (flare == nullptr && active) {
      x57a_visorFlareId = mgr.AllocateUniqueId();
      const CTransform4f xf(GetLctrTransform(rstl::string_l("Beacon_LCTR")));
      flare = rs_new CScriptVisorFlare(x57a_visorFlareId, rstl::string_l("DroneVisorFlare"),
                                       CEntityInfo(GetCurrentAreaId(), NullConnectionList), active,
                                       xf.GetTranslation(), CVisorFlare::kBM_Additive, true, 0.1f,
                                       1.f, 2.f, 0, 0, x57c_flares);
      mgr.AddObject(*flare);
    }
    mgr.DeliverScriptMsg(flare, GetUniqueId(), active ? kSM_Activate : kSM_Deactivate);
  }
}

void CDrone::UpdateVisorFlare(CStateManager& mgr) {
  CScriptVisorFlare* flare = TCastToPtr< CScriptVisorFlare >(mgr.ObjectById(x57a_visorFlareId));
  const CVector3f& direction =
      (mgr.GetPlayer()->GetTranslation() - GetTranslation()).AsNormalized();
  SetVisorFlareEnabled(mgr, CVector3f::Dot(direction, GetTransform().GetForward()) > 0.f);
  if (flare != nullptr) {
    const CTransform4f xf(GetLctrTransform(rstl::string_l("Beacon_LCTR")));
    flare->SetTranslation(xf.GetTranslation() + 0.1f * xf.GetForward());
  }
}

void CDrone::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                             float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile: {
    const CTransform4f xf(GetLctrTransform(node.GetLocatorName()));
    FireProjectile(mgr, xf);
    handled = true;
    break;
  }
  case kUE_DamageOn:
    if (IsAlive() && x835_24_) {
      if (!x824_activeLasers[0]) {
        SetLaserActive(mgr, 0, true);
        x824_activeLasers[0] = true;
        SetVisorFlareEnabled(mgr, true);
      } else if (x3fc_flavor == kFT_One) {
        SetLaserActive(mgr, 1, true);
        x824_activeLasers[1] = true;
      }
    }
    handled = true;
    break;
  case kUE_DamageOff:
    if (x824_activeLasers[0]) {
      SetLaserActive(mgr, 0, false);
      x824_activeLasers[0] = false;
      SetVisorFlareEnabled(mgr, false);
    } else if (x3fc_flavor == kFT_One) {
      SetLaserActive(mgr, 1, false);
      x824_activeLasers[1] = false;
    }
    handled = true;
    break;
  case kUE_FadeIn:
    if (x3fc_flavor == kFT_One) {
      x834_30_visible = true;
    }
    handled = true;
    break;
  case kUE_FadeOut:
    if (x3fc_flavor == kFT_One) {
      x834_30_visible = false;
    }
    handled = true;
    break;
  case kUE_Delete:
    if (x7d0_laserSfxHandle) {
      CSfxManager::RemoveEmitter(x7d0_laserSfxHandle);
      x7d0_laserSfxHandle = CSfxHandle();
    }
    MassiveDeath(mgr);
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CDrone::Touch(CActor& actor, CStateManager& mgr) {
  CPatterned::Touch(actor, mgr);
  if (CWeapon* weapon = TCastToPtr< CWeapon >(actor)) {
    if (IsAlive()) {
      x834_24_waveHit = weapon->GetType() == kWT_Wave;
      if (x3fc_flavor == kFT_One) {
        const CVector3f direction = weapon->GetTranslation() - GetTranslation();
        if (HitShield(direction)) {
          x5e8_shieldTime = 1.f;
        }
      }
    }
  }
}

void CDrone::FireProjectile(CStateManager& mgr, const CTransform4f& xf) {
  const CVector3f target = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
  if (CVector3f::Dot((target - xf.GetTranslation()).AsNormalized(), xf.GetForward()) > 0.8660254f) {
    CVector3f offset = CVector3f::Zero();
    if (mgr.Random()->Float() > 0.2f) {
      const CTransform4f lookAt(CTransform4f::LookAt(xf.GetTranslation(), target));
      const CUnitVector3f axis(lookAt.GetForward(), CUnitVector3f::kN_No);
      const CQuaternion rotation =
          CQuaternion::AxisAngle(axis, CRelAngle(mgr.Random()->Range(0.f, M_PIF)));
      offset = rotation.Transform(4.f * lookAt.GetRight());
    }
    const CTransform4f shot(CTransform4f::LookAt(xf.GetTranslation(), target + offset));
    LaunchProjectile(shot, mgr);
  } else {
    LaunchProjectile(xf, mgr);
  }
}

void CDrone::LaunchProjectile(const CTransform4f& xf, CStateManager& mgr) {
  static const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough));
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  TUniqueId hitId = kInvalidUniqueId;
  mgr.BuildNearList(nearList, xf.GetTranslation(), xf.GetForward(), 100000.f, filter, this);
  const CRayCastResult result = mgr.RayWorldIntersection(
      hitId, xf.GetTranslation(), xf.GetForward(), 100000.f, filter, nearList);
  if (result.IsValid()) {
    if (hitId == mgr.GetPlayer()->GetUniqueId()) {
      mgr.ApplyDamage(
          GetUniqueId(), hitId, GetUniqueId(), x5ac_laserDamageInfo,
          CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
          CVector3f::Zero());
    }
    const bool hot = (GetThermalFlags() & kTF_Hot) != 0;
    mgr.DoCollisionResponse(**x56c_collisionResponse, result, hitId,
                            x5ac_laserDamageInfo.GetWeaponMode(), true, hot);
  }
}

bool CDrone::HitShield(const CVector3f& dir) const {
  if (x3fc_flavor != kFT_One) {
    return false;
  }
  if (close_enough(x5dc_, 0.f)) {
    return false;
  }
  return CVector3f::Dot(GetLctrTransform(rstl::string_l("Shield_LCTR")).GetForward(),
                        dir.AsNormalized()) > 0.85f;
}

EWeaponCollisionResponseTypes CDrone::GetCollisionResponseType(const CVector3f&,
                                                               const CVector3f& dir,
                                                               const CWeaponMode&, int) const {
  EWeaponCollisionResponseTypes result = kWCR_Unknown36;
  if (x3fc_flavor == kFT_One && HitShield(-dir)) {
    x5e8_shieldTime = 1.f;
    result = kWCR_Unknown86;
  }
  return result;
}

const CDamageVulnerability* CDrone::GetDamageVulnerability(const CVector3f&, const CVector3f& dir,
                                                           const CDamageInfo&) const {
  if (x3fc_flavor == kFT_One && HitShield(-dir)) {
    x5e8_shieldTime = 1.f;
    return &CDamageVulnerability::ReflectVulnerability();
  }
  return CAi::GetDamageVulnerability();
}

void CDrone::Think(float dt, CStateManager& mgr) {
  if (x3fc_flavor == kFT_One) {
    const bool xray = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay;
    float alpha = 1.f;
    if (!xray) {
      alpha = rstl::max_val(0.f, x428_damageCooldownTimer / skDamageHitTime);
    }
    x42c_color.SetAlpha(alpha);
  }
  x403_25_enableStateMachine = !BodyCtrl()->IsElectrocuting();
  if (BodyCtrl()->IsElectrocuting() && (x824_activeLasers[0] || x824_activeLasers[1])) {
    x824_activeLasers[0] = false;
    x824_activeLasers[1] = false;
    SetLaserActive(mgr, 0, false);
    SetLaserActive(mgr, 1, false);
    SetVisorFlareEnabled(mgr, false);
  }
  CPatterned::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }
  x5c8_ -= dt;
  if (x7c4_ >= 0.f) {
    x7c4_ -= dt;
  }
  if (x5d0_ >= 0.f) {
    if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
      x5d0_ -= 3.f * dt;
    } else {
      x5d0_ -= dt;
    }
  }
  if (x624_ >= 0.f) {
    x624_ -= dt;
  }
  if (x644_ >= 0.f) {
    x644_ -= dt;
  }
  if (x824_activeLasers[0] || (x824_activeLasers[1] && IsAlive())) {
    UpdateLaser(mgr, dt);
    UpdateVisorFlare(mgr);
  }
  if (x834_25_ && IsAlive()) {
    UpdateScanner(mgr, dt);
  }
  const float distance = (mgr.GetPlayer()->GetTranslation() - GetTranslation()).MagSquared();
  if (x834_28_ && distance < x60c_ * x60c_) {
    mgr.PlayerState()->StaticInterference().RemoveSource(GetUniqueId());
    const float intensity = rstl::max_val(
        0.f, x608_ - mgr.GetPlayerState()->GetStaticInterference().GetTotalInterference());
    mgr.PlayerState()->StaticInterference().AddSource(GetUniqueId(), intensity, 0.2f);
  }
  if (!x834_28_ && distance < x614_ * x614_) {
    mgr.PlayerState()->StaticInterference().RemoveSource(GetUniqueId());
    const float intensity = rstl::max_val(
        0.f, x610_ - mgr.GetPlayerState()->GetStaticInterference().GetTotalInterference());
    mgr.PlayerState()->StaticInterference().AddSource(GetUniqueId(), intensity, 0.2f);
  }
  if (!x834_28_ && IsAlive() && !x835_25_) {
    x5e0_ -= dt;
    if (x5e0_ <= 0.f) {
      UpdateWaterRipples(mgr);
      x5e0_ = 0.1f;
    }
  }
  const float healthDiff = x604_ - HealthInfo(mgr)->GetHP();
  if (!close_enough(x600_, 0.f)) {
    x5d0_ -= healthDiff / x600_;
    x624_ -= healthDiff / x600_;
  }
  x604_ = HealthInfo(mgr)->GetHP();
  if (x3fc_flavor == kFT_One) {
    if (x834_30_visible) {
      x5dc_ = rstl::min_val(1.f, x5dc_ + 3.f * dt);
    } else {
      x5dc_ = rstl::max_val(0.f, x5dc_ - 3.f * dt);
    }
    x5e8_shieldTime = rstl::max_val(0.f, x5e8_shieldTime - dt);
    if (!close_enough(x5dc_, 0.f)) {
      if (!x7d0_laserSfxHandle && IsAlive()) {
        x7d0_laserSfxHandle =
            CSfxManager::AddEmitter(0xdd, GetTranslation(), CVector3f::Zero(), true, true,
                                    CSfxManager::kMedPriority, GetCurrentAreaId().Value());
      }
    } else if (x7d0_laserSfxHandle) {
      CSfxManager::RemoveEmitter(x7d0_laserSfxHandle);
      x7d0_laserSfxHandle.Clear();
    }
  }
  UpdateThermal(mgr, dt);
  UpdatePitchBend(dt);
  if (!x835_25_) {
    const CVector3f& scale = CVector3f(GetModelData()->GetScale());
    CGameCollision::AvoidStaticCollisionWithinRadius(mgr, *this, 8, dt, 0.25f, 3.5f * scale.GetY(),
                                                     3000.f, 0.5f);
  }
  if (x66c_ <= 0.f) {
    x668_ = mgr.RayStaticIntersection(GetTranslation(), CVector3f::Down(), 10000.f,
                                      CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)))
                .GetTime();
    x66c_ = 0.f;
  } else {
    x66c_ -= dt;
  }
  if (IsAlive() && x835_25_) {
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    CAABox bounds = GetBoundingBox();
    bounds.AccumulateBounds(GetTranslation() + 20.f * CVector3f::Down());
    mgr.BuildNearList(nearList, GetBoundingBox(),
                      CMaterialFilter::MakeInclude(CMaterialList(kMT_Trigger)), this);
    for (AUTO(it, nearList.begin()); it < nearList.end(); ++it) {
      if (const CScriptWater* water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(*it))) {
        const float height =
            water->GetTriggerBoundsWR().GetMaxPoint().GetZ() - GetTranslation().GetZ();
        if (height < 3.f) {
          const float speed = height < 1.5 ? 60.f : 20.f;
          const CVector3f move = speed * (dt * CVector3f::Down());
          const CVector3f impulse = GetMoveToORImpulseWR(GetTransform().TransposeRotate(move), dt);
          ApplyImpulseWR(impulse, CAxisAngle::Identity());
          break;
        }
      }
    }
  }
  if (IsAlive() && x668_ < x664_) {
    const CVector3f move = dt * (1.f * CVector3f::Up());
    const CVector3f impulse = GetMoveToORImpulseWR(GetTransform().TransposeRotate(move), dt);
    ApplyImpulseWR(impulse, CAxisAngle::Identity());
  }
  SetTargetable(IsAlive());
}

void CDrone::UpdateTouchBounds(float radius) {
  const CTransform4f xf(GetLctrTransform(rstl::string_l("Skeleton_Root")));
  const CVector3f center = xf.GetTranslation() - GetTranslation();
  const CVector3f extent(radius, radius, radius);
  const CAABox bounds(center - extent, center + extent);
  x690_colSphere.SetSphere(CSphere(center, radius));
  SetBoundingBox(bounds);
  x6b0_pathFind.SetCharacterRadius(0.25f + radius);
}

void CDrone::Patrol(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    SetLightEnabled(mgr, true);
    x834_25_ = true;
    break;
  case kStateMsg_Update: {
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    BuildNearList(kMT_Character, kMT_Player, 5.f, nearList, mgr);
    if (!nearList.empty()) {
      const CActor* actor = static_cast< const CActor* >(mgr.GetObjectById(nearList[0]));
      const CVector3f separation =
          x45c_steeringBehaviors.Separation(*this, actor->GetTranslation(), 5.f);
      if (!(separation == CVector3f::Zero())) {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(separation, CVector3f::Zero(), 0.5f));
      }
    }
    break;
  }
  case kStateMsg_Deactivate:
    SetLightEnabled(mgr, false);
    x834_25_ = false;
    break;
  }
  CPatterned::Patrol(mgr, msg, dt);
}

const CCollisionPrimitive* CDrone::GetCollisionPrimitive() const {
  if (x834_28_) {
    return CPhysicsActor::GetCollisionPrimitive();
  }
  return &x690_colSphere;
}

void CDrone::UpdateScanner(CStateManager& mgr, float dt) {
  x5d4_ = 1.2f * dt + x5d4_;
  if (x5d4_ > M_2PIF) {
    x5d4_ -= M_2PIF;
  }
  if (x5d4_ < 0.f) {
    x5d4_ = 0.f;
  }
  if (x5d8_ > M_2PIF) {
    x5d8_ -= M_2PIF;
  }
  if (x5d8_ < 0.f) {
    x5d8_ = 0.f;
  }
  const float angle = CMath::Clamp(0.f, 0.5f * (1.f + CMath::FastSinR(x5d4_)), 1.f);
  if (fpclassify(angle) != FP_SUBNORMAL) {
    x5d8_ += 0.03f * CMath::PowF(angle, 5.f);
  }
  const float x = 0.5f * CMath::FastCosR(x5d8_);
  const CVector3f scan(x, 1.f, 0.5f * CMath::FastSinR(2.05f * x5d8_));
  const CVector3f direction = GetTransform().Rotate(scan.AsNormalized());
  static const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough));
  const CTransform4f beacon(GetLctrTransform(rstl::string_l(sBeaconLocator)));
  TUniqueId hitId = kInvalidUniqueId;
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  nearList.push_back(mgr.GetPlayer()->GetUniqueId());
  const CRayCastResult result = mgr.RayWorldIntersection(
      hitId, beacon.GetTranslation() + 0.2f * direction, direction, 10000.f, filter, nearList);
  if (result.IsValid() && x578_lightId != kInvalidUniqueId) {
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(x578_lightId))) {
      light->SetTranslation(result.GetPoint());
      x7ac_lightPos = result.GetPoint();
    }
  }
}

void CDrone::UpdateLaser(CStateManager& mgr, float dt) {
  static const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_ProjectilePassthrough));
  const CTransform4f beacon(GetLctrTransform(rstl::string_l(sBeaconLocator)));
  for (int i = 0; i < x818_lasersTime.size(); ++i) {
    if (x818_lasersTime[i] < 1.f && x824_activeLasers[i]) {
      x818_lasersTime[i] += dt;
      const CVector3f target =
          CVector3f::Lerp(x7e0_lasersStart[i], x7fc_lasersEnd[i], x818_lasersTime[i]);
      const CVector3f direction = (target - beacon.GetTranslation()).AsNormalized();
      TUniqueId hitId = kInvalidUniqueId;
      CAABox bounds = CAABox::MakeMaxInvertedBox();
      bounds.AccumulateBounds(GetTranslation() + 1000.f * direction);
      bounds.AccumulateBounds(GetTranslation());
      rstl::reserved_vector< TUniqueId, 1024 > nearList;
      mgr.BuildNearList(nearList, bounds, filter, nullptr);
      const CRayCastResult result = mgr.RayWorldIntersection(
          hitId, beacon.GetTranslation() + 2.f * direction, direction, 10000.f, filter, nearList);
      if (result.IsValid()) {
        if (x7d8_laserIds[i] != kInvalidUniqueId) {
          if (CDroneLaser* laser = static_cast< CDroneLaser* >(mgr.ObjectById(x7d8_laserIds[i]))) {
            laser->SetTransform(beacon);
            laser->SetWallIntersection(mgr, result.GetPoint(), result.GetPlane().GetNormal());
          }
        }
        if (CPlayer* player = TCastToPtr< CPlayer >(mgr.ObjectById(hitId))) {
          if (x420_curDamageRemTime <= 0.f) {
            mgr.ApplyDamage(
                GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
                CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial), CMaterialList()),
                CVector3f::Zero());
            x420_curDamageRemTime = x424_damageWaitTime;
            mgr.PlayerState()->StaticInterference().AddSource(GetUniqueId(), 0.3f, 1.f);
            x628_ = 0.5f;
            CSfxManager::AddEmitter(x7cc_laserSfx, result.GetPoint(), CVector3f(0.f, 0.f, 0.f),
                                    true, false, CSfxManager::kMedPriority,
                                    GetCurrentAreaId().Value());
          }
        }
        if (hitId != GetUniqueId()) {
          if (CPatterned* patterned = TCastToPtr< CPatterned >(mgr.ObjectById(hitId))) {
            x834_31_attackOver = true;
            UpdateAnimation(GetModelData()->GetAnimationData()->GetAnimTimeRemaining(
                                rstl::string_l("Whole Body")),
                            mgr, true);
          }
        }
      }
    }
  }
}

static CRayCastResult FindBestCollision(const CStateManager& mgr, TUniqueId& hitId,
                                        const CVector3f& pos, const CVector3f& dir,
                                        const rstl::reserved_vector< TUniqueId, 1024 >& nearList,
                                        const CRayCastResult& initial) {
  CRayCastResult best = initial;
  float length = best.GetTime();
  for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
    if (const CPhysicsActor* const actor =
            TCastToConstPtr< CPhysicsActor >(mgr.GetObjectById(*it))) {
      const CRayCastResult result = actor->GetCollisionPrimitive()->CastRay(
          pos, dir, length, CMaterialFilter::skPassEverything, actor->GetPrimitiveTransform());
      if (!result.IsValid()) {
        continue;
      }
      if (result.GetTime() < length || !best.IsValid()) {
        length = result.GetTime();
        best = result;
        hitId = actor->GetUniqueId();
      }
    }
    if (const CScriptWater* const water = TCastToConstPtr< CScriptWater >(mgr.GetObjectById(*it))) {
      const CCollidableAABox bounds(water->GetTriggerBoundsWR(), water->GetMaterialList());
      const CRayCastResult result = bounds.CastRay(
          pos, dir, length, CMaterialFilter::skPassEverything, CTransform4f::Identity());
      if (!result.IsValid()) {
        continue;
      }
      if (result.GetTime() < length || !best.IsValid()) {
        length = result.GetTime();
        best = result;
        hitId = water->GetUniqueId();
      }
    }
    if (close_enough(length, 0.f)) {
      break;
    }
  }
  return best;
}

void CDrone::UpdateWaterRipples(CStateManager& mgr) {
  static const CMaterialFilter nearFilter =
      CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Trigger), CMaterialList(kMT_Character));
  static const CMaterialFilter rayFilter =
      CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Trigger), CMaterialList(kMT_Character));
  const CVector3f direction = -GetTransform().GetUp();
  CAABox bounds(GetBoundingBox());
  bounds.AccumulateBounds(GetBoundingBox().GetMinPoint() + -6.f * GetTransform().GetUp());
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  mgr.BuildNearList(nearList, bounds, nearFilter, nullptr);
  if (nearList.size() > 0) {
    TUniqueId hitId = kInvalidUniqueId;
    CRayCastResult result(mgr.RayStaticIntersection(GetTranslation(), direction, 7.f, rayFilter));
    result = FindBestCollision(mgr, hitId, GetTranslation(), direction, nearList, result);
    if (result.IsValid() && hitId != kInvalidUniqueId) {
      if (CScriptWater* water = TCastToPtr< CScriptWater >(mgr.ObjectById(hitId))) {
        const CVector3f point = result.GetPoint();
        water->FluidPlane().AddRipple(0.65f, GetUniqueId(), point, *water, mgr);
      }
    }
  }
}

void CDrone::SetLaserActive(CStateManager& mgr, int laserIdx, bool active) {
  if (active && x7d8_laserIds[laserIdx] == kInvalidUniqueId) {
    x7d8_laserIds[laserIdx] = mgr.AllocateUniqueId();
    mgr.AddObject(*rs_new CDroneLaser(x7d8_laserIds[laserIdx], GetCurrentAreaId(), GetTransform(),
                                      x568_laserParticlesId));
  }
  if (CEntity* laser = mgr.ObjectById(x7d8_laserIds[laserIdx])) {
    mgr.DeliverScriptMsg(laser, GetUniqueId(), active ? kSM_Activate : kSM_Deactivate);
  }
}

void CDrone::KnockBack(const CVector3f& dir, CStateManager& mgr, const CDamageInfo& info,
                       float magnitude, bool direct, const bool inDeferred) {
  if (IsAlive()) {
    CPatterned::KnockBack(dir, mgr, info, magnitude, direct, inDeferred);
    if (x460_knockBackController.GetActiveParms().x0_animState != kAR_Invalid) {
      x630_ = 0.5f;
      x634_ = 1.f;
    }
  }
}

bool CDrone::ShouldAttack(CStateManager& mgr, float) {
  if (x5d0_ <= 0.f) {
    if (CTeamAiMgr* team = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x688_teamMgr))) {
      if (team->HasTeamAiRole(GetUniqueId())) {
        return team->AddProjectileAttacker(GetUniqueId());
      }
    }
    return true;
  }
  return false;
}

bool CDrone::HearShot(CStateManager& mgr, float) {
  rstl::reserved_vector< TUniqueId, 1024 > nearList;
  BuildNearList(kMT_Projectile, kMT_Player, 10.f, nearList, mgr);
  for (AUTO(it, nearList.begin()); it != nearList.end(); ++it) {
    if (const CWeapon* weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(*it))) {
      if (weapon->GetType() != kWT_AI) {
        return true;
      }
    }
  }
  return false;
}

void CDrone::BuildNearList(EMaterialTypes includeMat, EMaterialTypes excludeMat, float radius,
                           rstl::reserved_vector< TUniqueId, 1024 >& list, CStateManager& mgr) {
  const CVector3f pos = GetTranslation();
  const CVector3f extent(radius, radius, radius);
  const CAABox bounds(pos - extent, pos + extent);
  const CMaterialFilter filter =
      CMaterialFilter::MakeIncludeExclude(CMaterialList(includeMat), CMaterialList(excludeMat));
  mgr.BuildNearList(list, bounds, filter, nullptr);
}

bool CDrone::InRange(CStateManager& mgr, float) {
  const float distance = (mgr.GetPlayer()->GetTranslation() - GetTranslation()).MagSquared();
  return distance > x2fc_minAttackRange * x2fc_minAttackRange &&
         distance < x300_maxAttackRange * x300_maxAttackRange;
}

bool CDrone::CodeTrigger(CStateManager&, float) { return x834_29_codeTrigger; }

bool CDrone::AttackOver(CStateManager&, float) { return x834_31_attackOver; }

void CDrone::Dodge(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x7c8_ = 0;
    x630_ = 0.5f;
    x634_ = 1.f;
    if (x3fc_flavor == kFT_One) {
      x834_30_visible = true;
    }
    break;
  case kStateMsg_Update:
    switch (x7c8_) {
    case 0:
      BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
      switch (x58c_prevDodgeDir) {
      case pas::kSD_Left:
        BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Right, pas::kStep_Dodge));
        x58c_prevDodgeDir = pas::kSD_Right;
        break;
      case pas::kSD_Right:
        BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Up, pas::kStep_Dodge));
        x58c_prevDodgeDir = pas::kSD_Up;
        break;
      case pas::kSD_Up:
        BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Down, pas::kStep_Dodge));
        x58c_prevDodgeDir = pas::kSD_Down;
        break;
      case pas::kSD_Down:
        BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Left, pas::kStep_Dodge));
        x58c_prevDodgeDir = pas::kSD_Left;
        break;
      }
      x7c8_ = 1;
      break;
    case 1:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Step) {
        x7c8_ = 2;
      }
      break;
    }
    BodyCtrl()->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() +
                                                 CVector3f(0.f, 0.f, 1.f) - GetTranslation());
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CDrone::Deactivate(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    DeathDelete(mgr);
    break;
  }
}

bool CDrone::LineOfSight(CStateManager& mgr, float) {
  const CVector3f pos = GetTranslation();
  const CMaterialFilter filter =
      CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid, kMT_Character),
                                          CMaterialList(kMT_Player, kMT_ProjectilePassthrough));
  const CVector3f target = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
  return mgr.RayCollideWorld(pos, target, filter, this);
}

bool CDrone::ShouldFire(CStateManager& mgr, float) {
  if (mgr.GetPlayer()->GetMorphballTransitionState() != CPlayer::kMS_Morphed && x624_ <= 0.f) {
    const CMaterialFilter filter =
        CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid, kMT_Character),
                                            CMaterialList(kMT_Player, kMT_ProjectilePassthrough));
    const CVector3f target = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    const CTransform4f right(GetLctrTransform(rstl::string_l(sRightGunLocator)));
    if (mgr.RayCollideWorld(right.GetTranslation(), target, filter, this)) {
      const CTransform4f left(GetLctrTransform(rstl::string_l(sLeftGunLocator)));
      if (mgr.RayCollideWorld(left.GetTranslation(), target, filter, this)) {
        return true;
      }
    }
  }
  return false;
}

bool CDrone::SpotPlayer(CStateManager& mgr, float) {
  if ((mgr.GetPlayer()->GetTranslation() - GetTranslation()).MagSquared() >
      x3bc_detectionRange * x3bc_detectionRange) {
    return false;
  }
  if (!LineOfSight(mgr, 0.f)) {
    return false;
  }
  const CVector3f forward =
      (GetTransform().GetForward() + GetTransform().GetRight() * x5cc_).AsNormalized();
  return CVector3f::Dot(
             forward,
             (mgr.GetPlayer()->GetAimPosition(mgr, 0.f) - GetTranslation()).AsNormalized()) > 0.5;
}

void CDrone::Dead(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x460_knockBackController.SetAutoResetImpulse(false);
    if (x834_24_waveHit) {
      SetMomentumWR(CVector3f(0.f, 0.f, -GetWeight()));
    } else {
      Stop();
      SetVelocityWR(CVector3f::Zero());
      SetMomentumWR(CVector3f::Zero());
    }
    x401_26_disableMove = true;
    x5c8_ = 0.f;
    SetVisorFlareEnabled(mgr, false);
    x7c8_ = 0;
    break;
  case kStateMsg_Update:
    switch (x7c8_) {
    case 0:
      if (x834_24_waveHit) {
        BodyCtrl()->CommandMgr().DeliverCmd(
            CBCHurledCmd(CVector3f::Zero(), CVector3f::Zero(), false));
        x7c8_ = 1;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCKnockDownCmd(CVector3f::Zero(), pas::kS_Zero));
        x7c8_ = 1;
        Stop();
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CDrone::Death(CStateManager& mgr, const CVector3f& direction, EScriptObjectState state) {
  if (IsAlive()) {
    x824_activeLasers[0] = false;
    x824_activeLasers[1] = false;
    SetLaserActive(mgr, 0, false);
    SetLaserActive(mgr, 1, false);
    SetVisorFlareEnabled(mgr, false);
    if (x3e4_lastHP - HealthInfo(mgr)->GetHP() >= x3d8_xDamageThreshold && !x834_24_waveHit) {
      x400_28_pendingMassiveDeath = true;
      if (x3e0_xDamageDelay <= 0.f) {
        const CVector3f pos = GetTranslation();
        const CVector3f target = pos - direction;
        const CTransform4f xf(CTransform4f::LookAt(pos, target) *
                              CTransform4f::RotateX(CRelAngle::FromDegrees(45.f)));
        SetTransform(xf);
      }
    } else {
      StateMachineState().SetState(mgr, *this, GetStateMachine(), rstl::string_l("Dead"));
    }
    if (BodyCtrl()->GetPercentageFrozen() > 0.f) {
      BodyCtrl()->UnFreeze();
    }
    x400_25_alive = false;
    SendScriptMsgs(state, mgr, kSM_None);
  }
}

void CDrone::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CPatterned::PreRender(mgr, frustum);
  if (x3fc_flavor == kFT_One) {
    if (HasModelData() && HasAnimation()) {
      const bool visible = GetModelAlphau8(mgr);
      if (!visible) {
        AnimationData()->BuildPose();
      }
    }
  }
}

void CDrone::Render(const CStateManager& mgr) const {
  const TAreaId area = GetCurrentAreaId();
  const bool isOne = x3fc_flavor == kFT_One;
  const bool visible = GetModelAlphau8(mgr) != 0;
  if (!isOne || visible) {
    const bool xray = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay;
    if (isOne && xray) {
      CElementGen::SetSubtractBlend(true);
      CElementGen::SetMoveRedToAlphaBuffer(true);
      CGraphics::SetFog(kRFM_PerspLin, 0.f, 75.f, CColor::Black());
      GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirst();
      mgr.SetupFogForArea3XRange(area);
    }
    CPatterned::Render(mgr);
    if (isOne && xray) {
      CGraphics::SetFog(kRFM_PerspLin, 0.f, 75.f, CColor::Black());
      GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnLast();
      mgr.SetupFogForArea(area);
      CElementGen::SetSubtractBlend(false);
      CElementGen::SetMoveRedToAlphaBuffer(false);
    }
    if (isOne && !close_enough(x5dc_, 0.f)) {
      const CColor base = CColor(1.f, 1.f, 1.f, x5dc_);
      const CColor color = CColor::Lerp(base, CColor(1.f, 0.f, 0.f, 1.f), x5e8_shieldTime);
      const CTransform4f xf(GetLctrTransform(rstl::string_l("Shield_LCTR")));
      const CModelFlags flags =
          CModelFlags(CModelFlags::kT_Additive2, color).DepthCompareUpdate(true, true);
      x82c_shieldModel->Render(mgr, xf, GetActorLights(), flags);
    }
  }
}

void CDrone::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  CPatterned::AddToRenderer(frustum, mgr);
}

void CDrone::TargetPlayer(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x3b8_turnSpeed = x5ec_turnSpeed;
    BodyCtrl()->SetTurnSpeed(x5ec_turnSpeed);
    if (BodyCtrl()->GetLocomotionType() != pas::kLT_Combat) {
      BodyCtrl()->SetLocomotionType(pas::kLT_Combat);
    }
    BodyCtrl()->BodyStateInfo().SetMaximumPitch(CMath::Deg2Rad(60.f));
    SetDestPos(mgr.GetPlayer()->GetAimPosition(mgr, 0.f));
    x400_24_hitByPlayerProjectile = false;
    if (x3fc_flavor == kFT_One) {
      x834_30_visible = true;
    }
    StateMachineState().SetDelay(rstl::max_val(0.3f, x624_));
    break;
  case kStateMsg_Update: {
    const CVector3f target =
        (mgr.GetPlayer()->GetAimPosition(mgr, 0.f) - GetTranslation()).AsNormalized();
    BodyCtrl()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(FLT_EPSILON * GetTransform().GetForward(), target, 1.f));
    BodyCtrl()->CommandMgr().DeliverTargetVector(target);
    AvoidOtherAIs(mgr);
    if (x630_ <= 0.f) {
      x634_ = 0.f;
    }
    break;
  }
  case kStateMsg_Deactivate:
    SetDestPos(mgr.GetPlayer()->GetTranslation() + CVector3f(0.f, 0.f, x664_));
    break;
  }
}

void CDrone::Retreat(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x7c8_ = 0;
    if (x3fc_flavor == kFT_One) {
      x834_30_visible = true;
    }
    StateMachineState().SetDelay(x65c_);
    break;
  case kStateMsg_Update:
    switch (x7c8_) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Step) {
        x7c8_ = 1;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Backward, pas::kStep_Normal));
      }
      break;
    case 1:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Step) {
        x7c8_ = 2;
      }
      break;
    case 2:
      x7c8_ = 0;
      break;
    }
    BodyCtrl()->CommandMgr().DeliverTargetVector(
        (mgr.GetPlayer()->GetTranslation() - GetTranslation()).AsNormalized());
    if (x630_ <= 0.f) {
      x634_ = 0.333333f;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CDrone::SpecialAttack(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    if (x3fc_flavor == kFT_One) {
      x834_30_visible = true;
    }
    StateMachineState().SetDelay(x660_);
    BodyCtrl()->SetLocomotionType(pas::kLT_Internal10);
    break;
  case kStateMsg_Update: {
    BodyCtrl()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(GetTransform().GetForward(), CVector3f::Zero(), 1.f));
    const CVector3f center =
        0.5f * (mgr.GetPlayer()->GetAimPosition(mgr, 0.f) + mgr.GetPlayer()->GetTranslation());
    const CVector3f target = center - GetTranslation();
    if (((x668_ < x664_ && target.GetZ() > 0.f) || x668_ > x664_) && target.CanBeNormalized()) {
      const CVector3f move = dt * (target.AsNormalized() * x5e4_);
      const CVector3f impulse = GetMoveToORImpulseWR(GetTransform().TransposeRotate(move), dt);
      ApplyImpulseWR(impulse, CAxisAngle::Identity());
    }
    break;
  }
  case kStateMsg_Deactivate:
    break;
  }
}

void CDrone::Flee(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    x7c8_ = 0;
    x832_24_ = 0;
    const CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid));
    if (mgr.RayStaticIntersection(GetTranslation(), -GetTransform().GetForward(), 4.f, filter)
            .IsValid()) {
      if (mgr.Random()->Float() < 0.5f) {
        x832_24_ = 1;
      } else {
        x832_24_ = 2;
      }
    }
    break;
  }
  case kStateMsg_Update:
    switch (x7c8_) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Step) {
        x7c8_ = 1;
      } else {
        CBodyStateCmdMgr& commands = BodyCtrl()->CommandMgr();
        switch (x832_24_) {
        case 0:
          commands.DeliverCmd(CBCStepCmd(pas::kSD_Backward, pas::kStep_BreakDodge));
          break;
        case 1:
          commands.DeliverCmd(CBCStepCmd(pas::kSD_Left, pas::kStep_Normal));
          break;
        case 2:
          commands.DeliverCmd(CBCStepCmd(pas::kSD_Right, pas::kStep_Normal));
          break;
        }
      }
      break;
    case 1:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Step) {
        x7c8_ = 2;
      }
      break;
    case 2:
      break;
    }
    BodyCtrl()->CommandMgr().DeliverTargetVector(
        (mgr.GetPlayer()->GetTranslation() - GetTranslation()).AsNormalized());
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CDrone::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    x7c8_ = 0;
    break;
  case kStateMsg_Update:
    switch (x7c8_) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_Taunt) {
        x7c8_ = 1;
      } else {
        BodyCtrl()->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_One));
      }
      break;
    case 1:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Taunt) {
        x7c8_ = 2;
      }
      break;
    case 2:
      break;
    }
    break;
  case kStateMsg_Deactivate:
    SendScriptMsgs(kSS_Zero, mgr, kSM_None);
    break;
  }
}

bool CDrone::AnimOver(CStateManager&, float) { return x7c8_ == 2; }

void CDrone::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    x7c8_ = 0;
    BodyCtrl()->BodyStateInfo().SetMaximumPitch(CMath::Deg2Rad(60.f));
    CTeamAiMgr* team = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x688_teamMgr));
    if (team && team->HasTeamAiRole(GetUniqueId())) {
      const bool added = team->AddProjectileAttacker(GetUniqueId());
      int state = 2;
      if (added) {
        state = 0;
      }
      x7c8_ = state;
    } else {
      x7c8_ = 0;
    }
    ++x830_;
    x834_27_ = true;
    break;
  }
  case kStateMsg_Update:
    switch (x7c8_) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_ProjectileAttack) {
        x7c8_ = 1;
      } else {
        CBodyStateCmdMgr& commands = BodyCtrl()->CommandMgr();
        commands.DeliverCmd(CBCProjectileAttackCmd(x834_27_ ? pas::kS_One : pas::kS_Zero,
                                                   mgr.GetPlayer()->GetTranslation(), false));
      }
      break;
    case 1: {
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_ProjectileAttack) {
        x7c8_ = 2;
      }
      const CVector3f target = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
      const float height = target.GetZ() - GetTranslation().GetZ();
      if ((x668_ < x664_ && height > 0.f) || x668_ > x664_) {
        if (height > x620_ || height < -x620_) {
          float scale = CMath::AbsF(height) > 4.f ? 1.f : 0.25f * CMath::AbsF(height);
          const float speed = x5e4_;
          const CVector3f move = 1.f * (speed * CVector3f(0.f, 0.f, height < 0.f ? -scale : scale));
          const CVector3f impulse = GetMoveToORImpulseWR(GetTransform().TransposeRotate(move), 1.f);
          ApplyImpulseWR(impulse, CAxisAngle::Identity());
        }
        if (x630_ <= 0.f) {
          if (height > 0.1f || height < -0.1f) {
            x634_ = 0.f;
          } else {
            x634_ = 0.333333f;
          }
        }
      } else {
        x634_ = 0.f;
      }
      break;
    }
    }
    BodyCtrl()->CommandMgr().DeliverTargetVector(
        (mgr.GetPlayer()->GetTranslation() - GetTranslation()).AsNormalized());
    break;
  case kStateMsg_Deactivate:
    BodyCtrl()->BodyStateInfo().SetMaximumPitch(0.f);
    if (CTeamAiMgr* team = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x688_teamMgr))) {
      if (team->IsProjectileAttacker(GetUniqueId())) {
        team->RemoveProjectileAttacker(GetUniqueId());
      }
    }
    x624_ = x5fc_;
    break;
  }
}

void CDrone::Active(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    StateMachineState().SetDelay(x5f0_);
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    x5d0_ = x5f8_;
    break;
  }
}

// TODO: Match the forward-vector temporary in the target-position expression.
void CDrone::Attack(CStateManager& mgr, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate: {
    x7c8_ = 0;
    x834_31_attackOver = false;
    const CVector3f aim = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    const CTransform4f& xf = GetTransform();
    const CVector3f direction = (aim - GetTranslation()).AsNormalized();
    const CVector3f& forward = xf.GetForward();
    const CVector3f target =
        CVector3f::Dot(direction, forward) < 0.8f ? GetTranslation() + 10.f * xf.GetForward() : aim;
    switch (mgr.Random()->Next() % 4) {
    case 0: {
      {
        const CVector3f right(3.f * xf.Get00(), 3.f * xf.Get10(), 3.f * xf.Get20());
        const CVector3f up(4.f * xf.Get02(), 4.f * xf.Get12(), 4.f * xf.Get22());
        x7e0_lasersStart[0] = target + right - up;
      }
      {
        const CVector3f right(3.f * xf.Get00(), 3.f * xf.Get10(), 3.f * xf.Get20());
        const CVector3f up(4.f * xf.Get02(), 4.f * xf.Get12(), 4.f * xf.Get22());
        x7fc_lasersEnd[0] = target - right + up;
      }
      {
        const CVector3f right(3.f * xf.Get00(), 3.f * xf.Get10(), 3.f * xf.Get20());
        const CVector3f up(4.f * xf.Get02(), 4.f * xf.Get12(), 4.f * xf.Get22());
        x7e0_lasersStart[1] = target - right - up;
      }
      {
        const CVector3f right(3.f * xf.Get00(), 3.f * xf.Get10(), 3.f * xf.Get20());
        const CVector3f up(4.f * xf.Get02(), 4.f * xf.Get12(), 4.f * xf.Get22());
        x7fc_lasersEnd[1] = target + right + up;
      }
      break;
    }
    case 1: {
      {
        const CVector3f right(3.f * xf.Get00(), 3.f * xf.Get10(), 3.f * xf.Get20());
        const CVector3f up(4.f * xf.Get02(), 4.f * xf.Get12(), 4.f * xf.Get22());
        x7e0_lasersStart[0] = target + right + up;
      }
      {
        const CVector3f right(3.f * xf.Get00(), 3.f * xf.Get10(), 3.f * xf.Get20());
        const CVector3f up(4.f * xf.Get02(), 4.f * xf.Get12(), 4.f * xf.Get22());
        x7fc_lasersEnd[0] = target - right - up;
      }
      {
        const CVector3f right(3.f * xf.Get00(), 3.f * xf.Get10(), 3.f * xf.Get20());
        const CVector3f up(4.f * xf.Get02(), 4.f * xf.Get12(), 4.f * xf.Get22());
        x7e0_lasersStart[1] = target - right + up;
      }
      {
        const CVector3f right(3.f * xf.Get00(), 3.f * xf.Get10(), 3.f * xf.Get20());
        const CVector3f up(4.f * xf.Get02(), 4.f * xf.Get12(), 4.f * xf.Get22());
        x7fc_lasersEnd[1] = target + right - up;
      }
      break;
    }
    case 2: {
      {
        const CVector3f right(4.f * xf.Get00(), 4.f * xf.Get10(), 4.f * xf.Get20());
        const CVector3f up(3.f * xf.Get02(), 3.f * xf.Get12(), 3.f * xf.Get22());
        x7e0_lasersStart[0] = target - right - up;
      }
      {
        const CVector3f right(4.f * xf.Get00(), 4.f * xf.Get10(), 4.f * xf.Get20());
        const CVector3f up(3.f * xf.Get02(), 3.f * xf.Get12(), 3.f * xf.Get22());
        x7fc_lasersEnd[0] = target + right + up;
      }
      {
        const CVector3f right(4.f * xf.Get00(), 4.f * xf.Get10(), 4.f * xf.Get20());
        const CVector3f up(3.f * xf.Get02(), 3.f * xf.Get12(), 3.f * xf.Get22());
        x7e0_lasersStart[1] = target + right - up;
      }
      {
        const CVector3f right(4.f * xf.Get00(), 4.f * xf.Get10(), 4.f * xf.Get20());
        const CVector3f up(3.f * xf.Get02(), 3.f * xf.Get12(), 3.f * xf.Get22());
        x7fc_lasersEnd[1] = target - right + up;
      }
      break;
    }
    case 3: {
      {
        const CVector3f right(4.f * xf.Get00(), 4.f * xf.Get10(), 4.f * xf.Get20());
        const CVector3f up(3.f * xf.Get02(), 3.f * xf.Get12(), 3.f * xf.Get22());
        x7e0_lasersStart[0] = target - right + up;
      }
      {
        const CVector3f right(4.f * xf.Get00(), 4.f * xf.Get10(), 4.f * xf.Get20());
        const CVector3f up(3.f * xf.Get02(), 3.f * xf.Get12(), 3.f * xf.Get22());
        x7fc_lasersEnd[0] = target + right - up;
      }
      {
        const CVector3f right(4.f * xf.Get00(), 4.f * xf.Get10(), 4.f * xf.Get20());
        const CVector3f up(3.f * xf.Get02(), 3.f * xf.Get12(), 3.f * xf.Get22());
        x7e0_lasersStart[1] = target + right + up;
      }
      {
        const CVector3f right(4.f * xf.Get00(), 4.f * xf.Get10(), 4.f * xf.Get20());
        const CVector3f up(3.f * xf.Get02(), 3.f * xf.Get12(), 3.f * xf.Get22());
        x7fc_lasersEnd[1] = target - right - up;
      }
      break;
    }
    }
    x818_lasersTime[0] = 0.f;
    x818_lasersTime[1] = 0.f;
    x835_24_ = true;
    break;
  }
  case kStateMsg_Update:
    switch (x7c8_) {
    case 0:
      if (BodyCtrl()->GetCurrentStateId() == pas::kAS_ProjectileAttack) {
        x7c8_ = 1;
      } else {
        CBodyStateCmdMgr& commands = BodyCtrl()->CommandMgr();
        commands.DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_Two, mgr.GetPlayer()->GetTranslation(), false));
      }
      break;
    case 1:
      if (BodyCtrl()->GetCurrentStateId() != pas::kAS_ProjectileAttack) {
        x7c8_ = 2;
      }
      break;
    }
    if (x630_ <= 0.f) {
      x634_ = 0.f;
    }
    break;
  case kStateMsg_Deactivate:
    x824_activeLasers[0] = false;
    x824_activeLasers[1] = false;
    SetLaserActive(mgr, 0, false);
    SetLaserActive(mgr, 1, false);
    SetVisorFlareEnabled(mgr, false);
    x5d0_ = x5f4_;
    x835_24_ = false;
    break;
  }
}

void CDrone::PathFindEx(CStateManager& mgr, EStateMsg msg, float dt) {
  CPatterned::PathFind(mgr, msg, dt);
  switch (msg) {
  case kStateMsg_Activate: {
    CVector3f target = mgr.GetPlayer()->GetTranslation() + CVector3f(0.f, 0.f, x664_);
    if (GetSearchPath()->Search(GetTranslation(), target) != CPathFindSearch::kR_Success &&
        (GetSearchPath()->GetResult() == CPathFindSearch::kR_NoDestPoint ||
         GetSearchPath()->GetResult() == CPathFindSearch::kR_NoPath)) {
      if (GetSearchPath()->FindClosestReachablePoint(GetTranslation(), target) ==
          CPathFindSearch::kR_Success) {
        GetSearchPath()->Search(GetTranslation(), target);
        SetDestPos(target);
      }
    }
    break;
  }
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CDrone::PathFind(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate: {
    CVector3f target = x2e0_destPos;
    if (GetSearchPath()->Search(GetTranslation(), target) != CPathFindSearch::kR_Success &&
        (GetSearchPath()->GetResult() == CPathFindSearch::kR_NoDestPoint ||
         GetSearchPath()->GetResult() == CPathFindSearch::kR_NoPath)) {
      if (GetSearchPath()->FindClosestReachablePoint(GetTranslation(), target) ==
          CPathFindSearch::kR_Success) {
        GetSearchPath()->Search(GetTranslation(), target);
        SetDestPos(target);
      }
    }
    if (x3fc_flavor == kFT_One) {
      x834_30_visible = true;
    }
    break;
  }
  case kStateMsg_Update: {
    CPatterned::PathFind(mgr, msg, dt);
    CBodyStateCmdMgr& commands = BodyCtrl()->CommandMgr();
    commands.BlendSteeringCmds();
    CVector3f move = commands.GetMoveVector();
    if (move.CanBeNormalized()) {
      move.Normalize();
      commands.ClearLocomotionCmds();
      ApplyImpulseWR(GetMass() * (x5e4_ * move), CAxisAngle::Identity());
      const CVector3f target =
          (mgr.GetPlayer()->GetAimPosition(mgr, 0.f) - GetTranslation()).AsNormalized();
      commands.DeliverCmd(CBCLocomotionCmd(FLT_EPSILON * GetTransform().GetForward(), target, 1.f));
      commands.DeliverTargetVector(target);
      AvoidOtherAIs(mgr);
      if (x630_ <= 0.f) {
        x634_ = 0.333333f;
      }
    } else if (x630_ <= 0.f) {
      x634_ = 0.f;
    }
    break;
  }
  case kStateMsg_Deactivate:
    CPatterned::PathFind(mgr, msg, dt);
    break;
  }
}

void CDrone::TargetCover(CStateManager&, EStateMsg msg, float) {
  switch (msg) {
  case kStateMsg_Activate:
    break;
  case kStateMsg_Update: {
    const CVector3f move = 1.f * (x5e4_ * CVector3f(0.f, 0.f, 1.f));
    const CVector3f velocity = GetMoveToORImpulseWR(GetTransform().TransposeRotate(move), 1.f);
    ApplyImpulseWR(velocity, CAxisAngle::Identity());
    break;
  }
  case kStateMsg_Deactivate:
    break;
  }
}

bool CDrone::Leash(CStateManager& mgr, float) {
  return (mgr.GetPlayer()->GetTranslation() - GetTranslation()).MagSquared() <
         x3c8_leashRadius * x3c8_leashRadius;
}

bool CDrone::ShouldMove(CStateManager&, float) { return x644_ <= 0.f; }

bool CDrone::CoverCheck(CStateManager&, float) {
  if (close_enough(x67c_, CVector3f::Zero())) {
    return true;
  }
  const CVector3f diff = x670_ - GetTranslation();
  return CVector3f::Dot(x67c_, diff) < 0.f || diff.MagSquared() < 0.25f;
}

void CDrone::Cover(CStateManager& mgr, EStateMsg msg, float dt) {
  switch (msg) {
  case kStateMsg_Activate:
    x67c_ = CVector3f::Zero();
    x670_ = GetTranslation();
    for (int i = 0; i < 4; ++i) {
      const float distance = (x64c_ - x648_) * mgr.Random()->Float() + x648_;
      const int random = mgr.Random()->Next();
      CRelAngle angle(0.f);
      if ((random >> 3) & 1) {
        const float minAngle = 90.f - x650_;
        const float maxAngle = 90.f + x654_;
        angle = CRelAngle::FromDegrees((maxAngle - minAngle) * mgr.Random()->Float() + minAngle);
      } else {
        const float minAngle = 270.f - x654_;
        const float maxAngle = 270.f + x650_;
        angle = CRelAngle::FromDegrees((maxAngle - minAngle) * mgr.Random()->Float() + minAngle);
      }
      const CVector3f& direction =
          (mgr.GetPlayer()->GetAimPosition(mgr, 0.f) - GetTranslation()).AsNormalized();
      const CVector3f end =
          GetTranslation() + CQuaternion::ZRotation(angle).Transform(distance * direction);
      if (mgr.RayCollideWorld(GetTranslation(), end,
                              CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)), this)) {
        x670_ = end;
        x67c_ = end - GetTranslation();
        if (x67c_.CanBeNormalized()) {
          x67c_.Normalize();
        }
        break;
      }
    }
    break;
  case kStateMsg_Update: {
    const CVector3f move = dt * (x658_ * x67c_);
    const CVector3f impulse = GetMoveToORImpulseWR(GetTransform().TransposeRotate(move), dt);
    ApplyImpulseWR(impulse, CAxisAngle::Identity());
    const CVector3f target =
        (mgr.GetPlayer()->GetAimPosition(mgr, 0.f) - GetTranslation()).AsNormalized();
    BodyCtrl()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(FLT_EPSILON * GetTransform().GetForward(), target, 1.f));
    break;
  }
  case kStateMsg_Deactivate:
    x644_ = (x640_ - x63c_) * mgr.Random()->Float() + x63c_;
    break;
  }
}

bool CDrone::CanRenderUnsorted(const CStateManager& mgr) const {
  if (!close_enough(x5dc_, 0.f)) {
    return false;
  }
  return CPatterned::CanRenderUnsorted(mgr);
}

void CDrone::Burn(float, float) {}

void CDrone::AvoidOtherAIs(CStateManager& mgr) {
  if (BodyCtrl()->GetCurrentStateId() != pas::kAS_Step) {
    rstl::reserved_vector< TUniqueId, 1024 > nearList;
    BuildNearList(kMT_Character, kMT_Player, x61c_, nearList, mgr);
    if (nearList.size() > 0) {
      CVector3f nearestPos = CVector3f::Zero();
      float minDistance = FLT_MAX;
      for (int i = 0; i < nearList.size(); ++i) {
        if (const CActor* actor = static_cast< const CActor* >(mgr.GetObjectById(nearList[i]))) {
          const float distance = (actor->GetTranslation() - GetTranslation()).MagSquared();
          if (nearList[i] != GetUniqueId() && distance < minDistance) {
            nearestPos = actor->GetTranslation();
            minDistance = distance;
          }
        }
      }
      if (!(nearestPos == CVector3f::Zero()) && minDistance < x61c_ * x61c_) {
        const float side = CVector3f::Dot(GetTransform().GetRight(), nearestPos - GetTranslation());
        if (side < -0.2f) {
          BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Right, pas::kStep_Normal));
        } else if (side > 0.2f) {
          BodyCtrl()->CommandMgr().DeliverCmd(CBCStepCmd(pas::kSD_Left, pas::kStep_Normal));
        }
      }
    }
  }
}

void CDrone::SquadAdd(CStateManager& mgr) {
  if (x688_teamMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* team = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x688_teamMgr))) {
      team->AssignTeamAiRole(*this, CTeamAiRole::kTAR_Projectile, CTeamAiRole::kTAR_Melee,
                             CTeamAiRole::kTAR_Invalid);
    }
  }
}

void CDrone::SquadRemove(CStateManager& mgr) {
  if (x688_teamMgr != kInvalidUniqueId) {
    if (CTeamAiMgr* team = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(x688_teamMgr))) {
      if (team->IsPartOfTeam(GetUniqueId())) {
        team->RemoveTeamAiRole(GetUniqueId());
      }
    }
  }
}

void CDrone::UpdateThermal(CStateManager& mgr, float dt) {
  switch (x832_27_) {
  case 0:
    if (x628_ > 0.f) {
      x832_27_ = 2;
    }
    break;
  case 1:
    if (x628_ <= 0.f) {
      x832_27_ = 3;
    }
    break;
  case 2:
    x62c_ += dt / 0.25f;
    if (x62c_ >= 1.f) {
      x62c_ = 1.f;
      x832_27_ = 1;
    }
    break;
  case 3:
    x62c_ -= dt / 2.f;
    if (x62c_ <= 0.f) {
      x62c_ = 0.f;
      x832_27_ = 0;
    }
    break;
  }
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Thermal && x62c_ > 0.f) {
    mgr.AddThermalColdScale2(x62c_);
    mgr.Player()->TryToBreakOrbit(mgr.GetPlayer()->GetOrbitTargetId(),
                                  CPlayer::kOB_ActivateOrbitSource, mgr);
  }
  if (x628_ > 0.f) {
    x628_ -= dt;
  }
}

void CDrone::UpdatePitchBend(float dt) {
  if (x630_ > 0.f) {
    x630_ -= dt;
  }
  const CSfxHandle sfx = GetSfxHandle();
  const float step = dt / 0.15f;
  x638_ += CMath::Clamp(-step, x634_ - x638_, step);
  if (sfx) {
    const uint pitch = 8192.f + 8191.f * x638_;
    if (x835_25_) {
      CSfxManager::PitchBend(sfx, pitch - 8192);
    } else {
      CSfxManager::PitchBend(sfx, pitch);
    }
  }
}

// TODO: Match the out-of-line destruction of x57c_flares.
CDrone::~CDrone() {}
