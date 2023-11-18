#include "Runtime/Particle/CElementGen.hpp"

#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Character/CActorLights.hpp"
#include "Runtime/Graphics/CCubeRenderer.hpp"
#include "Runtime/Graphics/CGX.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Graphics/Shaders/CElementGenShaders.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CParticleGlobals.hpp"
#include "Runtime/Particle/CParticleElectric.hpp"
#include "Runtime/Particle/CParticleSwoosh.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"
#include "Runtime/Particle/CWarp.hpp"

#define MAX_GLOBAL_PARTICLES 2560

namespace metaforce {
namespace {
logvisor::Module Log("metaforce::CElementGen");

// constexpr std::array ShadClsSizes{
//     sizeof(SParticleInstanceTex),
//     sizeof(SParticleInstanceIndTex),
//     sizeof(SParticleInstanceNoTex),
// };
} // Anonymous namespace

u16 CElementGen::g_GlobalSeed = 99;
bool CElementGen::g_subtractBlend = false;

int CElementGen::g_ParticleAliveCount;
int CElementGen::g_ParticleSystemAliveCount;
bool CElementGen::g_ParticleSystemInitialized = false;
bool CElementGen::sMoveRedToAlphaBuffer = false;
CParticle* CElementGen::g_currentParticle = nullptr;

// std::vector<SParticleInstanceTex> g_instTexData;
// std::vector<SParticleInstanceIndTex> g_instIndTexData;
// std::vector<SParticleInstanceNoTex> g_instNoTexData;

void CElementGen::Initialize() {
  if (g_ParticleSystemInitialized)
    return;

  g_ParticleAliveCount = 0;
  g_ParticleSystemAliveCount = 0;
  g_ParticleSystemInitialized = true;

  /* Compile shaders */
  CElementGenShaders::Initialize();
}

void CElementGen::Shutdown() { CElementGenShaders::Shutdown(); }

CElementGen::CElementGen(TToken<CGenDescription> gen, EModelOrientationType orientType, EOptionalSystemFlags flags)
: x1c_genDesc(std::move(gen))
, x2c_orientType(orientType)
, x26d_27_enableOPTS(True(flags & EOptionalSystemFlags::Two))
, x27c_randState(x94_randomSeed) {
  CGenDescription* desc = x1c_genDesc.GetObj();
  x28_loadedGenDesc = desc;

  if (desc->x54_x40_TEXR)
    desc->x54_x40_TEXR->GetValueTexture(0).GetObj();
  if (desc->x58_x44_TIND)
    desc->x58_x44_TIND->GetValueTexture(0).GetObj();

  CGlobalRandom globRnd(x27c_randState);
  if (CIntElement* seedElem = desc->x1c_x10_SEED.get()) {
    int seedVal;
    seedElem->GetValue(x74_curFrame, seedVal);
    x94_randomSeed = seedVal;
  }
  x27c_randState.SetSeed(x94_randomSeed);
  ++g_ParticleSystemAliveCount;
  x26c_25_LIT_ = desc->x44_29_x30_29_LIT_;
  x26c_26_AAPH = desc->x44_26_x30_26_AAPH;
  x26c_27_ZBUF = desc->x44_27_x30_27_ZBUF;
  x26c_28_zTest = true;
  x26c_29_ORNT = desc->x30_30_ORNT;
  x26c_30_MBLR = x26c_29_ORNT ? false : desc->x44_30_x31_24_MBLR;

  if (CIntElement* mbspElem = desc->x48_x34_MBSP.get())
    mbspElem->GetValue(x74_curFrame, x270_MBSP);

  // x280_VELSources' entries should be tightly packed
  size_t idx = 0;
  if (CModVectorElement* elem = desc->x7c_x68_VEL1.get()) {
    x280_VELSources[idx] = elem;
    x278_hasVMD[idx] = desc->x45_26_x31_28_VMD1;
    idx++;
  }

  if (CModVectorElement* elem = desc->x80_x6c_VEL2.get()) {
    x280_VELSources[idx] = elem;
    x278_hasVMD[idx] = desc->x45_27_x31_29_VMD2;
    idx++;
  }

  if (CModVectorElement* elem = desc->x84_x70_VEL3.get()) {
    x280_VELSources[idx] = elem;
    x278_hasVMD[idx] = desc->x45_28_x31_30_VMD3;
    idx++;
  }

  if (CModVectorElement* elem = desc->x88_x74_VEL4.get()) {
    x280_VELSources[idx] = elem;
    x278_hasVMD[idx] = desc->x45_29_x31_31_VMD4;
    idx++;
  }

  if (desc->x10c_ADV1 || desc->x110_ADV2 || desc->x114_ADV3 || desc->x118_ADV4 || desc->x11c_ADV5 || desc->x120_ADV6 ||
      desc->x124_ADV7 || desc->x128_ADV8)
    x26d_28_enableADV = true;

  if (CIntElement* cssdElem = desc->xa0_x8c_CSSD.get())
    cssdElem->GetValue(0, x2a0_CSSD);

  if (CIntElement* pisyElem = desc->xc8_xb4_PISY.get()) {
    pisyElem->GetValue(0, x2a8_PISY);
    if (x2a8_PISY <= 0)
      x2a8_PISY = 1;
  }

  if (CIntElement* sisyElem = desc->xcc_xb8_SISY.get())
    sisyElem->GetValue(0, x2a4_SISY);

  if (CIntElement* sssdElem = desc->xe4_xd0_SSSD.get())
    sssdElem->GetValue(0, x2ac_SSSD);

  if (CVectorElement* sspoElem = desc->xe8_xd4_SSPO.get()) {
    sspoElem->GetValue(0, x2b0_SSPO);
    if (!sspoElem->IsFastConstant())
      x26c_24_translationDirty = true;
  }

  if (CIntElement* sesdElem = desc->xf8_xe4_SESD.get())
    sesdElem->GetValue(0, x2bc_SESD);

  if (CVectorElement* sepoElem = desc->xfc_xe8_SEPO.get()) {
    sepoElem->GetValue(0, x2c0_SEPO);
    if (!sepoElem->IsFastConstant())
      x26c_24_translationDirty = true;
  }

  if (CVectorElement* pofsElem = desc->x18_xc_POFS.get()) {
    pofsElem->GetValue(x74_curFrame, xf4_POFS);
    if (!pofsElem->IsFastConstant())
      x26c_24_translationDirty = true;
  }

  if (CIntElement* psltElem = desc->xc_x0_PSLT.get())
    psltElem->GetValue(0, x268_PSLT);
  else
    x268_PSLT = INT_MAX;

  if (CIntElement* maxpElem = desc->x28_x1c_MAXP.get()) {
    maxpElem->GetValue(x74_curFrame, x90_MAXP);
  }

  s32 count = std::min(256, x90_MAXP);
  x30_particles.reserve(count);
  if (x26d_28_enableADV) {
    x60_advValues.resize(count);
  }
  if (x2c_orientType == EModelOrientationType::One)
    x50_parentMatrices.resize(x90_MAXP);

  x26c_31_LINE = desc->x44_24_x30_24_LINE;
  x26d_24_FXLL = desc->x44_25_x30_25_FXLL;

  if (CIntElement* ltypElem = desc->x100_xec_LTYP.get()) {
    int ltyp;
    ltypElem->GetValue(x74_curFrame, ltyp);
    switch (LightType(ltyp)) {
    case LightType::None:
    default:
      x308_lightType = LightType::None;
      break;
    case LightType::Directional:
      x308_lightType = LightType::Directional;
      break;
    case LightType::Custom:
      x308_lightType = LightType::Custom;
      break;
    case LightType::Spot:
      x308_lightType = LightType::Spot;
      break;
    }
  }

  if (CIntElement* lfotElem = desc->x114_x100_LFOT.get()) {
    int lfot;
    lfotElem->GetValue(x74_curFrame, lfot);
    switch (EFalloffType(lfot)) {
    case EFalloffType::Constant:
      x32c_falloffType = EFalloffType::Constant;
      break;
    case EFalloffType::Linear:
    default:
      x32c_falloffType = EFalloffType::Linear;
      break;
    case EFalloffType::Quadratic:
      x32c_falloffType = EFalloffType::Quadratic;
      break;
    }
  }

  if (x26c_31_LINE) {
    CUVElement* texr = desc->x54_x40_TEXR.get();
    // TODO
    //    aurora::gfx::TextureHandle tex;
    //    if (texr)
    //      tex = texr->GetValueTexture(0).GetObj()->GetTexture();
    int maxVerts = x90_MAXP;
    //    m_lineRenderer.reset(
    //        new CLineRenderer(CLineRenderer::EPrimitiveMode::Lines, maxVerts * 2, tex, x26c_26_AAPH, x26c_28_zTest));
  } else {
    m_shaderClass = CElementGenShaders::GetShaderClass(*this);
  }

  _RecreatePipelines();
  //  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
  //    CElementGenShaders::BuildShaderDataBinding(ctx, *this);
  //    return true;
  //  } BooTrace);
}

CElementGen::~CElementGen() {
  --g_ParticleSystemAliveCount;
  g_ParticleAliveCount -= x30_particles.size();
}

bool CElementGen::Update(double t) {
  s32 oldMax = x90_MAXP;
  s32 oldMBSP = x270_MBSP;
  CParticleGlobals::SParticleSystem* prevSystem = CParticleGlobals::instance()->m_currentParticleSystem;
  CParticleGlobals::SParticleSystem thisSystem{FOURCC('PART'), this};
  CParticleGlobals::instance()->m_currentParticleSystem = &thisSystem;
  CGenDescription* desc = x1c_genDesc.GetObj();
  CIntElement* pswtElem = desc->x10_x4_PSWT.get();
  if (pswtElem && !x26d_25_warmedUp) {
    int pswt = 0;
    pswtElem->GetValue(x74_curFrame, pswt);
    // Log.report(logvisor::Info, FMT_STRING("Running warmup on particle system 0x%08x for %d ticks."), desc, pswt);
    InternalUpdate((1.f / 60.f) * pswt);
    x26d_25_warmedUp = true;
  }
  bool ret = InternalUpdate(t);
  CParticleGlobals::instance()->m_currentParticleSystem = prevSystem;

  if (oldMax < x90_MAXP || oldMBSP < x270_MBSP) {
    //_RecreatePipelines();
  }
  return ret;
}

void CElementGen::_RecreatePipelines() {
  size_t maxInsts = x26c_30_MBLR ? 2560 * 2 : 2560; // x26c_30_MBLR ? (x270_MBSP * x90_MAXP) : x90_MAXP;
  maxInsts = (maxInsts == 0 ? 256 : maxInsts);

  //  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
  //    if (!x26c_31_LINE) {
  //      m_instBuf = ctx.newDynamicBuffer(boo::BufferUse::Vertex, ShadClsSizes[size_t(m_shaderClass)], maxInsts);
  //      m_uniformBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(SParticleUniforms), 1);
  //    }
  //    if (x28_loadedGenDesc->x45_24_x31_26_PMUS) {
  //      m_instBufPmus = ctx.newDynamicBuffer(boo::BufferUse::Vertex, ShadClsSizes[size_t(m_shaderClass)], maxInsts);
  //      m_uniformBufPmus = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(SParticleUniforms), 1);
  //    }
  //    return true;
  //  } BooTrace);
}

bool CElementGen::InternalUpdate(double dt) {
  CGlobalRandom gr(x27c_randState);
  CGenDescription* desc = x1c_genDesc.GetObj();

  double dt1 = 1.0 / 60.0;
  if (std::fabs(dt - 1.0 / 60.0) >= 1.0 / 60000.0)
    dt1 = dt;
  double t = x74_curFrame / 60.0;
  CParticleGlobals::instance()->SetEmitterTime(x74_curFrame);

  if (CRealElement* pstsElem = desc->x14_x8_PSTS.get()) {
    float psts;
    pstsElem->GetValue(x74_curFrame, psts);
    double dt1Scaled = psts * dt1;
    dt1 = std::max(0.0, dt1Scaled);
  }

  x78_curSeconds += dt1;

  if (x26c_30_MBLR && dt > 0.0) {
    if (CIntElement* mbspElem = desc->x48_x34_MBSP.get())
      mbspElem->GetValue(x74_curFrame, x270_MBSP);
  }

  int frameUpdateCount = 0;
  while (t < x78_curSeconds && std::fabs(t - x78_curSeconds) >= 1.0 / 60000.0) {
    x2d4_aabbMin.splat(FLT_MAX);
    x2e0_aabbMax.splat(-FLT_MAX);
    x2ec_maxSize = 0.f;
    CParticleGlobals::instance()->SetEmitterTime(x74_curFrame);
    UpdateExistingParticles();
    CParticleGlobals::instance()->SetParticleLifetime(x268_PSLT);

    if (x74_curFrame < x268_PSLT && x88_particleEmission) {
      float grte = 0.f;
      if (CRealElement* grteElem = desc->x2c_x20_GRTE.get()) {
        if (grteElem->GetValue(x74_curFrame, grte)) {
          x30_particles.clear();
          return true;
        }
      }

      grte = std::max(0.f, grte * x98_generatorRate);
      x8c_generatorRemainder += grte;
      int genCount = floorf(x8c_generatorRemainder);
      x8c_generatorRemainder = x8c_generatorRemainder - genCount;

      if (CIntElement* maxpElem = desc->x28_x1c_MAXP.get())
        maxpElem->GetValue(x74_curFrame, x90_MAXP);

      CreateNewParticles(genCount);
    }

    if (x26c_24_translationDirty)
      UpdatePSTranslationAndOrientation();

    if (x308_lightType != LightType::None)
      UpdateLightParameters();

    UpdateChildParticleSystems(1 / 60.0);

    ++frameUpdateCount;
    ++x74_curFrame;
    t += 1 / 60.0;
  }

  if (std::fabs(t - x78_curSeconds) < 1.0 / 60000.0) {
    x78_curSeconds = t;
    x80_timeDeltaScale = 1.f;
  } else {
    UpdateChildParticleSystems(dt1 - (frameUpdateCount / 60.0));
    x80_timeDeltaScale = 1.f - (t - x78_curSeconds) * 60.f;
  }

  BuildParticleSystemBounds();

  return false;
}

void CElementGen::AccumulateBounds(const zeus::CVector3f& pos, float size) {
  x2e0_aabbMax[0] = std::max(pos[0], float(x2e0_aabbMax[0]));
  x2e0_aabbMax[1] = std::max(pos[1], float(x2e0_aabbMax[1]));
  x2e0_aabbMax[2] = std::max(pos[2], float(x2e0_aabbMax[2]));
  x2d4_aabbMin[0] = std::min(pos[0], float(x2d4_aabbMin[0]));
  x2d4_aabbMin[1] = std::min(pos[1], float(x2d4_aabbMin[1]));
  x2d4_aabbMin[2] = std::min(pos[2], float(x2d4_aabbMin[2]));
  x2ec_maxSize = std::max(size, x2ec_maxSize);
}

void CElementGen::UpdateAdvanceAccessParameters(u32 activeParticleCount, s32 particleFrame) {
  CGenDescription* desc = x28_loadedGenDesc;

  if (activeParticleCount >= x60_advValues.size()) {
    Log.report(logvisor::Fatal, FMT_STRING("activeParticleCount ({}) >= advValues size ({})"), activeParticleCount,
               x60_advValues.size());
  }

  std::array<float, 8>& arr = x60_advValues[activeParticleCount];
  CParticleGlobals::instance()->m_particleAccessParameters = &arr;

  if (CRealElement* adv1 = desc->x10c_ADV1.get()) {
    adv1->GetValue(particleFrame, arr[0]);
  }
  if (CRealElement* adv2 = desc->x110_ADV2.get()) {
    adv2->GetValue(particleFrame, arr[1]);
  }
  if (CRealElement* adv3 = desc->x114_ADV3.get()) {
    adv3->GetValue(particleFrame, arr[2]);
  }
  if (CRealElement* adv4 = desc->x118_ADV4.get()) {
    adv4->GetValue(particleFrame, arr[3]);
  }
  if (CRealElement* adv5 = desc->x11c_ADV5.get()) {
    adv5->GetValue(particleFrame, arr[4]);
  }
  if (CRealElement* adv6 = desc->x120_ADV6.get()) {
    adv6->GetValue(particleFrame, arr[5]);
  }
  if (CRealElement* adv7 = desc->x124_ADV7.get()) {
    adv7->GetValue(particleFrame, arr[6]);
  }
  if (CRealElement* adv8 = desc->x128_ADV8.get()) {
    adv8->GetValue(particleFrame, arr[7]);
  }
}

bool CElementGen::UpdateVelocitySource(size_t idx, s32 particleFrame, CParticle& particle) {
  bool err;
  if (x278_hasVMD[idx]) {
    zeus::CVector3f localVel = x208_orientationInverse * particle.x1c_vel;
    zeus::CVector3f localPos = x208_orientationInverse * (particle.x4_pos - xdc_translation);
    err = x280_VELSources[idx]->GetValue(particleFrame, localVel, localPos);
    particle.x1c_vel = x1d8_orientation.rotate(localVel);
    particle.x4_pos = x1d8_orientation.rotate(localPos) + xdc_translation;
  } else {
    err = x280_VELSources[idx]->GetValue(particleFrame, particle.x1c_vel, particle.x4_pos);
  }

  if (err) {
    particle.x0_endFrame = -1;
    return true;
  }

  return false;
}

void CElementGen::UpdateExistingParticles() {
  CGenDescription* desc = x1c_genDesc.GetObj();

  x25c_activeParticleCount = 0;
  CParticleGlobals::instance()->SetEmitterTime(x74_curFrame);
  CParticleGlobals::instance()->m_particleAccessParameters = nullptr;

  for (auto it = x30_particles.begin(); it != x30_particles.end();) {
    CParticle& particle = *it;

    if (particle.x0_endFrame < x74_curFrame) {
      --g_ParticleAliveCount;
      if (it + 1 == x30_particles.end()) {
        x30_particles.pop_back();
        break;
      } else {
        particle = x30_particles.back();

        if (x2c_orientType == EModelOrientationType::One)
          x50_parentMatrices[x25c_activeParticleCount] = x50_parentMatrices[x30_particles.size() - 1];

        if (x26d_28_enableADV)
          x60_advValues[x25c_activeParticleCount] = x60_advValues[x30_particles.size() - 1];

        x30_particles.pop_back();
        if (particle.x0_endFrame < x74_curFrame)
          continue;
      }
    }

    particle.x10_prevPos = particle.x4_pos;
    particle.x4_pos += particle.x1c_vel;

    g_currentParticle = &particle;

    CParticleGlobals::instance()->SetParticleLifetime(particle.x0_endFrame - particle.x28_startFrame);
    const int particleFrame = x74_curFrame - particle.x28_startFrame;
    CParticleGlobals::instance()->UpdateParticleLifetimeTweenValues(particleFrame);

    if (x26d_28_enableADV) {
      UpdateAdvanceAccessParameters(x25c_activeParticleCount, particleFrame);
    }

    ++x25c_activeParticleCount;

    for (size_t i = 0; i < x280_VELSources.size(); ++i) {
      if (!x280_VELSources[i]) {
        break;
      }
      UpdateVelocitySource(i, particleFrame, particle);
    }

    if (x26c_31_LINE) {
      if (CRealElement* leng = desc->x20_x14_LENG.get())
        leng->GetValue(particleFrame, particle.x2c_lineLengthOrSize);
      if (CRealElement* widt = desc->x24_x18_WIDT.get())
        widt->GetValue(particleFrame, particle.x30_lineWidthOrRota);
    } else {
      if (CRealElement* rota = desc->x50_x3c_ROTA.get())
        rota->GetValue(particleFrame, particle.x30_lineWidthOrRota);
      if (CRealElement* size = desc->x4c_x38_SIZE.get())
        size->GetValue(particleFrame, particle.x2c_lineLengthOrSize);
    }

    if (CColorElement* colr = desc->x30_x24_COLR.get())
      colr->GetValue(particleFrame, particle.x34_color);

    AccumulateBounds(particle.x4_pos, particle.x2c_lineLengthOrSize);
    ++it;
  }

  if (x30_particles.empty())
    return;

  for (CWarp* warp : x4_modifierList)
    if (warp->UpdateWarp())
      warp->ModifyParticles(x30_particles);
}

void CElementGen::CreateNewParticles(int count) {
  CGenDescription* desc = x1c_genDesc.GetObj();

  if (!g_ParticleSystemInitialized) {
    Initialize();
  }

  if (x30_particles.size() >= x90_MAXP) {
    return;
  }

  if (count + x30_particles.size() > x90_MAXP) {
    count = x90_MAXP - x30_particles.size();
  }

  if (g_ParticleAliveCount + count > 2560) {
    count = 2560 - g_ParticleAliveCount;
  }

  CGlobalRandom gr(x27c_randState);
  x30_particles.reserve(count + x90_MAXP);
  if (x26d_28_enableADV && x60_advValues.size() < count + x30_particles.size()) {
    x60_advValues.resize(std::min(int(x60_advValues.size() * 2), x90_MAXP));
  }

  CParticleGlobals::instance()->m_particleAccessParameters = nullptr;

  for (int i = 0; i < count; ++i) {
    CParticle& particle = x30_particles.emplace_back();
    ++g_ParticleAliveCount;
    u32 particleCount = x30_particles.size() - 1;
    ++x25c_activeParticleCount;
    ++x260_cumulativeParticles;
    if (x2c_orientType == EModelOrientationType::One) {
      x50_parentMatrices[x30_particles.size() - 1] = x1d8_orientation.buildMatrix3f();
    }

    particle.x28_startFrame = x74_curFrame;
    if (CIntElement* ltme = desc->x34_x28_LTME.get()) {
      ltme->GetValue(0, particle.x0_endFrame);
    }
    CParticleGlobals::instance()->SetParticleLifetime(particle.x0_endFrame);
    CParticleGlobals::instance()->UpdateParticleLifetimeTweenValues(0);
    g_currentParticle = &particle;
    if (x26d_28_enableADV) {
      UpdateAdvanceAccessParameters(particleCount, 0);
    }
    particle.x0_endFrame += x74_curFrame;

    if (CColorElement* colr = desc->x30_x24_COLR.get()) {
      colr->GetValue(0, particle.x34_color);
    } else {
      particle.x34_color = zeus::skWhite;
    }

    if (CEmitterElement* emtr = desc->x40_x2c_EMTR.get()) {
      emtr->GetValue(x74_curFrame, particle.x4_pos, particle.x1c_vel);
      zeus::CVector3f compXf1 = (x13c_globalScaleTransformInverse * x1a8_localScaleTransformInverse) * xdc_translation;
      zeus::CVector3f compXf2 = x1d8_orientation.rotate(particle.x4_pos);
      particle.x4_pos = compXf1 + compXf2 + xf4_POFS;
      particle.x1c_vel = x1d8_orientation.rotate(particle.x1c_vel);
    } else {
      zeus::CVector3f compXf1 = (x13c_globalScaleTransformInverse * x1a8_localScaleTransformInverse) * xdc_translation;
      particle.x4_pos = compXf1 + xf4_POFS;
      particle.x1c_vel.zeroOut();
    }
    particle.x10_prevPos = particle.x4_pos;

    if (x26c_31_LINE) {
      if (CRealElement* leng = desc->x20_x14_LENG.get())
        leng->GetValue(0, particle.x2c_lineLengthOrSize);
      else
        particle.x2c_lineLengthOrSize = 1.f;

      if (CRealElement* widt = desc->x24_x18_WIDT.get())
        widt->GetValue(0, particle.x30_lineWidthOrRota);
      else
        particle.x30_lineWidthOrRota = 1.f;
    } else {
      if (CRealElement* rota = desc->x50_x3c_ROTA.get())
        rota->GetValue(0, particle.x30_lineWidthOrRota);
      else
        particle.x30_lineWidthOrRota = 0.f;

      if (CRealElement* size = desc->x4c_x38_SIZE.get())
        size->GetValue(0, particle.x2c_lineLengthOrSize);
      else
        particle.x2c_lineLengthOrSize = 0.1f;
    }

    AccumulateBounds(particle.x4_pos, particle.x2c_lineLengthOrSize);
  }
}

void CElementGen::UpdatePSTranslationAndOrientation() {
  CGenDescription* desc = x1c_genDesc.GetObj();

  CGlobalRandom gr(x27c_randState);
  if (x268_PSLT < x74_curFrame)
    return;

  if (CVectorElement* pofs = desc->x18_xc_POFS.get())
    pofs->GetValue(x74_curFrame, xf4_POFS);

  if (CVectorElement* sspo = desc->xe8_xd4_SSPO.get())
    sspo->GetValue(x74_curFrame, x2b0_SSPO);

  if (CVectorElement* sepo = desc->xfc_xe8_SEPO.get())
    sepo->GetValue(x74_curFrame, x2c0_SEPO);
}

std::unique_ptr<CParticleGen> CElementGen::ConstructChildParticleSystem(const TToken<CGenDescription>& desc) const {
  OPTICK_EVENT();
  auto ret = std::make_unique<CElementGen>(desc, EModelOrientationType::Normal,
                                           x26d_27_enableOPTS ? EOptionalSystemFlags::Two : EOptionalSystemFlags::One);
  ret->x26d_26_modelsUseLights = x26d_26_modelsUseLights;
  ret->SetGlobalTranslation(xe8_globalTranslation);
  ret->SetGlobalOrientation(x22c_globalOrientation);
  ret->SetGlobalScale(x100_globalScale);
  ret->SetLocalScale(x16c_localScale);
  ret->SetTranslation(xdc_translation);
  ret->SetOrientation(x1d8_orientation);
  ret->SetParticleEmission(x88_particleEmission);
  ret->SetModulationColor(x338_moduColor);
  return ret;
}

void CElementGen::UpdateChildParticleSystems(double dt) {
  CGenDescription* desc = x1c_genDesc.GetObj();

  CGlobalRandom gr(x27c_randState);

  SChildGeneratorDesc& icts = desc->x8c_x78_ICTS;
  if (icts && x84_prevFrame != x74_curFrame && x2a0_CSSD == x74_curFrame) {
    int ncsyVal = 1;
    if (CIntElement* ncsy = desc->x9c_x88_NCSY.get())
      ncsy->GetValue(x74_curFrame, ncsyVal);

    CGenDescription* ictsDesc = icts.GetObj();
    if (!(x26d_27_enableOPTS && ictsDesc->x45_31_x32_25_OPTS)) {
      x290_activePartChildren.reserve(ncsyVal + x290_activePartChildren.size());
      for (int i = 0; i < ncsyVal; ++i) {
        std::unique_ptr<CParticleGen> chGen = ConstructChildParticleSystem(*icts);
        x290_activePartChildren.emplace_back(std::move(chGen));
      }
    }
  }

  SChildGeneratorDesc& iits = desc->xb8_xa4_IITS;
  if (iits && x84_prevFrame != x74_curFrame && x74_curFrame < x268_PSLT && x88_particleEmission == 1 &&
      x74_curFrame >= x2a4_SISY && ((x74_curFrame - x2a4_SISY) % x2a8_PISY) == 0) {
    CGenDescription* iitsDesc = iits.GetObj();
    if (!(x26d_27_enableOPTS && iitsDesc->x45_31_x32_25_OPTS)) {
      std::unique_ptr<CParticleGen> chGen = ConstructChildParticleSystem(*iits);
      x290_activePartChildren.emplace_back(std::move(chGen));
    }
  }

  CSpawnSystemKeyframeData* kssm = desc->xd0_xbc_KSSM.get();
  if (kssm && x84_prevFrame != x74_curFrame && x74_curFrame < x268_PSLT) {
    u16 backupSeed = g_GlobalSeed;
    u16 incSeed = backupSeed;

    std::vector<CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo>& systems =
        kssm->GetSpawnedSystemsAtFrame(x74_curFrame);
    x290_activePartChildren.reserve(x290_activePartChildren.size() + systems.size());
    for (CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo& system : systems) {
      TLockedToken<CGenDescription>& token = system.GetToken();
      if (!(x26d_27_enableOPTS && token.GetObj()->x45_31_x32_25_OPTS)) {
        g_GlobalSeed = incSeed;
        std::unique_ptr<CParticleGen> chGen = ConstructChildParticleSystem(token);
        x290_activePartChildren.emplace_back(std::move(chGen));
      }
      incSeed += 1;
    }

    g_GlobalSeed = backupSeed;
  }

  SChildGeneratorDesc& idts = desc->xa4_x90_IDTS;
  if (idts && x74_curFrame == x268_PSLT && x84_prevFrame != x74_curFrame) {
    int ndsyVal = 1;
    if (CIntElement* ndsy = desc->xb4_xa0_NDSY.get())
      ndsy->GetValue(0, ndsyVal);

    CGenDescription* idtsDesc = idts.GetObj();
    if (!(x26d_27_enableOPTS && idtsDesc->x45_31_x32_25_OPTS)) {
      x290_activePartChildren.reserve(ndsyVal + x290_activePartChildren.size());
      for (int i = 0; i < ndsyVal; ++i) {
        std::unique_ptr<CParticleGen> chGen = ConstructChildParticleSystem(*idts);
        x290_activePartChildren.emplace_back(std::move(chGen));
      }
    }
  }

  SSwooshGeneratorDesc& sswh = desc->xd4_xc0_SSWH;
  if (sswh && x84_prevFrame != x74_curFrame && x74_curFrame == x2ac_SSSD) {
    std::unique_ptr<CParticleGen> sswhGen = std::make_unique<CParticleSwoosh>(*sswh, 0);
    sswhGen->SetGlobalTranslation(xe8_globalTranslation);
    sswhGen->SetGlobalScale(x100_globalScale);
    sswhGen->SetLocalScale(x16c_localScale);
    sswhGen->SetTranslation(xdc_translation + x2b0_SSPO);
    sswhGen->SetOrientation(x1d8_orientation);
    sswhGen->SetParticleEmission(x88_particleEmission);
    x290_activePartChildren.emplace_back(std::move(sswhGen));
  }

  SElectricGeneratorDesc& selc = desc->xec_xd8_SELC;
  if (selc && x84_prevFrame != x74_curFrame && x74_curFrame == x2bc_SESD) {
    std::unique_ptr<CParticleGen> selcGen = std::make_unique<CParticleElectric>(*selc);
    selcGen->SetGlobalTranslation(xe8_globalTranslation);
    selcGen->SetGlobalScale(x100_globalScale);
    selcGen->SetLocalScale(x16c_localScale);
    selcGen->SetTranslation(xdc_translation + x2c0_SEPO);
    selcGen->SetOrientation(x1d8_orientation);
    selcGen->SetParticleEmission(x88_particleEmission);
    x290_activePartChildren.emplace_back(std::move(selcGen));
  }

  for (auto p = x290_activePartChildren.begin(); p != x290_activePartChildren.end();) {
    std::unique_ptr<CParticleGen>& ch = *p;

    ch->Update(dt);
    if (ch->IsSystemDeletable()) {
      p = x290_activePartChildren.erase(p);
      continue;
    }

    ++p;
  }

  x84_prevFrame = x74_curFrame;
}

void CElementGen::UpdateLightParameters() {
  CGenDescription* desc = x1c_genDesc.GetObj();

  if (CColorElement* lclr = desc->x104_xf0_LCLR.get())
    lclr->GetValue(x74_curFrame, x30c_LCLR);

  if (CRealElement* lint = desc->x108_xf4_LINT.get())
    lint->GetValue(x74_curFrame, x310_LINT);

  switch (x308_lightType) {
  default:
  case LightType::None:
  case LightType::Custom:
  case LightType::Spot: {
    if (CVectorElement* loff = desc->x10c_xf8_LOFF.get())
      loff->GetValue(x74_curFrame, x314_LOFF);

    if (CRealElement* lfor = desc->x118_x104_LFOR.get())
      lfor->GetValue(x74_curFrame, x330_LFOR);

    if (x308_lightType == LightType::Spot) {
      if (CRealElement* lsla = desc->x11c_x108_LSLA.get())
        lsla->GetValue(x74_curFrame, x334_LSLA);
    }
    [[fallthrough]];
  }
  case LightType::Directional: {
    if (x308_lightType != LightType::Custom) {
      if (CVectorElement* ldir = desc->x110_xfc_LDIR.get())
        ldir->GetValue(x74_curFrame, x320_LDIR);
    }
  }
  }
}

u32 CElementGen::GetParticleCountAllInternal() const {
  u32 ret = x25c_activeParticleCount;

  for (const std::unique_ptr<CParticleGen>& ch : x290_activePartChildren)
    if (ch->Get4CharId() == FOURCC('PART'))
      ret += static_cast<CElementGen&>(*ch).GetParticleCountAll();

  return ret;
}

void CElementGen::EndLifetime() {
  x268_PSLT = 0;
  for (std::unique_ptr<CParticleGen>& ch : x290_activePartChildren) {
    if (ch->Get4CharId() == FOURCC('PART'))
      static_cast<CElementGen&>(*ch).EndLifetime();
    else
      ch->SetParticleEmission(false);
  }
}

void CElementGen::ForceParticleCreation(int amount) {
  CParticleGlobals::SParticleSystem* prevSystem = CParticleGlobals::instance()->m_currentParticleSystem;
  CParticleGlobals::SParticleSystem thisSystem{FOURCC('PART'), this};
  CParticleGlobals::instance()->m_currentParticleSystem = &thisSystem;
  CParticleGlobals::instance()->SetEmitterTime(x74_curFrame);
  CreateNewParticles(amount);
  CParticleGlobals::instance()->m_currentParticleSystem = prevSystem;
}

void CElementGen::BuildParticleSystemBounds() {
  zeus::CAABox aabb;
  bool accumulated = false;

  for (std::unique_ptr<CParticleGen>& ch : x290_activePartChildren) {
    auto chBounds = ch->GetBounds();
    if (chBounds) {
      accumulated = true;
      aabb.accumulateBounds(chBounds.value());
    }
  }

  x264_recursiveParticleCount = GetParticleCountAllInternal();
  if (GetParticleCount() > 0) {
    zeus::CVector3f scale = x100_globalScale * x2ec_maxSize;
    zeus::CTransform xf = (x10c_globalScaleTransform * x22c_globalOrientation) * x178_localScaleTransform;
    zeus::CAABox box = zeus::CAABox(x2d4_aabbMin, x2e0_aabbMax).getTransformedAABox(xf);
    zeus::CVector3f min = box.min + xe8_globalTranslation - scale;
    zeus::CVector3f max = box.max + xe8_globalTranslation + scale;
    x2f0_systemBounds = zeus::CAABox(min, max);
  } else
    x2f0_systemBounds = zeus::CAABox();

  if (accumulated)
    x2f0_systemBounds.accumulateBounds(aabb);
}

u32 CElementGen::GetSystemCount() const {
  u32 ret = 0;
  for (const std::unique_ptr<CParticleGen>& child : x290_activePartChildren) {
    if (child->Get4CharId() == FOURCC('PART')) {
      ret += static_cast<const CElementGen&>(*child).GetSystemCount();
    } else {
      ret += 1;
    }
  }

  return ret + (x25c_activeParticleCount != 0);
}

void CElementGen::Render() {
  SCOPED_GRAPHICS_DEBUG_GROUP(fmt::format(FMT_STRING("CElementGen::Render {}"), *x1c_genDesc.GetObjectTag()).c_str(),
                              zeus::skYellow);

  CGenDescription* desc = x1c_genDesc.GetObj();

  x274_backupLightActive = CGraphics::g_LightActive;
  CGraphics::DisableAllLights();

  for (std::unique_ptr<CParticleGen>& child : x290_activePartChildren)
    child->Render();

  CParticleGlobals::SParticleSystem* prevSystem = CParticleGlobals::instance()->m_currentParticleSystem;
  CParticleGlobals::SParticleSystem thisSystem{FOURCC('PART'), this};
  CParticleGlobals::instance()->m_currentParticleSystem = &thisSystem;

  if (!x30_particles.empty()) {
    if (desc->x5c_x48_PMDL || desc->x45_24_x31_26_PMUS) {
      RenderModels();
    }
    if (x26c_31_LINE) {
      RenderLines();
    } else {
      RenderParticles();
    }
  }

  CParticleGlobals::instance()->m_currentParticleSystem = prevSystem;
}

void CElementGen::RenderModels() {
  SCOPED_GRAPHICS_DEBUG_GROUP(fmt::format(FMT_STRING("CElementGen::RenderModels")).c_str(), zeus::skYellow);

  CParticleGlobals::instance()->m_particleAccessParameters = nullptr;
  if (x26d_26_modelsUseLights) {
    CGraphics::SetLightState(x274_backupLightActive);
  } else {
    CGraphics::SetAmbientColor(zeus::skWhite);
  }
  CGlobalRandom gr(x27c_randState);

  CGenDescription* desc = x1c_genDesc.GetObj();

  SUVElementSet uvs = {0.f, 0.f, 1.f, 1.f};
  CUVElement* texr = desc->x54_x40_TEXR.get();
  CTexture* cachedTex = nullptr;
  bool texConst = true;
  bool moveRedToAlphaBuffer = false;

  if (desc->x45_24_x31_26_PMUS) {
    if (sMoveRedToAlphaBuffer && desc->x44_31_x31_25_PMAB && desc->x54_x40_TEXR) {
      moveRedToAlphaBuffer = true;
    }

    if (desc->x44_31_x31_25_PMAB) {
      CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, false);
      if (moveRedToAlphaBuffer) {
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::One, ERglBlendFactor::One, ERglLogicOp::Clear);
      } else {
        CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One,
                                ERglLogicOp::Clear);
        CGraphics::SetAlphaCompare(ERglAlphaFunc::Greater, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);
      }
    } else {
      CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, true);
      CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                              ERglLogicOp::Clear);
      CGraphics::SetAlphaCompare(ERglAlphaFunc::Greater, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);
    }

    CGraphics::SetCullMode(ERglCullMode::None);

    if (texr) {
      CParticle& target = x30_particles[0];
      int partFrame = x74_curFrame - target.x28_startFrame;
      cachedTex = texr->GetValueTexture(partFrame).GetObj();
      cachedTex->Load(GX_TEXMAP0, EClampMode::Repeat);

      CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::kEnvModulate);
      if (moveRedToAlphaBuffer) {
        CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
        CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
        CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
        CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP1);
        CGX::SetNumTevStages(2);
        constexpr std::array vtxDescList{
            GXVtxDescList{GX_VA_POS, GX_DIRECT},
            GXVtxDescList{GX_VA_CLR0, GX_DIRECT},
            GXVtxDescList{GX_VA_TEX0, GX_DIRECT},
            GXVtxDescList{GX_VA_NULL, GX_NONE},
        };
        CGX::SetVtxDescv(vtxDescList.data());
        CGX::SetChanCtrl(CGX::EChannelId::Channel0, {});
        CGX::SetNumChans(1);
        CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
        CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        CGX::SetNumTexGens(1);
      } else {
        CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::kEnvPassthru);
      }

      texConst = texr->HasConstantTexture();
      texr->GetValueUV(partFrame, uvs);
    } else {
      CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::kEnvPassthru);
      CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::kEnvPassthru);
    }
  }

  zeus::CTransform orient = zeus::CTransform();
  if (!desc->x45_25_x31_27_PMOO)
    orient = x1d8_orientation;
  orient = orient * x22c_globalOrientation;

  CVectorElement* pmrt = desc->x70_x5c_PMRT.get();
  bool pmrtConst = false;
  if (pmrt)
    pmrtConst = pmrt->IsFastConstant();

  zeus::CVector3f trans = (x13c_globalScaleTransformInverse * x1a8_localScaleTransformInverse) * xe8_globalTranslation;

  zeus::CTransform rot = zeus::CTransform();
  if (pmrtConst) {
    zeus::CVector3f pmrtVal;
    pmrt->GetValue(x74_curFrame, pmrtVal);
    rot = zeus::CTransform::RotateZ(zeus::degToRad(pmrtVal[2]));
    rot.rotateLocalY(zeus::degToRad(pmrtVal[1]));
    rot.rotateLocalX(zeus::degToRad(pmrtVal[0]));
  }
  rot = orient * rot;

  CParticleGlobals::instance()->SetEmitterTime(x74_curFrame);
  zeus::CColor col = x338_moduColor;

  zeus::CVector3f pmopVec;
  auto matrixIt = x50_parentMatrices.begin();
  for (size_t i = 0; i < x30_particles.size(); ++i) {
    CParticle& particle = x30_particles[i];
    g_currentParticle = &particle;

    if (particle.x0_endFrame == -1) {
      if (x2c_orientType == EModelOrientationType::One)
        ++matrixIt;
      continue;
    }
    CParticleGlobals::instance()->SetParticleLifetime(particle.x0_endFrame - particle.x28_startFrame);
    int partFrame = x74_curFrame - particle.x28_startFrame - 1;
    CParticleGlobals::instance()->UpdateParticleLifetimeTweenValues(partFrame);
    if (x26d_28_enableADV) {
      CParticleGlobals::instance()->m_particleAccessParameters = &x60_advValues[i];
    }

    CVectorElement* pmop = desc->x6c_x58_PMOP.get();
    if (pmop)
      pmop->GetValue(partFrame, pmopVec);

    zeus::CTransform partTrans = zeus::CTransform::Translate(particle.x4_pos + trans);
    if (x2c_orientType == EModelOrientationType::One) {
      zeus::CTransform partRot(*matrixIt);
      zeus::CVector3f pmopRotateOffset = (orient * partRot) * pmopVec;
      partTrans = partTrans * partRot;
      partTrans += pmopRotateOffset;
    } else {
      partTrans += orient * pmopVec;
    }

    if (pmrtConst) {
      partTrans = partTrans * rot;
    } else {
      if (pmrt) {
        zeus::CVector3f pmrtVal;
        pmrt->GetValue(partFrame, pmrtVal);
        rot = zeus::CTransform::RotateZ(zeus::degToRad(pmrtVal[2]));
        rot.rotateLocalY(zeus::degToRad(pmrtVal[1]));
        rot.rotateLocalX(zeus::degToRad(pmrtVal[0]));
        partTrans = partTrans * (orient * rot);
      } else {
        partTrans = partTrans * rot;
      }
    }

    CVectorElement* pmsc = desc->x74_x60_PMSC.get();
    if (pmsc) {
      zeus::CVector3f pmscVal;
      pmsc->GetValue(partFrame, pmscVal);
      partTrans = partTrans * zeus::CTransform::Scale(pmscVal);
    }

    CColorElement* pmcl = desc->x78_x64_PMCL.get();
    if (pmcl) {
      pmcl->GetValue(partFrame, col);
      col *= x338_moduColor;
    }

    CGraphics::SetModelMatrix((x10c_globalScaleTransform * partTrans) * x178_localScaleTransform);

    if (desc->x45_24_x31_26_PMUS) {
      if (moveRedToAlphaBuffer) {
        CGX::Begin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition3f32(0.5f, 0.f, 0.5f);
        GXColor4f32(col);
        GXTexCoord2f32(uvs.xMax, uvs.yMax);
        GXPosition3f32(-0.5f, 0.f, 0.5f);
        GXColor4f32(col);
        GXTexCoord2f32(uvs.xMin, uvs.yMax);
        GXPosition3f32(-0.5f, 0.f, -0.5f);
        GXColor4f32(col);
        GXTexCoord2f32(uvs.xMin, uvs.yMin);
        GXPosition3f32(0.5f, 0.f, -0.5f);
        GXColor4f32(col);
        GXTexCoord2f32(uvs.xMax, uvs.yMin);
        CGX::End();
      } else {
        CGraphics::StreamBegin(GX_QUADS);
        CGraphics::StreamColor(col);
        CGraphics::StreamTexcoord(uvs.xMax, uvs.yMax);
        CGraphics::StreamVertex(0.5f, 0.f, 0.5f);
        CGraphics::StreamTexcoord(uvs.xMin, uvs.yMax);
        CGraphics::StreamVertex(-0.5f, 0.f, 0.5f);
        CGraphics::StreamTexcoord(uvs.xMin, uvs.yMin);
        CGraphics::StreamVertex(-0.5f, 0.f, -0.5f);
        CGraphics::StreamTexcoord(uvs.xMax, uvs.yMin);
        CGraphics::StreamVertex(0.5f, 0.f, -0.5f);
        CGraphics::StreamEnd();
      }
    } else {
      CModel* model = desc->x5c_x48_PMDL.GetObj();
      if (g_subtractBlend) {
        model->Draw({5, 0, 1, zeus::CColor{1.f, 0.5f}});
      } else if (desc->x44_31_x31_25_PMAB) {
        model->Draw({7, 0, 1, col});
      } else if (1.f == col.a()) {
        model->Draw({0, 0, 3, zeus::skWhite});
      } else {
        model->Draw({5, 0, 1, zeus::CColor{1.f, col.a()}});
      }
    }

    if (x2c_orientType == EModelOrientationType::One)
      ++matrixIt;
  }

  if (x26d_26_modelsUseLights) {
    CGraphics::DisableAllLights();
  }

  CGraphics::SetCullMode(ERglCullMode::Front);
  CTevCombiners::ResetStates();
  if (moveRedToAlphaBuffer) {
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
  }
  CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);
}

