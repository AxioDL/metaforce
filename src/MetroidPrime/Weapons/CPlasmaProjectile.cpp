#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "Weapons/CWeaponDescription.hpp"
#include "dolphin/gx.h"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"

#include <Kyoto/Particles/CElementGen.hpp>

const int CPlasmaProjectile::kMaxPlasmaLights = 3;
const float CPlasmaProjectile::kInvMaxPlasmaLights = 1.f / CCast::ToReal32(kMaxPlasmaLights - 1);

CPlasmaProjectile::CPlasmaProjectile(const TToken< CWeaponDescription >& wDesc,
                                     const rstl::string& name, EWeaponType wType,
                                     const CBeamInfo& bInfo, const CTransform4f& xf,
                                     const EMaterialTypes matType, const CDamageInfo& dInfo,
                                     const TUniqueId uid, const TAreaId aid, const TUniqueId owner,
                                     const CWeaponAssetInfo& res, const bool growingBeam,
                                     const EProjectileAttrib attribs)
: CBeamProjectile(wDesc, name, wType, xf, bInfo.GetLength(), bInfo.GetRadius(),
                  bInfo.GetTravelSpeed(), matType, dInfo, uid, aid, owner, attribs, growingBeam)
, mBeamAttributes(bInfo.GetBeamAttributes())
, mLifeTime(bInfo.GetLifeTime())
, mPulseSpeed(bInfo.GetPulseSpeed())
, mShutdownTime(bInfo.GetShutdownTime())
, mExpansionSpeed(bInfo.GetExpansionSpeed())
, mMaxLength(bInfo.GetLength() * (1 / 32.f))
, mInnerColor(bInfo.GetInnerColor())
, mOuterColor(bInfo.GetOuterColor())
, mPhazonDamage(CDamageInfo())
, mExpansionState(kES_Inactive)
, mBeamWidth(0.f)
, mLifeTimer(0.f)
, mExpansionT(0.f)
, mExpansion(0.f)
, mBeamAngle(0.f)
, mEnergyPulseStartY(0.f)
, mShutdownTimer(0.f)
, mContactPulseTimer(0.f)
, mEnergyPulseTimer(0.f)
, mPlayerEffectPulseTimer(0.f)
, mPlayerDamageDuration(0.f)
, mPlayerDamageTimer(0.f)
, mTexture(gpSimplePool->GetObj(SObjectTag('TXTR', bInfo.GetTextureId())))
, mGlowTexture(gpSimplePool->GetObj(SObjectTag('TXTR', bInfo.GetGlowTextureId())))
, mContactFxDesc(gpSimplePool->GetObj(SObjectTag('PART', bInfo.GetContactFXId())))
, mPulseFxDesc(gpSimplePool->GetObj(SObjectTag('PART', bInfo.GetPulseFXId())))
, mContactGen(rs_new CElementGen(mContactFxDesc, CElementGen::kMOT_One))
, mPulseGen(rs_new CElementGen(mPulseFxDesc, CElementGen::kMOT_Normal))
, mFreezeSteamTxtr(res.data[0])
, mFreezeIceTxtr(res.data[1])
, mVisorElectric(res.data[2] != kInvalidAssetId
                         ? rstl::optional_object< TToken< CElectricDescription > >(
                               gpSimplePool->GetObj(SObjectTag('ELSC', res.data[2])))
                         : rstl::optional_object_null())
, mVisorParticle(res.data[3] != kInvalidAssetId
                         ? rstl::optional_object< TToken< CGenDescription > >(
                               gpSimplePool->GetObj(SObjectTag('PART', res.data[3])))
                         : rstl::optional_object_null())
, mFreezeSfx(CSfxManager::TranslateSFXID(res.data[4]))
, mElectricSfx(CSfxManager::TranslateSFXID(res.data[5]))
, x548_24_(false)
, mEnableEnergyPulse(true)
, mFiring(false)
, mTexturesLoaded(false)
, mDrawOwnerFirst(growingBeam)
, mActivePlayerPhazon(false) {
  mTexture.Lock();
  mGlowTexture.Lock();
  mContactGen->SetGlobalScale(
      CVector3f(bInfo.GetContactFxScale(), bInfo.GetContactFxScale(), bInfo.GetContactFxScale()));
  mPulseGen->SetGlobalScale(
      CVector3f(bInfo.GetPulseFxScale(), bInfo.GetPulseFxScale(), bInfo.GetPulseFxScale()));
  mContactGen->SetParticleEmission(false);
  mPulseGen->SetParticleEmission(false);
}

ENTITY_ACCEPT_IMPL(CPlasmaProjectile)

