#include "Runtime/MP1/World/CMetroidPrimeExo.hpp"

#include "DataSpec/DNAMP1/ScriptObjects/MetroidPrimeStage1.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CParticleElectric.hpp"
#include "Runtime/Particle/CParticleSwoosh.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CProjectedShadow.hpp"
#include "Runtime/World/CWorld.hpp"
#include "Runtime/World/ScriptLoader.hpp"
#include "Runtime/MP1/World/CMetroidPrimeRelay.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {

namespace {
std::array<pas::ELocomotionType, 3> skLocomotions{{
    pas::ELocomotionType::Internal10,
    pas::ELocomotionType::Internal11,
    pas::ELocomotionType::Internal12,
}};
std::array<pas::ETauntType, 3> skTaunts {{
    pas::ETauntType::One,
    pas::ETauntType::Two,
    pas::ETauntType::Zero,
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
  for (int i = 0; i < 4; ++i)
    ret.emplace_back(in);
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
  sub80276528(mgr);
  sub80278bd8(dt, mgr);
  sub80278f14(dt, mgr);
  sub8027894c(mgr);
  sub80278044(dt, mgr);
  sub8027815c(dt);
  sub80277570(dt, mgr);
  sub80274e6c(dt, mgr);
  sub80273910(dt, mgr);
  sub80273f10(mgr);
  sub80277e30(mgr);
  sub80275780(dt);
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
    sub80278b60(mgr, true);
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
      sub802766e4(EScriptObjectState::MaxReached, mgr);
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
    sub802788c8(mgr);
    x3b4_speed = 1.f;
  } else if (msg == EStateMsg::Update) {
    if (x107c_ >= 0.f || x1084_ >= 0.f) {
      return;
    }

    x107c_ = x1080_;
    x1084_ = 0.90000004f;
    x1078_ = mgr.GetActiveRandom()->Next() % 3;
    GetBodyController()->SetLocomotionType(skLocomotions[x1078_]);
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

bool CMetroidPrimeExo::InMaxRange(CStateManager& mgr, float arg) { return CPatterned::InMaxRange(mgr, arg); }

bool CMetroidPrimeExo::PlayerSpot(CStateManager& mgr, float arg) { return CPatterned::PlayerSpot(mgr, arg); }

bool CMetroidPrimeExo::ShouldAttack(CStateManager& mgr, float arg) { return CAi::ShouldAttack(mgr, arg); }

bool CMetroidPrimeExo::ShouldDoubleSnap(CStateManager& mgr, float arg) { return CAi::ShouldDoubleSnap(mgr, arg); }

bool CMetroidPrimeExo::InPosition(CStateManager& mgr, float arg) { return CPatterned::InPosition(mgr, arg); }

bool CMetroidPrimeExo::ShouldTurn(CStateManager& mgr, float arg) { return CAi::ShouldTurn(mgr, arg); }

bool CMetroidPrimeExo::CoverCheck(CStateManager& mgr, float arg) { return CAi::CoverCheck(mgr, arg); }

bool CMetroidPrimeExo::CoverFind(CStateManager& mgr, float arg) { return CAi::CoverFind(mgr, arg); }

bool CMetroidPrimeExo::CoveringFire(CStateManager& mgr, float arg) { return CAi::CoveringFire(mgr, arg); }

bool CMetroidPrimeExo::AggressionCheck(CStateManager& mgr, float arg) { return CAi::AggressionCheck(mgr, arg); }

bool CMetroidPrimeExo::AttackOver(CStateManager& mgr, float arg) { return CAi::AttackOver(mgr, arg); }

bool CMetroidPrimeExo::ShouldFire(CStateManager& mgr, float arg) { return CAi::ShouldFire(mgr, arg); }

bool CMetroidPrimeExo::ShouldFlinch(CStateManager& mgr, float arg) { return CAi::ShouldFlinch(mgr, arg); }

bool CMetroidPrimeExo::ShouldRetreat(CStateManager& mgr, float arg) { return CAi::ShouldRetreat(mgr, arg); }

bool CMetroidPrimeExo::ShouldCrouch(CStateManager& mgr, float arg) { return CAi::ShouldCrouch(mgr, arg); }

bool CMetroidPrimeExo::ShouldMove(CStateManager& mgr, float arg) { return CAi::ShouldMove(mgr, arg); }

bool CMetroidPrimeExo::AIStage(CStateManager& mgr, float arg) { return CAi::AIStage(mgr, arg); }

bool CMetroidPrimeExo::StartAttack(CStateManager& mgr, float arg) { return CAi::StartAttack(mgr, arg); }

bool CMetroidPrimeExo::ShouldSpecialAttack(CStateManager& mgr, float arg) { return CAi::ShouldSpecialAttack(mgr, arg); }

bool CMetroidPrimeExo::CodeTrigger(CStateManager& mgr, float arg) { return CPatterned::CodeTrigger(mgr, arg); }

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

void CMetroidPrimeExo::sub80275780(float mgr) {}

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

void CMetroidPrimeExo::sub802764c4(CStateManager& mgr, TUniqueId uid, TAreaId aid) {}

void CMetroidPrimeExo::sub80276528(CStateManager& mgr) {
  if (!x914_24_) {
    return;
  }

  TAreaId curAreaId = mgr.GetWorld()->GetCurrentAreaId();
  if (GetAreaIdAlways() == curAreaId) {
    if (x1444_25_) {
      x1444_25_ = false;
      sub802766e4(EScriptObjectState::MaxReached, mgr);
    }
  } else if (!sub80276ec0(mgr, curAreaId)) {
    x1444_25_ = true;
  } else {
    sub802764c4(mgr, GetUniqueId(), curAreaId);

    for (size_t i = 0; i < x56c_collisionManager->GetNumCollisionActors(); ++i) {
      sub802764c4(mgr, x56c_collisionManager->GetCollisionDescFromIndex(i).GetCollisionActorId(), curAreaId);
    }

    for (const auto& uid : xb24_) {
      sub802764c4(mgr, uid, curAreaId);
    }

    sub802764c4(mgr, xeac_, curAreaId);
    UpdateRelay(mgr, GetAreaIdAlways());
    sub802766e4(EScriptObjectState::MaxReached, mgr);
  }
}

void CMetroidPrimeExo::sub802766e4(EScriptObjectState state, CStateManager& mgr) {}

void CMetroidPrimeExo::sub80276754(CStateManager& mgr) {}

zeus::CVector3f CMetroidPrimeExo::sub802769e0(CStateManager& mgr, bool b1) { return zeus::CVector3f(); }

zeus::CVector3f CMetroidPrimeExo::sub80276b3c(CStateManager& mgr, EScriptObjectState state, EScriptObjectMessage msg) {
  return zeus::CVector3f();
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
  if (tmpEditorId == kInvalidEditorId) {
    TUniqueId uid = mgr.GetIdForScript(tmpEditorId);
    x568_relayId = uid;
    if (TCastToPtr<CMetroidPrimeRelay> relay = mgr.ObjectById(uid)) {
      relay->SetMetroidPrimeExoId(GetUniqueId());
    }
  }

  sub80276754(mgr);
  sub80273d38(mgr);
}

bool CMetroidPrimeExo::sub80276ec0(CStateManager& mgr, TAreaId w2) {
  TEditorId tmpId = kInvalidEditorId;

  for (const auto* ent : mgr.GetAllObjectList()) {
    if (TCastToConstPtr<CMetroidPrimeRelay> relay = ent) {
      tmpId = relay->GetEditorId();
    }
  }

  return tmpId != kInvalidEditorId;
}

void CMetroidPrimeExo::sub80277224(float f1, CStateManager& mgr) {}

void CMetroidPrimeExo::sub80277380(CStateManager& mgr, bool b1) {}

void CMetroidPrimeExo::sub80277570(float f1, CStateManager& mgr) {}

zeus::CVector3f CMetroidPrimeExo::sub80778c4(CStateManager& mgr) { return zeus::CVector3f(); }

void CMetroidPrimeExo::sub80277b74(CStateManager& mgr) {}

void CMetroidPrimeExo::sub80277c04(CStateManager& mgr) {}

void CMetroidPrimeExo::sub80277e30(CStateManager& mgr) {}

void CMetroidPrimeExo::sub80278044(float f1, CStateManager& mgr) {}

void CMetroidPrimeExo::sub802781e0(const zeus::CColor& col) {}

void CMetroidPrimeExo::sub8027815c(float f1) {}

void CMetroidPrimeExo::sub8027827c(TUniqueId uid, CStateManager& mgr) {}

void CMetroidPrimeExo::sub80278508(CStateManager& mgr, int w1, bool b1) {}

void CMetroidPrimeExo::sub802786fc(CStateManager& mgr) {}

void CMetroidPrimeExo::sub80278800(CStateManager& mgr, bool b) {}

void CMetroidPrimeExo::sub802788c8(CStateManager& mgr) {}

void CMetroidPrimeExo::sub8027894c(CStateManager& mgr) {}

void CMetroidPrimeExo::sub80278b60(CStateManager& mgr, bool b1) {}

void CMetroidPrimeExo::sub80278bd8(float f1, CStateManager& mgr) {}

void CMetroidPrimeExo::sub80278cc8(TUniqueId uid, CStateManager& mgr) {}

void CMetroidPrimeExo::sub80278f14(float, CStateManager& mgr) {}

void CMetroidPrimeExo::sub8027903c() {}

void CMetroidPrimeExo::sub8027c22c(int w1, int w2) {}

void CMetroidPrimeExo::SetupCollisionActorManager(CStateManager& mgr) {}

} // namespace urde::MP1