void CElementGen::RenderLines() {
  SCOPED_GRAPHICS_DEBUG_GROUP(fmt::format(FMT_STRING("CElementGen::RenderLines")).c_str(), zeus::skYellow);

  CGenDescription* desc = x1c_genDesc.GetObj();
  CGlobalRandom gr(x27c_randState);

  zeus::CTransform systemViewPointMatrix(CGraphics::g_ViewMatrix);
  systemViewPointMatrix.origin.zeroOut();
  zeus::CTransform systemCameraMatrix = systemViewPointMatrix.inverse() * x22c_globalOrientation;
  systemViewPointMatrix =
      ((zeus::CTransform::Translate(xe8_globalTranslation) * x10c_globalScaleTransform) * systemViewPointMatrix) *
      x178_localScaleTransform;
  CGraphics::SetModelMatrix(systemViewPointMatrix);

  CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);

  if (x26c_26_AAPH) {
    CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, false);
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One, ERglLogicOp::Clear);
  } else {
    CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, true);
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                            ERglLogicOp::Clear);
  }

  CRealElement* widt = desc->x24_x18_WIDT.get();
  bool widtConst = false;
  if (widt)
    widtConst = widt->IsConstant();

  CUVElement* texr = desc->x54_x40_TEXR.get();
  SUVElementSet uvs = {0.f, 0.f, 1.f, 1.f};
  bool constTexr = true;
  bool constUVs = true;
  CTexture* cachedTex = nullptr;
  zeus::CColor moduColor = zeus::skWhite;
  if (texr) {
    CParticle& target = x30_particles[0];
    int partFrame = x74_curFrame - target.x28_startFrame;
    cachedTex = texr->GetValueTexture(partFrame).GetObj();
    cachedTex->Load(GX_TEXMAP0, EClampMode::Repeat);

    /* Set TEXC * RASC */

    if (x338_moduColor != zeus::skBlack) {
      /* Add RASC * PREVC pass for MODU color loaded into channel mat-color */
      moduColor = x338_moduColor;
    }

    constTexr = texr->HasConstantTexture();
    texr->GetValueUV(partFrame, uvs);
    constUVs = texr->HasConstantUV();
  }

  float constWidth = 1.f;
  if (widtConst) {
    widt->GetValue(0, constWidth);
    constWidth = std::max(0.f, std::min(constWidth, 42.5f));
  }

  // m_lineRenderer->Reset();

  for (auto& particle : x30_particles) {
    g_currentParticle = &particle;

    int partFrame = x74_curFrame - particle.x28_startFrame;

    if (!constTexr) {
      CTexture* tex = texr->GetValueTexture(partFrame).GetObj();
      if (tex != cachedTex) {
        tex->Load(GX_TEXMAP0, EClampMode::Repeat);
        cachedTex = tex;
      }
    }

    if (!constUVs)
      texr->GetValueUV(partFrame, uvs);

    zeus::CVector3f dVec = particle.x4_pos - particle.x10_prevPos;
    if (x26d_24_FXLL)
      if (dVec.magSquared() >= 0.f)
        dVec.normalize();

    zeus::CVector3f p1 = systemCameraMatrix * particle.x4_pos;
    zeus::CVector3f p2 = systemCameraMatrix * (particle.x2c_lineLengthOrSize * dVec + particle.x4_pos);

    if (widtConst) {
      // m_lineRenderer->AddVertex(p1, particle.x34_color, constWidth, {uvs.xMin, uvs.yMin});
      // m_lineRenderer->AddVertex(p2, particle.x34_color, constWidth, {uvs.xMax, uvs.yMax});
    } else if (widt) {
      float width = 1.f;
      widt->GetValue(0, width);
      width = std::max(0.f, std::min(width, 42.5f));
      // m_lineRenderer->AddVertex(p1, particle.x34_color, width, {uvs.xMin, uvs.yMin});
      // m_lineRenderer->AddVertex(p2, particle.x34_color, width, {uvs.xMax, uvs.yMax});
    }
  }

  // m_lineRenderer->Render(g_Renderer->IsThermalVisorHotPass(), moduColor);
}

