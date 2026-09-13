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
, x478_beamAttributes(bInfo.GetBeamAttributes())
, x47c_lifeTime(bInfo.GetLifeTime())
, x480_pulseSpeed(bInfo.GetPulseSpeed())
, x484_shutdownTime(bInfo.GetShutdownTime())
, x488_expansionSpeed(bInfo.GetExpansionSpeed())
, x48c_maxLength(bInfo.GetLength() * (1 / 32.f))
, x490_innerColor(bInfo.GetInnerColor())
, x494_outerColor(bInfo.GetOuterColor())
, x498_phazonDamage(CDamageInfo())
, x4b4_expansionState(kES_Inactive)
, x4b8_beamWidth(0.f)
, x4bc_lifeTimer(0.f)
, x4c0_expansionT(0.f)
, x4c4_expansion(0.f)
, x4c8_beamAngle(0.f)
, x4cc_energyPulseStartY(0.f)
, x4d0_shutdownTimer(0.f)
, x4d4_contactPulseTimer(0.f)
, x4d8_energyPulseTimer(0.f)
, x4dc_playerEffectPulseTimer(0.f)
, x4e0_playerDamageDuration(0.f)
, x4e4_playerDamageTimer(0.f)
, x4e8_texture(gpSimplePool->GetObj(SObjectTag('TXTR', bInfo.GetTextureId())))
, x4f4_glowTexture(gpSimplePool->GetObj(SObjectTag('TXTR', bInfo.GetGlowTextureId())))
, x500_contactFxDesc(gpSimplePool->GetObj(SObjectTag('PART', bInfo.GetContactFXId())))
, x50c_pulseFxDesc(gpSimplePool->GetObj(SObjectTag('PART', bInfo.GetPulseFXId())))
, x518_contactGen(rs_new CElementGen(x500_contactFxDesc, CElementGen::kMOT_One))
, x51c_pulseGen(rs_new CElementGen(x50c_pulseFxDesc, CElementGen::kMOT_Normal))
, x524_freezeSteamTxtr(res.data[0])
, x528_freezeIceTxtr(res.data[1])
, x52c_visorElectric(res.data[2] != kInvalidAssetId
                         ? rstl::optional_object< TToken< CElectricDescription > >(
                               gpSimplePool->GetObj(SObjectTag('ELSC', res.data[2])))
                         : rstl::optional_object_null())
, x538_visorParticle(res.data[3] != kInvalidAssetId
                         ? rstl::optional_object< TToken< CGenDescription > >(
                               gpSimplePool->GetObj(SObjectTag('PART', res.data[3])))
                         : rstl::optional_object_null())
, x544_freezeSfx(CSfxManager::TranslateSFXID(res.data[4]))
, x546_electricSfx(CSfxManager::TranslateSFXID(res.data[5]))
, x548_24_(false)
, x548_25_enableEnergyPulse(true)
, x548_26_firing(false)
, x548_27_texturesLoaded(false)
, x548_28_drawOwnerFirst(growingBeam)
, x548_29_activePlayerPhazon(false) {
  x4e8_texture.Lock();
  x4f4_glowTexture.Lock();
  x518_contactGen->SetGlobalScale(
      CVector3f(bInfo.GetContactFxScale(), bInfo.GetContactFxScale(), bInfo.GetContactFxScale()));
  x51c_pulseGen->SetGlobalScale(
      CVector3f(bInfo.GetPulseFxScale(), bInfo.GetPulseFxScale(), bInfo.GetPulseFxScale()));
  x518_contactGen->SetParticleEmission(false);
  x51c_pulseGen->SetParticleEmission(false);
}

ENTITY_ACCEPT_IMPL(CPlasmaProjectile)

void CPlasmaProjectile::Touch(CActor&, CStateManager&) {}

