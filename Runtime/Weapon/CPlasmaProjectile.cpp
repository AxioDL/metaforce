#include "Runtime/Weapon/CPlasmaProjectile.hpp"

#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/World/CGameLight.hpp"
#include "Runtime/World/CHUDBillboardEffect.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {

CPlasmaProjectile::RenderObjects::RenderObjects(boo::IGraphicsDataFactory::Context& ctx,
                                                boo::ObjToken<boo::ITexture> tex,
                                                boo::ObjToken<boo::ITexture> glowTex)
: m_beamStrip1(ctx, 8, CColoredStripShader::Mode::Additive, {})
, m_beamStrip2(ctx, 10, CColoredStripShader::Mode::FullAdditive, tex)
, m_beamStrip3(ctx, 18, CColoredStripShader::Mode::FullAdditive, tex)
, m_beamStrip4(ctx, 14, CColoredStripShader::Mode::Additive, glowTex)
, m_beamStrip1Sub(ctx, 8, CColoredStripShader::Mode::Subtractive, {})
, m_beamStrip2Sub(ctx, 10, CColoredStripShader::Mode::Subtractive, tex)
, m_beamStrip3Sub(ctx, 18, CColoredStripShader::Mode::Subtractive, tex)
, m_beamStrip4Sub(ctx, 14, CColoredStripShader::Mode::Subtractive, glowTex)
, m_motionBlurStrip(ctx, 16, CColoredStripShader::Mode::Alpha, {}) {}

CPlasmaProjectile::CPlasmaProjectile(const TToken<CWeaponDescription>& wDesc, std::string_view name, EWeaponType wType,
                                     const CBeamInfo& bInfo, const zeus::CTransform& xf, EMaterialTypes matType,
                                     const CDamageInfo& dInfo, TUniqueId uid, TAreaId aid, TUniqueId owner,
                                     const PlayerEffectResoures& res, bool growingBeam, EProjectileAttrib attribs)
: CBeamProjectile(wDesc, name, wType, xf, bInfo.GetLength(), bInfo.GetRadius(), bInfo.GetTravelSpeed(), matType, dInfo,
                  uid, aid, owner, attribs, growingBeam)
, x478_beamAttributes(bInfo.GetBeamAttributes())
, x47c_lifeTime(bInfo.GetLifeTime())
, x480_pulseSpeed(bInfo.GetPulseSpeed())
, x484_shutdownTime(bInfo.GetShutdownTime())
, x488_expansionSpeed(bInfo.GetExpansionSpeed())
, x48c_(bInfo.GetLength() / 32.f)
, x490_innerColor(bInfo.GetInnerColor())
, x494_outerColor(bInfo.GetOuterColor())
, x548_28_drawOwnerFirst(growingBeam) {
  x4e8_texture = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), bInfo.GetTextureId()});
  x4f4_glowTexture = g_SimplePool->GetObj(SObjectTag{FOURCC('TXTR'), bInfo.GetGlowTextureId()});
  x500_contactFxDesc = g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), bInfo.GetContactFxId()});
  x50c_pulseFxDesc = g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), bInfo.GetPulseFxId()});
  x518_contactGen = std::make_unique<CElementGen>(x500_contactFxDesc, CElementGen::EModelOrientationType::One);
  x51c_pulseGen = std::make_unique<CElementGen>(x50c_pulseFxDesc, CElementGen::EModelOrientationType::Normal);
  x524_freezeSteamTxtr = res[0];
  x528_freezeIceTxtr = res[1];
  if (res[2].IsValid()) {
    x52c_visorElectric = g_SimplePool->GetObj(SObjectTag{FOURCC('ELSC'), res[2]});
  }
  if (res[3].IsValid()) {
    x538_visorParticle = g_SimplePool->GetObj(SObjectTag{FOURCC('PART'), res[3]});
  }
  x544_freezeSfx = CSfxManager::TranslateSFXID(u16(res[4].Value()));
  x546_electricSfx = CSfxManager::TranslateSFXID(u16(res[5].Value()));
  x518_contactGen->SetGlobalScale(zeus::CVector3f(bInfo.GetContactFxScale()));
  x51c_pulseGen->SetGlobalScale(zeus::CVector3f(bInfo.GetPulseFxScale()));
  x518_contactGen->SetParticleEmission(false);
  x51c_pulseGen->SetParticleEmission(false);

  CGraphics::CommitResources([this](boo::IGraphicsDataFactory::Context& ctx) {
    m_renderObjs.emplace(ctx, x4e8_texture->GetBooTexture(), x4f4_glowTexture->GetBooTexture());
    return true;
  } BooTrace);
}

