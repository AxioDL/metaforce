#include "Runtime/MP1/World/CMetroidPrimeExo.hpp"

#include "DataSpec/DNAMP1/ScriptObjects/MetroidPrimeStage1.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/MP1/World/CMetroidPrimeRelay.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CParticleElectric.hpp"
#include "Runtime/Particle/CParticleSwoosh.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CProjectedShadow.hpp"
#include "Runtime/World/CWorld.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/ScriptLoader.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {

namespace {
std::array<pas::ELocomotionType, 3> skLocomotions{{
    pas::ELocomotionType::Internal10,
    pas::ELocomotionType::Internal11,
    pas::ELocomotionType::Internal12,
}};
std::array<pas::ETauntType, 3> skTaunts{{
    pas::ETauntType::One,
    pas::ETauntType::Two,
    pas::ETauntType::Zero,
}};

std::array<SSphereJointInfo, 3> skSphereJoints{{
    {"Sphere_LCTR", 1.5f},
    {"Skeleton_Root", 2.3f},
    {"Head_LockON_SDK", 0.92f},
}};

std::array<SOBBJointInfo, 23> skBodyJoints{{
    {"R_shoulder", "R_elbow", {0.6, 0.6, 0.6}},          {"R_elbow", "R_wrist", {0.3f, 0.3f, 0.3f}},
    {"R_wrist", "R_hand_LCTR", {0.3f, 0.3f, 0.3f}},      {"R_hand_LCTR", "R_leg_LCTR", {0.4f, 1.2f, 0.4f}},
    {"R_front_1", "R_front_2", {0.2f, 0.2f, 0.2f}},      {"R_front_2", "R_front_3", {0.2f, 0.2f, 0.2f}},
    {"R_front_3", "F_R_leg_LCTR", {0.2f, 0.2f, 0.7f}},   {"R_stinger_1", "R_stinger_2", {0.2f, 0.2f, 0.2f}},
    {"R_stinger_2", "R_spike_LCTR", {0.2f, 0.2f, 0.2f}}, {"L_shoulder", "L_elbow", {0.6, 0.6, 0.6}},
    {"L_elbow", "L_wrist", {0.3f, 0.3f, 0.3f}},          {"L_wrist", "L_hand_LCTR", {0.3f, 0.3f, 0.3f}},
    {"L_hand_LCTR", "L_leg_LCTR", {0.4f, 1.2f, 0.4f}},   {"L_front_1", "L_front_2", {0.2f, 0.2f, 0.2f}},
    {"L_front_2", "L_front_3", {0.2f, 0.2f, 0.2f}},      {"L_front_3", "F_L_leg_LCTR", {0.2f, 0.2f, 0.7f}},
    {"L_stinger_1", "L_stinger_2", {0.4f, 0.4f, 0.4f}},  {"L_stinger_2", "L_spike_LCTR", {0.2f, 0.2f, 0.2f}},
    {"B_shoulder", "B_elbow", {0.8f, 0.8f, 0.8f}},       {"B_elbow", "B_wrist", {0.7f, 0.7f, 0.7f}},
    {"B_wrist", "B_leg_LCTR", {0.6f, 0.1f, 0.6f}},       {"Head_LCTR", "Horn_LCTR", {0.8f, 0.1f, 0.6f}},
    {"Jaw_1", "C_bottomtooth", {2.f, 0.2f, 0.5f}},
}};

std::array<float, 4> skHealthConstants{{
    2420.f,
    1760.f,
    880.f,
    0.f,
}};
} // namespace
SPrimeStruct2B::SPrimeStruct2B(CInputStream& in)
: x0_propertyCount(in.readUint32Big())
, x4_particle1(in.readUint32Big())
, x8_particle2(in.readUint32Big())
, xc_particle3(in.readUint32Big())
, x10_dInfo(in)
, x2c_(in.readFloatBig())
, x30_(in.readFloatBig())
, x34_texture(in.readUint32Big())
, x38_(CSfxManager::TranslateSFXID(u16(in.readUint32Big())))
, x3a_(CSfxManager::TranslateSFXID(u16(in.readUint32Big()))) {}

SPrimeStruct5::SPrimeStruct5(CInputStream& in)
: x0_propertyCount(in.readUint32Big())
, x4_(in.readUint32Big())
, x8_(in.readUint32Big())
, xc_(in.readUint32Big())
, x10_(in.readUint32Big())
, x14_(in.readUint32Big())
, x18_(in.readUint32Big())
, x1c_(in.readUint32Big())
, x20_(in.readUint32Big()) {}

