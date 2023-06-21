#include "Runtime/MP1/World/CMetroidPrimeExo.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"
#include "Runtime/MP1/World/CEnergyBall.hpp"
#include "Runtime/MP1/World/CIceAttackProjectile.hpp"
#include "Runtime/MP1/World/CMetroidPrimeRelay.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CParticleElectric.hpp"
#include "Runtime/Particle/CParticleSwoosh.hpp"
#include "Runtime/Weapon/CPlasmaProjectile.hpp"
#include "Runtime/World/CHUDBillboardEffect.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CProjectedShadow.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/World/CWorld.hpp"
#include "Runtime/World/ScriptLoader.hpp"

#include "Runtime/Streams/IOStreams.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace metaforce::MP1 {

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

std::array<std::string_view, 2> skDrillerLocators{{"driller_LCTR1"sv, "driller_LCTR2"sv}};

std::array<std::string_view, 4> skEffectNames{{
    "Flame_Head"sv,
    "Flame_HeadLockOn"sv,
    "Flame_Lshoulder"sv,
    "Flame_Rshoulder"sv,
}};

std::array<std::string_view, 20> skLegLocators{{
    "R_front_2"sv,
    "L_front_2"sv,
    "R_front_1"sv,
    "L_front_1"sv,
    ""sv,
    ""sv,
    "R_elbow"sv,
    "L_elbow"sv,
    ""sv,
    ""sv,
    "Head"sv,
    "Head_LCTR"sv,
    ""sv,
    ""sv,
    "R_shoulder"sv,
    "L_shoulder"sv,
    "R_stinger_2"sv,
    "L_stinger_2"sv,
    "R_spike_LCTR"sv,
    "L_spike_LCTR"sv,
}};

std::array<std::string_view, 6> skBoneTrackingNames{{
    "L_eye_1"sv,
    "L_eye_2"sv,
    "L_eye_3"sv,
    "R_eye_1"sv,
    "R_eye_2"sv,
    "R_eye_3"sv,
}};

std::array<std::array<s32, 3>, 14> skSomeMeleeValues{{
    {{3, 3, 3}},
    {{-1, -1, -1}},
    {{2, 2, 2}},
    {{5, 5, 5}},
    {{8, 8, 8}},
    {{11, 11, 11}},
    {{1, 1, 1}},
    {{4, 4, 4}},
    {{7, 7, 7}},
    {{4, 7, 1}},
    {{-1, -1, -1}},
    {{-1, 2, -1}},
    {{-1, -1, -1}},
    {{0, 0, 0}},
}};

std::array<s32, 17> skSomeValues1{{7, 5, 18, 18, 18, 18, 18, 18, 18, 7, 5, 7, 17, 18, 9, 2, 11}};

std::array<std::array<pas::ELocomotionType, 3>, 14> skSomeValues2{{
    {{pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid}},
    {{pas::ELocomotionType::Internal10, pas::ELocomotionType::Internal11, pas::ELocomotionType::Internal12}},
    {{pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid}},
    {{pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid}},
    {{pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid}},
    {{pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid}},
    {{pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid}},
    {{pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid}},
    {{pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid}},
    {{pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid}},
    {{pas::ELocomotionType::Internal8, pas::ELocomotionType::Internal8, pas::ELocomotionType::Internal8}},
    {{pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid}},
    {{pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid}},
    {{pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid, pas::ELocomotionType::Invalid}},
}};
} // namespace
SPrimeStruct2B::SPrimeStruct2B(CInputStream& in)
: x0_propertyCount(in.ReadLong())
, x4_particle1(in.Get<CAssetId>())
, x8_particle2(in.Get<CAssetId>())
, xc_particle3(in.Get<CAssetId>())
, x10_dInfo(in)
, x2c_(in.ReadFloat())
, x30_(in.ReadFloat())
, x34_texture(in.Get<CAssetId>())
, x38_(CSfxManager::TranslateSFXID(u16(in.ReadLong())))
, x3a_(CSfxManager::TranslateSFXID(u16(in.ReadLong()))) {}

SPrimeStruct4::SPrimeStruct4(CInputStream& in)
: x0_beamInfo(in)
, x44_(in.ReadLong())
, x48_dInfo1(in)
, x64_struct5(CPlasmaProjectile::LoadPlayerEffectResources(in))
, x88_(in.ReadFloat())
, x8c_dInfo2(in) {}

SPrimeStruct6::SPrimeStruct6(CInputStream& in)
: x0_propertyCount(in.ReadLong()), x4_damageVulnerability(in), x6c_color(in.Get<zeus::CColor>()) {
  x70_[0] = in.ReadLong();
  x70_[1] = in.ReadLong();
}

static CPatternedInfo LoadPatternedInfo(CInputStream& in) {
  std::pair<bool, u32> pcount = CPatternedInfo::HasCorrectParameterCount(in);
  return CPatternedInfo(in, pcount.second);
}

struct SExoCameraShakePoint {
  float x0_attackTime{};
  float x4_sustainTime{};
  float x8_duration{};
  float xc_magnitude{};
  SExoCameraShakePoint() = default;
  SExoCameraShakePoint(CInputStream& in)
  : x0_attackTime(in.ReadFloat())
  , x4_sustainTime(in.ReadFloat())
  , x8_duration(in.ReadFloat())
  , xc_magnitude(in.ReadFloat()) {}
};

struct SExoCameraShakerComponent {
  bool x0_useModulation{};
  SExoCameraShakePoint x4_am{};
  SExoCameraShakePoint x14_fm{};
  SExoCameraShakerComponent() = default;
  explicit SExoCameraShakerComponent(CInputStream& in)
  : x0_useModulation(in.ReadBool()), x4_am(in.Get<SExoCameraShakePoint>()), x14_fm(in.Get<SExoCameraShakePoint>()) {}
};

struct SExoCameraShakeData {
  bool x0_useSfx{};
  float x4_duration{};
  float x8_sfxDist{};
  std::array<SExoCameraShakerComponent, 3> xc_components{};
  SExoCameraShakeData() = default;
  explicit SExoCameraShakeData(CInputStream& in)
  : x0_useSfx(in.ReadBool()), x4_duration(in.ReadFloat()), x8_sfxDist(in.ReadFloat()) {
    for (auto& component : xc_components) {
      component = in.Get<SExoCameraShakerComponent>();
    }
  }
};

static SCameraShakePoint BuildCameraShakePoint(SExoCameraShakePoint& sp) {
  return {false, sp.x0_attackTime, sp.x4_sustainTime, sp.x8_duration, sp.xc_magnitude};
}

static CCameraShakerComponent BuildCameraShakerComponent(SExoCameraShakerComponent& comp) {
  return {comp.x0_useModulation, BuildCameraShakePoint(comp.x4_am), BuildCameraShakePoint(comp.x14_fm)};
}

static CCameraShakeData LoadCameraShakeData(CInputStream& in) {
  auto shakeData = in.Get<SExoCameraShakeData>();
  return {shakeData.x4_duration,
          shakeData.x8_sfxDist,
          u32(shakeData.x0_useSfx),
          zeus::skZero3f,
          BuildCameraShakerComponent(shakeData.xc_components[0]),
          BuildCameraShakerComponent(shakeData.xc_components[1]),
          BuildCameraShakerComponent(shakeData.xc_components[2])};
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
  for (int i = 0; i < 4; ++i) {
    ret.emplace_back(in);
  }
  return ret;
}

SPrimeExoParameters::SPrimeExoParameters(CInputStream& in)
: x0_propertyCount(in.ReadLong())
, x4_patternedInfo(LoadPatternedInfo(in))
, x13c_actorParms(ScriptLoader::LoadActorParameters(in))
, x1a4_(in.ReadLong())
, x1a8_(LoadCameraShakeData(in))
, x27c_(LoadCameraShakeData(in))
, x350_(LoadCameraShakeData(in))
, x424_(in)
, x460_particle1(in.Get<CAssetId>())
, x464_(LoadPrimeStruct4s(in))
, x708_wpsc1(in.Get<CAssetId>())
, x70c_dInfo1(in)
, x728_shakeData1(LoadCameraShakeData(in))
, x7fc_wpsc2(in.Get<CAssetId>())
, x800_dInfo2(in)
, x81c_shakeData2(LoadCameraShakeData(in))
, x8f0_(in)
, x92c_(in)
, x948_(LoadCameraShakeData(in))
, xa1c_particle2(in.Get<CAssetId>())
, xa20_swoosh(in.Get<CAssetId>())
, xa24_particle3(in.Get<CAssetId>())
, xa28_particle4(in.Get<CAssetId>())
, xa2c_(LoadPrimeStruct6s(in)) {}