void CPlasmaProjectile::Touch(CActor&, CStateManager&) {}

float CPlasmaProjectile::UpdateBeamState(float dt, CStateManager& mgr) {
  switch (mExpansionState) {
  case kES_Attack:
    if (mExpansionT > 0.5f) {
      mExpansionState = kES_Sustain;
    } else {
      mExpansionT += dt * mExpansionSpeed;
    }
    break;
  case kES_Sustain:
    if (mBeamAttributes & 4) {
      if (mLifeTimer > mLifeTime) {
        mExpansionState = kES_Release;
      } else {
        mLifeTimer += dt;
      }
    }
    break;
  case kES_Release:
    mExpansionT += dt * mExpansionSpeed;
    if (mExpansionT > 1.f) {
      mExpansionT = 1.f;
      mExpansionState = kES_Done;
      mEnableEnergyPulse = false;
    }
    break;
  case kES_Done:
    mShutdownTimer += dt;
    if (mShutdownTimer > mShutdownTime &&
        (mContactGen.get() ? mContactGen->GetParticleCountAll() <= 0 : true)) {
      mExpansionState = kES_Inactive;
      ResetBeam(mgr, true);
    }
    break;
  default:
    break;
  }
  return -4.f * mExpansionT * (mExpansionT - 1.f);
}

void CPlasmaProjectile::MakeBillboardEffect(
    const rstl::optional_object< TToken< CGenDescription > >& particle,
    const rstl::optional_object< TToken< CElectricDescription > >& electric,
    const rstl::string& name, CStateManager& mgr) {
  mgr.AddObject(rs_new CHUDBillboardEffect(
      particle, electric, mgr.AllocateUniqueId(), true, name,
      CHUDBillboardEffect::GetNearClipDistance(mgr), CHUDBillboardEffect::GetScaleForPOV(mgr),
      CColor(1.f, 1.f, 1.f, 1.f), CVector3f(1.f, 1.f, 1.f), CVector3f(0.f, 0.f, 0.f)));
}

void CPlasmaProjectile::UpdatePlayerEffects(float dt, CStateManager& mgr) {
  CPlayer& player = *mgr.Player();
  mPlayerEffectPulseTimer -= dt;
  if ((mExpansionState == kES_Attack || mExpansionState == kES_Sustain) &&
      mPlayerEffectPulseTimer <= 0.f && GetDamageType() == kDT_Actor &&
      GetCollisionActorId() == player.GetUniqueId()) {
    if ((mBeamAttributes & 8) && mActivePlayerPhazon != true) {
      mActivePlayerPhazon = true;
      mPlayerDamageTimer = 0.f;
      player.IncrementEnvironmentDamage();
    }
    switch (GetType()) {
    case kWT_Ice: {
      const ushort freezeSfx = mFreezeSfx;
      player.SetFrozenState(mgr, mFreezeSteamTxtr, freezeSfx, mFreezeIceTxtr);
      break;
    }
    case kWT_Wave:
      if (mVisorElectric) {
        MakeBillboardEffect(rstl::optional_object_null(), mVisorElectric,
                            rstl::string_l("PlasmaElectricFx"), mgr);
        CSfxManager::SfxStart(mElectricSfx);
        mgr.Player()->SetHudDisable(3.f);
        mgr.Player()->TryToBreakOrbit(mgr.Player()->GetOrbitTargetId(),
                                      CPlayer::kOB_ActivateOrbitSource, mgr);
        mgr.PlayerState()->StaticInterference().AddSource(GetUniqueId(), 0.2f, 3.f);
      }
      break;
    case kWT_Plasma:
      if (mVisorParticle) {
        MakeBillboardEffect(mVisorParticle, rstl::optional_object_null(),
                            rstl::string_l("PlasmaVisorFx"), mgr);
      }
      break;
    default:
      break;
    }
    mPlayerEffectPulseTimer = 0.75f;
  }
  if (mActivePlayerPhazon) {
    mgr.ApplyDamage(GetUniqueId(), player.GetUniqueId(), GetOwnerId(),
                    mPhazonDamage.MakeScaledForTime(dt), GetFilter(), CVector3f::Zero());
    mPlayerDamageTimer += dt;
    if (mPlayerDamageTimer >= mPlayerDamageDuration) {
      player.DecrementEnvironmentDamage();
      mPlayerDamageTimer = 0.f;
      mActivePlayerPhazon = false;
    }
  }
}