SPrimeStruct4::SPrimeStruct4(CInputStream& in)
: x0_beamInfo(in), x44_(in.readUint32Big()), x48_dInfo1(in), x64_struct5(in), x88_(in.readFloatBig()), x8c_dInfo2(in) {}

SPrimeStruct6::SPrimeStruct6(CInputStream& in)
: x0_propertyCount(in.readUint32Big())
, x4_damageVulnerability(in)
, x6c_color(zeus::CColor::ReadRGBABig(in))
, x70_(in.readUint32Big())
, x74_(in.readUint32Big()) {}

static CPatternedInfo LoadPatternedInfo(CInputStream& in) {
  std::pair<bool, u32> pcount = CPatternedInfo::HasCorrectParameterCount(in);
  return CPatternedInfo(in, pcount.second);
}

using CameraShakeData = DataSpec::DNAMP1::MetroidPrimeStage1::MassivePrimeStruct::CameraShakeData;

static SCameraShakePoint BuildCameraShakePoint(CameraShakeData::CameraShakerComponent::CameraShakePoint& sp) {
  return SCameraShakePoint(false, sp.attackTime, sp.sustainTime, sp.duration, sp.magnitude);
}

static CCameraShakerComponent BuildCameraShakerComponent(CameraShakeData::CameraShakerComponent& comp) {
  return CCameraShakerComponent(comp.useModulation, BuildCameraShakePoint(comp.am), BuildCameraShakePoint(comp.fm));
}

static CCameraShakeData LoadCameraShakeData(CInputStream& in) {
  CameraShakeData shakeData;
  shakeData.read(in);
  return CCameraShakeData(shakeData.duration, shakeData.sfxDist, u32(shakeData.useSfx), zeus::skZero3f,
                          BuildCameraShakerComponent(shakeData.shakerComponents[0]),
                          BuildCameraShakerComponent(shakeData.shakerComponents[1]),
                          BuildCameraShakerComponent(shakeData.shakerComponents[2]));
}

static rstl::reserved_vector<SPrimeStruct4, 4> LoadPrimeStruct4s(CInputStream& in) {
  rstl::reserved_vector<SPrimeStruct4, 4> ret;
  for (int i = 0; i < 4; ++i) {
    ret.emplace_back(in);
  }
  return ret;
}

static rstl::reserved_vector<SPrimeStruct6, 4> LoadPrimeStruct6s(CInputStream& in) {
  rstl::reserved_vector<SPrimeStruct6, 4> ret;
  for (int i = 0; i < 4; ++i)
    ret.emplace_back(in);
  return ret;
}

SPrimeExoParameters::SPrimeExoParameters(CInputStream& in)
: x0_propertyCount(in.readUint32Big())
, x4_patternedInfo(LoadPatternedInfo(in))
, x13c_actorParms(ScriptLoader::LoadActorParameters(in))
, x1a4_(in.readUint32Big())
, x1a8_(LoadCameraShakeData(in))
, x27c_(LoadCameraShakeData(in))
, x350_(LoadCameraShakeData(in))
, x424_(in)
, x460_particle1(in.readUint32Big())
, x464_(LoadPrimeStruct4s(in))
, x708_wpsc1(in.readUint32Big())
, x70c_dInfo1(in)
, x728_shakeData1(LoadCameraShakeData(in))
, x7fc_wpsc2(in.readUint32Big())
, x800_dInfo2(in)
, x81c_shakeData2(LoadCameraShakeData(in))
, x8f0_(in)
, x92c_(in)
, x948_(LoadCameraShakeData(in))
, xa1c_particle2(in.readUint32Big())
, xa20_swoosh(in.readUint32Big())
, xa24_particle3(in.readUint32Big())
, xa28_particle4(in.readUint32Big())
, xa2c_(LoadPrimeStruct6s(in)) {}

SPrimeExoRoomParameters::SPrimeExoRoomParameters(CInputStream& in) {
  u32 propCount = std::min(u32(14), in.readUint32Big());
  for (u32 i = 0; i < propCount; ++i) {
    x0_.push_back(in.readFloatBig());
  }
}