CMetroidPrimeAttackWeights::CMetroidPrimeAttackWeights(CInputStream& in) {
  u32 propCount = std::min(14, in.ReadLong());
  for (u32 i = 0; i < propCount; ++i) {
    x0_.push_back(in.ReadFloat());
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
, x91c_(pw1)
, x930_(struct2b)
, xc48_(g_SimplePool->GetObj({FOURCC('PART'), particle1}))
, xc50_(std::make_unique<CElementGen>(xc48_, CElementGen::EModelOrientationType::Normal,
                                      CElementGen::EOptionalSystemFlags::One))
, xc78_(wpsc1, dInfo1)
, xca0_(shakeData4)
, xd74_(wpsc2, dInfo2)
, xd9c_(shakeData5)
, xe70_(projectileInfo)
, xeb4_(dInfo3)
, xed0_(shakeData6)
, xfa4_(g_SimplePool->GetObj("Effect_Electric"sv))
, xfac_(std::make_unique<CParticleElectric>(xfa4_))
, x1014_(g_SimplePool->GetObj({FOURCC('PART'), particle3}))
, x101c_(g_SimplePool->GetObj({FOURCC('PART'), particle4}))
, x1024_(std::make_unique<CElementGen>(x1014_, CElementGen::EModelOrientationType::Normal,
                                       CElementGen::EOptionalSystemFlags::One))
, x108c_(shakeData1)
, x1294_(shakeData2)
, x1368_(shakeData3)
, x143c_(std::make_unique<CProjectedShadow>(128, 128, true)) {
  for (const auto& struct4 : struct4s) {
    x96c_.emplace_back(struct4.x0_beamInfo);
    xb30_.emplace_back(struct4.x64_struct5);
    xbc4_.emplace_back(struct4.x8c_dInfo2);
    xa80_.emplace_back(struct4.x44_, struct4.x48_dInfo1);
  }

  x460_knockBackController.SetAutoResetImpulse(false);
  x460_knockBackController.SetEnableBurn(false);
  x460_knockBackController.SetEnableFreeze(false);
  xc78_.Token().Lock();
  xd74_.Token().Lock();
  xfc4_.emplace_back(g_SimplePool->GetObj({FOURCC('PART'), particle2}));
  xfc4_.emplace_back(g_SimplePool->GetObj({FOURCC('PART'), particle2}));
  xfd8_.emplace_back(g_SimplePool->GetObj({FOURCC('SWSH'), swoosh}));
  xfd8_.emplace_back(g_SimplePool->GetObj({FOURCC('SWSH'), swoosh}));
  xfec_.emplace_back(std::make_unique<CElementGen>(xfc4_[0]));
  xfec_.emplace_back(std::make_unique<CElementGen>(xfc4_[1]));
  x1000_.emplace_back(std::make_unique<CParticleSwoosh>(xfd8_[0], 0));
  x1000_.emplace_back(std::make_unique<CParticleSwoosh>(xfd8_[1], 0));
  x102c_.push_back(0.3f);
  x102c_.push_back(0.3f);
  x1038_.push_back(0.f);
  x1038_.push_back(2.f);
}

void CMetroidPrimeExo::PreThink(float dt, CStateManager& mgr) {
  CPatterned::PreThink(dt, mgr);
  if (!GetActive()) {
    return;
  }

  if (TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(x8cc_headColActor)) {
    x8c8_ = colAct->GetHealthInfo(mgr)->GetHP();
  }
}

void CMetroidPrimeExo::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }
  UpdateAreaId(mgr);
  UpdateBoneTracking(dt, mgr);
  UpdateCollision(dt, mgr);
  UpdateHealthInfo(mgr);
  UpdateColorChange(dt, mgr);
  UpdateHeadAnimation(dt);
  UpdatePlasmaProjectile(dt, mgr);
  UpdateParticles(dt, mgr);
  UpdateEnergyBalls(dt, mgr);
  UpdatePhysicsDummy(mgr);
  UpdateContactDamage(mgr);
  UpdateTimers(dt);
  UpdateSfxEmitter(dt, mgr);
  UpdateElectricEffect(dt, mgr);
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
    DoContactDamage(other, mgr);
    break;
  case EScriptObjectMessage::Registered:
    CreateShadow(false);
    x450_bodyController->Activate(mgr);
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Internal11);
    SetupBoneTracking();
    SetupCollisionActorManager(mgr);
    SetEyesParticleEffectState(mgr, true);
    SetBoneTrackingTarget(mgr, true);
    CreatePlasmaProjectiles(mgr);
    CreatePhysicsDummy(mgr);
    EnableParticles(mgr, true);
    CreateHUDBillBoard(mgr);
    mgr.GetPlayer().SetFrozenTimeoutBias(2.f);
    break;
  case EScriptObjectMessage::Deleted: {
    x56c_collisionManager->Destroy(mgr);
    FreePlasmaProjectiles(mgr);
    sub802740cc(mgr);
    FreeBillboard(mgr);
    mgr.GetPlayer().SetFrozenTimeoutBias(0.f);
    break;
  }
  case EScriptObjectMessage::InitializedInArea:
    RemoveMaterial(EMaterialTypes::AIBlock, mgr);
    UpdateRelay(mgr, GetAreaIdAlways());
    if (GetAreaIdAlways() == mgr.GetWorld()->GetCurrentAreaId()) {
      SendStateToRelay(EScriptObjectState::MaxReached, mgr);
    }

    if (xfac_) {
      xfac_->SetParticleEmission(false);
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
  x143c_->RenderShadowBuffer(mgr, *GetModelData(), GetTransform(), 1, zeus::skZero3f, 1.f, 5.f);
  x143c_->Set_x98(0.8f);
}

void CMetroidPrimeExo::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  CPatterned::AddToRenderer(frustum, mgr);

  if (frustum.aabbFrustumTest(*xc50_->GetBounds())) {
    g_Renderer->AddParticleGen(*xc50_);
  }

  if (frustum.aabbFrustumTest(*xfac_->GetBounds())) {
    g_Renderer->AddParticleGen(*xfac_);
  }

  if (frustum.aabbFrustumTest(*x1024_->GetBounds())) {
    g_Renderer->AddParticleGen(*x1024_);
  }

  for (size_t i = 0; i < 2; ++i) {
    if (frustum.aabbFrustumTest(*xfec_[i]->GetBounds())) {
      g_Renderer->AddParticleGen(*xfec_[i]);
    }

    if (x1054_24_) {
      g_Renderer->AddParticleGen(*x1000_[i]);
    }
  }
}

void CMetroidPrimeExo::Render(CStateManager& mgr) {
  g_Renderer->SetGXRegister1Color(x8d8_beamColor);
  CPatterned::Render(mgr);
}

bool CMetroidPrimeExo::CanRenderUnsorted(const CStateManager& mgr) const {
  return mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::XRay;
}

void CMetroidPrimeExo::Touch(CActor& act, CStateManager& mgr) {
  // Empty
}