void CPlasmaProjectile::UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  mTexturesLoaded = mTexture.TryCache() && mGlowTexture.TryCache();
  CauseDamage(mExpansionState == kES_Attack || mExpansionState == kES_Sustain);
  CBeamProjectile::UpdateFx(xf, dt, mgr);
  UpdatePlayerEffects(dt, mgr);
  rstl::reserved_vector< CVector3f, 8 >& cache = PointCache();
  if (mBeamAttributes & 1) {
    for (int i = 1; i < 8; ++i) {
      const int idx = 8 - i;
      cache[idx] = cache[idx - 1];
    }
    cache[0] = GetCurrentPos();
  }
  const bool contact = GetDamageType() != kDT_None ? mEnableEnergyPulse : false;
  if (mContactGen.get()) {
    mContactPulseTimer -= dt;
    if (contact && mContactPulseTimer <= 0.f) {
      mContactGen->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), GetSurfaceNormal()));
      mContactGen->SetTranslation(GetCurrentPos() + 0.001f * GetSurfaceNormal());
      mContactGen->SetParticleEmission(true);
      mContactPulseTimer = 1.f / 16.f;
    } else {
      mContactGen->SetParticleEmission(false);
    }
    mContactGen->Update(dt);
  }
  const float expansion = UpdateBeamState(dt, mgr);
  UpdateEnergyPulse(dt);
  mBeamAngle += 720.f * dt;
  if (mBeamAngle > 360.f) {
    mBeamAngle = 0.f;
  }
  mBeamWidth = expansion * GetMaxRadius();
  mExpansion = expansion;
  mEnergyPulseStartY += dt * mPulseSpeed;
  if (mEnergyPulseStartY > 5.f) {
    mEnergyPulseStartY = 0.f;
  }
  UpdateLights(expansion, dt, mgr);
}

bool CPlasmaProjectile::CanRenderUnsorted(const CStateManager&) const { return false; }

void CPlasmaProjectile::AddToRenderer(const CFrustumPlanes&, const CStateManager& mgr) const {
  if (GetActive()) {
    gpRender->AddParticleGen(*mContactGen);
    if (mBeamAttributes & 2) {
      gpRender->AddParticleGen(*mPulseGen);
    }
  }
  const CVector3f pos = GetBeamTransform().GetTranslation();
  EnsureRendered(mgr, pos, GetSortingBounds(mgr));
}

void CPlasmaProjectile::Render(const CStateManager& mgr) const {
  if (!GetActive()) {
    return;
  }
  CTransform4f xf = GetBeamTransform();
  int flags = 0;
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay) {
    flags = 0x10;
  }
  if (!(mBeamAttributes & 1)) {
    xf.AddTranslation(mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr));
  }
  gpRender->SetDepthReadWrite(true, false);
  if ((mBeamAttributes & 1) && mEnableEnergyPulse && mExpansionState != kES_Attack) {
    RenderMotionBlur();
  }
  gpRender->SetModelMatrix(xf);
  RenderBeam(3, 0.25f * mBeamWidth, CColor(1.f, 1.f, 1.f, 0.3f), flags | 4);
  gpRender->SetModelMatrix(xf * CTransform4f::RotateY(CRelAngle::FromDegrees(mBeamAngle)));
  RenderBeam(4, 0.5f * mBeamWidth, mInnerColor, flags | 1);
  gpRender->SetModelMatrix(xf * CTransform4f::RotateY(CRelAngle::FromDegrees(-mBeamAngle)));
  RenderBeam(8, mBeamWidth, mOuterColor, flags | 3);
  gpRender->SetModelMatrix(xf);
  RenderBeam(6, 1.25f * mBeamWidth, mOuterColor, flags | 0xd);
}

void CPlasmaProjectile::Fire(const CTransform4f& xf, CStateManager& mgr, const bool b) {
  SetActive(true);
  SetLightsActive(true, mgr);
  rstl::reserved_vector< CVector3f, 8 >& cache = PointCache();
  mEnableEnergyPulse = true;
  mFiring = true;
  x548_24_ = b;
  mExpansionState = kES_Attack;
  if (mBeamAttributes & 1) {
    for (int i = 0; i < 8; ++i) {
      cache[i] = xf.GetTranslation();
    }
  }
}

void CPlasmaProjectile::ResetBeam(CStateManager& mgr, bool fullReset) {
  if (fullReset) {
    SetActive(false);
    SetLightsActive(false, mgr);
    mLifeTimer = 0.f;
    mExpansionT = 0.f;
    mBeamAngle = 0.f;
    mShutdownTimer = 0.f;
    mBeamAngle = 0.f;
    mContactPulseTimer = 0.f;
    mEnergyPulseTimer = 0.f;
    mPlayerEffectPulseTimer = 0.f;
    mExpansionState = kES_Inactive;
    mFiring = false;
    mContactGen->SetParticleEmission(false);
    mPulseGen->SetParticleEmission(false);
  } else {
    mFiring = false;
    mExpansionState = kES_Release;
    mContactGen->SetParticleEmission(false);
    mPulseGen->SetParticleEmission(false);
  }
}

