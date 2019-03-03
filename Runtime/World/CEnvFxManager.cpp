#include "CEnvFxManager.hpp"
#include "Graphics/CTexture.hpp"
#include "CActor.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "CRandom16.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"
#include "CHUDBillboardEffect.hpp"
#include "World/CWorld.hpp"
#include "World/CPlayer.hpp"
#include "Collision/CGameCollision.hpp"
#include "Collision/CInternalRayCastStructure.hpp"
#include "World/CScriptTrigger.hpp"
#include "World/CScriptWater.hpp"
#include "Graphics/CBooRenderer.hpp"

namespace urde {

static rstl::reserved_vector<zeus::CVector2f, 256> g_SnowForces;

CEnvFxManagerGrid::CEnvFxManagerGrid(const zeus::CVector2i& position, const zeus::CVector2i& extent,
                                     const std::vector<CVectorFixed8_8>& initialParticles, int reserve,
                                     CEnvFxManager& parent, boo::IGraphicsDataFactory::Context& ctx)
: x4_position(position)
, xc_extent(extent)
, x1c_particles(initialParticles)
, m_instBuf(parent.m_instPool.allocateBlock(CGraphics::g_BooFactory, reserve))
, m_uniformBuf(parent.m_uniformPool.allocateBlock(CGraphics::g_BooFactory))
, m_lineRenderer(ctx, CLineRenderer::EPrimitiveMode::Lines, reserve * 2, parent.x40_txtrEnvGradient->GetBooTexture(),
                 true, true) {
  x1c_particles.reserve(reserve);
  CEnvFxShaders::BuildShaderDataBinding(ctx, parent, *this);
}

CEnvFxManager::CEnvFxManager() {
  x40_txtrEnvGradient = g_SimplePool->GetObj("TXTR_EnvGradient");
  x40_txtrEnvGradient->GetBooTexture()->setClampMode(boo::TextureClampMode::ClampToEdge);
  xb58_envRainSplash = g_SimplePool->GetObj("PART_EnvRainSplash");
  xb74_txtrSnowFlake = g_SimplePool->GetObj("TXTR_SnowFlake");
  xc48_underwaterFlake = g_SimplePool->GetObj("TXTR_UnderwaterFlake");
  CRandom16 random(0);
  CGraphics::CommitResources([this](boo::IGraphicsDataFactory::Context& ctx) {
    m_fogUniformBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(CGraphics::g_Fog), 1);
    for (int i = 0; i < 8; ++i)
      for (int j = 0; j < 8; ++j)
        x50_grids.emplace_back(zeus::CVector2i{j * 2048, i * 2048}, zeus::CVector2i{2048, 2048},
                               std::vector<CVectorFixed8_8>{}, 171, *this, ctx);
    return true;
  } BooTrace);
  for (int i = 0; i < 16; ++i)
    xb84_snowZDeltas.emplace_back(0.f, 0.f, random.Range(-2.f, -4.f));
}

void CEnvFxManager::SetSplashEffectRate(float rate, const CStateManager& mgr) {
  if (TCastToPtr<CHUDBillboardEffect> splashEffect = mgr.ObjectById(xb68_envRainSplashId))
    if (splashEffect->IsElementGen())
      splashEffect->GetParticleGen()->SetGeneratorRate(rate);
}

/* Used to be MIDI scale */
static float CalcRainVolume(float f) {
  if (f < 0.1f)
    return (f / 0.1f * 74.f) / 127.f;
  else
    return (f / 0.9f * 21.f + 74.f) / 127.f;
}

/* Used to be MIDI scale */
static float CalcRainPitch(float f) { return f - 1.f; }