void CMetroidPrimeExo::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  if (type == EUserEventType::ScreenShake) {
    // TODO: Implement
    return;
  }
  if (type == EUserEventType::EffectOff) {
    xc50_->SetParticleEmission(false);
    return;
  }
  if (type == EUserEventType::EffectOn && x92c_ == 7) {
    xc50_->SetParticleEmission(true);
    if (auto* ent = static_cast<CPlasmaProjectile*>(mgr.ObjectById(xb24_plasmaProjectileIds[x570_]))) {
      zeus::CColor col = ent->GetInnerColor();
      col.a() = 1.f;
      xc50_->SetModulationColor(col);
    }
    return;
  }
  if (type == EUserEventType::DamageOn) {
    if (x92c_ == 11) {
      EnableParticles(mgr, true);
      x1054_26_ = true;
    } else if (x92c_ == 7) {
      FirePlasmaProjectile(mgr, true);
    }
  } else if (type == EUserEventType::DamageOff) {
    if (x92c_ == 1) {
      EnableParticles(mgr, false);
    } else if (x92c_ == 7) {
      FirePlasmaProjectile(mgr, false);
    }
    return;
  }
  if (type == EUserEventType::Projectile) {
    if (x92c_ == 6) {
      auto xf = zeus::lookAt(GetLctrTransform(node.GetLocatorName()).origin + zeus::CVector3f{0.f, 0.f, 1.5f},
                             mgr.GetPlayer().GetTranslation());
      float randAngle = zeus::degToRad(mgr.GetActiveRandom()->Range(-20.f, 20.f));
      xf.rotateLocalZ(randAngle);
      TUniqueId uid = mgr.AllocateUniqueId();
      auto gen1 = g_SimplePool->GetObj(SObjectTag{SBIG('PART'), x930_.x4_particle1});
      auto gen2 = g_SimplePool->GetObj(SObjectTag{SBIG('PART'), x930_.x8_particle2});
      auto gen3 = g_SimplePool->GetObj(SObjectTag{SBIG('PART'), x930_.xc_particle3});
      mgr.AddObject(new CIceAttackProjectile(gen1, gen2, gen3, uid, GetAreaIdAlways(), GetUniqueId(), true, xf,
                                             x930_.x10_dInfo, {-1.f, 1.f}, x930_.x2c_, zeus::degToRad(x930_.x30_),
                                             x930_.x34_texture, x930_.x38_, x930_.x3a_, {}));
    }
  }
  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CMetroidPrimeExo::SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x330_stateMachineState.SetDelay(zeus::clamp(0.2f, x924_ * 0.25f, 1.f));
  } else if (msg == EStateMsg::Update) {
    sub80275800(mgr);
  } else if (msg == EStateMsg::Deactivate) {
    sub802738d4(mgr);
    x1054_27_ = false;
  }
}

void CMetroidPrimeExo::Run(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x92c_ = 10;
    x1084_ = 1.9666666f;
    TUniqueId wpId = GetNextAttackWaypoint(mgr, true);
    if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(wpId)) {
      GetBodyController()->SetLocomotionType(sub80275e14(1));
      x2dc_destObj = wpId;
      SetDestPos(wp->GetTranslation());
      x2ec_reflectedDestPos = GetTranslation();
      x328_24_inPosition = false;
    }
    SetEyesParticleEffectState(mgr, false);
  } else if (msg == EStateMsg::Update) {
    ApproachDest(mgr);
  } else if (msg == EStateMsg::Deactivate) {
    x92c_ = 0;
    x1078_ = 1;
    GetBodyController()->SetLocomotionType(skLocomotions[x1078_]);
    SetEyesParticleEffectState(mgr, true);
    sub802738d4(mgr);
  }
}

void CMetroidPrimeExo::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x92c_ = 6;
    x1084_ = 0.2f;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, sub80275e34(9));
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x92c_ = 0;
    sub802738d4(mgr);
    x1254_ = 2;
  }
}

void CMetroidPrimeExo::TurnAround(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x92c_ = 9;
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Step, &CPatterned::TryStep, 3);
    x450_bodyController->SetLocomotionType(skLocomotions[x1078_]);
    x1078_ = 1;
    zeus::CVector3f vec = sub8027464c(mgr);
    if ((vec - GetTranslation()).normalized().dot(mgr.GetPlayer().GetTranslation() - GetTranslation()) < 15.f) {
      sub802747b8(arg, mgr, vec - GetTranslation());
    }
  } else if (msg == EStateMsg::Deactivate) {
    x92c_ = 0;
    x32c_animState = EAnimState::NotReady;
  }
}

void CMetroidPrimeExo::Active(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x107c_ = 0.4f;
    x1084_ = x1088_;
    x3b4_speed = 1.f;
  } else if (msg == EStateMsg::Update) {
    if ((x570_ != 0 || x1078_ != 1) && x107c_ < 0.f && x1084_ < 0.f) {
      x107c_ = x1080_;
      x1084_ = 0.9f;
      x1078_ = mgr.GetActiveRandom()->Next() % 3;
      GetBodyController()->SetLocomotionType(skLocomotions[x1078_]);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x1088_ = 0.2f;
  }
}

void CMetroidPrimeExo::InActive(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x1084_ = x1080_;
    x1084_ = 0.2f;
    x400_24_hitByPlayerProjectile = false;
    x914_24_ = true;
    SetEyesParticleEffectState(mgr, false);
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
    SetEyesParticleEffectState(mgr, true);
    sub802738d4(mgr);
    x1084_ = x1080_;
    x1088_ = x1084_;
    mgr.SetBossParams(GetUniqueId(), 2860.0f, 91);
    x8d0_ = x8d4_;
  }
}

void CMetroidPrimeExo::CoverAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x92c_ = 1;
    x1084_ = 1.9666666f;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, sub80275e34(0));
    zeus::CVector3f vec = (16.f * GetTransform().frontVector()) + GetTranslation();
    zeus::CVector3f direction = (vec - GetTranslation()).normalized();
    if (direction.dot(mgr.GetPlayer().GetTranslation() - GetTranslation()) < 15.f) {
      sub802747b8(arg, mgr, vec - mgr.GetPlayer().GetTranslation());
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x92c_ = 0;
    x1078_ = 1;
    GetBodyController()->SetLocomotionType(skLocomotions[x1078_]);
  }
}

void CMetroidPrimeExo::Crouch(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, 5);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    sub802738d4(mgr);
  }
}

void CMetroidPrimeExo::Taunt(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Taunt, &CPatterned::TryTaunt, u32(skTaunts[x1078_]));
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

void CMetroidPrimeExo::Suck(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    GetBodyController()->SetLocomotionType(sub80275e14(10));
    x92c_ = 11;
    x1054_25_ = false;
    x1084_ = 0.8f;
    mgr.GetPlayer().AttachActorToPlayer(GetUniqueId(), false);
  } else if (msg == EStateMsg::Deactivate) {
    mgr.GetPlayer().DetachActorFromPlayer();
    x1078_ = 1;
    GetBodyController()->SetLocomotionType(skLocomotions[x1078_]);
    x92c_ = 0;
    EnableParticles(mgr, false);
    sub802738d4(mgr);
    x1088_ = 0.6f;
    if (mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Morphed) {
      mgr.GetPlayer().SetOrbitRequestForTarget(GetUniqueId(), CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
      x402_28_isMakingBigStrike = true;
      x504_damageDur = 0.35f;
      mgr.SendScriptMsgAlways(mgr.GetPlayer().GetUniqueId(), GetUniqueId(), EScriptObjectMessage::Damage);
      mgr.GetPlayer().ApplyImpulseWR(
          60.f * (mgr.GetPlayer().GetMass() * (mgr.GetPlayer().GetTranslation() - GetTranslation()).normalized()), {});
    }
    x1054_27_ = true;
  }
}

void CMetroidPrimeExo::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x92c_ = 7;
    x1088_ = 1.0999999f;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::ProjectileAttack, &CPatterned::TryProjectileAttack, sub80275e34(x1254_));
    if (x32c_animState == EAnimState::Repeat) {
      x1078_ = 1;
      GetBodyController()->SetLocomotionType(skLocomotions[x1078_]);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x92c_ = 0;
    sub802738d4(mgr);
    x1088_ = 1.2166667f;
    xc50_->SetParticleEmission(false);
    FirePlasmaProjectile(mgr, false);
    x1254_ = 2;
  }
}

void CMetroidPrimeExo::Flinch(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    SetEyesParticleEffectState(mgr, false);
    DisableHeadOrbitAndTarget(mgr);
    x8f4_28_ = false;
    x8f4_27_ = false;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::KnockBack, &CPatterned::TryKnockBack_Front, 5);
    if (x428_damageCooldownTimer < 0.25f * 0.33f) {
      x428_damageCooldownTimer = 0.33f;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    SetEyesParticleEffectState(mgr, true);
    x1078_ = 1;
    GetBodyController()->SetLocomotionType(skLocomotions[x1078_]);
    EnableHeadOrbitAndTarget(mgr);
  }
}

void CMetroidPrimeExo::Dodge(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x1078_ = 1;
    GetBodyController()->SetLocomotionType(skLocomotions[x1078_]);
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Step, &CPatterned::TryStep, 0);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