void CElementGen::RenderParticles() {
  CGenDescription* desc = x1c_genDesc.GetObj();
  CGlobalRandom gr(x27c_randState);

  if (IsIndirectTextured()) {
    RenderParticlesIndirectTexture();
    return;
  }

  SCOPED_GRAPHICS_DEBUG_GROUP(fmt::format(FMT_STRING("CElementGen::RenderParticles")).c_str(), zeus::skYellow);

  CRealElement* size = desc->x4c_x38_SIZE.get();
  if (size && size->IsConstant()) {
    float sizeVal;
    size->GetValue(0, sizeVal);
    if (sizeVal == 0.f) {
      size->GetValue(1, sizeVal);
      if (sizeVal == 0.f)
        return;
    }
  }

  bool hasModuColor = x338_moduColor != zeus::skWhite;
  CGraphics::SetCullMode(ERglCullMode::None);
  zeus::CTransform systemModelMatrix(CGraphics::g_ViewMatrix);
  systemModelMatrix.origin.zeroOut();
  zeus::CTransform systemCameraMatrix = systemModelMatrix.inverse() * x22c_globalOrientation;
  systemModelMatrix =
      ((zeus::CTransform::Translate(xe8_globalTranslation) * x10c_globalScaleTransform) * systemModelMatrix) *
      x178_localScaleTransform;
  if (x26c_29_ORNT)
    CGraphics::SetModelMatrix(systemModelMatrix * systemCameraMatrix);
  else
    CGraphics::SetModelMatrix(systemModelMatrix);

  CGraphics::SetAlphaCompare(ERglAlphaFunc::Greater, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);

  SUVElementSet uvs = {0.f, 0.f, 1.f, 1.f};
  bool constUVs = true;
  CTexture* cachedTex = nullptr;

  auto* rota = x28_loadedGenDesc->x50_x3c_ROTA.get();
  bool noRota = rota == nullptr;
  if (rota != nullptr && rota->IsConstant()) {
    float value = 1.f;
    rota->GetValue(0, value);
    if (value == 0.f) {
      value = 1.f;
      rota->GetValue(1, value);
      if (value == 0.f) {
        noRota = true;
      }
    }
  }

  auto* texr = x28_loadedGenDesc->x54_x40_TEXR.get();
  if (texr != nullptr) {
    CParticle& target = x30_particles[0];
    int partFrame = x74_curFrame - target.x28_startFrame;
    cachedTex = texr->GetValueTexture(partFrame).GetObj();
    cachedTex->Load(GX_TEXMAP0, EClampMode::Repeat);

    CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::kEnvModulate);
    if (hasModuColor) {
      /* Add RASC * PREVC pass for MODU color loaded into channel mat-color */
      static const CTevCombiners::CTevPass kEnvModuColor{
          {GX_CC_ZERO, GX_CC_CPREV, GX_CC_RASC, GX_CC_ZERO},
          {GX_CA_ZERO, GX_CA_APREV, GX_CA_RASA, GX_CA_ZERO},
      };
      CGraphics::SetTevOp(ERglTevStage::Stage1, kEnvModuColor);
    } else {
      CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::kEnvPassthru);
    }

    texr->GetValueUV(partFrame, uvs);
    constUVs = texr->HasConstantUV();
  } else {
    CGraphics::SetTevOp(ERglTevStage::Stage0, CTevCombiners::kEnvPassthru);
    CGraphics::SetTevOp(ERglTevStage::Stage1, CTevCombiners::kEnvPassthru);
  }

  constexpr std::array vtxDescList{
      GXVtxDescList{GX_VA_POS, GX_DIRECT},
      GXVtxDescList{GX_VA_CLR0, GX_DIRECT},
      GXVtxDescList{GX_VA_TEX0, GX_DIRECT},
      GXVtxDescList{GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(vtxDescList.data());
  GXTevStageID nextStage;
  if (hasModuColor) {
    CGX::SetNumChans(2);
    nextStage = GX_TEVSTAGE2;
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);
    CGX::SetChanAmbColor(CGX::EChannelId::Channel1, zeus::skBlack);
    CGX::SetChanMatColor(CGX::EChannelId::Channel1, x338_moduColor);
    CGX::SetChanCtrl(CGX::EChannelId::Channel1, {});
  } else {
    CGX::SetNumChans(1);
    nextStage = GX_TEVSTAGE1;
  }

  bool moveRedToAlphaBuffer = sMoveRedToAlphaBuffer;
  if (g_subtractBlend) {
    CGraphics::SetDepthWriteMode(x26c_28_zTest, ERglEnum::LEqual, false);
    CGX::SetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    if (moveRedToAlphaBuffer) {
      CGX::SetTevColorIn(nextStage, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
      CGX::SetTevAlphaIn(nextStage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
      CGX::SetStandardTevColorAlphaOp(nextStage);
      CGX::SetTevOrder(nextStage, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
      GXSetTevSwapMode(nextStage, GX_TEV_SWAP0, GX_TEV_SWAP1);
      nextStage = GXTevStageID(nextStage + 1);
    }
  } else if (moveRedToAlphaBuffer) {
    CGraphics::SetDepthWriteMode(x26c_28_zTest, ERglEnum::LEqual, false);
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::One, ERglBlendFactor::One, ERglLogicOp::Clear);
    CGX::SetTevColorIn(nextStage, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
    CGX::SetTevAlphaIn(nextStage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
    CGX::SetStandardTevColorAlphaOp(nextStage);
    CGX::SetTevOrder(nextStage, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevSwapMode(nextStage, GX_TEV_SWAP0, GX_TEV_SWAP1);
    nextStage = GXTevStageID(nextStage + 1);
  } else if (x26c_26_AAPH) {
    CGraphics::SetDepthWriteMode(x26c_28_zTest, ERglEnum::LEqual, false);
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One, ERglLogicOp::Clear);
  } else {
    CGraphics::SetDepthWriteMode(x26c_28_zTest, ERglEnum::LEqual, x26c_27_ZBUF);
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                            ERglLogicOp::Clear);
  }
  CGX::SetNumTevStages(nextStage);
  CGX::SetNumTexGens(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetChanCtrl(CGX::EChannelId::Channel0, false, GX_SRC_REG, GX_SRC_VTX, {}, GX_DF_NONE, GX_AF_NONE);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  if (constUVs) {
    GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_RGBA8, 1);
  } else {
    GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  }

  int mbspVal = std::max(1, x270_MBSP);
  if (x26c_30_MBLR) {
    CGX::Begin(GX_QUADS, GX_VTXFMT6, mbspVal * x30_particles.size() * 4);
  } else {
    CGX::Begin(GX_QUADS, GX_VTXFMT6, mbspVal * 4);
  }

  std::vector<CParticleListItem> sortItems;
  if (desc->x44_28_x30_28_SORT) {
    sortItems.reserve(x30_particles.size());

    for (size_t i = 0; i < x30_particles.size(); ++i) {
      const CParticle& particle = x30_particles[i];
      sortItems.emplace_back(s16(i));
      CParticleListItem& sortItem = sortItems.back();
      sortItem.x4_viewPoint =
          systemCameraMatrix * ((particle.x4_pos - particle.x10_prevPos) * x80_timeDeltaScale + particle.x10_prevPos);
    }

    std::sort(sortItems.begin(), sortItems.end(), [](const CParticleListItem& a, const CParticleListItem& b) -> bool {
      return a.x4_viewPoint[1] > b.x4_viewPoint[1];
    });
  }

  CParticleGlobals::instance()->SetEmitterTime(x74_curFrame);
  if (!x26c_30_MBLR) {
    if (!desc->x44_28_x30_28_SORT && constUVs && !x26c_29_ORNT) {
      if (noRota) {
        if (zeus::close_enough(x80_timeDeltaScale, 1.f)) {
          RenderBasicParticlesNoRotNoTS(systemCameraMatrix);
        } else {
          RenderBasicParticlesNoRotTS(systemCameraMatrix);
        }
      } else if (zeus::close_enough(x80_timeDeltaScale, 1.f)) {
        RenderBasicParticlesRotNoTS(systemCameraMatrix);
      } else {
        RenderBasicParticlesRotTS(systemCameraMatrix);
      }
    } else if (!x26c_29_ORNT) {
      for (size_t i = 0; i < x30_particles.size(); ++i) {
        const int partIdx = desc->x44_28_x30_28_SORT ? sortItems[i].x0_partIdx : int(i);
        CParticle& particle = x30_particles[partIdx];
        g_currentParticle = &particle;

        const int partFrame = x74_curFrame - particle.x28_startFrame - 1;
        zeus::CVector3f viewPoint;
        if (desc->x44_28_x30_28_SORT) {
          viewPoint = sortItems[i].x4_viewPoint;
        } else {
          viewPoint = systemCameraMatrix *
                      ((particle.x4_pos - particle.x10_prevPos) * x80_timeDeltaScale + particle.x10_prevPos);
        }

        const float size = 0.5f * particle.x2c_lineLengthOrSize;
        if (!constUVs) {
          CParticleGlobals::instance()->SetParticleLifetime(particle.x0_endFrame - particle.x28_startFrame);
          CParticleGlobals::instance()->UpdateParticleLifetimeTweenValues(partFrame);
          texr->GetValueUV(partFrame, uvs);
        }

        if (noRota) {
          GXPosition3f32(viewPoint.x() + size, viewPoint.y(), viewPoint.z() + size);
          GXColor4f32(particle.x34_color);
          GXTexCoord2f32(uvs.xMax, uvs.yMax);
          GXPosition3f32(viewPoint.x() - size, viewPoint.y(), viewPoint.z() + size);
          GXColor4f32(particle.x34_color);
          GXTexCoord2f32(uvs.xMin, uvs.yMax);
          GXPosition3f32(viewPoint.x() - size, viewPoint.y(), viewPoint.z() - size);
          GXColor4f32(particle.x34_color);
          GXTexCoord2f32(uvs.xMin, uvs.yMin);
          GXPosition3f32(viewPoint.x() + size, viewPoint.y(), viewPoint.z() - size);
          GXColor4f32(particle.x34_color);
          GXTexCoord2f32(uvs.xMax, uvs.yMin);
        } else {
          const float theta = zeus::degToRad(particle.x30_lineWidthOrRota);
          const float sinT = std::sin(theta) * size;
          const float cosT = std::cos(theta) * size;
          GXPosition3f32(viewPoint.x() + (sinT + cosT), viewPoint.y(), viewPoint.z() + (cosT - sinT));
          GXColor4f32(particle.x34_color);
          GXTexCoord2f32(uvs.xMax, uvs.yMax);
          GXPosition3f32(viewPoint.x() + (sinT - cosT), viewPoint.y(), viewPoint.z() + (sinT + cosT));
          GXColor4f32(particle.x34_color);
          GXTexCoord2f32(uvs.xMin, uvs.yMax);
          GXPosition3f32(viewPoint.x() - (sinT + cosT), viewPoint.y(), viewPoint.z() - (cosT - sinT));
          GXColor4f32(particle.x34_color);
          GXTexCoord2f32(uvs.xMin, uvs.yMin);
          GXPosition3f32(viewPoint.x() + (-sinT + cosT), viewPoint.y(), viewPoint.z() + (-cosT - sinT));
          GXColor4f32(particle.x34_color);
          GXTexCoord2f32(uvs.xMax, uvs.yMin);
        }
      }
    } else {
      for (size_t i = 0; i < x30_particles.size(); ++i) {
        const int partIdx = desc->x44_28_x30_28_SORT ? sortItems[i].x0_partIdx : int(i);
        CParticle& particle = x30_particles[partIdx];
        g_currentParticle = &particle;

        const int partFrame = x74_curFrame - particle.x28_startFrame - 1;
        zeus::CVector3f viewPoint =
            ((particle.x4_pos - particle.x10_prevPos) * x80_timeDeltaScale + particle.x10_prevPos);
        const float width = !desc->x50_x3c_ROTA ? 1.f : particle.x30_lineWidthOrRota;
        zeus::CVector3f dir;
        if (particle.x1c_vel.canBeNormalized()) {
          dir = particle.x1c_vel.normalized();
        } else {
          zeus::CVector3f delta = particle.x4_pos - particle.x10_prevPos;
          if (delta.canBeNormalized())
            dir = delta.normalized();
          else
            dir = zeus::skUp;
        }

        zeus::CVector3f foreVec = particle.x2c_lineLengthOrSize * dir;
        zeus::CVector3f rightVec;
        if (desc->x30_31_RSOP) {
          rightVec = dir.cross(CGraphics::g_ViewMatrix.basis[1]);
          if (rightVec.canBeNormalized()) {
            rightVec = rightVec.normalized() * (particle.x2c_lineLengthOrSize * width);
          } else {
            rightVec = dir.cross((CGraphics::g_ViewMatrix.origin - particle.x4_pos).normalized());
            if (rightVec.canBeNormalized()) {
              rightVec = rightVec.normalized() * (particle.x2c_lineLengthOrSize * width);
            }
          }
        } else {
          rightVec = foreVec.cross(CGraphics::g_ViewMatrix.basis[1]) * width;
        }

        if (!constUVs) {
          CParticleGlobals::instance()->SetParticleLifetime(particle.x0_endFrame - particle.x28_startFrame);
          CParticleGlobals::instance()->UpdateParticleLifetimeTweenValues(partFrame);
          texr->GetValueUV(partFrame, uvs);
        }

//        switch (m_shaderClass) {
//        case CElementGenShaders::EShaderClass::Tex: {
//          SParticleInstanceTex& inst = g_instTexData.emplace_back();
//          viewPoint += rightVec * 0.5f;
//          inst.pos[0] = zeus::CVector4f{viewPoint + 0.5f * foreVec};
//          inst.pos[1] = zeus::CVector4f{viewPoint - 0.5f * foreVec};
//          viewPoint -= rightVec;
//          inst.pos[2] = zeus::CVector4f{viewPoint + 0.5f * foreVec};
//          inst.pos[3] = zeus::CVector4f{viewPoint - 0.5f * foreVec};
//          inst.color = particle.x34_color;
//          inst.uvs[0] = {uvs.xMax, uvs.yMax};
//          inst.uvs[1] = {uvs.xMin, uvs.yMax};
//          inst.uvs[2] = {uvs.xMax, uvs.yMin};
//          inst.uvs[3] = {uvs.xMin, uvs.yMin};
//          break;
//        }
//        case CElementGenShaders::EShaderClass::NoTex: {
//          SParticleInstanceNoTex& inst = g_instNoTexData.emplace_back();
//          viewPoint += rightVec * 0.5f;
//          inst.pos[0] = zeus::CVector4f{viewPoint + 0.5f * foreVec};
//          inst.pos[1] = zeus::CVector4f{viewPoint - 0.5f * foreVec};
//          viewPoint -= rightVec;
//          inst.pos[2] = zeus::CVector4f{viewPoint + 0.5f * foreVec};
//          inst.pos[3] = zeus::CVector4f{viewPoint - 0.5f * foreVec};
//          inst.color = particle.x34_color;
//          break;
//        }
//        default:
//          break;
//        }
      }
    }

    switch (m_shaderClass) {
    case CElementGenShaders::EShaderClass::Tex:
      //      m_instBuf->load(g_instTexData.data(), g_instTexData.size() * sizeof(SParticleInstanceTex));
      //      CGraphics::DrawInstances(0, 4, g_instTexData.size());
      break;
    case CElementGenShaders::EShaderClass::NoTex:
      //      m_instBuf->load(g_instNoTexData.data(), g_instNoTexData.size() * sizeof(SParticleInstanceNoTex));
      //      CGraphics::DrawInstances(0, 4, g_instNoTexData.size());
      break;
    default:
      break;
    }
  } else {
//    switch (m_shaderClass) {
//    case CElementGenShaders::EShaderClass::Tex:
//      g_instTexData.clear();
//      g_instTexData.reserve(x30_particles.size() * mbspVal);
//      break;
//    case CElementGenShaders::EShaderClass::NoTex:
//      g_instNoTexData.clear();
//      g_instNoTexData.reserve(x30_particles.size() * mbspVal);
//      break;
//    default:
//      Log.report(logvisor::Fatal, FMT_STRING("unexpected particle shader class"));
//      break;
//    }
    const float mbspFac = 1.f / float(mbspVal);
    for (size_t i = 0; i < x30_particles.size(); ++i) {
      const int partIdx = desc->x44_28_x30_28_SORT ? sortItems[i].x0_partIdx : int(i);
      CParticle& particle = x30_particles[partIdx];
      g_currentParticle = &particle;

      const int partFrame = x74_curFrame - particle.x28_startFrame - 1;

      if (!constUVs) {
        CParticleGlobals::instance()->SetParticleLifetime(particle.x0_endFrame - particle.x28_startFrame);
        CParticleGlobals::instance()->UpdateParticleLifetimeTweenValues(partFrame);
        texr->GetValueUV(partFrame, uvs);
      }

      zeus::CVector3f dVec = particle.x4_pos - particle.x10_prevPos;
      zeus::CVector3f vec = dVec * x80_timeDeltaScale + particle.x10_prevPos;
      zeus::CVector3f mbspVec = dVec * mbspFac;
      float size = 0.5f * particle.x2c_lineLengthOrSize;
      if (0.f == particle.x30_lineWidthOrRota) {
        for (int j = 0; j < mbspVal; ++j) {
          vec += mbspVec;
          zeus::CVector3f vec2 = systemCameraMatrix * vec;

//          switch (m_shaderClass) {
//          case CElementGenShaders::EShaderClass::Tex: {
//            SParticleInstanceTex& inst = g_instTexData.emplace_back();
//            inst.pos[0] = zeus::CVector4f{vec2.x() + size, vec2.y(), vec2.z() + size, 1.f};
//            inst.pos[1] = zeus::CVector4f{vec2.x() - size, vec2.y(), vec2.z() + size, 1.f};
//            inst.pos[2] = zeus::CVector4f{vec2.x() + size, vec2.y(), vec2.z() - size, 1.f};
//            inst.pos[3] = zeus::CVector4f{vec2.x() - size, vec2.y(), vec2.z() - size, 1.f};
//            inst.color = particle.x34_color;
//            inst.uvs[0] = {uvs.xMax, uvs.yMax};
//            inst.uvs[1] = {uvs.xMin, uvs.yMax};
//            inst.uvs[2] = {uvs.xMax, uvs.yMin};
//            inst.uvs[3] = {uvs.xMin, uvs.yMin};
//            break;
//          }
//          case CElementGenShaders::EShaderClass::NoTex: {
//            SParticleInstanceNoTex& inst = g_instNoTexData.emplace_back();
//            inst.pos[0] = zeus::CVector4f{vec2.x() + size, vec2.y(), vec2.z() + size, 1.f};
//            inst.pos[1] = zeus::CVector4f{vec2.x() - size, vec2.y(), vec2.z() + size, 1.f};
//            inst.pos[2] = zeus::CVector4f{vec2.x() + size, vec2.y(), vec2.z() - size, 1.f};
//            inst.pos[3] = zeus::CVector4f{vec2.x() - size, vec2.y(), vec2.z() - size, 1.f};
//            inst.color = particle.x34_color;
//            break;
//          }
//          default:
//            break;
//          }
        }
      } else {
        float theta = zeus::degToRad(particle.x30_lineWidthOrRota);
        float sinT = std::sin(theta) * size;
        float cosT = std::cos(theta) * size;

        for (int j = 0; j < mbspVal; ++j) {
          vec += mbspVec;
          zeus::CVector3f vec2 = systemCameraMatrix * vec;

//          switch (m_shaderClass) {
//          case CElementGenShaders::EShaderClass::Tex: {
//            SParticleInstanceTex& inst = g_instTexData.emplace_back();
//            inst.pos[0] = zeus::CVector4f{vec2.x() + sinT + cosT, vec2.y(), vec2.z() + cosT - sinT, 1.f};
//            inst.pos[1] = zeus::CVector4f{vec2.x() + sinT - cosT, vec2.y(), vec2.z() + sinT + cosT, 1.f};
//            inst.pos[2] = zeus::CVector4f{vec2.x() + (cosT - sinT), vec2.y(), vec2.z() + (-cosT - sinT), 1.f};
//            inst.pos[3] = zeus::CVector4f{vec2.x() - (sinT + cosT), vec2.y(), vec2.z() - (cosT - sinT), 1.f};
//            inst.color = particle.x34_color;
//            inst.uvs[0] = {uvs.xMax, uvs.yMax};
//            inst.uvs[1] = {uvs.xMin, uvs.yMax};
//            inst.uvs[2] = {uvs.xMax, uvs.yMin};
//            inst.uvs[3] = {uvs.xMin, uvs.yMin};
//            break;
//          }
//          case CElementGenShaders::EShaderClass::NoTex: {
//            SParticleInstanceNoTex& inst = g_instNoTexData.emplace_back();
//            inst.pos[0] = zeus::CVector4f{vec2.x() + sinT + cosT, vec2.y(), vec2.z() + cosT - sinT, 1.f};
//            inst.pos[1] = zeus::CVector4f{vec2.x() + sinT - cosT, vec2.y(), vec2.z() + sinT + cosT, 1.f};
//            inst.pos[2] = zeus::CVector4f{vec2.x() + (cosT - sinT), vec2.y(), vec2.z() + (-cosT - sinT), 1.f};
//            inst.pos[3] = zeus::CVector4f{vec2.x() - (sinT + cosT), vec2.y(), vec2.z() - (cosT - sinT), 1.f};
//            inst.color = particle.x34_color;
//            break;
//          }
//          default:
//            break;
//          }
        }
      }
    }
    switch (m_shaderClass) {
    case CElementGenShaders::EShaderClass::Tex:
      //      m_instBuf->load(g_instTexData.data(), g_instTexData.size() * sizeof(SParticleInstanceTex));
      //      CGraphics::DrawInstances(0, 4, g_instTexData.size());
      break;
    case CElementGenShaders::EShaderClass::NoTex:
      //      m_instBuf->load(g_instNoTexData.data(), g_instNoTexData.size() * sizeof(SParticleInstanceNoTex));
      //      CGraphics::DrawInstances(0, 4, g_instNoTexData.size());
      break;
    default:
      break;
    }
  }

  CGX::End();
  if (moveRedToAlphaBuffer) {
    GXSetTevSwapMode(GXTevStageID(nextStage - 1), GX_TEV_SWAP0, GX_TEV_SWAP0);
  }
  CGraphics::SetCullMode(ERglCullMode::Front);
  CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);
}

void CElementGen::RenderParticlesIndirectTexture() {
  SCOPED_GRAPHICS_DEBUG_GROUP(fmt::format(FMT_STRING("CElementGen::RenderParticlesIndirectTexture")).c_str(), zeus::skYellow);

  CGenDescription* desc = x1c_genDesc.GetObj();

  zeus::CTransform systemViewPointMatrix(CGraphics::g_ViewMatrix);
  systemViewPointMatrix.origin.zeroOut();
  zeus::CTransform systemCameraMatrix = systemViewPointMatrix.inverse() * x22c_globalOrientation;
  systemViewPointMatrix =
      ((zeus::CTransform::Translate(xe8_globalTranslation) * x10c_globalScaleTransform) * systemViewPointMatrix) *
      x178_localScaleTransform;
  CGraphics::SetModelMatrix(systemViewPointMatrix);

//  SParticleUniforms uniformData = {CGraphics::GetPerspectiveProjectionMatrix(/*true*/) *
//                                       CGraphics::g_GXModelView.toMatrix4f(),
//                                   {1.f, 1.f, 1.f, 1.f}};
  //  m_uniformBuf->load(&uniformData, sizeof(SParticleUniforms));

  CGraphics::SetAlphaCompare(ERglAlphaFunc::Always, 0, ERglAlphaOp::And, ERglAlphaFunc::Always, 0);

  if (x26c_26_AAPH) {
    CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, true);
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::One, ERglLogicOp::Clear);
  } else {
    CGraphics::SetDepthWriteMode(true, ERglEnum::LEqual, x26c_27_ZBUF);
    CGraphics::SetBlendMode(ERglBlendMode::Blend, ERglBlendFactor::SrcAlpha, ERglBlendFactor::InvSrcAlpha,
                            ERglLogicOp::Clear);
  }

  CUVElement* texr = desc->x54_x40_TEXR.get();
  CParticle& firstParticle = x30_particles[0];
  int partFrame = x74_curFrame - firstParticle.x28_startFrame;
  CTexture* cachedTex = texr->GetValueTexture(partFrame).GetObj();
  cachedTex->Load(GX_TEXMAP0, EClampMode::Repeat);

  SUVElementSet uvs = {0.f, 0.f, 1.f, 1.f};
  bool constTexr = texr->HasConstantTexture();
  texr->GetValueUV(partFrame, uvs);
  bool constUVs = texr->HasConstantUV();

  CUVElement* tind = desc->x58_x44_TIND.get();
  CTexture* cachedIndTex = tind->GetValueTexture(partFrame).GetObj();
  cachedIndTex->Load(GX_TEXMAP2, EClampMode::Repeat);

  SUVElementSet uvsInd = {0.f, 0.f, 1.f, 1.f};
  bool constIndTexr = tind->HasConstantTexture();
  bool constIndUVs = tind->HasConstantUV();
  tind->GetValueUV(partFrame, uvsInd);

  std::vector<CParticleListItem> sortItems;
  if (desc->x44_28_x30_28_SORT) {
    sortItems.reserve(x30_particles.size());

    for (size_t i = 0; i < x30_particles.size(); ++i) {
      const CParticle& particle = x30_particles[i];
      sortItems.emplace_back(s16(i));
      CParticleListItem& sortItem = sortItems.back();
      sortItem.x4_viewPoint =
          systemCameraMatrix * ((particle.x4_pos - particle.x10_prevPos) * x80_timeDeltaScale + particle.x10_prevPos);
    }

    std::sort(sortItems.begin(), sortItems.end(), [](const CParticleListItem& a, const CParticleListItem& b) -> bool {
      return a.x4_viewPoint[1] >= b.x4_viewPoint[1];
    });
  }

//  g_instIndTexData.clear();
//  g_instIndTexData.reserve(x30_particles.size());

  //  if (!x30_particles.empty())
  //    CGraphics::SetShaderDataBinding(m_normalDataBind[g_Renderer->IsThermalVisorHotPass()]);

  for (size_t i = 0; i < x30_particles.size(); ++i) {
    const int partIdx = desc->x44_28_x30_28_SORT ? sortItems[i].x0_partIdx : int(i);
    CParticle& particle = x30_particles[partIdx];
    g_currentParticle = &particle;

    const int thisPartFrame = x74_curFrame - particle.x28_startFrame;
    zeus::CVector3f viewPoint;
    if (desc->x44_28_x30_28_SORT) {
      viewPoint = sortItems[i].x4_viewPoint;
    } else {
      viewPoint =
          systemCameraMatrix * ((particle.x4_pos - particle.x10_prevPos) * x80_timeDeltaScale + particle.x10_prevPos);
    }

    if (!constTexr) {
      CTexture* tex = texr->GetValueTexture(thisPartFrame).GetObj();
      if (tex != cachedTex) {
        tex->Load(GX_TEXMAP0, EClampMode::Repeat);
        cachedTex = tex;
      }
    }

    if (!constIndTexr) {
      CTexture* tex = tind->GetValueTexture(thisPartFrame).GetObj();
      if (tex != cachedIndTex) {
        tex->Load(GX_TEXMAP2, EClampMode::Repeat);
        cachedIndTex = tex;
      }
    }

    if (!constUVs)
      texr->GetValueUV(thisPartFrame, uvs);

    if (!constIndUVs)
      tind->GetValueUV(thisPartFrame, uvsInd);

    float size = 0.5f * particle.x2c_lineLengthOrSize;
    zeus::CVector3f p1 = {viewPoint.x() - size, viewPoint.y(), viewPoint.z() - size};
    zeus::CVector3f p2 = {viewPoint.x() + size, viewPoint.y(), viewPoint.z() + size};
    SClipScreenRect clipRect = CGraphics::ClipScreenRectFromMS(p1, p2);

    if (!clipRect.x0_valid)
      continue;

//    CGraphics::ResolveSpareTexture(clipRect);

//    SParticleInstanceIndTex& inst = g_instIndTexData.emplace_back();
//    inst.pos[0] = zeus::CVector4f{viewPoint.x() + size, viewPoint.y(), viewPoint.z() + size, 1.f};
//    inst.pos[1] = zeus::CVector4f{viewPoint.x() - size, viewPoint.y(), viewPoint.z() + size, 1.f};
//    inst.pos[2] = zeus::CVector4f{viewPoint.x() + size, viewPoint.y(), viewPoint.z() - size, 1.f};
//    inst.pos[3] = zeus::CVector4f{viewPoint.x() - size, viewPoint.y(), viewPoint.z() - size, 1.f};
//    inst.color = particle.x34_color;
//    inst.texrTindUVs[0] = zeus::CVector4f{uvs.xMax, uvs.yMax, uvsInd.xMax, uvsInd.yMax};
//    inst.texrTindUVs[1] = zeus::CVector4f{uvs.xMin, uvs.yMax, uvsInd.xMin, uvsInd.yMax};
//    inst.texrTindUVs[2] = zeus::CVector4f{uvs.xMax, uvs.yMin, uvsInd.xMax, uvsInd.yMin};
//    inst.texrTindUVs[3] = zeus::CVector4f{uvs.xMin, uvs.yMin, uvsInd.xMin, uvsInd.yMin};
    //    switch (CGraphics::g_BooPlatform) {
    //    case boo::IGraphicsDataFactory::Platform::OpenGL:
    //      inst.sceneUVs =
    //          zeus::CVector4f{clipRect.x18_uvXMin, clipRect.x24_uvYMax, clipRect.x1c_uvXMax, clipRect.x20_uvYMin};
    //      break;
    //    default:
//    inst.sceneUVs =
//        zeus::CVector4f{clipRect.x18_uvXMin, 1.f - clipRect.x24_uvYMax, clipRect.x1c_uvXMax, 1.f - clipRect.x20_uvYMin};
    //      break;
    //    }
    //    CGraphics::DrawInstances(0, 4, 1, g_instIndTexData.size() - 1);
  }

//  if (g_instIndTexData.size()) {
    //    m_instBuf->load(g_instIndTexData.data(), g_instIndTexData.size() * sizeof(SParticleInstanceIndTex));
    // TODO! this looks like a bug
    // CGraphics::SetShaderDataBinding(m_normalDataBind);
    // CGraphics::DrawInstances(0, 4, g_instIndTexData.size());
//  }
}