CMetroidPrimeExo::CMetroidPrimeExo(
    TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf, CModelData&& mData,
    const CPatternedInfo& pInfo, const CActorParameters& aParms, u32 pw1, const CCameraShakeData& shakeData1,
    const CCameraShakeData& shakeData2, const CCameraShakeData& shakeData3, const SPrimeStruct2B& struct2b,
    CAssetId particle1, const rstl::reserved_vector<SPrimeStruct4, 4>& struct4s, CAssetId wpsc1,
    const CDamageInfo& dInfo1, const CCameraShakeData& shakeData4, CAssetId wpsc2, const CDamageInfo& dInfo2,
    const CCameraShakeData& shakeData5, const SPrimeProjectileInfo& projectileInfo, const CDamageInfo& dInfo3,
    const CCameraShakeData& shakeData6, CAssetId particle2, CAssetId swoosh, CAssetId particle3, CAssetId particle4,
    const rstl::reserved_vector<SPrimeStruct6, 4>& struct6s)
: CPatterned(ECharacter::MetroidPrimeExo, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Flyer, EColliderType::One, EBodyType::Flyer, aParms, EKnockBackVariant::Large)
, x588_(struct6s)
, x8e8_headUpAdditiveBodyAnimIndex(
      GetModelData()->GetAnimationData()->GetCharacterInfo().GetAnimationIndex("B_headup_additive_body"sv))
, x918_(pw1)
, x930_(struct2b)
, xc48_(g_SimplePool->GetObj({FOURCC('PART'), particle1}))
, xc54_(std::make_unique<CElementGen>(xc48_, CElementGen::EModelOrientationType::Normal,
                                      CElementGen::EOptionalSystemFlags::One))
, xc78_(wpsc1, dInfo1)
, xca0_(shakeData4)
, xd74_(wpsc2, dInfo2)
, xd9c_(shakeData5)
, xe70_(projectileInfo)
, xeb4_(dInfo3)
, xed0_(shakeData6)
, xfa4_(g_SimplePool->GetObj("Effect_Electric"sv))
, xfb0_(std::make_unique<CParticleElectric>(xfa4_))
, x1014_(g_SimplePool->GetObj({FOURCC('PART'), particle3}))
, x1024_(std::make_unique<CElementGen>(x1014_, CElementGen::EModelOrientationType::Normal,
                                       CElementGen::EOptionalSystemFlags::One))
, x108c_(shakeData1)
, x1294_(shakeData2)
, x1368_(shakeData3)
, x143c_(std::make_unique<CProjectedShadow>(128, 128, true)) {}

void CMetroidPrimeExo::PreThink(float dt, CStateManager& mgr) { CPatterned::PreThink(dt, mgr); }

void CMetroidPrimeExo::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }
  UpdateAreaId(mgr);
  UpdateBoneTracking(dt, mgr);
  UpdateCollision(dt, mgr);
  UpdateHealthInfo(mgr);
  sub80278044(dt, mgr);
  sub8027815c(dt);
  sub80277570(dt, mgr);
  sub80274e6c(dt, mgr);
  sub80273910(dt, mgr);
  sub80273f10(mgr);
  sub80277e30(mgr);
  UpdateTimers(dt);
  sub80276164(dt, mgr);
  sub80275e54(dt, mgr);
}

void CMetroidPrimeExo::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Activate:
    x56c_collisionManager->SetActive(mgr, true);
    break;
  case EScriptObjectMessage::Deactivate:
    x56c_collisionManager->SetActive(mgr, false);
    break;
  case EScriptObjectMessage::Start:
    x1444_24_ = true;
    break;
  case EScriptObjectMessage::Touched:
    sub80278cc8(other, mgr);
    break;
  case EScriptObjectMessage::Registered:
    CreateShadow(false);
    x450_bodyController->Activate(mgr);
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Internal11);
    sub8027903c();
    SetupCollisionActorManager(mgr);
    sub80278800(mgr, true);
    SetBoneTrackingTarget(mgr, true);
    sub80277c04(mgr);
    sub802740fc(mgr);
    sub802755ac(mgr, true);
    sub8027447c(mgr);
    mgr.GetPlayer().SetFrozenTimeoutBias(2.f);
    break;
  case EScriptObjectMessage::Deleted: {
    x56c_collisionManager->Destroy(mgr);
    sub80277b74(mgr);
    sub802740cc(mgr);
    sub8027444c(mgr);
    mgr.GetPlayer().SetFrozenTimeoutBias(0.f);
    break;
  }
  case EScriptObjectMessage::InitializedInArea:
    RemoveMaterial(EMaterialTypes::AIBlock, mgr);
    UpdateRelay(mgr, GetAreaIdAlways());
    if (GetAreaIdAlways() == mgr.GetWorld()->GetCurrentAreaId()) {
      SendStateToRelay(EScriptObjectState::MaxReached, mgr);
    }

    if (xfb0_) {
      xfb0_->SetParticleEmission(false);
    }
    break;
  case EScriptObjectMessage::Damage:
    sub8027827c(other, mgr);
    [[fallthrough]];
  case EScriptObjectMessage::InvulnDamage:
    return;
  default:
    break;
  }
  CPatterned::AcceptScriptMsg(msg, other, mgr);
}