void CPlasmaProjectile::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CPlasmaProjectile::SetLightsActive(bool active, CStateManager& mgr) {
  for (TUniqueId lid : x468_lights) {
    if (lid != kInvalidUniqueId) {
      if (TCastToPtr<CGameLight> light = mgr.ObjectById(lid)) {
        light->SetActive(active);
      }
    }
  }
}

void CPlasmaProjectile::CreatePlasmaLights(u32 sourceId, const CLight& l, CStateManager& mgr) {
  DeletePlasmaLights(mgr);
  x468_lights.reserve(3);
  for (size_t i = 0; i < x468_lights.capacity(); ++i) {
    const TUniqueId lid = mgr.AllocateUniqueId();
    auto* light = new CGameLight(lid, GetAreaId(), GetActive(), "", GetTransform(), GetUniqueId(), l, sourceId, 0, 0.f);
    mgr.AddObject(light);
    x468_lights.push_back(lid);
  }
}

void CPlasmaProjectile::DeletePlasmaLights(CStateManager& mgr) {
  for (TUniqueId lid : x468_lights)
    mgr.FreeScriptObject(lid);
  x468_lights.clear();
}

void CPlasmaProjectile::UpdateLights(float expansion, float dt, CStateManager& mgr) {
  if (x520_weaponGen) {
    x520_weaponGen->Update(dt);
    CLight l = x520_weaponGen->GetLight();
    l.SetColor(zeus::CColor::lerp(zeus::skClear, l.GetColor(), expansion));
    float halfLen = 0.5f * GetCurrentLength();
    float y = 0.f;
    for (TUniqueId lid : x468_lights) {
      if (TCastToPtr<CGameLight> light = mgr.ObjectById(lid)) {
        light->SetTransform({});
        light->SetTranslation(GetBeamTransform() * zeus::CVector3f(0.f, y, 0.f));
        light->SetLight(l);
      }
      y += halfLen;
    }
  }
}

void CPlasmaProjectile::UpdateEnergyPulse(float dt) {
  if (GetDamageType() != EDamageType::None && x548_25_enableEnergyPulse) {
    x4d8_energyPulseTimer -= dt;
    if (x4d8_energyPulseTimer <= 0.f) {
      x4d8_energyPulseTimer = 2.f * dt;
      x51c_pulseGen->SetParticleEmission(true);
      float t = GetCurrentLength() / GetMaxLength();
      for (float i = 0.f; i <= t; i += 0.1f) {
        float y = i * GetMaxLength() + x4cc_energyPulseStartY;
        if (y > GetCurrentLength())
          continue;
        x51c_pulseGen->SetTranslation({0.f, y, 0.f});
        x51c_pulseGen->ForceParticleCreation(1);
      }
      x51c_pulseGen->SetGlobalOrientAndTrans(GetBeamTransform());
      x51c_pulseGen->SetParticleEmission(false);
    }
  }
  x51c_pulseGen->Update(dt);
}

void CPlasmaProjectile::RenderMotionBlur() {
  CGraphics::SetModelMatrix({});
  zeus::CColor color1 = x494_outerColor;
  zeus::CColor color2 = x494_outerColor;
  color1.a() = 63.f / 255.f;
  color2.a() = 0.f;
  std::array<CColoredStripShader::Vert, 16> verts;
  for (size_t i = 0; i < verts.size() / 2; ++i) {
    auto& v1 = verts[i * 2];
    auto& v2 = verts[i * 2 + 1];
    v1.m_pos = GetBeamTransform().origin;
    v1.m_color = zeus::CColor::lerp(color1, color2, float(i) / 8.f);
    v2.m_pos = GetPointCache()[i];
    v2.m_color = v1.m_color;
  }
  m_renderObjs->m_motionBlurStrip.draw(zeus::skWhite, verts.size(), verts.data());
}