void CMetroidPrimeExo::Retreat(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    SendStateToRelay(EScriptObjectState::Zero, mgr);
    if (TCastToConstPtr<CScriptWaypoint> wp =
            mgr.GetObjectById(GetWaypointForBehavior(mgr, EScriptObjectState::CloseIn, EScriptObjectMessage::Follow))) {
      SetTransform(wp->GetTransform());
    }
    x1078_ = 1;
    GetBodyController()->SetLocomotionType(skLocomotions[x1078_]);
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Scripted, &CPatterned::TryScripted, x918_);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    if (TCastToConstPtr<CScriptWaypoint> wp =
            mgr.GetObjectById(GetWaypointForBehavior(mgr, EScriptObjectState::Retreat, EScriptObjectMessage::Follow))) {
      SetTransform(wp->GetTransform());
    }
    ++x91c_;
  }
}

void CMetroidPrimeExo::Cover(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x92c_ = 12;
    x1084_ = 1.2666667f;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::ProjectileAttack, &CPatterned::TryProjectileAttack, sub80275e34(13));
    if (x32c_animState == EAnimState::Repeat) {
      x1078_ = 1;
      GetBodyController()->SetLocomotionType(skLocomotions[x1078_]);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x92c_ = 0;
    sub802738d4(mgr);
    x1254_ = 2;
  }
}

void CMetroidPrimeExo::Approach(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, 2);
    if (x32c_animState == EAnimState::Repeat) {
      x1078_ = 1;
      GetBodyController()->SetLocomotionType(skLocomotions[x1078_]);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    SetEyesParticleEffectState(mgr, true);
    sub802738d4(mgr);
  }
}

void CMetroidPrimeExo::Enraged(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg != EStateMsg::Activate) {
    return;
  }
  sub802786fc(mgr);
}

void CMetroidPrimeExo::SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    if (x1254_ == 2) {
      x92c_ = 2;
    } else if (x1254_ == 3) {
      x92c_ = 3;
    } else if (x1254_ == 4) {
      x92c_ = 4;
    } else if (x1254_ == 5) {
      x92c_ = 5;
    }
    x1084_ = 1.2666667f;
    sub80274054(mgr);
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::ProjectileAttack, &CPatterned::TryProjectileAttack, sub80275e34(x1254_));
    if (x32c_animState == EAnimState::Repeat) {
      x1078_ = 1;
      GetBodyController()->SetLocomotionType(skLocomotions[x1078_]);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x92c_ = 0;
    sub802738d4(mgr);
  }
}

void CMetroidPrimeExo::Growth(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg != EStateMsg::Activate) {
    return;
  }
  x3b4_speed = 1.4f;
}

void CMetroidPrimeExo::Land(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg != EStateMsg::Activate) {
    return;
  }

  sub802786fc(mgr);
}

bool CMetroidPrimeExo::TooClose(CStateManager& mgr, float arg) { return CPatterned::TooClose(mgr, arg); }