void CMetroidPrimeExo::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  CPatterned::PreRender(mgr, frustum);
}

void CMetroidPrimeExo::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  CPatterned::AddToRenderer(frustum, mgr);
}

void CMetroidPrimeExo::Render(CStateManager& mgr) { CPatterned::Render(mgr); }

bool CMetroidPrimeExo::CanRenderUnsorted(const CStateManager& mgr) const { return CPatterned::CanRenderUnsorted(mgr); }

void CMetroidPrimeExo::Touch(CActor& act, CStateManager& mgr) { CPatterned::Touch(act, mgr); }

void CMetroidPrimeExo::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CMetroidPrimeExo::SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) { CAi::SelectTarget(mgr, msg, arg); }

void CMetroidPrimeExo::Run(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Run(mgr, msg, arg); }

void CMetroidPrimeExo::Attack(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Attack(mgr, msg, arg); }

void CMetroidPrimeExo::TurnAround(CStateManager& mgr, EStateMsg msg, float arg) { CAi::TurnAround(mgr, msg, arg); }

void CMetroidPrimeExo::Active(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Active(mgr, msg, arg); }

void CMetroidPrimeExo::InActive(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x1084_ = x1080_;
    x1084_ = 0.2f;
    x400_24_hitByPlayerProjectile = false;
    x914_24_ = true;
    sub80278800(mgr, false);
    x1078_ = 1;
    GetBodyController()->SetLocomotionType(skLocomotions[x1078_]);
    UpdateHeadHealthInfo(mgr);
    x3b4_speed = 1.f;
  } else if (msg == EStateMsg::Update) {
    if (x107c_ < 0.f && x1084_ < 0.f) {
      x107c_ = x1080_;
      x1084_ = 0.9f;
      x1078_ = mgr.GetActiveRandom()->Next() % 3;
      GetBodyController()->SetLocomotionType(skLocomotions[x1078_]);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x914_24_ = false;
    x1444_24_ = false;
    sub80278800(mgr, true);
    sub802738d4(mgr);
    x1084_ = x1080_;
    x1088_ = x1084_;
    mgr.SetBossParams(GetUniqueId(), 2860.0, 91);
  }
}

void CMetroidPrimeExo::CoverAttack(CStateManager& mgr, EStateMsg msg, float arg) { CAi::CoverAttack(mgr, msg, arg); }

void CMetroidPrimeExo::Crouch(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Crouch(mgr, msg, arg); }

void CMetroidPrimeExo::Taunt(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Taunt(mgr, msg, arg); }

void CMetroidPrimeExo::Suck(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Suck(mgr, msg, arg); }

void CMetroidPrimeExo::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  CAi::ProjectileAttack(mgr, msg, arg);
}

void CMetroidPrimeExo::Flinch(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Flinch(mgr, msg, arg); }

void CMetroidPrimeExo::Dodge(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Dodge(mgr, msg, arg); }

void CMetroidPrimeExo::Retreat(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Retreat(mgr, msg, arg); }

void CMetroidPrimeExo::Cover(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Cover(mgr, msg, arg); }

void CMetroidPrimeExo::Approach(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Approach(mgr, msg, arg); }

void CMetroidPrimeExo::Enraged(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Enraged(mgr, msg, arg); }

void CMetroidPrimeExo::SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  CAi::SpecialAttack(mgr, msg, arg);
}

void CMetroidPrimeExo::Growth(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Growth(mgr, msg, arg); }

void CMetroidPrimeExo::Land(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Land(mgr, msg, arg); }

bool CMetroidPrimeExo::TooClose(CStateManager& mgr, float arg) { return CPatterned::TooClose(mgr, arg); }

bool CMetroidPrimeExo::InMaxRange(CStateManager& mgr, float arg) {
  return !CPatterned::InMaxRange(mgr, arg) && !sub80277224(3.f, mgr) && !sub80277224(7.f, mgr) &&
         !sub80277224(15.f, mgr);
}

bool CMetroidPrimeExo::PlayerSpot(CStateManager& mgr, float arg) { return mgr.GetPlayer().GetFrozenState(); }