void CElementGen::SetOrientation(const zeus::CTransform& orientation) {
  x1d8_orientation = orientation;
  x208_orientationInverse = x1d8_orientation.basis.transposed();

  for (const std::unique_ptr<CParticleGen>& ch : x290_activePartChildren)
    ch->SetOrientation(orientation);
}

void CElementGen::SetTranslation(const zeus::CVector3f& translation) {
  xdc_translation = translation;

  for (const std::unique_ptr<CParticleGen>& ch : x290_activePartChildren) {
    switch (ch->Get4CharId().toUint32()) {
    case SBIG('ELSC'):
      ch->SetTranslation(translation + x2c0_SEPO);
      break;
    case SBIG('SWHC'):
      ch->SetTranslation(translation + x2b0_SSPO);
      break;
    default:
      ch->SetTranslation(translation);
      break;
    }
  }
}

void CElementGen::SetGlobalOrientation(const zeus::CTransform& rotation) {
  x22c_globalOrientation.setRotation(rotation);

  for (const std::unique_ptr<CParticleGen>& ch : x290_activePartChildren)
    ch->SetGlobalOrientation(x22c_globalOrientation);
}

void CElementGen::SetGlobalTranslation(const zeus::CVector3f& translation) {
  xe8_globalTranslation = translation;

  for (const std::unique_ptr<CParticleGen>& ch : x290_activePartChildren)
    ch->SetGlobalTranslation(translation);
}