bool CMetroidPrimeExo::InMaxRange(CStateManager& mgr, float arg) {
  return CPatterned::InMaxRange(mgr, arg) || (sub80277224(3.f, mgr) && sub80277224(7.f, mgr) && sub80277224(15.f, mgr));
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

bool CMetroidPrimeExo::AttackOver(CStateManager& mgr, float arg) { return x8f4_28_ || x8f4_27_ || x1054_25_; }

bool CMetroidPrimeExo::ShouldFire(CStateManager& mgr, float arg) { return x1254_ == 6 || x1254_ == 7 || x1254_ == 8; }

bool CMetroidPrimeExo::ShouldFlinch(CStateManager& mgr, float arg) { return x8f4_27_; }

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

CProjectileInfo* CMetroidPrimeExo::GetProjectileInfo() {
  if (x92c_ == 5) {
    return &xd74_;
  }
  if (x92c_ < 5 && x92c_ > 1) {
    return &xc78_;
  }

  return nullptr;
}

void CMetroidPrimeExo::sub802738d4(CStateManager& mgr) { x920_ = mgr.GetActiveRandom()->Range(x924_, x928_); }

void CMetroidPrimeExo::UpdateEnergyBalls(float dt, CStateManager& mgr) {
  if (x1074_ > 0.f) {
    for (size_t i = 0; i < x106c_energyBallIds.size(); ++i) {
      if (auto* ball = CPatterned::CastTo<CEnergyBall>(mgr.ObjectById(x106c_energyBallIds[i]))) {
        ball->SetTransform(GetLctrTransform(skDrillerLocators[i]));
      }
    }
  } else {
    x106c_energyBallIds.clear();
  }
}

u32 CMetroidPrimeExo::CountEnergyBalls(CStateManager& mgr) {
  u32 ret = 0;
  for (auto* ent : mgr.GetPhysicsActorObjectList()) {
    if (CPatterned::CastTo<CEnergyBall>(ent) != nullptr && ent->GetAreaIdAlways() == GetAreaIdAlways() &&
        ent->GetActive()) {
      ++ret;
    }
  }
  return ret;
}

void CMetroidPrimeExo::DeactivatePatrolObjects(CStateManager& mgr) {
  const TCastToConstPtr<CMetroidPrimeRelay> relay = mgr.GetObjectById(x568_relayId);
  x1058_.clear();
  if (relay) {
    for (const auto& conn : relay->GetConnectionList()) {
      if (conn.x0_state != EScriptObjectState::Patrol) {
        continue;
      }
      if (auto* ent = mgr.ObjectById(mgr.GetIdForScript(conn.x8_objId))) {
        ent->AcceptScriptMsg(EScriptObjectMessage::Deactivate, GetUniqueId(), mgr);
        x1058_.push_back(conn.x8_objId);
        if (x1058_.size() >= 4) {
          break;
        }
      }
    }
  }
}

void CMetroidPrimeExo::UpdatePhysicsDummy(CStateManager& mgr) {
  if (TCastToPtr<CPhysicsActor> physAct = mgr.ObjectById(xeac_)) {
    zeus::CVector3f diffVec = mgr.GetPlayer().GetTranslation() - GetTranslation();
    if (!zeus::close_enough(diffVec, zeus::skZero3f)) {
      zeus::CVector3f direction = diffVec.normalized();
      physAct->SetVelocityWR((direction.dot(GetTransform().frontVector()) > 0.f ? 7.5f : 5.f) * direction);
    } else {
      physAct->SetVelocityWR(zeus::skZero3f);
    }
  }
}

void CMetroidPrimeExo::sub80274054(CStateManager& mgr) {
  if (TCastToPtr<CActor> act = mgr.ObjectById(xeac_)) {
    act->SetTranslation(mgr.GetPlayer().GetTranslation());
  }
}

void CMetroidPrimeExo::sub802740cc(CStateManager& mgr) {}

void CMetroidPrimeExo::CreatePhysicsDummy(CStateManager& mgr) {
  xeac_ = mgr.AllocateUniqueId();
  mgr.AddObject(new CPhysicsDummy(xeac_, true, ""sv, CEntityInfo(GetAreaIdAlways(), NullConnectionList)));
}

void CMetroidPrimeExo::SetBillboardEmission(CStateManager& mgr, bool emission) {
  if (TCastToPtr<CHUDBillboardEffect> billboard = mgr.ObjectById(x1044_billboardId)) {
    billboard->GetParticleGen()->SetParticleEmission(emission);
  }
}

void CMetroidPrimeExo::FreeBillboard(CStateManager& mgr) { mgr.FreeScriptObject(x1044_billboardId); }

zeus::CVector3f CMetroidPrimeExo::sub8027464c(CStateManager& mgr) {
  TUniqueId uid = GetWaypointForBehavior(mgr, EScriptObjectState::Attack, EScriptObjectMessage::Follow);

  float dVar4 = 0.f;
  zeus::CVector3f tmpVec;
  for (; uid != kInvalidUniqueId;) {
    auto wp = TCastToConstPtr<CScriptWaypoint>(mgr.GetObjectById(uid));
    if (wp) {
      tmpVec += wp->GetTranslation();
      dVar4 += 1.f;
    }
    uid = wp->NextWaypoint(mgr);
  }

  if (dVar4 <= 0.f) {
    return zeus::skZero3f;
  }

  return (1.f / dVar4) * tmpVec;
}

void CMetroidPrimeExo::CreateHUDBillBoard(CStateManager& mgr) {
  x1044_billboardId = mgr.AllocateUniqueId();
  mgr.AddObject(new CHUDBillboardEffect(
      {x101c_}, std::nullopt, x1044_billboardId, true, ""sv, CHUDBillboardEffect::GetNearClipDistance(mgr),
      CHUDBillboardEffect::GetScaleForPOV(mgr), zeus::skWhite, zeus::skOne3f, zeus::skZero3f));
}

void CMetroidPrimeExo::sub802747b8(float f1, CStateManager& mgr, const zeus::CVector3f& vec) {
  if (mgr.RayCollideWorld(mgr.GetPlayer().GetTranslation(), mgr.GetPlayer().GetTranslation() + (0.2f * zeus::skDown),
                          CMaterialFilter::MakeInclude({EMaterialTypes::Floor, EMaterialTypes::Platform}), this)) {
    mgr.GetPlayer().ApplyImpulseWR((10.f * mgr.GetPlayer().GetMass()) * zeus::skUp, {});
    mgr.GetPlayer().SetMoveState(CPlayer::EPlayerMovementState::ApplyJump, mgr);
  }
  zeus::CVector3f vec2 = vec;
  vec2.z() = 0.f;
  if (!zeus::close_enough(vec2, zeus::skZero3f)) {
    mgr.GetPlayer().ApplyImpulseWR(f1 * (mgr.GetPlayer().GetMass() * (120.f * vec2)), {});
    mgr.GetPlayer().UseCollisionImpulses();
    mgr.GetPlayer().SetAccelerationChangeTimer(2.f * f1);
  }
}

void CMetroidPrimeExo::sub802749e8(float f1, float f2, float f3, const zeus::CVector3f& vec1,
                                   const zeus::CVector3f& vec2, s32 idx) {
  auto diffVec = vec2 - vec1;
  const float dist = diffVec.magnitude();
  const auto& elemGen = xfec_[idx];
  const auto& swooshGen = x1000_[idx];

  if (!zeus::close_enough(diffVec, zeus::skZero3f)) {
    zeus::CVector3f v1 = vec1;
    auto lookAtXf = zeus::lookAt(zeus::skZero3f, diffVec);
    elemGen->SetParticleEmission(true);
    s32 count = static_cast<s32>(2.f * dist + 1.f);
    for (s32 i = 0; i < count; ++i) {
      float dVar14 = i * static_cast<int>(1.f / static_cast<float>(count));
      float dVar11 = f1 * (dVar14 * std::cos(static_cast<float>(i) + f3));
      float fVar15 = static_cast<float>(std::sin(i));
      const auto v = (i < 1) ? zeus::skZero3f : lookAtXf * zeus::CVector3f{dVar11, 0.f, f2 * (dVar14 * fVar15)};
      elemGen->SetTranslation(v1 + v);
      elemGen->ForceParticleCreation(1);
      v1 += v1 + diffVec;
    }

    elemGen->SetParticleEmission(false);

    auto& swooshes = swooshGen->GetSwooshes();
    count = swooshes.size() - 1;
    v1 = vec1;
    float fVar14 = vec1.x();
    float fVar1 = vec1.y();
    float fVar2 = vec1.z();
    float fVar3 = swooshes[count].x30_irot;
    float dVar12 = 1.f / static_cast<float>(count);
    diffVec = diffVec * dVar12;
    for (s32 i = 0; i <= count; ++i) {
      float vec1Z = fVar3;
      float dVar13 = fVar2;
      float vec1X = fVar1;
      float vec1Y = fVar14;
      float dVar9 = static_cast<float>(i) * dVar12;
      fVar14 = std::cos(static_cast<float>(i) + f3);
      float dVar11 = f1 * (dVar9 * fVar14);
      fVar14 = std::sin(i);
      const auto v = (i < 1) ? zeus::skZero3f : lookAtXf * zeus::CVector3f{dVar11, 0.f, f2 * (dVar9 * fVar14)};

      swooshes[i].xc_translation = {vec1Y + v.x(), vec1X + fVar14, dVar13 + fVar1};
      swooshes[i].x38_orientation = lookAtXf;
      fVar3 = swooshes[i].x30_irot;
      fVar14 = vec1Y + diffVec.x();
      fVar1 = vec1X + diffVec.y();
      swooshes[i].x30_irot = vec1Z;
      fVar2 = dVar13 + diffVec.z();
    }
  }
}

void CMetroidPrimeExo::UpdateParticles(float f1, CStateManager& mgr) {
  if (GetBodyController()->GetPercentageFrozen() > 0.f && x1054_24_) {
    EnableParticles(mgr, false);
    x1054_25_ = true;
  }

  constexpr std::array<std::string_view, 2> EyeLocators{{
      "L_eye_3"sv,
      "R_eye_3"sv,
  }};
  float dVar9 = 0.f;
  for (size_t i = 0; i < 2; ++i) {
    x102c_[i] -= f1;
    x1038_[i] += f1;
    zeus::CTransform xf = GetLctrTransform(EyeLocators[i]);
    zeus::CVector3f off =
        (mgr.GetPlayer().GetTranslation() + (g_tweakPlayer->GetPlayerBallHalfExtent() * zeus::skUp) +
         (g_tweakPlayer->GetPlayerBallHalfExtent() * xf.origin - mgr.GetPlayer().GetTranslation()).normalized());
    if (x1054_24_) {
      float dVar13 = zeus::clamp(0.f, 2.f * (x102c_[i] / 0.3f) - 1.f, 1.f);
      float dVar12 = 1.f - dVar13;
      float tmp = zeus::clamp(0.f, 0.5f + (x102c_[i] / 0.3f), 1.f);
      zeus::CVector3f local_154 = off * dVar12 + (xf.origin * dVar13);
      sub802749e8(2.f * tmp, 0.5f * tmp, x1038_[i], xf.origin, local_154, i);
      if (tmp <= 0.f) {
        if (mgr.RayCollideWorld(xf.origin, local_154,
                                CMaterialFilter::MakeIncludeExclude(
                                    {EMaterialTypes::Solid}, {EMaterialTypes::Player, EMaterialTypes::CollisionActor}),
                                this)) {
          x1054_25_ = true;
        }
      } else if (dVar13 <= 0.f) {
        dVar9 += 1.f - tmp;
      }
      x1024_->SetTranslation(local_154);
    } else if (CParticleSwoosh::GetAliveParticleSystemCount() != 0) {
      float tmp1 = zeus::clamp(0.f, 3.f * (x102c_[i] / 0.3f) - 2.f, 1.f);
      float tmp2 = zeus::clamp(0.f, 0.5f + (x102c_[i] / 0.3f), 1.f);
      sub802749e8(tmp2, tmp2, x1038_[i], xf.origin, xf.origin * (1.f - tmp1) + (off * tmp1), i);
      x1000_[i]->Update(f1);
    }
    xfec_[i]->Update(f1);
  }
  x1024_->Update(f1);
  if ((0.5f * dVar9) > 0.9f && x1054_24_) {
    x1024_->SetParticleEmission(true);
    SetBillboardEmission(mgr, true);
  }

  if (zeus::close_enough(0.f, dVar9)) {
    return;
  }

  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed) {
    if (mgr.GetPlayer().GetMorphBall()->IsBoosting()) {
      x1054_25_ = true;
    }
    if (mgr.GetPlayer().GetAttachedActorStruggle() == 1.f) {
      x1054_25_ = true;
    }
  }
  zeus::CTransform jawXf = GetLctrTransform("Jaw_1"sv);
  zeus::CVector3f direction = (jawXf.origin - mgr.GetPlayer().GetTranslation()).normalized();

  float offX = (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed ? 31.49f
                : mgr.GetPlayer().GetOrbitState() == CPlayer::EPlayerOrbitState::NoOrbit                 ? 136.5f
                                                                                                         : 20.4749f);
  if ((x1054_26_ && dVar9 > 0.75f) || x1048_ > 0.f) {
    if (!x1054_26_) {
      x1048_ -= f1;
    } else {
      x1054_26_ = false;
      x1048_ = std::sqrt(1.5f / GravityConstant());
    }
    mgr.GetPlayer().ApplyImpulseWR(f1 * (mgr.GetPlayer().GetMass() * std::sqrt(1.5f * GravityConstant()) * zeus::skUp),
                                   {});
    mgr.GetPlayer().SetMoveState(CPlayer::EPlayerMovementState::ApplyJump, mgr);
  }

  mgr.GetPlayer().ApplyImpulseWR(f1 * ((0.5f * dVar9) * offX + direction), {});
  mgr.GetPlayer().UseCollisionImpulses();
  mgr.GetPlayer().SetAccelerationChangeTimer(2.f * f1);
}