bool CMetroidPrimeExo::ShouldAttack(CStateManager& mgr, float arg) { return x1254_ == 9; }

bool CMetroidPrimeExo::ShouldDoubleSnap(CStateManager& mgr, float arg) {
  return !(!x328_24_inPosition && x2dc_destObj == kInvalidUniqueId && sub80277224(11.f, mgr));
}

bool CMetroidPrimeExo::InPosition(CStateManager& mgr, float arg) { return x1084_ <= 0.f; }

bool CMetroidPrimeExo::ShouldTurn(CStateManager& mgr, float arg) {
  return GetTransform().frontVector().dot(mgr.GetPlayer().GetTranslation() - GetTranslation()) < 0.f;
}

bool CMetroidPrimeExo::CoverCheck(CStateManager& mgr, float arg) { return sub80277224(-8.f, mgr); }

bool CMetroidPrimeExo::CoverFind(CStateManager& mgr, float arg) { return x1254_ == 12; }

bool CMetroidPrimeExo::CoveringFire(CStateManager& mgr, float arg) { return x1254_ == 13; }

bool CMetroidPrimeExo::AggressionCheck(CStateManager& mgr, float arg) {
  return (mgr.GetPlayer().GetTranslation() - GetLctrTransform("Jaw_1"sv).origin).magSquared() < 324.f;
}

bool CMetroidPrimeExo::AttackOver(CStateManager& mgr, float arg) { return !x8f4_28_ && x8f4_27_ && !x8f4_25_; }

bool CMetroidPrimeExo::ShouldFire(CStateManager& mgr, float arg) { return x1254_ == 6 || x1254_ == 7 || x1254_ == 8; }

bool CMetroidPrimeExo::ShouldFlinch(CStateManager& mgr, float arg) { return x8f4_24_; }

bool CMetroidPrimeExo::ShouldRetreat(CStateManager& mgr, float arg) { return x8f4_28_; }

bool CMetroidPrimeExo::ShouldCrouch(CStateManager& mgr, float arg) { return x1254_ == 10; }

bool CMetroidPrimeExo::ShouldMove(CStateManager& mgr, float arg) { return x1254_ == 1; }

bool CMetroidPrimeExo::AIStage(CStateManager& mgr, float arg) {
  return (arg < 0.25f && x1078_ == 0) || (arg >= 0.75f && x1078_ == 2) || (x1078_ == 1 && arg > 0.25f && arg <= 0.75f);
}

bool CMetroidPrimeExo::StartAttack(CStateManager& mgr, float arg) { return x920_ <= 0.f; }

bool CMetroidPrimeExo::ShouldSpecialAttack(CStateManager& mgr, float arg) {
  return x1254_ == 2 || x1254_ == 3 || x1254_ == 4 || x1254_ == 5;
}

bool CMetroidPrimeExo::CodeTrigger(CStateManager& mgr, float arg) { return x1444_24_; }

CProjectileInfo* CMetroidPrimeExo::GetProjectileInfo() { return CPatterned::GetProjectileInfo(); }

void CMetroidPrimeExo::sub802738d4(CStateManager& mgr) {}

void CMetroidPrimeExo::sub80273910(float dt, CStateManager& mgr) {}

void CMetroidPrimeExo::sub80273c78(CStateManager& mgr) {}

void CMetroidPrimeExo::sub80273d38(CStateManager& mgr) {}

void CMetroidPrimeExo::sub80273f10(CStateManager& mgr) {}

void CMetroidPrimeExo::sub80274054(CStateManager& mgr) {}

void CMetroidPrimeExo::sub802740cc(CStateManager& mgr) {}

void CMetroidPrimeExo::sub802740fc(CStateManager& mgr) {}

void CMetroidPrimeExo::sub802743e0(CStateManager& mgr, u32) {}

void CMetroidPrimeExo::sub8027444c(CStateManager& mgr) {}

void CMetroidPrimeExo::sub8027447c(CStateManager& mgr) {}

void CMetroidPrimeExo::sub802747b8(float f1, CStateManager& mgr, const zeus::CVector3f& vec) {}

void CMetroidPrimeExo::sub80274e6c(float f1, CStateManager& mgr) {}

void CMetroidPrimeExo::sub802755ac(CStateManager& mgr, bool b1) {}

void CMetroidPrimeExo::sub802756b8(CStateManager& mgr) {}

void CMetroidPrimeExo::sub8027571c(CStateManager& mgr) {}