void CElementGen::SetGlobalScale(const zeus::CVector3f& scale) {
  x100_globalScale = scale;
  x10c_globalScaleTransform = zeus::CTransform::Scale(scale);
  x13c_globalScaleTransformInverse = zeus::CTransform::Scale(zeus::skOne3f / scale);

  for (const std::unique_ptr<CParticleGen>& ch : x290_activePartChildren)
    ch->SetGlobalScale(scale);
}

void CElementGen::SetLocalScale(const zeus::CVector3f& scale) {
  x16c_localScale = scale;
  x178_localScaleTransform = zeus::CTransform::Scale(scale);
  x1a8_localScaleTransformInverse = zeus::CTransform::Scale(zeus::skOne3f / scale);

  for (const std::unique_ptr<CParticleGen>& ch : x290_activePartChildren)
    ch->SetLocalScale(scale);
}

void CElementGen::SetGlobalOrientAndTrans(const zeus::CTransform& xf) {
  SetGlobalOrientation(xf);
  SetGlobalTranslation(xf.origin);
}

void CElementGen::SetParticleEmission(bool enabled) {
  x88_particleEmission = enabled;

  for (const std::unique_ptr<CParticleGen>& ch : x290_activePartChildren)
    ch->SetParticleEmission(enabled);
}