void CEnvFxManager::UpdateRainSounds(const CStateManager& mgr) {
  if (mgr.GetWorld()->GetNeededEnvFx() == EEnvFxType::Rain) {
    zeus::CTransform camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
    float rainVol = CalcRainVolume(x30_fxDensity);
    if (!xb6a_rainSoundActive) {
      xb6c_leftRainSound = CSfxManager::AddEmitter(SFXsfx09F0, zeus::skZero3f, zeus::skZero3f, false,
                                                   true, 0xff, kInvalidAreaId);
      xb70_rightRainSound = CSfxManager::AddEmitter(SFXsfx09F1, zeus::skZero3f, zeus::skZero3f, false,
                                                    true, 0xff, kInvalidAreaId);
      xb6a_rainSoundActive = true;
    }
    CSfxManager::UpdateEmitter(xb6c_leftRainSound, camXf.origin - camXf.basis[0], camXf.basis[0], rainVol);
    CSfxManager::UpdateEmitter(xb70_rightRainSound, camXf.origin + camXf.basis[0], -camXf.basis[0], rainVol);
    float rainPitch = CalcRainPitch(x30_fxDensity);
    CSfxManager::PitchBend(xb6c_leftRainSound, rainPitch);
    CSfxManager::PitchBend(xb70_rightRainSound, rainPitch);
  } else if (xb6a_rainSoundActive) {
    CSfxManager::RemoveEmitter(xb6c_leftRainSound);
    CSfxManager::RemoveEmitter(xb70_rightRainSound);
    xb6a_rainSoundActive = false;
  }
}

zeus::CVector3f CEnvFxManager::GetParticleBoundsToWorldScale() const {
  return (x0_particleBounds.max - x0_particleBounds.min) / 127.f;
}

zeus::CTransform CEnvFxManager::GetParticleBoundsToWorldTransform() const {
  return zeus::CTransform::Translate(x18_focusCellPosition) * zeus::CTransform::Translate(zeus::CVector3f(-31.75f)) *
         zeus::CTransform::Scale(GetParticleBoundsToWorldScale());
}

void CEnvFxManager::UpdateVisorSplash(CStateManager& mgr, float dt, const zeus::CTransform& camXf) {
  EEnvFxType fxType = mgr.GetWorld()->GetNeededEnvFx();
  if (xb68_envRainSplashId != kInvalidUniqueId)
    if (TCastToPtr<CHUDBillboardEffect> splashEffect = mgr.ObjectById(xb68_envRainSplashId))
      mgr.SetActorAreaId(*splashEffect, mgr.GetNextAreaId());
  float camUpness = camXf.basis[1].dot(zeus::skUp);
  float splashRateFactor;
  if (x24_enableSplash)
    splashRateFactor = std::max(0.f, camUpness) * x30_fxDensity;
  else
    splashRateFactor = 0.f;
  float forwardRateFactor = 0.f;
  if (x24_enableSplash && camUpness >= -0.1f) {
    zeus::CVector3f pRelVel = mgr.GetPlayer().GetTransform().transposeRotate(mgr.GetPlayer().GetVelocity());
    if (pRelVel.canBeNormalized()) {
      float velMag = pRelVel.magnitude();
      zeus::CVector3f normRelVel = pRelVel * (1.f / velMag);
      forwardRateFactor = std::min(velMag / 60.f, 1.f) * normRelVel.dot(zeus::skForward);
    }
  }
  float additionalFactor;
  if (fxType == EEnvFxType::Rain)
    additionalFactor = splashRateFactor + forwardRateFactor;
  else
    additionalFactor = 0.f;
  SetSplashEffectRate(xb54_baseSplashRate + additionalFactor, mgr);
  xb54_baseSplashRate = 0.f;
}

void CEnvFxManager::MoveWrapCells(s32 moveX, s32 moveY) {
  if (moveX == 0 && moveY == 0)
    return;
  bool r5 = std::fabs(moveX) >= 1.f || std::fabs(moveY) >= 1.f;
  s32 moveXMaj = moveX << 11;
  s32 moveYMaj = moveY << 11;
  for (int i = 0; i < 8; ++i) {
    s32 r28 = i - moveY;
    for (int j = 0; j < 8; ++j) {
      CEnvFxManagerGrid& grid = x50_grids[i * 8 + j];
      s32 r3 = j - moveX;
      if (!r5) {
        CEnvFxManagerGrid& otherGrid = x50_grids[r28 * 8 + r3];
        grid.x14_block = otherGrid.x14_block;
      } else {
        grid.x0_24_blockDirty = true;
      }
      grid.x4_position =
          zeus::CVector2i((moveXMaj + grid.x4_position.x) & 0x3fff, (moveYMaj + grid.x4_position.y) & 0x3fff);
    }
  }
}