void CMetroidPrimeExo::UpdateTimers(float dt) {
  if (GetBodyController()->GetPercentageFrozen() != 0.f) {
    return;
  }

  x107c_ -= dt;
  x1084_ -= dt * GetModelData()->GetAnimationData()->GetSpeedScale();
  x920_ -= dt;
}

void CMetroidPrimeExo::sub80275800(CStateManager& mgr) {}

void CMetroidPrimeExo::sub802759a8(CStateManager& mgr, int w1, int w2) {}

void CMetroidPrimeExo::sub80275b04(CStateManager& mgr, int w1, int w2) {}

void CMetroidPrimeExo::sub80275b68() {}

void CMetroidPrimeExo::sub80275c60(CStateManager& mgr, int w1, int w2) {}

void CMetroidPrimeExo::sub80275d68(int w1) {}

void CMetroidPrimeExo::sub80275e14(int w1) {}

void CMetroidPrimeExo::sub80275e34(int w1) {}

void CMetroidPrimeExo::sub80275e54(float f1, CStateManager& mgr) {}

void CMetroidPrimeExo::sub80276164(float f1, CStateManager& mgr) {}

void CMetroidPrimeExo::sub80276204(CStateManager& mgr, bool b1) {}

void CMetroidPrimeExo::sub8027639c(CStateManager& mgr, bool b1) {}

void CMetroidPrimeExo::SetActorAreaId(CStateManager& mgr, TUniqueId uid, TAreaId aid) {
  if (auto* act = static_cast<CActor*>(mgr.ObjectById(uid))) {
    mgr.SetActorAreaId(*act, aid);
  }
}

void CMetroidPrimeExo::UpdateAreaId(CStateManager& mgr) {
  if (!x914_24_) {
    return;
  }

  TAreaId curAreaId = mgr.GetWorld()->GetCurrentAreaId();
  if (GetAreaIdAlways() == curAreaId) {
    if (x1444_25_) {
      x1444_25_ = false;
      SendStateToRelay(EScriptObjectState::MaxReached, mgr);
    }
  } else if (!IsRelayValid(mgr, curAreaId)) {
    x1444_25_ = true;
  } else {
    SetActorAreaId(mgr, GetUniqueId(), curAreaId);

    for (size_t i = 0; i < x56c_collisionManager->GetNumCollisionActors(); ++i) {
      SetActorAreaId(mgr, x56c_collisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId(), curAreaId);
    }

    for (const auto& uid : xb24_) {
      SetActorAreaId(mgr, uid, curAreaId);
    }

    SetActorAreaId(mgr, xeac_, curAreaId);
    UpdateRelay(mgr, GetAreaIdAlways());
    SendStateToRelay(EScriptObjectState::MaxReached, mgr);
  }
}

void CMetroidPrimeExo::SendStateToRelay(EScriptObjectState state, CStateManager& mgr) {
  if (TCastToPtr<CMetroidPrimeRelay> relay = mgr.ObjectById(x568_relayId)) {
    relay->SendScriptMsgs(state, mgr, EScriptObjectMessage::None);
  }
}

void CMetroidPrimeExo::sub80276754(CStateManager& mgr) {}

TUniqueId CMetroidPrimeExo::sub802769e0(CStateManager& mgr, bool b1) {
  TUniqueId uid = sub80276b3c(mgr, EScriptObjectState::Attack, EScriptObjectMessage::Follow);
  float lastDot = 0.f;
  TUniqueId lastUid = kInvalidUniqueId;
  while (uid != kInvalidUniqueId) {
    if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(uid)) {
      float dot = GetTransform().frontVector().dot(wp->GetTranslation() - GetTranslation());

      if ((b1 && dot > 0.f && dot > lastDot) || (!b1 && dot < 0.f && dot < lastDot)) {
        lastUid = uid;
        lastDot = dot;
      }
      uid = wp->NextWaypoint(mgr);
    } else {
      uid = kInvalidUniqueId;
    }
  }

  return lastUid;
}

TUniqueId CMetroidPrimeExo::sub80276b3c(CStateManager& mgr, EScriptObjectState state, EScriptObjectMessage msg) {
  if (TCastToConstPtr<CMetroidPrimeRelay> relay = mgr.GetObjectById(x568_relayId)) {
    rstl::reserved_vector<TUniqueId, 8> uids;
    for (const auto& conn : relay->GetConnectionList()) {
      if (conn.x0_state != state || conn.x4_msg != msg) {
        continue;
      }

      TUniqueId uid = mgr.GetIdForScript(conn.x8_objId);
      const auto* ent = mgr.GetObjectById(uid);
      if (ent != nullptr && ent->GetActive()) {
        uids.push_back(uid);
        if (uids.size() == 8) {
          break;
        }
      }
    }

    return uids[mgr.GetActiveRandom()->Next() % uids.size()];
  }
  return kInvalidUniqueId;
}