void CMetroidPrimeExo::EnableParticles(CStateManager& mgr, bool b1) {
  for (size_t i = 0; i < 2; ++i) {
    x1000_[i]->SetParticleEmission(b1);
    if (!b1) {
      x1024_->SetParticleEmission(false);
      SetBillboardEmission(mgr, false);
    } else if (x1054_24_ != b1) {
      x102c_[i] = 0.3f;
      for (size_t j = 0; j < x1000_[i]->GetSwooshes().size(); ++j) {
        x1000_[i]->ForceOneUpdate(0.f);
      }
    }
  }
  x1054_24_ = b1;
}

void CMetroidPrimeExo::EnableHeadOrbitAndTarget(CStateManager& mgr) {
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x8cc_headColActor)) {
    colAct->AddMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
  }
}

void CMetroidPrimeExo::DisableHeadOrbitAndTarget(CStateManager& mgr) {
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x8cc_headColActor)) {
    colAct->RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
  }
}

void CMetroidPrimeExo::UpdateTimers(float dt) {
  if (GetBodyController()->GetPercentageFrozen() != 0.f) {
    return;
  }

  x107c_ -= dt;
  x1084_ -= dt * GetModelData()->GetAnimationData()->GetSpeedScale();
  x920_ -= dt;
}

void CMetroidPrimeExo::sub80275800(CStateManager& mgr) {
  TUniqueId tmpId = GetNextAttackWaypoint(mgr, true);

  u32 flags = 0x13c1;
  if (tmpId != kInvalidUniqueId) {
    if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(tmpId)) {
      if ((wp->GetTranslation() - GetTranslation()).magSquared() > 16.f) {
        flags = 0x13c3;
      }
    }
  }

  if (!x1054_27_) {
    flags |= 0x400;
  }
  if (GetTransform().frontVector().dot(mgr.GetPlayer().GetTranslation() - GetTranslation()) > 30.f) {
    flags |= 0x3c;
  }

  if (CountEnergyBalls(mgr) == 0) {
    flags |= 0x2000;
  }

  if (sub80277224(7.f, mgr)) {
    flags |= 0x800;
  }

  sub802759a8(mgr, flags);
}

void CMetroidPrimeExo::sub802759a8(CStateManager& mgr, u32 w1) {
  float dVar7 = 0.f;
  const auto& parms = x1160_[x570_];
  for (size_t i = 0; i < 14; ++i) {
    if (((w1 & (1 << i)) != 0u) && sub80275d68(i)) {
      dVar7 += sub80275b04(parms, i);
    }
  }

  if (dVar7 > 30000.0f) {
    sub80275b68();
  }

  dVar7 = mgr.GetActiveRandom()->Range(0.f, dVar7);
  x1254_ = -1;
  float dVar5 = 0.f;
  for (size_t i = 0; i < 13; ++i) {
    if (((w1 & (1 << i)) != 0u) && sub80275d68(i)) {
      float dVar6 = sub80275b04(parms, i);
      if ((dVar5 < dVar7) && (dVar7 < (dVar5 + dVar6))) {
        x1254_ = i;
        x1258_[i] += 3.f;
        break;
      }

      dVar5 += dVar6;
    }
  }
}

float CMetroidPrimeExo::sub80275b04(const CMetroidPrimeAttackWeights& roomParms, int w2) {
  float dVar1 = 0.f;
  if (!zeus::close_enough(0.f, x1258_[w2])) {
    const float tmpFloat = roomParms.GetFloatValue(w2);
    dVar1 = (tmpFloat * tmpFloat) / x1258_[w2];
  }

  return dVar1;
}

void CMetroidPrimeExo::sub80275b68() {
  float fVar9 = 0.f;
  for (float f : x1258_) {
    fVar9 += f;
  }

  if (zeus::close_enough(fVar9, 0.f)) {
    return;
  }

  for (float& f : x1258_) {
    f /= fVar9;
  }
}

void CMetroidPrimeExo::sub80275c60(CStateManager& mgr, int w1) {
  if (x570_ == -1) {
    return;
  }

  x1258_.clear();

  for (size_t i = 0; i < 14; ++i) {
    x1258_.push_back(x1160_[x570_].GetFloatValue(i));
  }

  if (x1078_ == -1) {
    return;
  }

  for (size_t i = 0; i < 40; ++i) {
    sub802759a8(mgr, -1);
  }
}

bool CMetroidPrimeExo::sub80275d68(int w1) {
  // TODO(antidote): Simplify expressions and rename globals
  const s32 iVar1 = skSomeValues1[w1];
  if (iVar1 == 7 || iVar1 == 18) {
    return (-1 - ((skSomeMeleeValues[w1][x1078_] >> 24) | (((skSomeMeleeValues[w1][x1078_] + 1) >> 24) >> 7))) != 0;
  }
  if (iVar1 == 5) {
    return (-1 - ((int(skSomeValues2[w1][x1078_]) >> 24) | (((int(skSomeValues2[w1][x1078_]) + 1) >> 24) >> 7))) != 0;
  }

  return iVar1 == 17;
}

pas::ELocomotionType CMetroidPrimeExo::sub80275e14(int w1) { return skSomeValues2[w1][x1078_]; }

u32 CMetroidPrimeExo::sub80275e34(int w1) const { return skSomeMeleeValues[w1][x1078_]; }

void CMetroidPrimeExo::UpdateElectricEffect(float dt, CStateManager& mgr) {
  if (!xfac_) {
    return;
  }

  xfac_->SetGlobalOrientation(GetTransform().getRotation());
  xfac_->SetGlobalTranslation(GetTranslation());
  xfac_->SetGlobalScale(GetModelData()->GetScale());
  if (xfc0_) {
    CSfxManager::UpdateEmitter(xfbc_, GetTranslation(), zeus::skZero3f, 1.f);
    xfac_->SetParticleEmission(true);
    const auto* animData = GetModelData()->GetAnimationData();
    for (size_t i = 0; i < 4; ++i) {
      xfac_->SetOverrideIPos(
          animData
              ->GetLocatorTransform(animData->GetLocatorSegId(skLegLocators[mgr.GetActiveRandom()->Range(0, 19)]),
                                    nullptr)
              .origin);
      xfac_->SetOverrideFPos(
          animData
              ->GetLocatorTransform(animData->GetLocatorSegId(skLegLocators[mgr.GetActiveRandom()->Range(0, 19)]),
                                    nullptr)
              .origin);
      xfac_->ForceParticleCreation(1);
    }

    xfac_->SetParticleEmission(false);
    xfb4_ -= dt;
    if (xfb4_ <= 0.f) {
      sub80276204(mgr, false);
    }
  }
  xfac_->Update(dt);
}

void CMetroidPrimeExo::UpdateSfxEmitter(float f1, CStateManager& mgr) {
  if (!xfc1_) {
    return;
  }

  xfb8_ -= f1;
  if (xfb8_ <= 0.f) {
    sub8027639c(mgr, false);
  }

  if (xfbc_) {
    CSfxManager::UpdateEmitter(xfbc_, GetTranslation(), zeus::skZero3f, 1.f);
  }
}

void CMetroidPrimeExo::sub80276204(CStateManager& mgr, bool b1) {
  if (b1 && xfc1_) {
    sub8027639c(mgr, false);
  }

  xfc0_ = b1;
  if (!b1) {
    CSfxManager::RemoveEmitter(xfbc_);
    xfbc_.reset();
    GetBodyController()->GetCommandMgr().DeliverCmd(CBodyStateCmd{EBodyStateCmd::StopReaction});
  } else {
    if (xfbc_) {
      xfbc_.reset();
    }
    xfbc_ = CSfxManager::AddEmitter(SFXsfx0519, GetTranslation(), zeus::skZero3f, 1.f, true, true, 127, kInvalidAreaId);
    GetBodyController()->GetCommandMgr().DeliverCmd(
        CBCAdditiveReactionCmd(pas::EAdditiveReactionType::Electrocution, 1.f, true));
  }
}