void CEnvFxManager::CalculateSnowForces(const CVectorFixed8_8& zVec,
                                        rstl::reserved_vector<CVectorFixed8_8, 256>& snowForces, EEnvFxType type,
                                        const zeus::CVector3f& oopbtws, float dt) {
  if (type == EEnvFxType::Snow) {
    CVectorFixed8_8 vecf;
    zeus::CVector3f vec;
    for (int i = 255; i >= 0; --i) {
      const zeus::CVector2f& force = g_SnowForces[i];
      zeus::CVector3f delta = zeus::CVector3f(force * dt) * oopbtws;
      vec += delta;
      CVectorFixed8_8 vecf2(vec);
      snowForces.push_back(vecf2 - vecf);
      vecf = vecf2;
    }

    for (int i = 0; i < snowForces.size(); ++i)
      snowForces[i] = snowForces[i] + zVec + CVectorFixed8_8(xb84_snowZDeltas[i & 0xf] * dt * oopbtws);
  }
}

void CEnvFxManager::BuildBlockObjectList(rstl::reserved_vector<TUniqueId, 1024>& list, CStateManager& mgr) {
  for (CEntity* ent : mgr.GetAllObjectList()) {
    TCastToPtr<CScriptTrigger> trig = ent;
    if (trig && (trig->GetTriggerFlags() & ETriggerFlags::BlockEnvironmentalEffects) != ETriggerFlags::None) {
      list.push_back(ent->GetUniqueId());
    } else if (TCastToPtr<CScriptWater> water = ent) {
      list.push_back(ent->GetUniqueId());
    }
  }
}

void CEnvFxManager::UpdateBlockedGrids(CStateManager& mgr, EEnvFxType type, const zeus::CTransform& camXf,
                                       const zeus::CTransform& xf, const zeus::CTransform& invXf) {
  zeus::CVector3f playerPos;
  if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Unmorphed)
    playerPos = camXf.origin;
  else
    playerPos = mgr.GetPlayer().GetBallPosition();
  zeus::CVector2i localPlayerPos((invXf * playerPos * 256.f).toVec2f());
  x2c_lastBlockedGridIdx = -1;
  x24_enableSplash = false;
  rstl::reserved_vector<TUniqueId, 1024> blockList;
  bool blockListBuilt = false;
  int blockedGrids = 0;
  for (int i = 0; i < x50_grids.size(); ++i) {
    CEnvFxManagerGrid& grid = x50_grids[i];
    if (blockedGrids < 8 && grid.x0_24_blockDirty) {
      if (type == EEnvFxType::UnderwaterFlake) {
        grid.x14_block = std::make_pair(true, float(-FLT_MAX));
      } else {
        CMaterialFilter filter =
            CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid, EMaterialTypes::Trigger},
                                                {EMaterialTypes::ProjectilePassthrough, EMaterialTypes::SeeThrough});
        zeus::CVector3f pos = xf * zeus::CVector3f((grid.x4_position + grid.xc_extent * 0).toVec2f() / 256.f) +
                              zeus::skUp * 500.f;
        CRayCastResult result =
            CGameCollision::RayStaticIntersection(mgr, pos, zeus::skDown, 1000.f, filter);
        if (result.IsValid()) {
          if (!blockListBuilt) {
            BuildBlockObjectList(blockList, mgr);
            blockListBuilt = true;
          }
          for (TUniqueId id : blockList) {
            if (TCastToConstPtr<CScriptTrigger> trig = mgr.GetObjectById(id)) {
              if (auto tb = trig->GetTouchBounds()) {
                CCollidableAABox caabb(*tb, {EMaterialTypes::Trigger});
                CRayCastResult result2 = caabb.CastRayInternal({pos, zeus::skDown, 1000.f, {}, filter});
                if (result2.IsValid() && result2.GetT() < result.GetT())
                  result = result2;
              }
            }
          }
        }
        ++blockedGrids;
        grid.x14_block = std::make_pair(result.IsValid(), result.GetPoint().z());
      }
      grid.x0_24_blockDirty = false;
    }
    zeus::CVector2i gridEnd = grid.x4_position + grid.xc_extent;
    if (localPlayerPos.x >= grid.x4_position.x && localPlayerPos.y >= grid.x4_position.y &&
        localPlayerPos.x < gridEnd.x && localPlayerPos.y < gridEnd.y && grid.x14_block.first &&
        grid.x14_block.second <= playerPos.z()) {
      x24_enableSplash = true;
      x2c_lastBlockedGridIdx = i;
    }
  }
}