void CMetroidPrimeExo::UpdateRelay(CStateManager& mgr, TAreaId areaId) {
  if (x568_relayId != kInvalidUniqueId) {
    if (TCastToPtr<CMetroidPrimeRelay> relay = mgr.ObjectById(x568_relayId)) {
      relay->SetMetroidPrimeExoId(kInvalidUniqueId);
    }
  }

  TEditorId tmpEditorId = kInvalidEditorId;
  for (auto* ent : mgr.GetAllObjectList()) {
    if (TCastToPtr<CMetroidPrimeRelay> relay = ent) {
      if (relay->GetActive() && relay->GetAreaIdAlways() == areaId) {
        tmpEditorId = relay->GetEditorId();
      }
    }
  }

  x568_relayId = kInvalidUniqueId;
  if (tmpEditorId != kInvalidEditorId) {
    TUniqueId uid = mgr.GetIdForScript(tmpEditorId);
    x568_relayId = uid;
    if (TCastToPtr<CMetroidPrimeRelay> relay = mgr.ObjectById(uid)) {
      relay->SetMetroidPrimeExoId(GetUniqueId());
    }
  }

  sub80276754(mgr);
  sub80273d38(mgr);
}

bool CMetroidPrimeExo::IsRelayValid(CStateManager& mgr, TAreaId aid) {
  TEditorId tmpId = kInvalidEditorId;

  for (const auto* ent : mgr.GetAllObjectList()) {
    if (TCastToConstPtr<CMetroidPrimeRelay> relay = ent) {
      if (relay->GetAreaIdAlways() == aid) {
        tmpId = relay->GetEditorId();
      }
    }
  }

  return tmpId != kInvalidEditorId;
}

bool CMetroidPrimeExo::sub80277224(float f1, CStateManager& mgr) {
  TUniqueId uid = sub802769e0(mgr, f1 >= 0.f);

  if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(uid)) {
    const float dist = (wp->GetTranslation() - GetTranslation()).magSquared();
    const float scaleMag = f1 * (0.57735026f * GetModelData()->GetScale().magnitude());
    if (f1 < 0.f) {
      return dist < scaleMag;
    }
    return dist > scaleMag;
  }
  return false;
}

void CMetroidPrimeExo::sub80277380(CStateManager& mgr, bool b1) {}

void CMetroidPrimeExo::sub80277570(float f1, CStateManager& mgr) {}

zeus::CVector3f CMetroidPrimeExo::sub80778c4(CStateManager& mgr) { return zeus::CVector3f(); }

void CMetroidPrimeExo::sub80277b74(CStateManager& mgr) {}

void CMetroidPrimeExo::sub80277c04(CStateManager& mgr) {}

void CMetroidPrimeExo::sub80277e30(CStateManager& mgr) {}

void CMetroidPrimeExo::sub80278044(float f1, CStateManager& mgr) {}

void CMetroidPrimeExo::sub80278130(const zeus::CColor& col) {
  x8e4_ = 0.f;
  x8f4_24_ = true;
  x8e0_ = col;
  x8dc_ = x8d8_;
}

void CMetroidPrimeExo::sub802781e0(const zeus::CColor& col) {}

void CMetroidPrimeExo::sub8027815c(float f1) {}

void CMetroidPrimeExo::sub8027827c(TUniqueId uid, CStateManager& mgr) {}

void CMetroidPrimeExo::sub80278508(CStateManager& mgr, int w1, bool b1) {}

void CMetroidPrimeExo::sub802786fc(CStateManager& mgr) {}

void CMetroidPrimeExo::sub80278800(CStateManager& mgr, bool b) {
  x8f4_25_ = b;
  sub80278508(mgr, x570_, b);
  GetModelData()->GetAnimationData()->SetParticleEffectState("Eyes"sv, b, mgr);

  if (!b) {
    sub80278130(zeus::skBlack);
  } else {
    sub80278130(x588_[x570_].x6c_color);
  }
}

void CMetroidPrimeExo::UpdateHeadHealthInfo(CStateManager& mgr) {
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x8cc_headColActor)) {
    *colAct->HealthInfo(mgr) = x8c0_;
  }
}