void CElementGen::SetModulationColor(const zeus::CColor& color) {
  x338_moduColor = color;

  for (const std::unique_ptr<CParticleGen>& ch : x290_activePartChildren)
    ch->SetModulationColor(color);
}

void CElementGen::SetGeneratorRate(float rate) {
  if (rate >= 0.0f)
    x98_generatorRate = rate;
  else
    x98_generatorRate = 0.0f;

  for (std::unique_ptr<CParticleGen>& child : x290_activePartChildren) {
    if (child->Get4CharId() == FOURCC('PART'))
      child->SetGeneratorRate(x98_generatorRate);
  }
}

const zeus::CTransform& CElementGen::GetOrientation() const { return x1d8_orientation; }

const zeus::CVector3f& CElementGen::GetTranslation() const { return xdc_translation; }

const zeus::CTransform& CElementGen::GetGlobalOrientation() const { return x22c_globalOrientation; }

const zeus::CVector3f& CElementGen::GetGlobalTranslation() const { return xe8_globalTranslation; }

const zeus::CVector3f& CElementGen::GetGlobalScale() const { return x100_globalScale; }

const zeus::CColor& CElementGen::GetModulationColor() const { return x338_moduColor; }

bool CElementGen::IsSystemDeletable() const {
  for (const std::unique_ptr<CParticleGen>& ch : x290_activePartChildren)
    if (!ch->IsSystemDeletable())
      return false;

  return x268_PSLT < x74_curFrame && x25c_activeParticleCount == 0;
}