void CEnvFxManager::CreateNewParticles(EEnvFxType type) {
  int maxCellParticleCount;
  if (type == EEnvFxType::Snow)
    maxCellParticleCount = 0x1c98;
  else if (type == EEnvFxType::Rain)
    maxCellParticleCount = 0x2af8;
  else if (type == EEnvFxType::UnderwaterFlake)
    maxCellParticleCount = 0x1fd6;
  else
    maxCellParticleCount = 0;
  maxCellParticleCount >>= 6;
  int cellParticleCount = maxCellParticleCount * x30_fxDensity;
  static u32 seedStash = 0;
  CRandom16 random(seedStash);
  for (auto it = x50_grids.rbegin(); it != x50_grids.rend(); ++it) {
    if (it->x14_block.first) {
      if (cellParticleCount > it->x1c_particles.size()) {
        if (cellParticleCount > it->x1c_particles.capacity())
          it->x1c_particles.reserve(maxCellParticleCount);
        int remCellParticleCount = cellParticleCount - it->x1c_particles.size();
        for (int i = 0; i < remCellParticleCount; ++i) {
          int x = random.Range(0.f, float(it->xc_extent.x));
          int y = random.Range(0.f, float(it->xc_extent.y));
          int z = 256.f * random.Range(0.f, 63.f);
          it->x1c_particles.emplace_back(x, y, z);
        }
      } else {
        it->x1c_particles.resize(cellParticleCount);
      }
    }
  }
  seedStash = random.GetSeed();
}

void CEnvFxManager::UpdateSnowParticles(const rstl::reserved_vector<CVectorFixed8_8, 256>& snowForces) {
  for (auto it = x50_grids.rbegin(); it != x50_grids.rend(); ++it) {
    int forceIt = int(x28_firstSnowForce);
    if (it->x14_block.first) {
      for (auto pit = it->x1c_particles.rbegin(); pit != it->x1c_particles.rend(); ++pit) {
        const CVectorFixed8_8& force = snowForces[forceIt];
        forceIt = (forceIt + 1) & 0xff;
        *pit = *pit + force;
        pit->z = s16(pit->z & 0x3fff);
      }
    }
  }
}

void CEnvFxManager::UpdateRainParticles(const CVectorFixed8_8& zVec, const zeus::CVector3f& oopbtws, float dt) {
  s16 deltaZ = zVec.z + s16(-40.f * dt * oopbtws.z() * 256.f);
  for (auto it = x50_grids.rbegin(); it != x50_grids.rend(); ++it)
    for (auto pit = it->x1c_particles.rbegin(); pit != it->x1c_particles.rend(); ++pit)
      pit->z = s16((pit->z + deltaZ) & 0x3fff);
}

void CEnvFxManager::UpdateUnderwaterParticles(const CVectorFixed8_8& zVec) {
  for (auto it = x50_grids.rbegin(); it != x50_grids.rend(); ++it)
    for (auto pit = it->x1c_particles.rbegin(); pit != it->x1c_particles.rend(); ++pit)
      pit->z = s16((pit->z + zVec.z) & 0x3fff);
}