void CPlasmaProjectile::RenderBeam(s32 subdivs, float width, const zeus::CColor& color, s32 flags,
                                   CColoredStripShader& shader) const {
  // Flags: 0x1: textured, 0x2: length controlled UVY 0x4: alpha controlled additive blend,
  // 0x8: glow texture, 0x10: subtractive blend
  if ((flags & 0x1) == 0 || (flags & 0x8) ? x4f4_glowTexture.IsLoaded() : x4e8_texture.IsLoaded()) {
    float angIncrement = 2.f * M_PIF / float(subdivs);
    float uvY1 = -(x4cc_energyPulseStartY / 16.f);
    float uvY2 = (uvY1 + float((flags & 0x3) == 0x3) != 0.f) ? 2.f : 0.5f * GetCurrentLength();
    std::array<CColoredStripShader::Vert, 18> verts;
    s32 numNodes = subdivs + 1;
    float angle = 0.f;
    bool flip = false;
    for (s32 i = 0; i < numNodes; ++i) {
      CColoredStripShader::Vert& v0 = verts[i * 2];
      CColoredStripShader::Vert& v1 = verts[i * 2 + 1];
      float x = std::cos(angle);
      float y = std::sin(angle);
      float uvX;
      if (flags & 0x8)
        uvX = 0.5f * y;
      else if (flip)
        uvX = width;
      else
        uvX = 0.f;
      flip ^= true;
      v0.m_pos = zeus::CVector3f(width * x, 0.f, width * y);
      v0.m_color = color;
      v0.m_uv = zeus::CVector2f(uvX, uvY1);
      v1.m_pos = zeus::CVector3f(width * x, GetCurrentLength(), width * y);
      v1.m_color = color;
      v1.m_uv = zeus::CVector2f(uvX, uvY2);
      angle += angIncrement;
    }
    shader.draw(zeus::skWhite, numNodes * 2, verts.data());
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
    x4d4_contactPulseTimer = 0.f;
    x4d8_energyPulseTimer = 0.f;
    x4dc_playerEffectPulseTimer = 0.f;
    x4b4_expansionState = EExpansionState::Inactive;
    x548_26_firing = false;
    x518_contactGen->SetParticleEmission(false);
    x51c_pulseGen->SetParticleEmission(false);
  } else {
    x548_26_firing = false;
    x4b4_expansionState = EExpansionState::Release;
    x518_contactGen->SetParticleEmission(false);
    x51c_pulseGen->SetParticleEmission(false);
  }
}

float CPlasmaProjectile::UpdateBeamState(float dt, CStateManager& mgr) {
  switch (x4b4_expansionState) {
  case EExpansionState::Attack:
    if (x4c0_expansionT > 0.5f)
      x4b4_expansionState = EExpansionState::Sustain;
    else
      x4c0_expansionT += dt * x488_expansionSpeed;
    break;
  case EExpansionState::Sustain:
    if (x478_beamAttributes & 0x4) {
      if (x4bc_lifeTimer > x47c_lifeTime)
        x4b4_expansionState = EExpansionState::Release;
      else
        x4bc_lifeTimer += dt;
    }
    break;
  case EExpansionState::Release:
    x4c0_expansionT += dt * x488_expansionSpeed;
    if (x4c0_expansionT > 1.f) {
      x4c0_expansionT = 1.f;
      x4b4_expansionState = EExpansionState::Done;
      x548_25_enableEnergyPulse = false;
    }
    break;
  case EExpansionState::Done:
    x4d0_shutdownTimer += dt;
    if (x4d0_shutdownTimer > x484_shutdownTime &&
        (!x518_contactGen || x518_contactGen->GetParticleCountAll() == 0)) {
      x4b4_expansionState = EExpansionState::Inactive;
      ResetBeam(mgr, true);
    }
    break;
  default:
    break;
  }
  return -4.f * x4c0_expansionT * (x4c0_expansionT - 1.f);
}

void CPlasmaProjectile::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  switch (msg) {
  case EScriptObjectMessage::Registered: {
    xe6_27_thermalVisorFlags = 2;
    const SChildGeneratorDesc& apsm = x170_projectile.GetWeaponDescription()->x34_APSM;
    if (apsm)
      x520_weaponGen = std::make_unique<CElementGen>(apsm.m_token);
    if (x520_weaponGen && x520_weaponGen->SystemHasLight())
      CreatePlasmaLights(x170_projectile.GetWeaponDescription().GetObjectTag()->id.Value(),
                         x520_weaponGen->GetLight(), mgr);
    else
      x520_weaponGen.reset();
    if (x548_28_drawOwnerFirst)
      xc6_nextDrawNode = xec_ownerId;
    mgr.AddWeaponId(xec_ownerId, xf0_weaponType);
    break;
  }
  case EScriptObjectMessage::Deleted:
    mgr.RemoveWeaponId(xec_ownerId, xf0_weaponType);
    DeletePlasmaLights(mgr);
    if (x548_29_activePlayerPhazon) {
      mgr.GetPlayer().DecrementEnvironmentDamage();
      x548_29_activePlayerPhazon = false;
    }
    break;
  default:
    break;
  }
  CGameProjectile::AcceptScriptMsg(msg, sender, mgr);
}