void CPlasmaProjectile::RenderBeam(int subdivs, float width, const CColor& color, int flags) const {
  CTexture* texture = nullptr;
  if (flags & 1) {
    CTexture* beamTexture = mTexture.GetObject();
    CTexture* glowTexture = mGlowTexture.GetObject();
    if (flags & 8) {
      texture = glowTexture;
    } else {
      texture = beamTexture;
    }
    if (!texture) {
      return;
    }
  }
  bool flip = false;
  const int count = subdivs + 1;
  const float angleStep = (2.f * M_PIF) / subdivs;
  const float uvY0 = -(0.0625f * mEnergyPulseStartY);
  const float uvY1 = uvY0 + ((flags & 3) == 3) ? 2.f : 0.5f * GetCurrentLength();
  const CVector3f beamEnd(0.f, GetCurrentLength(), 0.f);
  float angle = 0.f;
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  const GXVtxDescList vtxDesc[] = {{GX_VA_POS, GX_DIRECT},
                                   {GX_VA_CLR0, GX_DIRECT},
                                   {GX_VA_TEX0, GX_DIRECT},
                                   {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(vtxDesc);
  CGX::SetNumChans(1);
  CGX::SetNumTevStages(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  if (flags & 0x10) {
    CGX::SetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  } else if (flags & 4) {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
  } else {
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_CLEAR);
  }
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEXCOORD0, GX_IDENTITY, false,
                      GX_PTIDENTITY);
  if (flags & 1) {
    CGX::SetNumTexGens(1);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    texture->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  } else {
    CGX::SetNumTexGens(0);
    CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
  }
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  const uint rgba = color.GetColor_u32();
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, count * 2);
  for (int i = 0; i < count; ++i) {
    const float x = CMath::FastCosR(angle);
    const float z = CMath::FastSinR(angle);
    const float uvX = flags & 8 ? 0.5f * z : flip ? width : 0.f;
    flip ^= true;
    const float px = width * x;
    const float pz = width * z;
    const CVector3f pos(px, 0.f, pz);
    GXPosition3f32(pos.GetX(), pos.GetY(), pos.GetZ());
    GXColor1u32(rgba);
    GXTexCoord2f32(uvX, uvY0);
    const CVector3f end = pos + beamEnd;
    GXPosition3f32(end.GetX(), end.GetY(), end.GetZ());
    GXColor1u32(rgba);
    GXTexCoord2f32(uvX, uvY1);
    angle += angleStep;
  }
  CGX::End();
  if (flags & 8) {
    CGraphics::SetCullMode(kCM_Front);
  }
}

void CPlasmaProjectile::UpdateEnergyPulse(float dt) {
  if (GetDamageType() != kDT_None ? mEnableEnergyPulse : false) {
    mEnergyPulseTimer -= dt;
    if (mEnergyPulseTimer <= 0.f) {
      mEnergyPulseTimer = 2.f * dt;
      mPulseGen->SetParticleEmission(true);
      const float t = GetCurrentLength() / GetMaxLength();
      for (float i = 0.f; i <= t; i += 0.1f) {
        const float y = i * GetMaxLength() + mEnergyPulseStartY;
        if (y > GetCurrentLength()) {
          continue;
        }
        mPulseGen->SetTranslation(CVector3f(0.f, y, 0.f));
        mPulseGen->ForceParticleCreation(1);
      }
      mPulseGen->SetGlobalOrientAndTrans(GetBeamTransform());
      mPulseGen->SetParticleEmission(false);
    }
  }
  mPulseGen->Update(dt);
}