void CEnvFxManager::Update(float dt, CStateManager& mgr) {
  EEnvFxType fxType = mgr.GetWorld()->GetNeededEnvFx();
  zeus::CTransform camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
  if (mgr.GetCameraManager()->GetFluidCounter() != 0) {
    x2c_lastBlockedGridIdx = -1;
    x24_enableSplash = false;
    SetSplashEffectRate(0.f, mgr);
  }
  UpdateRainSounds(mgr);
  UpdateVisorSplash(mgr, dt, camXf);
  if (fxType == EEnvFxType::None) {
    for (auto it = x50_grids.rbegin(); it != x50_grids.rend(); ++it)
      if (it->x14_block.first)
        it->x1c_particles = std::vector<CVectorFixed8_8>();
  } else {
    float densityDelta = x34_targetFxDensity - x30_fxDensity;
    float densityDeltaDamper = std::min(std::fabs(densityDelta) / 0.15f, 1.f);
    float maxDensityDelta = x38_maxDensityDeltaSpeed / 11000.f * dt;
    if (std::fabs(densityDelta) > maxDensityDelta)
      densityDelta = (densityDelta > 0.f ? 1.f : -1.f) * maxDensityDelta;
    x30_fxDensity += densityDeltaDamper * densityDelta;
    zeus::CVector3f pbtws = GetParticleBoundsToWorldScale();
    zeus::CVector3f oopbtws = 1.f / pbtws;
    zeus::CVector3f forwardPoint = camXf.basis[1] * 23.8125f + camXf.origin;
    float modX = std::fmod(forwardPoint.x(), 7.9375f);
    float modY = std::fmod(forwardPoint.y(), 7.9375f);
    s32 moveX = (x18_focusCellPosition.x() - (forwardPoint.x() - modX)) / 7.9375f;
    x18_focusCellPosition.x() = forwardPoint.x() - modX;
    s32 moveY = (x18_focusCellPosition.y() - (forwardPoint.y() - modY)) / 7.9375f;
    x18_focusCellPosition.y() = forwardPoint.y() - modY;
    float deltaZ = x18_focusCellPosition.z() - forwardPoint.z();
    x18_focusCellPosition.z() = float(forwardPoint.z());
    MoveWrapCells(moveX, moveY);
    CVectorFixed8_8 zVec(oopbtws * zeus::CVector3f(0.f, 0.f, deltaZ));
    if (fxType == EEnvFxType::UnderwaterFlake)
      zVec.z += s16(256.f * 0.5f * dt);
    rstl::reserved_vector<CVectorFixed8_8, 256> snowForces;
    CalculateSnowForces(zVec, snowForces, fxType, oopbtws, dt);
    zeus::CTransform xf = GetParticleBoundsToWorldTransform();
    zeus::CTransform invXf = xf.inverse();
    UpdateBlockedGrids(mgr, fxType, camXf, xf, invXf);
    CreateNewParticles(fxType);
    switch (fxType) {
    case EEnvFxType::Snow:
      UpdateSnowParticles(snowForces);
      break;
    case EEnvFxType::Rain:
      UpdateRainParticles(zVec, oopbtws, dt);
      break;
    case EEnvFxType::UnderwaterFlake:
      UpdateUnderwaterParticles(zVec);
      break;
    default:
      break;
    }
    if (fxType == EEnvFxType::Snow)
      x28_firstSnowForce = std::fmod(1.f + x28_firstSnowForce, 256.f);
    else
      x28_firstSnowForce = std::fmod(0.125f + x28_firstSnowForce, 256.f);
  }
}

static zeus::CColor GetFlakeColor(const zeus::CMatrix4f& mvp, const CEnvFxShaders::Instance& inst) {
  float screenHeight =
      std::fabs(mvp.multiplyOneOverW(inst.positions[1]).y() - mvp.multiplyOneOverW(inst.positions[0]).y()) / 2.f;
  screenHeight -= (32.f / 480.f);
  screenHeight /= (32.f / 480.f);
  return zeus::CColor(1.f - zeus::clamp(0.f, screenHeight, 1.f), 1.f);
}