void CPlasmaProjectile::MakeBillboardEffect(const std::optional<TToken<CGenDescription>>& particle,
                                            const std::optional<TToken<CElectricDescription>>& electric,
                                            std::string_view name, CStateManager& mgr) {
  auto* effect = new CHUDBillboardEffect(particle, electric, mgr.AllocateUniqueId(), true, name,
                                         CHUDBillboardEffect::GetNearClipDistance(mgr),
                                         CHUDBillboardEffect::GetScaleForPOV(mgr),
                                         zeus::skWhite, zeus::skOne3f, zeus::skZero3f);
  mgr.AddObject(effect);
}

void CPlasmaProjectile::UpdatePlayerEffects(float dt, CStateManager& mgr) {
  x4dc_playerEffectPulseTimer -= dt;
  if ((x4b4_expansionState == EExpansionState::Attack || x4b4_expansionState == EExpansionState::Sustain) &&
      x4dc_playerEffectPulseTimer <= 0.f && GetDamageType() == EDamageType::Actor &&
      GetCollisionActorId() == mgr.GetPlayer().GetUniqueId()) {
    if ((x478_beamAttributes & 0x8) && !x548_29_activePlayerPhazon) {
      x548_29_activePlayerPhazon = true;
      x4e4_playerDamageTimer = 0.f;
      mgr.GetPlayer().IncrementEnvironmentDamage();
    }
    switch (xf0_weaponType) {
    case EWeaponType::Ice:
      mgr.GetPlayer().Freeze(mgr, x524_freezeSteamTxtr, x544_freezeSfx, x528_freezeIceTxtr);
      break;
    case EWeaponType::Wave:
      if (x52c_visorElectric) {
        MakeBillboardEffect({}, {x52c_visorElectric}, "PlasmaElectricFx"sv, mgr);
        CSfxManager::SfxStart(x546_electricSfx, 1.f, 0.f, false, 0x7f, false, kInvalidAreaId);
        mgr.GetPlayer().SetHudDisable(3.f, 0.5f, 2.5f);
        mgr.GetPlayer().SetOrbitRequestForTarget(mgr.GetPlayer().GetOrbitTargetId(),
                                                 CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
        mgr.GetPlayerState()->GetStaticInterference().AddSource(GetUniqueId(), 0.2f, 3.f);
      }
      break;
    case EWeaponType::Plasma:
      if (x538_visorParticle)
        MakeBillboardEffect({x538_visorParticle}, {}, "PlasmaVisorFx"sv, mgr);
      break;
    default:
      break;
    }
    x4dc_playerEffectPulseTimer = 0.75f;
  }
  if (x548_29_activePlayerPhazon) {
    CDamageInfo scaledDamage(x498_phazonDamage, dt);
    mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), xec_ownerId, scaledDamage, xf8_filter,
                    zeus::skZero3f);
    x4e4_playerDamageTimer += dt;
    if (x4e4_playerDamageTimer >= x4e0_playerDamageDuration) {
      mgr.GetPlayer().DecrementEnvironmentDamage();
      x4e4_playerDamageTimer = 0.f;
      x548_29_activePlayerPhazon = false;
    }
  }
}