float CPlasmaProjectile::UpdateBeamState(float dt, CStateManager& mgr) {
  switch (x4b4_expansionState) {
  case kES_Attack:
    if (x4c0_expansionT > 0.5f) {
      x4b4_expansionState = kES_Sustain;
    } else {
      x4c0_expansionT += dt * x488_expansionSpeed;
    }
    break;
  case kES_Sustain:
    if (x478_beamAttributes & 4) {
      if (x4bc_lifeTimer > x47c_lifeTime) {
        x4b4_expansionState = kES_Release;
      } else {
        x4bc_lifeTimer += dt;
      }
    }
    break;
  case kES_Release:
    x4c0_expansionT += dt * x488_expansionSpeed;
    if (x4c0_expansionT > 1.f) {
      x4c0_expansionT = 1.f;
      x4b4_expansionState = kES_Done;
      x548_25_enableEnergyPulse = false;
    }
    break;
  case kES_Done:
    x4d0_shutdownTimer += dt;
    if (x4d0_shutdownTimer > x484_shutdownTime &&
        (x518_contactGen.get() ? x518_contactGen->GetParticleCountAll() <= 0 : true)) {
      x4b4_expansionState = kES_Inactive;
      ResetBeam(mgr, true);
    }
    break;
  default:
    break;
  }
  return -4.f * x4c0_expansionT * (x4c0_expansionT - 1.f);
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
  x4dc_playerEffectPulseTimer -= dt;
  if ((x4b4_expansionState == kES_Attack || x4b4_expansionState == kES_Sustain) &&
      x4dc_playerEffectPulseTimer <= 0.f && GetDamageType() == kDT_Actor &&
      GetCollisionActorId() == player.GetUniqueId()) {
    if ((x478_beamAttributes & 8) && x548_29_activePlayerPhazon != true) {
      x548_29_activePlayerPhazon = true;
      x4e4_playerDamageTimer = 0.f;
      player.IncrementEnvironmentDamage();
    }
    switch (GetType()) {
    case kWT_Ice: {
      const ushort freezeSfx = x544_freezeSfx;
      player.SetFrozenState(mgr, x524_freezeSteamTxtr, freezeSfx, x528_freezeIceTxtr);
      break;
    }
    case kWT_Wave:
      if (x52c_visorElectric) {
        MakeBillboardEffect(rstl::optional_object_null(), x52c_visorElectric,
                            rstl::string_l("PlasmaElectricFx"), mgr);
        CSfxManager::SfxStart(x546_electricSfx);
        mgr.Player()->SetHudDisable(3.f);
        mgr.Player()->TryToBreakOrbit(mgr.Player()->GetOrbitTargetId(),
                                      CPlayer::kOB_ActivateOrbitSource, mgr);
        mgr.PlayerState()->StaticInterference().AddSource(GetUniqueId(), 0.2f, 3.f);
      }
      break;
    case kWT_Plasma:
      if (x538_visorParticle) {
        MakeBillboardEffect(x538_visorParticle, rstl::optional_object_null(),
                            rstl::string_l("PlasmaVisorFx"), mgr);
      }
      break;
    default:
      break;
    }
    x4dc_playerEffectPulseTimer = 0.75f;
  }
  if (x548_29_activePlayerPhazon) {
    mgr.ApplyDamage(GetUniqueId(), player.GetUniqueId(), GetOwnerId(),
                    x498_phazonDamage.MakeScaledForTime(dt), GetFilter(), CVector3f::Zero());
    x4e4_playerDamageTimer += dt;
    if (x4e4_playerDamageTimer >= x4e0_playerDamageDuration) {
      player.DecrementEnvironmentDamage();
      x4e4_playerDamageTimer = 0.f;
      x548_29_activePlayerPhazon = false;
    }
  }
}

void CPlasmaProjectile::UpdateFx(const CTransform4f& xf, float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  x548_27_texturesLoaded = x4e8_texture.TryCache() && x4f4_glowTexture.TryCache();
  CauseDamage(x4b4_expansionState == kES_Attack || x4b4_expansionState == kES_Sustain);
  CBeamProjectile::UpdateFx(xf, dt, mgr);
  UpdatePlayerEffects(dt, mgr);
  rstl::reserved_vector< CVector3f, 8 >& cache = PointCache();
  if (x478_beamAttributes & 1) {
    for (int i = 1; i < 8; ++i) {
      const int idx = 8 - i;
      cache[idx] = cache[idx - 1];
    }
    cache[0] = GetCurrentPos();
  }
  const bool contact = GetDamageType() != kDT_None ? x548_25_enableEnergyPulse : false;
  if (x518_contactGen.get()) {
    x4d4_contactPulseTimer -= dt;
    if (contact && x4d4_contactPulseTimer <= 0.f) {
      x518_contactGen->SetOrientation(CTransform4f::LookAt(CVector3f::Zero(), GetSurfaceNormal()));
      x518_contactGen->SetTranslation(GetCurrentPos() + 0.001f * GetSurfaceNormal());
      x518_contactGen->SetParticleEmission(true);
      x4d4_contactPulseTimer = 1.f / 16.f;
    } else {
      x518_contactGen->SetParticleEmission(false);
    }
    x518_contactGen->Update(dt);
  }
  const float expansion = UpdateBeamState(dt, mgr);
  UpdateEnergyPulse(dt);
  x4c8_beamAngle += 720.f * dt;
  if (x4c8_beamAngle > 360.f) {
    x4c8_beamAngle = 0.f;
  }
  x4b8_beamWidth = expansion * GetMaxRadius();
  x4c4_expansion = expansion;
  x4cc_energyPulseStartY += dt * x480_pulseSpeed;
  if (x4cc_energyPulseStartY > 5.f) {
    x4cc_energyPulseStartY = 0.f;
  }
  UpdateLights(expansion, dt, mgr);
}