void CEnvFxManagerGrid::RenderSnowParticles(const zeus::CTransform& camXf) const {
  zeus::CVector3f xVec = 0.2f * camXf.basis[0];
  zeus::CVector3f zVec = 0.2f * camXf.basis[2];
  zeus::CMatrix4f mvp = CGraphics::GetPerspectiveProjectionMatrix(false) * CGraphics::g_GXModelView.toMatrix4f();
  auto* bufOut = m_instBuf.access();
  for (const auto& particle : x1c_particles) {
    bufOut->positions[0] = particle.toVec3f();
    bufOut->uvs[0] = zeus::CVector2f(0.f, 0.f);
    bufOut->positions[1] = bufOut->positions[0] + zVec;
    bufOut->uvs[1] = zeus::CVector2f(0.f, 1.f);
    bufOut->positions[3] = bufOut->positions[1] + xVec;
    bufOut->uvs[3] = zeus::CVector2f(1.f, 1.f);
    bufOut->positions[2] = bufOut->positions[3] - zVec;
    bufOut->uvs[2] = zeus::CVector2f(1.f, 0.f);
    bufOut->color = GetFlakeColor(mvp, *bufOut);
    ++bufOut;
  }
  CGraphics::SetShaderDataBinding(m_snowBinding);
  CGraphics::DrawInstances(0, 4, x1c_particles.size());
}

void CEnvFxManagerGrid::RenderRainParticles(const zeus::CTransform& camXf) const {
  m_lineRenderer.Reset();
  float zOffset = 2.f * (1.f - std::fabs(camXf.basis[2].dot(zeus::skUp))) + 1.f;
  zeus::CColor color0(1.f, 10.f / 15.f);
  for (const auto& particle : x1c_particles) {
    zeus::CVector3f pos0 = particle.toVec3f();
    zeus::CVector3f pos1 = pos0;
    pos1.z() += zOffset;
    float uvy0 = pos0.z() * 10.f + m_uvyOffset;
    float uvy1 = pos1.z() * 10.f + m_uvyOffset;
    m_lineRenderer.AddVertex(pos0, zeus::skWhite, 1.f, {0.f, uvy0});
    m_lineRenderer.AddVertex(pos1, zeus::skClear, 1.f, {0.f, uvy1});
  }
  m_lineRenderer.Render(g_Renderer->IsThermalVisorHotPass(), zeus::CColor(1.f, 0.15f));
}

void CEnvFxManagerGrid::RenderUnderwaterParticles(const zeus::CTransform& camXf) const {
  zeus::CVector3f xVec = 0.5f * camXf.basis[0];
  zeus::CVector3f zVec = 0.5f * camXf.basis[2];
  zeus::CMatrix4f mvp = CGraphics::GetPerspectiveProjectionMatrix(false) * CGraphics::g_GXModelView.toMatrix4f();
  auto* bufOut = m_instBuf.access();
  for (const auto& particle : x1c_particles) {
    bufOut->positions[0] = particle.toVec3f();
    bufOut->uvs[0] = zeus::CVector2f(0.f, 0.f);
    bufOut->positions[1] = bufOut->positions[0] + zVec;
    bufOut->uvs[1] = zeus::CVector2f(0.f, 1.f);
    bufOut->positions[3] = bufOut->positions[1] + xVec;
    bufOut->uvs[3] = zeus::CVector2f(1.f, 1.f);
    bufOut->positions[2] = bufOut->positions[3] - zVec;
    bufOut->uvs[2] = zeus::CVector2f(1.f, 0.f);
    bufOut->color = GetFlakeColor(mvp, *bufOut);
    ++bufOut;
  }
  CGraphics::SetShaderDataBinding(m_underwaterBinding);
  CGraphics::DrawInstances(0, 4, x1c_particles.size());
}