void CPlasmaProjectile::UpdateFx(const zeus::CTransform& xf, float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  x548_27_texturesLoaded = x4e8_texture.IsLoaded() && x4f4_glowTexture.IsLoaded();
  CauseDamage(x4b4_expansionState == EExpansionState::Attack || x4b4_expansionState == EExpansionState::Sustain);
  CBeamProjectile::UpdateFx(xf, dt, mgr);
  UpdatePlayerEffects(dt, mgr);
  if (x478_beamAttributes & 0x1) {
    for (int i = 7; i > 0; --i)
      PointCache()[i] = PointCache()[i - 1];
    PointCache()[0] = GetCurrentPos();
  }
  if (x518_contactGen) {
    x4d4_contactPulseTimer -= dt;
    if ((GetDamageType() != EDamageType::None ? x548_25_enableEnergyPulse : false) && x4d4_contactPulseTimer <= 0.f) {
      x518_contactGen->SetOrientation(zeus::lookAt(zeus::skZero3f, GetSurfaceNormal()));
      x518_contactGen->SetTranslation(GetSurfaceNormal() * 0.001f + GetCurrentPos());
      x518_contactGen->SetParticleEmission(true);
      x4d4_contactPulseTimer = 1.f / 16.f;
    } else {
      x518_contactGen->SetParticleEmission(false);
    }
    x518_contactGen->Update(dt);
  }
  float modulation = UpdateBeamState(dt, mgr);
  UpdateEnergyPulse(dt);
  x4c8_beamAngle += 720.f * dt;
  if (x4c8_beamAngle > 360.f)
    x4c8_beamAngle = 0.f;
  x4b8_beamWidth = modulation * GetMaxRadius();
  x4c4_expansion = modulation;
  x4cc_energyPulseStartY += dt * x480_pulseSpeed;
  if (x4cc_energyPulseStartY > 5.f)
    x4cc_energyPulseStartY = 0.f;
  UpdateLights(modulation, dt, mgr);
}

void CPlasmaProjectile::Fire(const zeus::CTransform& xf, CStateManager& mgr, bool b) {
  SetActive(true);
  SetLightsActive(true, mgr);
  x548_25_enableEnergyPulse = true;
  x548_26_firing = true;
  x548_24_ = b;
  x4b4_expansionState = EExpansionState::Attack;
  if (x478_beamAttributes & 0x1)
    std::fill(PointCache().begin(), PointCache().end(), xf.origin);
}

void CPlasmaProjectile::Touch(CActor& other, CStateManager& mgr) {
  // Empty
}

bool CPlasmaProjectile::CanRenderUnsorted(const CStateManager& mgr) const {
  return false;
}

void CPlasmaProjectile::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  if (GetActive()) {
    g_Renderer->AddParticleGen(*x518_contactGen);
    if (x478_beamAttributes & 0x2) {
      g_Renderer->AddParticleGen(*x51c_pulseGen);
    }
  }
  EnsureRendered(mgr, GetBeamTransform().origin, GetSortingBounds(mgr));
}

void CPlasmaProjectile::Render(CStateManager& mgr) {
  if (!GetActive())
    return;
  SCOPED_GRAPHICS_DEBUG_GROUP("CPlasmaProjectile::Render", zeus::skOrange);

  zeus::CTransform xf = GetBeamTransform();

  // Subtractive blending for xray
  s32 flags = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay ? 0x10 : 0x0;

  if ((x478_beamAttributes & 0x1) == 0)
    xf.origin += mgr.GetCameraManager()->GetGlobalCameraTranslation(mgr);

  // Z test, no write

  if ((x478_beamAttributes & 0x1) && x548_25_enableEnergyPulse && x4b4_expansionState != EExpansionState::Attack)
    RenderMotionBlur();

  // Pass1: alpha-controlled additive
  CGraphics::SetModelMatrix(xf);
  RenderBeam(3, 0.25f * x4b8_beamWidth, zeus::CColor(1.f, 0.3f), flags | 0x4,
             (flags & 0x10) ? m_renderObjs->m_beamStrip1Sub : m_renderObjs->m_beamStrip1);

  // Pass2: textured
  CGraphics::SetModelMatrix(xf * zeus::CTransform::RotateY(zeus::degToRad(x4c8_beamAngle)));
  RenderBeam(4, 0.5f * x4b8_beamWidth, x490_innerColor, flags | 0x1,
             (flags & 0x10) ? m_renderObjs->m_beamStrip2Sub : m_renderObjs->m_beamStrip2);

  // Pass3: textured | length-controlled UVY
  CGraphics::SetModelMatrix(xf * zeus::CTransform::RotateY(zeus::degToRad(-x4c8_beamAngle)));
  RenderBeam(8, x4b8_beamWidth, x494_outerColor, flags | 0x3,
             (flags & 0x10) ? m_renderObjs->m_beamStrip3Sub : m_renderObjs->m_beamStrip3);

  // Pass4: textured | alpha-controlled additive | glow texture
  CGraphics::SetModelMatrix(xf);
  RenderBeam(6, 1.25f * x4b8_beamWidth, x494_outerColor, flags | 0xd,
             (flags & 0x10) ? m_renderObjs->m_beamStrip4Sub : m_renderObjs->m_beamStrip4);
}

} // namespace urde