bool CPlasmaProjectile::CanRenderUnsorted(const CStateManager&) const { return false; }

void CPlasmaProjectile::AddToRenderer(const CFrustumPlanes&, const CStateManager& mgr) const {
  if (GetActive()) {
    gpRender->AddParticleGen(*x518_contactGen);
    if (x478_beamAttributes & 2) {
      gpRender->AddParticleGen(*x51c_pulseGen);
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
  if (!(x478_beamAttributes & 1)) {
    xf.AddTranslation(mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr));
  }
  gpRender->SetDepthReadWrite(true, false);
  if ((x478_beamAttributes & 1) && x548_25_enableEnergyPulse && x4b4_expansionState != kES_Attack) {
    RenderMotionBlur();
  }
  gpRender->SetModelMatrix(xf);
  RenderBeam(3, 0.25f * x4b8_beamWidth, CColor(1.f, 1.f, 1.f, 0.3f), flags | 4);
  gpRender->SetModelMatrix(xf * CTransform4f::RotateY(CRelAngle::FromDegrees(x4c8_beamAngle)));
  RenderBeam(4, 0.5f * x4b8_beamWidth, x490_innerColor, flags | 1);
  gpRender->SetModelMatrix(xf * CTransform4f::RotateY(CRelAngle::FromDegrees(-x4c8_beamAngle)));
  RenderBeam(8, x4b8_beamWidth, x494_outerColor, flags | 3);
  gpRender->SetModelMatrix(xf);
  RenderBeam(6, 1.25f * x4b8_beamWidth, x494_outerColor, flags | 0xd);
}

void CPlasmaProjectile::Fire(const CTransform4f& xf, CStateManager& mgr, const bool b) {
  SetActive(true);
  SetLightsActive(true, mgr);
  rstl::reserved_vector< CVector3f, 8 >& cache = PointCache();
  x548_25_enableEnergyPulse = true;
  x548_26_firing = true;
  x548_24_ = b;
  x4b4_expansionState = kES_Attack;
  if (x478_beamAttributes & 1) {
    for (int i = 0; i < 8; ++i) {
      cache[i] = xf.GetTranslation();
    }
  }
}

void CPlasmaProjectile::ResetBeam(CStateManager& mgr, bool fullReset) {
  if (fullReset) {
    SetActive(false);
    SetLightsActive(false, mgr);
    x4bc_lifeTimer = 0.f;
    x4c0_expansionT = 0.f;
    x4c8_beamAngle = 0.f;
    x4d0_shutdownTimer = 0.f;
    x4c8_beamAngle = 0.f;
    x4d4_contactPulseTimer = 0.f;
    x4d8_energyPulseTimer = 0.f;
    x4dc_playerEffectPulseTimer = 0.f;
    x4b4_expansionState = kES_Inactive;
    x548_26_firing = false;
    x518_contactGen->SetParticleEmission(false);
    x51c_pulseGen->SetParticleEmission(false);
  } else {
    x548_26_firing = false;
    x4b4_expansionState = kES_Release;
    x518_contactGen->SetParticleEmission(false);
    x51c_pulseGen->SetParticleEmission(false);
  }
}

void CPlasmaProjectile::RenderBeam(int subdivs, float width, const CColor& color, int flags) const {
  CTexture* texture = nullptr;
  if (flags & 1) {
    CTexture* beamTexture = x4e8_texture.GetObject();
    CTexture* glowTexture = x4f4_glowTexture.GetObject();
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
  const float uvY0 = -(0.0625f * x4cc_energyPulseStartY);
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
  if (GetDamageType() != kDT_None ? x548_25_enableEnergyPulse : false) {
    x4d8_energyPulseTimer -= dt;
    if (x4d8_energyPulseTimer <= 0.f) {
      x4d8_energyPulseTimer = 2.f * dt;
      x51c_pulseGen->SetParticleEmission(true);
      const float t = GetCurrentLength() / GetMaxLength();
      for (float i = 0.f; i <= t; i += 0.1f) {
        const float y = i * GetMaxLength() + x4cc_energyPulseStartY;
        if (y > GetCurrentLength()) {
          continue;
        }
        x51c_pulseGen->SetTranslation(CVector3f(0.f, y, 0.f));
        x51c_pulseGen->ForceParticleCreation(1);
      }
      x51c_pulseGen->SetGlobalOrientAndTrans(GetBeamTransform());
      x51c_pulseGen->SetParticleEmission(false);
    }
  }
  x51c_pulseGen->Update(dt);
}

void CPlasmaProjectile::RenderMotionBlur() const {
  CGraphics::SetCullMode(kCM_None);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  gpRender->SetBlendMode_AlphaBlended();
  CColor color = x494_outerColor;
  const CVector3f origin = GetBeamTransform().GetTranslation();
  color.SetAlpha(x4c4_expansion);
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
    if (desc->x34_APSM) {
      x520_weaponGen = rs_new CElementGen(*desc->x34_APSM);
    }
    if (x520_weaponGen.get() && x520_weaponGen->SystemHasLight()) {
      const uint sourceId = desc.GetTag().GetId();
      CreatePlasmaLights(sourceId, x520_weaponGen->GetLight(), mgr);
    } else {
      x520_weaponGen = nullptr;
    }
    if (x548_28_drawOwnerFirst) {
      SetDrawParentId(GetOwnerId());
    }
    mgr.AddWeaponId(GetOwnerId(), GetType());
    break;
  }
  case kSM_Deleted:
    mgr.RemoveWeaponId(GetOwnerId(), GetType());
    DeletePlasmaLights(mgr);
    if (x548_29_activePlayerPhazon) {
      mgr.Player()->DecrementEnvironmentDamage();
      x548_29_activePlayerPhazon = false;
    }
    break;
  default:
    break;
  }
  CGameProjectile::AcceptScriptMsg(msg, sender, mgr);
}