void CEnvFxManagerGrid::Render(const zeus::CTransform& xf, const zeus::CTransform& invXf, const zeus::CTransform& camXf,
                               float fxDensity, EEnvFxType fxType, const CEnvFxManager& parent) const {
  if (!x1c_particles.empty() && x14_block.first) {
    CGraphics::SetModelMatrix(xf * zeus::CTransform::Translate(x4_position.toVec2f() / 256.f));
    parent.m_uniformData.mv = CGraphics::g_GXModelView.toMatrix4f();
    parent.m_uniformData.proj = CGraphics::GetPerspectiveProjectionMatrix(true);
    switch (fxType) {
    case EEnvFxType::Snow:
    case EEnvFxType::Rain: {
      zeus::CMatrix4f envTexMtx(true);
      envTexMtx[2][1] = 10.f;
      envTexMtx[3][1] = 0.5f - (invXf * (zeus::skUp * x14_block.second)).z() * 10.f;
      m_uvyOffset = envTexMtx[3][1];
      parent.m_uniformData.envMtx = envTexMtx;
      break;
    }
    default:
      break;
    }
    m_uniformBuf.access() = parent.m_uniformData;
    switch (fxType) {
    case EEnvFxType::Snow:
      RenderSnowParticles(camXf);
      break;
    case EEnvFxType::Rain:
      RenderRainParticles(camXf);
      break;
    case EEnvFxType::UnderwaterFlake:
      RenderUnderwaterParticles(camXf);
      break;
    default:
      break;
    }
  }
}

void CEnvFxManager::SetupSnowTevs(const CStateManager& mgr) const {
  mgr.GetCameraManager()->GetCurrentCamera(mgr);
  if (mgr.GetCameraManager()->GetFluidCounter() != 0) {
    g_Renderer->SetWorldFog(ERglFogMode::PerspExp, 0.f, 35.f, zeus::skBlack);
    m_uniformData.moduColor = zeus::CColor(1.f, 0.5f);
  } else {
    g_Renderer->SetWorldFog(ERglFogMode::PerspLin, 52.f, 57.f, zeus::skBlack);
  }

  // Blend One One
  // 2 stages

  // xb74_txtrSnowFlake
  // Texcoord0: 2x4, TEX0, GX_IDENTITY, no norm, GX_PTTIDENTITY
  // 0: Standard alpha, map0, tcg0
  // Color: Zero, Konst, TexC, Zero
  // Alpha: Zero, Konst, TexA, Zero

  // x40_txtrEnvGradient
  // Texcoord1: 2x4, POS, GX_TEXMTX5, no norm, GX_PTTIDENTITY
  // 0: Standard alpha, map0, tcg0
  // Color: Zero, TexC, CPrev, Zero
  // Alpha: Zero, Zero, Zero, TexA
}

void CEnvFxManager::SetupRainTevs() const {
  // Line-width 1px
  // Blend SrcAlpha One

  // x40_txtrEnvGradient
  // Texcoord0: 2x4, POS, GX_TEXMTX5, no norm, GX_PTTIDENTITY
  // 0: Standard alpha, map0, tcg0
  // Color: Zero, Zero, Zero, TexC
  // Alpha: Zero, RasA, Konst, Zero
  // Ras vertex color
  // KAlpha 0.15
}