void CMetroidPrimeExo::sub8027639c(CStateManager& mgr, bool b1) {
  if (b1 && xfc0_) {
    sub80276204(mgr, false);
  }

  for (size_t i = 0; i < 4; ++i) {
    GetModelData()->GetAnimationData()->SetParticleEffectState(skEffectNames[i], b1, mgr);
  }

  xfc1_ = b1;
  if (!b1) {
    CSfxManager::RemoveEmitter(xfbc_);
    xfbc_.reset();
  } else {
    if (xfbc_) {
      xfbc_.reset();
    }
    xfbc_ = CSfxManager::AddEmitter(SFXsfx051A, GetTranslation(), zeus::skZero3f, 1.f, true, true, 127, kInvalidAreaId);
  }
}

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

    for (const auto& uid : xb24_plasmaProjectileIds) {
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

void CMetroidPrimeExo::GetRelayState(CStateManager& mgr) {
  x1160_.clear();
  if (TCastToConstPtr<CMetroidPrimeRelay> relay = mgr.GetObjectById(x568_relayId)) {
    x1160_ = relay->GetRoomParameters();
    x8c0_ = relay->GetHealthInfo1();
    x924_ = relay->Get_xc84();
    x928_ = relay->Get_xc88();
    x1080_ = relay->Get_xc8c();
    x1440_ = relay->Get_xc90();
    x918_ = relay->Get_xcac();
    x584_ = relay->Get_xc94();
    x574_ = relay->Get_xc98();
    x8d4_ = relay->Get_xcb0();
    x57c_ = relay->Get_xcb4();
    sub80275c60(mgr, relay->Get_xcb4());
  }
}

TUniqueId CMetroidPrimeExo::GetNextAttackWaypoint(CStateManager& mgr, bool b1) {
  TUniqueId uid = GetWaypointForBehavior(mgr, EScriptObjectState::Attack, EScriptObjectMessage::Follow);
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

TUniqueId CMetroidPrimeExo::GetWaypointForBehavior(CStateManager& mgr, EScriptObjectState state,
                                                   EScriptObjectMessage msg) {
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
        if (uids.size() >= 8) {
          break;
        }
      }
    }

    if (!uids.empty()) {
      return uids[mgr.GetActiveRandom()->Next() % uids.size()];
    }
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

  GetRelayState(mgr);
  DeactivatePatrolObjects(mgr);
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
  TUniqueId uid = GetNextAttackWaypoint(mgr, f1 >= 0.f);

  if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(uid)) {
    const float scaleMag = f1 * (0.57735026 * GetModelData()->GetScale().magnitude());
    const float dist = GetTransform().frontVector().dot(wp->GetTranslation() - GetTranslation());
    return f1 < 0.f ? dist < scaleMag : dist > scaleMag;
  }
  return false;
}

void CMetroidPrimeExo::FirePlasmaProjectile(CStateManager& mgr, bool b1) {
  if (b1) {
    xc58_curPlasmaProjectile = x570_;
    if (auto* proj =
            static_cast<CPlasmaProjectile*>(mgr.ObjectById(xb24_plasmaProjectileIds[xc58_curPlasmaProjectile]))) {
      if (proj->GetDamageInfo().GetWeaponMode().GetType() == EWeaponType::Power) {
        proj->AddAttrib(EProjectileAttrib::BigStrike);
        proj->SetDamageDuration(1.4f);
      }

      xc60_ = GetTargetVector(mgr);
      xc6c_ = xc60_;
      xc5c_ = 0.f;
      proj->Fire(zeus::lookAt(GetLctrTransform("Jaw_1"sv).origin, xc60_), mgr, false);
    }
  } else {
    for (const auto& plasmaId : xb24_plasmaProjectileIds) {
      if (auto* proj = static_cast<CPlasmaProjectile*>(mgr.ObjectById(plasmaId))) {
        if (proj->IsFiring()) {
          proj->ResetBeam(mgr, false);
        }
      }
    }
  }
}

void CMetroidPrimeExo::UpdatePlasmaProjectile(float dt, CStateManager& mgr) {
  zeus::CTransform jawXf = GetLctrTransform("Jaw_1"sv);
  xc50_->SetTranslation(jawXf.origin);
  xc50_->SetOrientation(jawXf.getRotation());
  xc50_->Update(dt);
  if (xc58_curPlasmaProjectile >= 0 && xc58_curPlasmaProjectile < 4) {
    if (auto* ent =
            static_cast<CPlasmaProjectile*>(mgr.ObjectById(xb24_plasmaProjectileIds[xc58_curPlasmaProjectile]))) {
      if (!ent->GetActive()) {
        return;
      }

      if (GetBodyController()->GetPercentageFrozen() > 0.f) {
        FirePlasmaProjectile(mgr, false);
      }

      zeus::CTransform xf;
      xc5c_ = zeus::clamp(0.f, xc5c_ + dt, 1.4f);
      const float fVar1 = xc5c_ / 1.f;
      const float fVar2 = 1.f - fVar1;
      zeus::CVector3f vec1 = xc60_ * fVar2 + xc6c_ * fVar1;
      zeus::CVector3f vec2 = vec1 - jawXf.origin;
      if (vec2.normalized().dot(GetTransform().frontVector()) <= zeus::degToRad(40.f)) {
        xf = (zeus::CQuaternion::lookAt(GetTransform().frontVector(), vec2.normalized(), zeus::degToRad(45.f)) *
              zeus::CQuaternion(GetTransform().buildMatrix3f()))
                 .toTransform();
      } else {
        xf = zeus::lookAt(jawXf.origin, vec1);
      }

      ent->UpdateFx(xf, dt, mgr);
    }
  }
}

zeus::CVector3f CMetroidPrimeExo::GetTargetVector(CStateManager& mgr) {
  constexpr auto MatFilter = CMaterialFilter::MakeIncludeExclude(
      {EMaterialTypes::Solid}, {EMaterialTypes::Character, EMaterialTypes::Player, EMaterialTypes::Projectile});
  EntityList nearList;
  mgr.BuildNearList(nearList, GetTranslation(), zeus::skDown, 150.f, MatFilter, this);

  TUniqueId uid = kInvalidUniqueId;
  CRayCastResult res =
      mgr.RayWorldIntersection(uid, mgr.GetPlayer().GetTranslation(), zeus::skDown, 150.f, MatFilter, nearList);
  const auto pos = 0.5f * (mgr.GetPlayer().GetAimPosition(mgr, 0.f) + mgr.GetPlayer().GetTranslation());
  if (res.IsInvalid()) {
    return pos;
  }

  return res.GetPoint() + zeus::CVector3f{0.f, 0.f, pos.z() - mgr.GetPlayer().GetTranslation().z()};
}

void CMetroidPrimeExo::FreePlasmaProjectiles(CStateManager& mgr) {
  for (auto& uid : xb24_plasmaProjectileIds) {
    mgr.FreeScriptObject(uid);
    uid = kInvalidUniqueId;
  }
}

void CMetroidPrimeExo::CreatePlasmaProjectiles(CStateManager& mgr) {
  xc50_->SetParticleEmission(false);
  xc50_->SetGlobalScale(GetModelData()->GetScale());

  for (size_t i = 0; i < x96c_.size(); ++i) {
    xb24_plasmaProjectileIds[i] = mgr.AllocateUniqueId();
    CDamageInfo dInfo = xa80_[i].GetDamage();
    dInfo.SetWeaponMode(CWeaponMode(EWeaponType::PoisonWater));
    EProjectileAttrib flags = EProjectileAttrib::PlayerUnFreeze;
    if (xa80_[i].GetDamage().GetWeaponMode().GetType() == EWeaponType::Ice) {
      flags = EProjectileAttrib::None;
    }
    mgr.AddObject(new CPlasmaProjectile(xa80_[i].Token(), ""sv, xa80_[i].GetDamage().GetWeaponMode().GetType(),
                                        x96c_[i], {}, EMaterialTypes::Character, dInfo, xb24_plasmaProjectileIds[i],
                                        GetAreaIdAlways(), GetUniqueId(), xb30_[i], true, flags));
  }
}

void CMetroidPrimeExo::UpdateContactDamage(CStateManager& mgr) {
  const auto* mData = GetModelData();
  zeus::CVector3f max = GetTranslation() + zeus::CVector3f{(0.57735026f * mData->GetScale().magnitude()) * 6.f,
                                                           (0.57735026f * mData->GetScale().magnitude()) * 6.f,
                                                           (0.57735026f * mData->GetScale().magnitude()) * 5.5f};
  zeus::CVector3f min = GetTranslation() + zeus::CVector3f{(0.57735026f * mData->GetScale().magnitude()) * -6.f,
                                                           (0.57735026f * mData->GetScale().magnitude()) * -6.f,
                                                           (0.57735026f * mData->GetScale().magnitude()) * 2.f};
  x8f8_ = zeus::CAABox{min, max};

  if (mgr.GetPlayer().GetTouchBounds()->intersects(x8f8_) && x420_curDamageRemTime <= 0.f) {
    mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), GetContactDamage(),
                    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
    x420_curDamageRemTime = x424_damageWaitTime;
  }
}