void CPlasmaProjectile::RenderMotionBlur() const {
  CGraphics::SetCullMode(kCM_None);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  gpRender->SetBlendMode_AlphaBlended();
  CColor color = mOuterColor;
  const CVector3f origin = GetBeamTransform().GetTranslation();
  color.SetAlpha(mExpansion);
  uint color0 = color.GetColor_u32();
  uint color1 = color.GetColor_u32();
  color0 = (color0 & 0xffffff00) | 0x3f;
  color1 &= 0xffffff00;
  static const GXVtxDescList vtxDesc[] = {
      {GX_VA_POS, GX_DIRECT}, {GX_VA_CLR0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(vtxDesc);
  CGX::SetNumChans(1);
  CGX::SetNumTevStages(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetNumTexGens(0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 16);
  const rstl::reserved_vector< CVector3f, 8 >& points = GetPointCache();
  for (int i = 0; i < 8; ++i) {
    const uint rgba = CColor::Lerp(color0, color1, 0.125f * static_cast< float >(i));
    GXPosition3f32(origin.GetX(), origin.GetY(), origin.GetZ());
    GXColor1u32(rgba);
    const CVector3f& pos = points[i];
    GXPosition3f32(pos[kDX], pos[kDY], pos[kDZ]);
    GXColor1u32(rgba);
  }
  CGX::End();
  CGraphics::SetCullMode(kCM_Front);
}

void CPlasmaProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                                        CStateManager& mgr) {
  switch (msg) {
  case kSM_Registered: {
    SetThermalFlags(kTF_Hot);
    const TLockedToken< CWeaponDescription >& desc = GetProjectile().GetWeaponDescription();
    if (desc->mAPSM) {
      mWeaponGen = rs_new CElementGen(*desc->mAPSM);
    }
    if (mWeaponGen.get() && mWeaponGen->SystemHasLight()) {
      const uint sourceId = desc.GetTag().GetId();
      CreatePlasmaLights(sourceId, mWeaponGen->GetLight(), mgr);
    } else {
      mWeaponGen = nullptr;
    }
    if (mDrawOwnerFirst) {
      SetDrawParentId(GetOwnerId());
    }
    mgr.AddWeaponId(GetOwnerId(), GetType());
    break;
  }
  case kSM_Deleted:
    mgr.RemoveWeaponId(GetOwnerId(), GetType());
    DeletePlasmaLights(mgr);
    if (mActivePlayerPhazon) {
      mgr.Player()->DecrementEnvironmentDamage();
      mActivePlayerPhazon = false;
    }
    break;
  default:
    break;
  }
  CGameProjectile::AcceptScriptMsg(msg, sender, mgr);
}

void CPlasmaProjectile::SetLightsActive(bool active, CStateManager& mgr) {
  for (AUTO(it, mLights.begin()); it != mLights.end(); ++it) {
    const TUniqueId& id = *it;
    if (id != kInvalidUniqueId) {
      if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(id))) {
        light->SetActive(active);
      }
    }
  }
}

void CPlasmaProjectile::CreatePlasmaLights(uint sourceId, const CLight& light, CStateManager& mgr) {
  DeletePlasmaLights(mgr);
  mLights.reserve(kMaxPlasmaLights);
  for (int i = 0; i < kMaxPlasmaLights; ++i) {
    rstl::string name;
    TUniqueId id = mgr.AllocateUniqueId();
    mgr.AddObject(rs_new CGameLight(id, GetAreaId(), GetActive(), name, GetTransform(),
                                    GetUniqueId(), light, sourceId, 0, 0.f));
    mLights.push_back(id);
  }
}

void CPlasmaProjectile::DeletePlasmaLights(CStateManager& mgr) {
  for (AUTO(it, mLights.begin()); it != mLights.end(); ++it) {
    const TUniqueId& id = *it;
    if (id != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(id);
    }
  }
  mLights = rstl::vector< TUniqueId >();
}

void CPlasmaProjectile::UpdateLights(float expansion, float dt, CStateManager& mgr) {
  if (mWeaponGen.get() && mWeaponGen->SystemHasLight()) {
    mWeaponGen->Update(dt);
    CLight light = mWeaponGen->GetLight();
    light.SetColor(CColor(CColor::Lerp(0, light.GetColor().GetColor_u32(), expansion)));
    const float spacing = kInvMaxPlasmaLights * GetCurrentLength();
    float y = 0.f;
    for (AUTO(it, mLights.begin()); it != mLights.end(); ++it) {
      if (CGameLight* actor = TCastToPtr< CGameLight >(mgr.ObjectById(*it))) {
        CVector3f pos(0.f, y, 0.f);
        actor->SetTransform(CTransform4f::Identity());
        actor->SetTranslation(GetBeamTransform() * pos);
        actor->SetLight(light);
      }
      y += spacing;
    }
  }
}

void CPlasmaProjectile::SetPlayerSustainedDamage(float duration, const CDamageInfo& damage) {
  if (duration > 0.f) {
    mBeamAttributes |= 8;
    mPlayerDamageDuration = duration;
    mPhazonDamage = damage;
  } else {
    mBeamAttributes &= ~8;
  }
}

CBeamProjectile::~CBeamProjectile() {}