void CEnvFxManager::SetupUnderwaterTevs(const zeus::CTransform& invXf, const CStateManager& mgr) const {
  // Blend SrcAlpha InvSrcAlpha

  // xc48_underwaterFlake
  // Texcoord0: 2x4, TEX0, GX_IDENTITY, no norm, GX_PTTIDENTITY
  // Color: Zero, Zero, Zero, TexC
  // Alpha: Zero, Zero, Zero, TexA

  float waterTop = FLT_MAX;
  for (CEntity* ent : mgr.GetAllObjectList())
    if (TCastToPtr<CScriptWater> water = ent)
      if (auto tb = water->GetTouchBounds())
        waterTop = std::min(waterTop, float(tb->max.z()));
  zeus::CVector3f localWaterTop = invXf * (zeus::skUp * waterTop);
  zeus::CMatrix4f envTexMtx(true);
  envTexMtx[2][1] = -10.f;
  envTexMtx[3][1] = localWaterTop.z() * -10.f + 0.5f;
  // Load into texmtx5

  // x40_txtrEnvGradient
  // Texcoord1: 2x4, POS, GX_TEXMTX5, no norm, GX_PTTIDENTITY
  // MTX: y-scale -10.0 of Z, y-trans -10.0 * ()
  // 1: Standard alpha, map1, tcg1
  // Color: Zero, One, CPrev, Zero
  // Alpha: Zero, TexA, APrev, Zero
  // Swap: RGBR
}

void CEnvFxManager::Render(const CStateManager& mgr) const {
  EEnvFxType fxType = mgr.GetWorld()->GetNeededEnvFx();
  if (fxType != EEnvFxType::None) {
    if (mgr.GetPlayer().GetMorphballTransitionState() != CPlayer::EPlayerMorphBallState::Unmorphed ||
        (mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::Thermal &&
         (fxType != EEnvFxType::Snow || mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::EPlayerVisor::XRay))) {
      // No Cull
      // ZTest, No ZWrite
      zeus::CTransform xf = GetParticleBoundsToWorldTransform();
      zeus::CTransform invXf = xf.inverse();
      zeus::CTransform camXf = mgr.GetCameraManager()->GetCurrentCameraTransform(mgr);
      m_uniformData.moduColor = zeus::skWhite;
      switch (fxType) {
      case EEnvFxType::Snow:
        SetupSnowTevs(mgr);
        break;
      case EEnvFxType::Rain:
        SetupRainTevs();
        break;
      case EEnvFxType::UnderwaterFlake:
        SetupUnderwaterTevs(invXf, mgr);
        break;
      default:
        break;
      }
      m_fogUniformBuf->load(&CGraphics::g_Fog, sizeof(CGraphics::g_Fog));
      for (const auto& grid : x50_grids)
        grid.Render(xf, invXf, camXf, x30_fxDensity, fxType, *this);
      // Backface cull

      m_uniformPool.updateBuffers();
      m_instPool.updateBuffers();
    }
  }
}

void CEnvFxManager::AsyncLoadResources(CStateManager& mgr) {
  xb68_envRainSplashId = mgr.AllocateUniqueId();
  CHUDBillboardEffect* effect =
      new CHUDBillboardEffect(xb58_envRainSplash, {}, xb68_envRainSplashId, true, "VisorRainSplashes",
                              CHUDBillboardEffect::GetNearClipDistance(mgr), CHUDBillboardEffect::GetScaleForPOV(mgr),
                              zeus::skWhite, zeus::skOne3f, zeus::skZero3f);
  effect->SetRunIndefinitely(true);
  mgr.AddObject(effect);
}

void CEnvFxManager::SetFxDensity(s32 val, float density) {
  x34_targetFxDensity = density;
  x38_maxDensityDeltaSpeed = val;
}

void CEnvFxManager::AreaLoaded() {
  for (CEnvFxManagerGrid& grid : x50_grids)
    grid.x0_24_blockDirty = true;
}

void CEnvFxManager::Cleanup() {
  xb68_envRainSplashId = kInvalidUniqueId;
  xb6a_rainSoundActive = false;
  xb6c_leftRainSound.reset();
  xb70_rightRainSound.reset();
}

void CEnvFxManager::Initialize() {
  const SObjectTag* tag = g_ResFactory->GetResourceIdByName("DUMB_SnowForces");
  std::unique_ptr<u8[]> data = g_ResFactory->LoadResourceSync(*tag);
  athena::io::MemoryReader r(data.get(), 2048);
  for (int i = 0; i < 256; ++i)
    g_SnowForces.push_back(r.readVec2fBig());
}

} // namespace urde