std::optional<zeus::CAABox> CElementGen::GetBounds() const {
  if (GetParticleCountAll() == 0)
    return std::nullopt;
  else
    return {x2f0_systemBounds};
}

u32 CElementGen::GetParticleCount() const { return x25c_activeParticleCount; }

bool CElementGen::SystemHasLight() const { return x308_lightType != LightType::None; }

CLight CElementGen::GetLight() const {
  switch (x308_lightType) {
  case LightType::Directional:
    return CLight::BuildDirectional(x320_LDIR.normalized(), x30c_LCLR * x310_LINT);
  case LightType::Spot:
    return CLight::BuildSpot(x314_LOFF, x320_LDIR.normalized(), x30c_LCLR * x310_LINT, x334_LSLA);
  default: {
    float quad = x32c_falloffType == EFalloffType::Quadratic ? x330_LFOR : 0.f;
    float linear = x32c_falloffType == EFalloffType::Linear ? x330_LFOR : 0.f;
    float constant = x32c_falloffType == EFalloffType::Constant ? 1.f : 0.f;
    return CLight::BuildCustom(x314_LOFF, {1.f, 0.f, 0.f}, x30c_LCLR, constant, linear, quad, x310_LINT, 0.f, 0.f);
  }
  }
}

bool CElementGen::GetParticleEmission() const { return x88_particleEmission; }