void CPlasmaProjectile::SetLightsActive(bool active, CStateManager& mgr) {
  for (AUTO(it, x468_lights.begin()); it != x468_lights.end(); ++it) {
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
  x468_lights.reserve(kMaxPlasmaLights);
  for (int i = 0; i < kMaxPlasmaLights; ++i) {
    rstl::string name;
    TUniqueId id = mgr.AllocateUniqueId();
    mgr.AddObject(rs_new CGameLight(id, GetAreaId(), GetActive(), name, GetTransform(),
                                    GetUniqueId(), light, sourceId, 0, 0.f));
    x468_lights.push_back(id);
  }
}

void CPlasmaProjectile::DeletePlasmaLights(CStateManager& mgr) {
  for (AUTO(it, x468_lights.begin()); it != x468_lights.end(); ++it) {
    const TUniqueId& id = *it;
    if (id != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(id);
    }
  }
  x468_lights = rstl::vector< TUniqueId >();
}

void CPlasmaProjectile::UpdateLights(float expansion, float dt, CStateManager& mgr) {
  if (x520_weaponGen.get() && x520_weaponGen->SystemHasLight()) {
    x520_weaponGen->Update(dt);
    CLight light = x520_weaponGen->GetLight();
    light.SetColor(CColor(CColor::Lerp(0, light.GetColor().GetColor_u32(), expansion)));
    const float spacing = kInvMaxPlasmaLights * GetCurrentLength();
    float y = 0.f;
    for (AUTO(it, x468_lights.begin()); it != x468_lights.end(); ++it) {
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
    x478_beamAttributes |= 8;
    x4e0_playerDamageDuration = duration;
    x498_phazonDamage = damage;
  } else {
    x478_beamAttributes &= ~8;
  }
}

CBeamProjectile::~CBeamProjectile() {}