void CMetroidPrimeExo::UpdateColorChange(float f1, CStateManager& mgr) {
  if (!x8f4_24_) {
    return;
  }

  if (x8e4_ >= 1.f) {
    x8d8_beamColor = x8e0_;
    x8f4_24_ = false;
    GetModelData()->GetAnimationData()->SetParticleEffectState("ColorChange"sv, false, mgr);
  } else {
    x8e4_ = std::max(1.f, x8e4_ + f1 / 0.3f);
    x8d8_beamColor = zeus::CColor::lerp(x8dc_, x8e0_, x8e4_);
  }
}

void CMetroidPrimeExo::sub80278130(const zeus::CColor& col) {
  x8e4_ = 0.f;
  x8f4_24_ = true;
  x8e0_ = col;
  x8dc_ = x8d8_beamColor;
}

void CMetroidPrimeExo::UpdateHeadAnimation(float f1) {
  if (x8e8_headUpAdditiveBodyAnimIndex == -1) {
    return;
  }
  if (!x8f4_25_) {
    if (x8ec_ > 0.f) {
      x8ec_ = std::min(0.f, x8ec_ - f1 / 0.1f);
    }
  } else if (x8ec_ < 1.f) {
    x8ec_ = std::max(1.f, x8ec_ + f1 / 0.5f);
  }

  if (x8ec_ > 0.f || x8f4_26_) {
    if (x8ec_ <= FLT_EPSILON) {
      GetModelData()->GetAnimationData()->DelAdditiveAnimation(x8e8_headUpAdditiveBodyAnimIndex);
      x8f4_26_ = false;
    } else {
      GetModelData()->GetAnimationData()->AddAdditiveAnimation(x8e8_headUpAdditiveBodyAnimIndex, x8ec_, true, false);
      x8f4_26_ = true;
    }
  }
}

void CMetroidPrimeExo::sub8027827c(TUniqueId uid, CStateManager& mgr) {
  if (uid == x8cc_headColActor) {
    if (TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(uid)) {
      if (!IsAlive()) {
        return;
      }

      if (TCastToConstPtr<CWeapon> wp = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
        if (colAct->GetDamageVulnerability()->WeaponHurts(wp->GetDamageInfo().GetWeaponMode(), false)) {
          x428_damageCooldownTimer = 0.33f;
          if (wp->GetDamageInfo().GetWeaponMode().GetType() == EWeaponType::Ice) {
            if (TCastToPtr<CCollisionActor> wat = mgr.ObjectById(uid)) {
              wat->HealthInfo(mgr)->SetHP(wat->HealthInfo(mgr)->GetHP() -
                                          (0.5f * (x8c8_ - wat->GetHealthInfo(mgr)->GetHP())));
            }
          }
          if (wp->GetDamageInfo().GetWeaponMode().GetType() == EWeaponType::Wave &&
              (wp->GetDamageInfo().GetWeaponMode().IsCharged() || wp->GetDamageInfo().GetWeaponMode().IsComboed())) {
            xfb4_ = 1.5f;
            sub80276204(mgr, true);
          }

          if (wp->GetDamageInfo().GetWeaponMode().GetType() == EWeaponType::Plasma &&
              (wp->GetDamageInfo().GetWeaponMode().IsCharged() || wp->GetDamageInfo().GetWeaponMode().IsComboed())) {
            xfb8_ = 1.5f;
            sub8027639c(mgr, true);
            xfc1_ = true;
          }

          if (wp->GetDamageInfo().GetWeaponMode().GetType() == EWeaponType::Ice &&
              wp->GetDamageInfo().GetWeaponMode().IsComboed()) {
            Freeze(mgr, zeus::skZero3f, colAct->GetTranslation() - wp->GetTranslation(), 2.f);
          }
        }
      }
    }
  }
}

void CMetroidPrimeExo::sub80278508(CStateManager& mgr, int w1, bool b1) {
  if (x570_ != w1) {
    GetModelData()->GetAnimationData()->SetParticleEffectState("ColorChange", true, mgr);

    CAudioSys::C3DEmitterParmData emitterData{
        GetTranslation(), zeus::skZero3f, 1000.f, 0.1f, 1, SFXsfx0B9A, 1.f, 0.16f, false, 127};
    CSfxManager::AddEmitter(emitterData, true, 127, false, GetAreaIdAlways());
  }
  x570_ = w1;
  sub80278130(x588_[x570_].x6c_color);
  if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(x8cc_headColActor)) {
    if (!b1) {
      colAct->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty());
      mgr.GetPlayer().SetOrbitRequestForTarget(GetUniqueId(), CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
      colAct->RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
    } else {
      colAct->SetDamageVulnerability(x588_[x570_].x4_damageVulnerability);
      colAct->AddMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
    }
  }
}

void CMetroidPrimeExo::sub802786fc(CStateManager& mgr) {
  int w1 = 0;
  if (!x584_) {
    w1 = x570_;
    u32 iVar2 = 0;
    do {
      ++iVar2;
      if (iVar2 < 10) {
        w1 = x588_[w1].x70_[mgr.GetActiveRandom()->Next() & 1];
      } else if (iVar2 < 19) {
        w1 = x588_[w1].x70_[1];
      } else {
        break;
      }
    } while (((x57c_ & (1 << w1)) != 0u) || ((x580_ & (1 << w1)) != 0));
    x580_ |= 1 << w1;
  } else {
    w1 = x588_[x570_].x70_[mgr.GetActiveRandom()->Next() & 1];
  }

  sub80278508(mgr, w1, x8f4_25_);
  sub80275c60(mgr, w1);
}

void CMetroidPrimeExo::SetEyesParticleEffectState(CStateManager& mgr, bool b) {
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

    if (x91c_ > -1 && x91c_ < 4) {
      if (x914_24_) {
        HealthInfo(mgr)->SetHP(skHealthConstants[std::max(0, x91c_ - 1)]);
      } else {
        HealthInfo(mgr)->SetHP(std::max(0.f, hInfo->GetHP()) + skHealthConstants[x91c_] +
                               (static_cast<float>(x8d0_ - 1) * x8c0_.GetHP()));
      }
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

void CMetroidPrimeExo::DoContactDamage(TUniqueId uid, CStateManager& mgr) {
  if (!IsAlive()) {
    return;
  }

  if (TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(uid)) {
    if (colAct->GetLastTouchedObject() == mgr.GetPlayer().GetUniqueId()) {
      if (mgr.GetPlayer().GetFrozenState()) {
        mgr.GetPlayer().UnFreeze(mgr);
      }

      if (x420_curDamageRemTime <= 0.f) {
        mgr.ApplyDamage(GetUniqueId(), colAct->GetLastTouchedObject(), GetUniqueId(), GetContactDamage(),
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
        x420_curDamageRemTime = x424_damageWaitTime;
      }
    } else if (TCastToConstPtr<CActor> act = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
      if (!act->GetMaterialList().HasMaterial(EMaterialTypes::Platform)) {
        return;
      }

      mgr.ApplyDamage(GetUniqueId(), colAct->GetLastTouchedObject(), GetUniqueId(), GetContactDamage(),
                      CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), zeus::skZero3f);
    }
  }
}

void CMetroidPrimeExo::UpdateCollision(float dt, CStateManager& mgr) {
  x56c_collisionManager->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  zeus::CTransform xf = GetLocatorTransform("Skeleton_Root"sv);
  MoveCollisionPrimitive(GetTransform().rotate(GetModelData()->GetScale() * xf.origin));
}

void CMetroidPrimeExo::SetupBoneTracking() {
  for (size_t i = 0; i < 6; ++i) {
    x76c_.emplace_back(*GetModelData()->GetAnimationData(), skBoneTrackingNames[i], zeus::degToRad(80.f),
                       zeus::degToRad(180.f), EBoneTrackingFlags::NoParentOrigin);
  }
}

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

void CMetroidPrimeExo::CPhysicsDummy::Accept(IVisitor& visitor) { visitor.Visit(this); }
} // namespace metaforce::MP1