void CElementGen::DestroyParticles() {
  g_ParticleAliveCount -= x30_particles.size();
  x30_particles.clear();
  x50_parentMatrices.clear();

  for (const std::unique_ptr<CParticleGen>& ch : x290_activePartChildren)
    ch->DestroyParticles();
}

void CElementGen::Reset() {
  x30_particles.clear();
  x50_parentMatrices.clear();
  x290_activePartChildren.clear();

  x74_curFrame = 0;
  x78_curSeconds = 0.f;
  x84_prevFrame = -1;
  x25c_activeParticleCount = 0;
  x26d_25_warmedUp = false;
}

void CElementGen::SetMoveRedToAlphaBuffer(bool move) { sMoveRedToAlphaBuffer = move; }

void CElementGen::RenderBasicParticlesNoRotNoTS(const zeus::CTransform& xf) noexcept {
  for (const auto& particle : x30_particles) {
    const auto pos = xf * particle.x4_pos;
    const auto size = 0.5f * particle.x2c_lineLengthOrSize;
    GXPosition3f32(pos.x() + size, pos.y(), pos.z() + size);
    GXColor4f32(particle.x34_color);
    GXTexCoord2f32(1.f, 1.f);
    GXPosition3f32(pos.x() - size, pos.y(), pos.z() + size);
    GXColor4f32(particle.x34_color);
    GXTexCoord2f32(0.f, 1.f);
    GXPosition3f32(pos.x() - size, pos.y(), pos.z() - size);
    GXColor4f32(particle.x34_color);
    GXTexCoord2f32(0.f, 0.f);
    GXPosition3f32(pos.x() + size, pos.y(), pos.z() - size);
    GXColor4f32(particle.x34_color);
    GXTexCoord2f32(1.f, 0.f);
  }
}

void CElementGen::RenderBasicParticlesNoRotTS(const zeus::CTransform& xf) noexcept {
  for (const auto& particle : x30_particles) {
    const auto pos = xf * (x80_timeDeltaScale * (particle.x4_pos - particle.x10_prevPos) + particle.x10_prevPos);
    const auto size = 0.5f * particle.x2c_lineLengthOrSize;
    GXPosition3f32(pos.x() + size, pos.y(), pos.z() + size);
    GXColor4f32(particle.x34_color);
    GXTexCoord2f32(1.f, 1.f);
    GXPosition3f32(pos.x() - size, pos.y(), pos.z() + size);
    GXColor4f32(particle.x34_color);
    GXTexCoord2f32(0.f, 1.f);
    GXPosition3f32(pos.x() - size, pos.y(), pos.z() - size);
    GXColor4f32(particle.x34_color);
    GXTexCoord2f32(0.f, 0.f);
    GXPosition3f32(pos.x() + size, pos.y(), pos.z() - size);
    GXColor4f32(particle.x34_color);
    GXTexCoord2f32(1.f, 0.f);
  }
}

void CElementGen::RenderBasicParticlesRotNoTS(const zeus::CTransform& xf) noexcept {
  for (const auto& particle : x30_particles) {
    const auto pos = xf * particle.x4_pos;
    const auto size = 0.5f * particle.x2c_lineLengthOrSize;
    const float theta = zeus::degToRad(particle.x30_lineWidthOrRota);
    const float sinT = std::sin(theta) * size;
    const float cosT = std::cos(theta) * size;
    GXPosition3f32(pos.x() + (sinT + cosT), pos.y(), pos.z() + (cosT - sinT));
    GXColor4f32(particle.x34_color);
    GXTexCoord2f32(1.f, 1.f);
    GXPosition3f32(pos.x() + (sinT - cosT), pos.y(), pos.z() + (sinT + cosT));
    GXColor4f32(particle.x34_color);
    GXTexCoord2f32(0.f, 1.f);
    GXPosition3f32(pos.x() - (sinT + cosT), pos.y(), pos.z() - (cosT - sinT));
    GXColor4f32(particle.x34_color);
    GXTexCoord2f32(0.f, 0.f);
    GXPosition3f32(pos.x() + (-sinT + cosT), pos.y(), pos.z() + (-cosT - sinT));
    GXColor4f32(particle.x34_color);
    GXTexCoord2f32(1.f, 0.f);
  }
}

void CElementGen::RenderBasicParticlesRotTS(const zeus::CTransform& xf) noexcept {
  for (const auto& particle : x30_particles) {
    const auto pos = xf * (x80_timeDeltaScale * (particle.x4_pos - particle.x10_prevPos) + particle.x10_prevPos);
    const auto size = 0.5f * particle.x2c_lineLengthOrSize;
    const float theta = zeus::degToRad(particle.x30_lineWidthOrRota);
    const float sinT = std::sin(theta) * size;
    const float cosT = std::cos(theta) * size;
    GXPosition3f32(pos.x() + (sinT + cosT), pos.y(), pos.z() + (cosT - sinT));
    GXColor4f32(particle.x34_color);
    GXTexCoord2f32(1.f, 1.f);
    GXPosition3f32(pos.x() + (sinT - cosT), pos.y(), pos.z() + (sinT + cosT));
    GXColor4f32(particle.x34_color);
    GXTexCoord2f32(0.f, 1.f);
    GXPosition3f32(pos.x() - (sinT + cosT), pos.y(), pos.z() - (cosT - sinT));
    GXColor4f32(particle.x34_color);
    GXTexCoord2f32(0.f, 0.f);
    GXPosition3f32(pos.x() + (-sinT + cosT), pos.y(), pos.z() + (-cosT - sinT));
    GXColor4f32(particle.x34_color);
    GXTexCoord2f32(1.f, 0.f);
  }
}
} // namespace metaforce