void CMetroidPrimeExo::UpdateHealthInfo(CStateManager& mgr) {
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x8cc_headColActor)) {
    auto* hInfo = colAct->HealthInfo(mgr);
    if (hInfo->GetHP() <= 0.f && !x8f4_28_) {
      x8f4_28_ = true;
      --x8d0_;
      if (x8d0_ == 0) {
        x400_24_hitByPlayerProjectile = true;
      }
    }

    if (x8f4_28_) {
      UpdateHeadHealthInfo(mgr);
    }

    if (x91c_ < 0 || x91c_ > 3) {
      return;
    }
    if (x914_24_) {
      hInfo->SetHP(skHealthConstants[std::max(0, x91c_ - 1)]);
    } else {
      hInfo->SetHP(std::max(0.f, hInfo->GetHP()) + skHealthConstants[x91c_] +
                   (static_cast<float>(x8d0_ - 1) * x8c0_.GetHP()));
    }
  }
}

void CMetroidPrimeExo::SetBoneTrackingTarget(CStateManager& mgr, bool active) {
  for (auto& boneTracking : x76c_) {
    boneTracking.SetActive(active);
    boneTracking.SetTarget(mgr.GetPlayer().GetUniqueId());
  }
}

void CMetroidPrimeExo::UpdateBoneTracking(float dt, CStateManager& mgr) {
  CAnimData* animData = GetModelData()->GetAnimationData();
  animData->PreRender();
  for (auto tracking : x76c_) {
    tracking.Update(dt);
    tracking.PreRender(mgr, *animData, GetTransform(), GetModelData()->GetScale(), *GetBodyController());
  }

  if (xe4_30_outOfFrustum) {
    xe4_27_notInSortedLists = !x1054_24_;
  }
}

void CMetroidPrimeExo::sub80278cc8(TUniqueId uid, CStateManager& mgr) {}

void CMetroidPrimeExo::UpdateCollision(float dt, CStateManager& mgr) {
  x56c_collisionManager->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  zeus::CTransform xf = GetLocatorTransform("Skeleton_Root"sv);
  MoveCollisionPrimitive(GetTransform().rotate(GetModelData()->GetScale() * xf.origin));
}

void CMetroidPrimeExo::sub8027903c() {}

void CMetroidPrimeExo::sub8027c22c(int w1, int w2) {}

void CMetroidPrimeExo::SetupCollisionActorManager(CStateManager& mgr) {
  std::vector<CJointCollisionDescription> joints;
  joints.reserve(skBodyJoints.size() + skSphereJoints.size());
  for (auto& skBodyJoint : skBodyJoints) {
    CSegId to = GetModelData()->GetAnimationData()->GetLocatorSegId(skBodyJoint.to);
    CSegId from = GetModelData()->GetAnimationData()->GetLocatorSegId(skBodyJoint.from);
    joints.push_back(CJointCollisionDescription::OBBAutoSizeCollision(
        to, from, skBodyJoint.bounds, CJointCollisionDescription::EOrientationType::One,
        std::string(skBodyJoint.to) + std::string(skBodyJoint.from), 200.f));
  }

  for (auto& skSphereJoint : skSphereJoints) {
    joints.push_back(CJointCollisionDescription::SphereCollision(
        GetModelData()->GetAnimationData()->GetLocatorSegId(skSphereJoint.name), skSphereJoint.radius,
        skSphereJoint.name, 200.f));
  }

  x56c_collisionManager =
      std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, GetActive());

  for (size_t i = 0; i < x56c_collisionManager->GetNumCollisionActors(); ++i) {
    const auto& jInfo = x56c_collisionManager->GetCollisionDescFromIndex(i);
    if (jInfo.GetName() == "Head_LockON_SDK"sv) {
      x8cc_headColActor = jInfo.GetCollisionActorId();
    }

    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(jInfo.GetCollisionActorId())) {
      if (jInfo.GetCollisionActorId() != x1046_) {
        colAct->SetDamageVulnerability(*GetDamageVulnerability());
      }
    }
  }

  x56c_collisionManager->AddMaterial(mgr, CMaterialList(EMaterialTypes::AIJoint, EMaterialTypes::CameraPassthrough));
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {EMaterialTypes::Player}));
  AddMaterial(EMaterialTypes::ProjectilePassthrough, mgr);
  RemoveMaterial(EMaterialTypes::Solid, EMaterialTypes::Orbit, EMaterialTypes::Solid, mgr);
  UpdateHeadHealthInfo(mgr);
}

} // namespace urde::MP1
