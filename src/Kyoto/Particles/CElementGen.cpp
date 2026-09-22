#include "Kyoto/Particles/CElementGen.hpp"

#include "Kyoto/Basics/CStopwatch.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Graphics/CModel.hpp"
#include "Kyoto/Graphics/CModelFlags.hpp"
#include "Kyoto/Graphics/CTevCombiners.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "Kyoto/Particles/CParticleGlobals.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"
#include "Kyoto/Particles/CWarp.hpp"
#include "Kyoto/Particles/IElement.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

#include "alloca.h"
#include "math.h"

#include "dolphin/gx.h"

#pragma inline_max_size(250)
#pragma inline_max_total_size(20000) // for RenderParticles vector inlining

#ifdef __MWERKS__
#define frsqrte(x) static_cast< float >(__frsqrte(x))
#else
extern "C" float frsqrte(float);
#endif

// NOTE: This is a hack, trying to figure out what causes the wack GX codegen in
// CElementGen::RenderParticles. The `const CVector3f&` binding seems to help.
#define GXPositionVector3f(in)                                                                     \
  {                                                                                                \
    const CVector3f& vec = (in);                                                                   \
    GXPosition3f32(vec[0], vec[1], vec[2]);                                                        \
  }

bool CElementGen::sMoveRedToAlphaBuffer;
bool CElementGen::sSubtractBlend;

int CElementGen::mParticleAliveCount;
int CElementGen::mParticleSystemAliveCount;

double CElementGen::kTickTime = 1 / 60.0;
ushort CElementGen::sSeed = 99;
static bool sStaticListInitialized;

struct CParticleListItemViewPointComp {
  bool operator()(const CElementGen::CParticleListItem& a,
                  const CElementGen::CParticleListItem& b) const {
    return a.x4_viewPoint.GetY() > b.x4_viewPoint.GetY() ? true : false;
  }
};

// name?

struct CTexturedParticleListItemViewPointComp {
  bool operator()(const CElementGen::CTexturedParticleListItem& a,
                  const CElementGen::CTexturedParticleListItem& b) const {
    return a.x4_viewPoint.GetY() > b.x4_viewPoint.GetY() ? true : false;
  }
};

static const GXVtxDescList sVtxDescList[] = {
    {GX_VA_POS, GX_DIRECT},
    {GX_VA_CLR0, GX_DIRECT},
    {GX_VA_TEX0, GX_DIRECT},
    {GX_VA_NULL, GX_NONE},
};

CElementGen::CElementGen(TToken< CGenDescription > gen, EModelOrientationType orientType,
                         EOptionalSystemFlags optFlags)
: x1c_genDesc(gen)
, x28_loadedGenDesc(*x1c_genDesc)
, x2c_orientType(orientType)
, x70_internalStartFrame(0)
, x74_curFrame(0)
, x78_curSeconds(0.0)
, x84_prevFrame(-1)
, x88_particleEmission(true)
, x8c_generatorRemainder(0.f)
, x90_MAXP(0)
, x94_randomSeed(sSeed)
, x98_generatorRate(1.f)
, xdc_translation(CVector3f::Zero())
, xe8_globalTranslation(CVector3f::Zero())
, xf4_POFS(CVector3f::Zero())
, x100_globalScale(1.f, 1.f, 1.f)
, x10c_globalScaleTransform(CTransform4f::Identity())
, x13c_globalScaleTransformInverse(CTransform4f::Identity())
, x16c_localScale(1.f, 1.f, 1.f)
, x178_localScaleTransform(CTransform4f::Identity())
, x1a8_localScaleTransformInverse(CTransform4f::Identity())
, x1d8_orientation(CTransform4f::Identity())
, x208_orientationInverse(CMatrix3f::Identity())
, x22c_globalOrientation(CTransform4f::Identity())
, x25c_activeParticleCount(0)
, x260_cumulativeParticles(0)
, x264_recursiveParticleCount(0)
, x26c_24_translationDirty(false)
, x26c_28_zTest(true)
, x26c_29_ORNT(false)
, x26d_25_warmedUp(false)
, x26d_26_modelsUseLights(false)
, x26d_27_enableOPTS(optFlags & kOSF_Two)
, x26d_28_enableADV(false)
, x274_backupLightActive(false)
, x27c_randState(x94_randomSeed)
, x2b0_SSPO(CVector3f::Zero())
, x2c0_SEPO(CVector3f::Zero())
, x2cc(0.f)
, x2d0(0.f)
, x2d4_aabbMin(CVector3f::Zero())
, x2e0_aabbMax(CVector3f::Zero())
, x2ec_maxSize(0.f)
, x2f0_systemBounds(CAABox::MakeMaxInvertedBox())
, x308_lightType(kLT_None)
, x30c_LCLR(CColor::White())
, x310_LINT(1.f)
, x314_LOFF(CVector3f::Zero())
, x320_LDIR(1.f, 0.f, 0.f)
, x32c_falloffType(kFT_Linear)
, x330_LFOR(1.f)
, x334_LSLA(45.f)
, x338_moduColor(0xFFFFFFFF) {
  CGlobalRandom gr(x27c_randState);

  if (CIntElement* seed = x28_loadedGenDesc->x10_SEED) {
    int seedVal;
    seed->GetValue(x74_curFrame, seedVal);
    x94_randomSeed = seedVal;
  }
  x27c_randState.SetSeed(x94_randomSeed);
  ++mParticleSystemAliveCount;

  x26c_25_LIT_ = x28_loadedGenDesc->x30_29_LIT_;
  x26c_26_AAPH = x28_loadedGenDesc->x30_26_AAPH;
  x26c_27_ZBUF = x28_loadedGenDesc->x30_27_ZBUF;
  x26c_29_ORNT = x28_loadedGenDesc->x30_30_ORNT;
  x26c_30_MBLR = x26c_29_ORNT ? false : x28_loadedGenDesc->x31_24_MBLR;

  if (CIntElement* mbsp = x28_loadedGenDesc->x34_MBSP) {
    mbsp->GetValue(x74_curFrame, x270_MBSP);
  }

  int idx = 0;
  if (CModVectorElement* vel1 = x28_loadedGenDesc->x68_VEL1) {
    x280_VELSources[idx] = vel1;
    x278_hasVMD[idx] = x28_loadedGenDesc->x31_28_VMD1;
    ++idx;
  }
  if (CModVectorElement* vel2 = x28_loadedGenDesc->x6c_VEL2) {
    x280_VELSources[idx] = vel2;
    x278_hasVMD[idx] = x28_loadedGenDesc->x31_29_VMD2;
    ++idx;
  }
  if (CModVectorElement* vel3 = x28_loadedGenDesc->x70_VEL3) {
    x280_VELSources[idx] = vel3;
    x278_hasVMD[idx] = x28_loadedGenDesc->x31_30_VMD3;
    ++idx;
  }
  if (CModVectorElement* vel4 = x28_loadedGenDesc->x74_VEL4) {
    x280_VELSources[idx] = vel4;
    x278_hasVMD[idx] = x28_loadedGenDesc->x31_31_VMD4;
    ++idx;
  }
  for (int i = idx; i < 4; ++i) {
    x280_VELSources[i] = nullptr;
  }

  if (x28_loadedGenDesc->x10c_ADV1 || x28_loadedGenDesc->x110_ADV2 ||
      x28_loadedGenDesc->x114_ADV3 || x28_loadedGenDesc->x118_ADV4 ||
      x28_loadedGenDesc->x11c_ADV5 || x28_loadedGenDesc->x120_ADV6 ||
      x28_loadedGenDesc->x124_ADV7 || x28_loadedGenDesc->x128_ADV8) {
    x26d_28_enableADV = true;
  }

  for (int i = 0; i < 16; ++i) {
    x9c_externalVars[i] = 0.f;
  }

  x2a0_CSSD = 0;
  if (CIntElement* cssd = x28_loadedGenDesc->x8c_CSSD) {
    cssd->GetValue(0, x2a0_CSSD);
  }

  x2a8_PISY = 16;
  if (CIntElement* pisy = x28_loadedGenDesc->xb4_PISY) {
    pisy->GetValue(0, x2a8_PISY);
    if (x2a8_PISY <= 0) {
      x2a8_PISY = 1;
    }
  }

  x2a4_SISY = 16;
  if (CIntElement* sisy = x28_loadedGenDesc->xb8_SISY) {
    sisy->GetValue(0, x2a4_SISY);
  }

  x2ac_SSSD = 0;
  if (CIntElement* sssd = x28_loadedGenDesc->xd0_SSSD) {
    sssd->GetValue(0, x2ac_SSSD);
  }

  if (x28_loadedGenDesc->xd4_SSPO) {
    x28_loadedGenDesc->xd4_SSPO->GetValue(0, x2b0_SSPO);
    if (!x28_loadedGenDesc->xd4_SSPO->IsFastConstant()) {
      x26c_24_translationDirty = true;
    }
  }

  x2bc_SESD = 0;
  if (CIntElement* sesd = x28_loadedGenDesc->xe4_SESD) {
    sesd->GetValue(0, x2bc_SESD);
  }

  if (x28_loadedGenDesc->xe8_SEPO) {
    x28_loadedGenDesc->xe8_SEPO->GetValue(0, x2c0_SEPO);
    if (!x28_loadedGenDesc->xe8_SEPO->IsFastConstant()) {
      x26c_24_translationDirty = true;
    }
  }

  if (x28_loadedGenDesc->xc_POFS) {
    x28_loadedGenDesc->xc_POFS->GetValue(x74_curFrame, xf4_POFS);
    if (!x28_loadedGenDesc->xc_POFS->IsFastConstant()) {
      x26c_24_translationDirty = true;
    }
  }

  if (CIntElement* pslt = x28_loadedGenDesc->x0_PSLT) {
    pslt->GetValue(0, x268_PSLT);
  } else {
    x268_PSLT = 0x7FFFFF;
  }

  if (CIntElement* maxp = x28_loadedGenDesc->x1c_MAXP) {
    maxp->GetValue(x74_curFrame, x90_MAXP);
  }

  int count = x90_MAXP;
  if (count > 256) {
    count = 256;
  }
  x30_particles.reserve(count);

  if (x26d_28_enableADV) {
    x60_advValues.assign(count);
  }

  if (x2c_orientType == kMOT_One) {
    x50_parentMatrices.assign(x90_MAXP, CMatrix3f::Identity());
  }

  x26c_31_LINE = x28_loadedGenDesc->x30_24_LINE;
  x26d_24_FXLL = x28_loadedGenDesc->x30_25_FXLL;

  if (x28_loadedGenDesc->xec_LTYP != nullptr) {
    int ltypVal = 0;
    x28_loadedGenDesc->xec_LTYP->GetValue(x74_curFrame, ltypVal);
    switch (LightType(ltypVal)) {
    case kLT_None:
      x308_lightType = kLT_None;
      break;
    case kLT_Custom:
      x308_lightType = kLT_Custom;
      break;
    case kLT_Directional:
      x308_lightType = kLT_Directional;
      break;
    case kLT_Spot:
      x308_lightType = kLT_Spot;
      break;
    default:
      x308_lightType = kLT_None;
      break;
    }

    if (x28_loadedGenDesc->x100_LFOT != nullptr) {
      int lfotVal = 1;
      x28_loadedGenDesc->x100_LFOT->GetValue(x74_curFrame, lfotVal);
      switch (static_cast< EFalloffType >(lfotVal)) {
      case kFT_Constant:
        x32c_falloffType = kFT_Constant;
        break;
      case kFT_Linear:
        x32c_falloffType = kFT_Linear;
        break;
      case kFT_Quadratic:
        x32c_falloffType = kFT_Quadratic;
        break;
      default:
        x32c_falloffType = kFT_Linear;
        break;
      }
    }
  }
}

CElementGen::~CElementGen() {
  --mParticleSystemAliveCount;
  mParticleAliveCount -= x30_particles.size();

  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    delete *it;
  }
}

void CElementGen::Initialize() {
  if (sStaticListInitialized)
    return;
  mParticleAliveCount = 0;
  mParticleSystemAliveCount = 0;
  sStaticListInitialized = true;
}

void CElementGen::ShutDown() { sStaticListInitialized = false; }

void CElementGen::SetTranslation(const CVector3f& translation) {
  xdc_translation = translation;
  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    CParticleGen* ch = *it;
    if (ch->Get4CharId() == 'ELSC') {
      ch->SetTranslation(translation + x2c0_SEPO);
    } else if (ch->Get4CharId() == 'SWHC') {
      ch->SetTranslation(translation + x2b0_SSPO);
    } else {
      ch->SetTranslation(translation);
    }
  }
}

void CElementGen::SetGlobalTranslation(const CVector3f& translation) {
  xe8_globalTranslation = translation;
  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    (*it)->SetGlobalTranslation(translation);
  }
}

void CElementGen::SetModulationColor(const CColor& col) {
  x338_moduColor = col;
  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    (*it)->SetModulationColor(col);
  }
}

const CColor& CElementGen::GetModulationColor() const { return x338_moduColor; }

void CElementGen::SetGlobalScale(const CVector3f& scale) {
  x100_globalScale = scale;
  x10c_globalScaleTransform = CTransform4f::Scale(x100_globalScale.GetX(), x100_globalScale.GetY(),
                                                  x100_globalScale.GetZ());
  x13c_globalScaleTransformInverse = CTransform4f::Scale(
      1.f / x100_globalScale.GetX(), 1.f / x100_globalScale.GetY(), 1.f / x100_globalScale.GetZ());
  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    (*it)->SetGlobalScale(scale);
  }
}

void CElementGen::SetLocalScale(const CVector3f& scale) {
  x16c_localScale = scale;
  x178_localScaleTransform =
      CTransform4f::Scale(x16c_localScale.GetX(), x16c_localScale.GetY(), x16c_localScale.GetZ());
  x1a8_localScaleTransformInverse = CTransform4f::Scale(
      1.f / x16c_localScale.GetX(), 1.f / x16c_localScale.GetY(), 1.f / x16c_localScale.GetZ());
  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    (*it)->SetLocalScale(scale);
  }
}

void CElementGen::SetOrientation(const CTransform4f& orientation) {
  x1d8_orientation = orientation;
  x208_orientationInverse = x1d8_orientation.GetQuickInverse().BuildMatrix3f();
  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    (*it)->SetOrientation(orientation);
  }
}

void CElementGen::SetGlobalOrientation(const CTransform4f& orientation) {
  x22c_globalOrientation.SetRotation(orientation);
  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    (*it)->SetGlobalOrientation(x22c_globalOrientation);
  }
}

void CElementGen::SetGlobalOrientAndTrans(const CTransform4f& xf) {
  SetGlobalOrientation(xf);
  SetGlobalTranslation(xf.GetTranslation());
}

const bool CElementGen::Update(double dt) {
  CParticleGlobals::SParticleSystem thisSystem('PART', this);

  if (x28_loadedGenDesc->x4_PSWT && !x26d_25_warmedUp) {
    int pswt = 0;
    x28_loadedGenDesc->x4_PSWT->GetValue(x74_curFrame, pswt);
    InternalUpdate(kTickTime * pswt);
    x26d_25_warmedUp = true;
  }

  return InternalUpdate(dt);
}

bool CElementGen::InternalUpdate(double dt) {
  CStopwatch sw;
  CGlobalRandom gr(x27c_randState);

  int frameUpdateCount = 0;
  double t = x74_curFrame * kTickTime;
  double dt1 = close_enough(dt, kTickTime) ? kTickTime : dt;

  CParticleGlobals::SetEmitterTime(x74_curFrame);

  if (x28_loadedGenDesc->x8_PSTS) {
    float psts = 1.0f;
    x28_loadedGenDesc->x8_PSTS->GetValue(x74_curFrame, psts);
    dt1 *= psts;
    dt1 = rstl::max_val(0.0, dt1);
  }

  x78_curSeconds += dt1;

  if (x26c_30_MBLR && dt > 0.0) {
    if (x28_loadedGenDesc->x34_MBSP) {
      x28_loadedGenDesc->x34_MBSP->GetValue(x74_curFrame, x270_MBSP);
    }
  }

  while (t < x78_curSeconds && !close_enough(t, x78_curSeconds)) {
    x2d4_aabbMin = CVector3f(FLT_MAX, FLT_MAX, FLT_MAX);
    x2e0_aabbMax = CVector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);
    x2ec_maxSize = 0.0f;

    CParticleGlobals::SetEmitterTime(x74_curFrame);
    UpdateExistingParticles();
    CParticleGlobals::SetParticleLifetime(x268_PSLT);

    if (x74_curFrame < x268_PSLT && x88_particleEmission) {
      float grte = 0.0f;
      if (x28_loadedGenDesc->x20_GRTE->GetValue(x74_curFrame, grte)) {
        x30_particles.clear();
        return true;
      }

      grte = rstl::max_val(0.0f, grte * x98_generatorRate);
      x8c_generatorRemainder += grte;
      int genCount = static_cast< int >(floor(x8c_generatorRemainder));
      x8c_generatorRemainder -= static_cast< float >(genCount);

      if (x28_loadedGenDesc->x1c_MAXP) {
        x28_loadedGenDesc->x1c_MAXP->GetValue(x74_curFrame, x90_MAXP);
      }

      CreateNewParticles(genCount);
    }

    if (x26c_24_translationDirty) {
      UpdatePSTranslationAndOrientation();
    }

    if (x308_lightType != kLT_None) {
      UpdateLightParameters();
    }

    UpdateChildParticleSystems(kTickTime);

    ++frameUpdateCount;
    t += kTickTime;
    ++x74_curFrame;
  }

  if (close_enough(t, x78_curSeconds)) {
    x78_curSeconds = t;
    x80_timeDeltaScale = 1.0f;
  } else {
    UpdateChildParticleSystems(dt1 - (double)frameUpdateCount * kTickTime);
    x80_timeDeltaScale = 1.0f - static_cast< float >((t - x78_curSeconds) / kTickTime);
  }

  BuildParticleSystemBounds();

  x2cc = sw.GetElapsedTime();
  return false;
}

void CElementGen::UpdateLightParameters() {
  x28_loadedGenDesc->xf0_LCLR->GetValue(x74_curFrame, x30c_LCLR);
  x28_loadedGenDesc->xf4_LINT->GetValue(x74_curFrame, x310_LINT);
  if (x308_lightType != kLT_Directional) {
    x28_loadedGenDesc->xf8_LOFF->GetValue(x74_curFrame, x314_LOFF);
    x28_loadedGenDesc->x104_LFOR->GetValue(x74_curFrame, x330_LFOR);
    if (x308_lightType == kLT_Spot) {
      x28_loadedGenDesc->x108_LSLA->GetValue(x74_curFrame, x334_LSLA);
    }
  }
  if (x308_lightType != kLT_Custom) {
    x28_loadedGenDesc->xfc_LDIR->GetValue(x74_curFrame, x320_LDIR);
  }
}

void CElementGen::UpdateAdvanceAccessParameters(int particleFrame, int particleIndex) {
  float* params = x60_advValues[particleFrame].values;
  CParticleGlobals::mParticleAccessParameters = params;
  if (x28_loadedGenDesc->x10c_ADV1) {
    x28_loadedGenDesc->x10c_ADV1->GetValue(particleIndex, params[0]);
  }
  if (x28_loadedGenDesc->x110_ADV2) {
    x28_loadedGenDesc->x110_ADV2->GetValue(particleIndex, params[1]);
  }
  if (x28_loadedGenDesc->x114_ADV3) {
    x28_loadedGenDesc->x114_ADV3->GetValue(particleIndex, params[2]);
  }
  if (x28_loadedGenDesc->x118_ADV4) {
    x28_loadedGenDesc->x118_ADV4->GetValue(particleIndex, params[3]);
  }
  if (x28_loadedGenDesc->x11c_ADV5) {
    x28_loadedGenDesc->x11c_ADV5->GetValue(particleIndex, params[4]);
  }
  if (x28_loadedGenDesc->x120_ADV6) {
    x28_loadedGenDesc->x120_ADV6->GetValue(particleIndex, params[5]);
  }
  if (x28_loadedGenDesc->x124_ADV7) {
    x28_loadedGenDesc->x124_ADV7->GetValue(particleIndex, params[6]);
  }
  if (x28_loadedGenDesc->x128_ADV8) {
    x28_loadedGenDesc->x128_ADV8->GetValue(particleIndex, params[7]);
  }
}

bool CElementGen::UpdateVelocitySource(int velIdx, int frame, CParticle& particle) {
  bool reset;
  if (x278_hasVMD[velIdx]) {
    CVector3f localVel = x208_orientationInverse * particle.x1c_vel;
    CVector3f localPos = x208_orientationInverse * (particle.x4_pos - xdc_translation);
    reset = x280_VELSources[velIdx]->GetValue(frame, localVel, localPos);
    particle.x1c_vel = x1d8_orientation.Rotate(localVel);
    particle.x4_pos = x1d8_orientation.Rotate(localPos) + xdc_translation;
  } else {
    reset = x280_VELSources[velIdx]->GetValue(frame, particle.x1c_vel, particle.x4_pos);
  }
  if (reset) {
    particle.x0_endFrame = -1;
    return true;
  }
  return false;
}

void CElementGen::UpdateExistingParticles() {
  AUTO(p, x30_particles.begin());
  x25c_activeParticleCount = 0;
  CParticleGlobals::SetEmitterTime(x74_curFrame);
  CParticleGlobals::mParticleAccessParameters = NULL;

  while (p != x30_particles.end()) {
    if (p->x0_endFrame < x74_curFrame) {
      --mParticleAliveCount;
      if (p + 1 == x30_particles.end()) {
        x30_particles.x4_count = x30_particles.size() - 1;
        break;
      }
      *p = x30_particles.back();
      if (x2c_orientType == kMOT_One) {
        x50_parentMatrices[x25c_activeParticleCount] = x50_parentMatrices[x30_particles.size() - 1];
      }
      if (x26d_28_enableADV) {
        x60_advValues[x25c_activeParticleCount] = x60_advValues[x30_particles.size() - 1];
      }
      --x30_particles.x4_count;
      if (p != x30_particles.end()) {
        if (p->x0_endFrame < x74_curFrame) {
          continue;
        }
      }
    }
    p->x10_prevPos = p->x4_pos;
    p->x4_pos += p->x1c_vel;
    int particleFrame = x74_curFrame - p->x28_startFrame;
    CParticleGlobals::mCurrentParticle = &*p;
    CParticleGlobals::SetParticleLifetime(p->x0_endFrame - p->x28_startFrame);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(particleFrame);
    if (x26d_28_enableADV) {
      UpdateAdvanceAccessParameters(x25c_activeParticleCount, particleFrame);
    }
    ++x25c_activeParticleCount;
    if (x280_VELSources[0] != NULL) {
      UpdateVelocitySource(0, particleFrame, *p);
      if (x280_VELSources[1] != NULL) {
        UpdateVelocitySource(1, particleFrame, *p);
        if (x280_VELSources[2] != NULL) {
          UpdateVelocitySource(2, particleFrame, *p);
          if (x280_VELSources[3] != NULL) {
            UpdateVelocitySource(3, particleFrame, *p);
          }
        }
      }
    }
    if (x26c_31_LINE) {
      if (CRealElement* leng = x28_loadedGenDesc->x14_LENG) {
        leng->GetValue(particleFrame, p->x2c_lineLengthOrSize);
      }
      if (CRealElement* widt = x28_loadedGenDesc->x18_WIDT) {
        widt->GetValue(particleFrame, p->x30_lineWidthOrRota);
      }
    } else {
      if (CRealElement* rota = x28_loadedGenDesc->x3c_ROTA) {
        rota->GetValue(particleFrame, p->x30_lineWidthOrRota);
      }
      if (CRealElement* size = x28_loadedGenDesc->x38_SIZE) {
        size->GetValue(particleFrame, p->x2c_lineLengthOrSize);
      }
    }
    if (CColorElement* colr = x28_loadedGenDesc->x24_COLR) {
      colr->GetValue(particleFrame, p->x34_color);
    }
    AccumulateBounds(p->x4_pos, p->x2c_lineLengthOrSize);
    ++p;
  }

  if (x30_particles.size() > 0) {
    rstl::list< CWarp* >::iterator it = x4_modifiersList.begin();
    rstl::list< CWarp* >::iterator endIt = x4_modifiersList.end();
    while (it != endIt) {
      if ((*it)->UpdateWarp()) {
        CParticle* base = x30_particles.data();
        (*it)->ModifyParticles(x30_particles.size(), sizeof(CParticle), &base->x0_endFrame,
                               &base->x10_prevPos, &base->x4_pos, &base->x1c_vel, &base->x34_color,
                               &base->x2c_lineLengthOrSize, &base->x30_lineWidthOrRota);
      }
      ++it;
    }
  }
}

void CElementGen::CreateNewParticles(int count) {
  if (!sStaticListInitialized) {
    Initialize();
  }

  int newCount = count;
  if (x30_particles.size() >= x90_MAXP) {
    return;
  }

  if (newCount + x30_particles.size() > x90_MAXP) {
    newCount = x90_MAXP - x30_particles.size();
  }

  if (newCount + mParticleAliveCount > 0xa00) {
    newCount = 0xa00 - mParticleAliveCount;
  }

  CGlobalRandom gr(x27c_randState);
  x30_particles.reserve(newCount + x30_particles.size());

  if (x26d_28_enableADV && x60_advValues.capacity() < newCount + x30_particles.size()) {
    int newCap = x60_advValues.capacity() * 2;
    x60_advValues.reserve(rstl::min_val(x90_MAXP, newCap));
    while (x60_advValues.size() < x60_advValues.capacity()) {
      x60_advValues.push_back(CAdvancedValues());
    }
  }

  CParticleGlobals::mParticleAccessParameters = NULL;

  for (int i = 0; i < newCount; ++i) {
    // TODO: The x1c_vel values somehow get frsp'd, despite being
    // floats from CVector3f::Zero and being stored as floats in CParticle.
    x30_particles.push_back(CParticle());

    int particleIndex = x30_particles.size() - 1;
    ++mParticleAliveCount;
    ++x25c_activeParticleCount;
    ++x260_cumulativeParticles;

    if (x2c_orientType == kMOT_One) {
      x50_parentMatrices[particleIndex] = x1d8_orientation.BuildMatrix3f();
    }

    CParticle& particle = x30_particles[particleIndex];
    particle.x28_startFrame = x74_curFrame;

    if (CIntElement* ltme = x28_loadedGenDesc->x28_LTME) {
      ltme->GetValue(0, particle.x0_endFrame);
    }

    CParticleGlobals::SetParticleLifetime(particle.x0_endFrame);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(0);
    CParticleGlobals::mCurrentParticle = &particle;

    if (x26d_28_enableADV) {
      UpdateAdvanceAccessParameters(particleIndex, 0);
    }

    particle.x0_endFrame += x74_curFrame;

    if (CColorElement* colr = x28_loadedGenDesc->x24_COLR) {
      colr->GetValue(0, particle.x34_color);
    } else {
      particle.x34_color = CColor(0xFFFFFFFF);
    }

    if (CEmitterElement* emtr = x28_loadedGenDesc->x2c_EMTR) {
      emtr->GetValue(x74_curFrame, particle.x4_pos, particle.x1c_vel);
      particle.x4_pos =
          x1d8_orientation.Rotate(particle.x4_pos) +
          ((x13c_globalScaleTransformInverse * x1a8_localScaleTransformInverse) * xdc_translation) +
          xf4_POFS;
      particle.x1c_vel = x1d8_orientation.Rotate(particle.x1c_vel);
    } else {
      const CVector3f& compPos =
          (x13c_globalScaleTransformInverse * x1a8_localScaleTransformInverse) * xdc_translation;
      particle.x4_pos = compPos + xf4_POFS;
      particle.x1c_vel = CVector3f::Zero();
    }

    particle.x10_prevPos = particle.x4_pos;

    if (x26c_31_LINE) {
      if (CRealElement* leng = x28_loadedGenDesc->x14_LENG) {
        leng->GetValue(0, particle.x2c_lineLengthOrSize);
      } else {
        particle.x2c_lineLengthOrSize = 1.f;
      }
      if (CRealElement* widt = x28_loadedGenDesc->x18_WIDT) {
        widt->GetValue(0, particle.x30_lineWidthOrRota);
      } else {
        particle.x30_lineWidthOrRota = 1.f;
      }
    } else {
      if (CRealElement* rota = x28_loadedGenDesc->x3c_ROTA) {
        rota->GetValue(0, particle.x30_lineWidthOrRota);
      } else {
        particle.x30_lineWidthOrRota = 0.f;
      }
      if (CRealElement* size = x28_loadedGenDesc->x38_SIZE) {
        size->GetValue(0, particle.x2c_lineLengthOrSize);
      } else {
        particle.x2c_lineLengthOrSize = 0.1f;
      }
    }

    AccumulateBounds(particle.x4_pos, particle.x2c_lineLengthOrSize);
  }
}

void CElementGen::UpdatePSTranslationAndOrientation() {
  CGlobalRandom gr(x27c_randState);
  if (x268_PSLT >= x74_curFrame) {
    if (CVectorElement* pofs = x28_loadedGenDesc->xc_POFS) {
      pofs->GetValue(x74_curFrame, xf4_POFS);
    }
    if (CVectorElement* sspo = x28_loadedGenDesc->xd4_SSPO) {
      sspo->GetValue(x74_curFrame, x2b0_SSPO);
    }
    if (CVectorElement* sepo = x28_loadedGenDesc->xe8_SEPO) {
      sepo->GetValue(x74_curFrame, x2c0_SEPO);
    }
  }
}

CElementGen* CElementGen::ConstructChildParticleSystem(TToken< CGenDescription > desc) const {
  CElementGen* child =
      rs_new CElementGen(desc, kMOT_Normal, x26d_27_enableOPTS ? kOSF_Two : kOSF_One);
  child->x26d_26_modelsUseLights = x26d_26_modelsUseLights;
  child->SetGlobalTranslation(xe8_globalTranslation);
  child->SetGlobalOrientation(x22c_globalOrientation);
  child->SetGlobalScale(x100_globalScale);
  child->SetLocalScale(x16c_localScale);
  child->SetTranslation(xdc_translation);
  child->SetOrientation(x1d8_orientation);
  child->SetParticleEmission(x88_particleEmission);
  child->SetModulationColor(x338_moduColor);
  return child;
}

void CElementGen::UpdateChildParticleSystems(double dt) {
  if (close_enough(dt, 0.0, 1e-7)) {
    return;
  }

  CGlobalRandom gr(x27c_randState);

  // ICTS - child particle systems spawned at CSSD frame
  if (x28_loadedGenDesc->x78_ICTS && x84_prevFrame != x74_curFrame && x74_curFrame == x2a0_CSSD) {
    int ncsyVal = 1;
    if (CIntElement* ncsy = x28_loadedGenDesc->x88_NCSY) {
      ncsy->GetValue(x74_curFrame, ncsyVal);
    }
    x290_activePartChildren.reserve(ncsyVal + x290_activePartChildren.size());
    for (int i = 0; i < ncsyVal; ++i) {
      TLockedToken< CGenDescription > icsToken = x28_loadedGenDesc->x78_ICTS->GetToken();
      bool descOPTS = icsToken->x32_25_OPTS;
      if (x26d_27_enableOPTS && descOPTS) {
        break;
      }
      x290_activePartChildren.push_back(ConstructChildParticleSystem(icsToken));
    }
  }

  // IITS - child particle systems spawned periodically
  if (x28_loadedGenDesc->xa4_IITS && x84_prevFrame != x74_curFrame && x74_curFrame < x268_PSLT &&
      x88_particleEmission == true && x74_curFrame >= x2a4_SISY &&
      ((x74_curFrame - x2a4_SISY) % x2a8_PISY) == 0) {
    TLockedToken< CGenDescription > iitsToken = x28_loadedGenDesc->xa4_IITS->GetToken();
    bool iitsOPTS = iitsToken->x32_25_OPTS;
    if (!(x26d_27_enableOPTS && iitsOPTS)) {
      x290_activePartChildren.reserve(x290_activePartChildren.size() + 1);
      x290_activePartChildren.push_back(ConstructChildParticleSystem(iitsToken));
    }
  }

  // KSSM - spawn system keyframe data
  if (x28_loadedGenDesc->xbc_KSSM.get() != nullptr && x84_prevFrame != x74_curFrame &&
      x74_curFrame < x268_PSLT) {
    const ushort backupSeed = sSeed;
    rstl::vector< CSpawnSystemKeyframeData::CSpawnSystemKeyframeInfo >& spawns =
        x28_loadedGenDesc->xbc_KSSM->GetSpawnedSystemsAtFrame(x74_curFrame);
    x290_activePartChildren.reserve(spawns.size() + x290_activePartChildren.size());
    for (int i = 0; i < spawns.size(); ++i) {
      TLockedToken< CGenDescription > kssmToken = *spawns[i].GetToken();
      bool kssmOPTS = kssmToken->x32_25_OPTS;
      if (x26d_27_enableOPTS && kssmOPTS) {
        continue;
      }
      sSeed = backupSeed + i;
      x290_activePartChildren.push_back(ConstructChildParticleSystem(kssmToken));
    }
    sSeed = backupSeed;
  }

  // IDTS - child particle systems spawned at death
  if (x74_curFrame == x268_PSLT && x84_prevFrame != x74_curFrame && x28_loadedGenDesc->x90_IDTS) {
    int ndsyVal = 1;
    if (CIntElement* ndsy = x28_loadedGenDesc->xa0_NDSY) {
      ndsy->GetValue(0, ndsyVal);
    }
    x290_activePartChildren.reserve(ndsyVal + x290_activePartChildren.size());
    for (int i = 0; i < ndsyVal; ++i) {
      TLockedToken< CGenDescription > idtsToken = x28_loadedGenDesc->x90_IDTS->GetToken();
      bool idtsOPTS = idtsToken->x32_25_OPTS;
      if (x26d_27_enableOPTS && idtsOPTS) {
        break;
      }
      x290_activePartChildren.push_back(ConstructChildParticleSystem(idtsToken));
    }
  }

  // SSWH - swoosh child particle system
  if (x28_loadedGenDesc->xc0_SSWH && x84_prevFrame != x74_curFrame && x74_curFrame == x2ac_SSSD) {
    CParticleSwoosh* swoosh = rs_new CParticleSwoosh(*x28_loadedGenDesc->xc0_SSWH, 0);
    swoosh->SetGlobalTranslation(xe8_globalTranslation);
    swoosh->SetGlobalScale(x100_globalScale);
    swoosh->SetLocalScale(x16c_localScale);
    swoosh->SetTranslation(xdc_translation + x2b0_SSPO);
    swoosh->SetOrientation(x1d8_orientation);
    const bool emission = x88_particleEmission;
    swoosh->SetParticleEmission(emission);
    x290_activePartChildren.reserve(x290_activePartChildren.size() + 1);
    x290_activePartChildren.push_back(swoosh);
  }

  // SELC - electric child particle system
  if (x28_loadedGenDesc->xd8_SELC && x84_prevFrame != x74_curFrame && x74_curFrame == x2bc_SESD) {
    CParticleElectric* electric = rs_new CParticleElectric(*x28_loadedGenDesc->xd8_SELC);
    electric->SetGlobalTranslation(xe8_globalTranslation);
    electric->SetGlobalScale(x100_globalScale);
    electric->SetLocalScale(x16c_localScale);
    electric->SetTranslation(xdc_translation + x2c0_SEPO);
    electric->SetOrientation(x1d8_orientation);
    electric->SetParticleEmission(x88_particleEmission);
    x290_activePartChildren.reserve(x290_activePartChildren.size() + 1);
    x290_activePartChildren.push_back(electric);
  }

  // Update all child particle systems
  AUTO(it, x290_activePartChildren.begin());
  while (it != x290_activePartChildren.end()) {
    CParticleGen* child = *it;
    child->Update(dt);
    if (child->IsSystemDeletable() == true) {
      delete child;
      it = x290_activePartChildren.erase(it);
    } else {
      ++it;
    }
  }

  x84_prevFrame = x74_curFrame;
}

void CElementGen::SetParticleEmission(bool emission) {
  x88_particleEmission = emission;
  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    (*it)->SetParticleEmission(emission);
  }
}

void CElementGen::ForceParticleCreation(int amount) {
  CParticleGlobals::SParticleSystem thisSystem('PART', this);
  CParticleGlobals::SetEmitterTime(x74_curFrame);
  CreateNewParticles(amount);
}

void CElementGen::EndLifetime() {
  x268_PSLT = 0;
  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    CParticleGen* child = *it;
    if (child->Get4CharId() == 'PART') {
      static_cast< CElementGen* >(child)->EndLifetime();
    } else {
      child->SetParticleEmission(false);
    }
  }
}

void CElementGen::DestroyParticles() {
  mParticleAliveCount -= x30_particles.size();
  x30_particles.clear();
  if (x2c_orientType == kMOT_One) {
    x50_parentMatrices.clear();
  }
  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    (*it)->DestroyParticles();
  }
}

bool CElementGen::IsSystemDeletable() const {
  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    if (!(*it)->IsSystemDeletable()) {
      return false;
    }
  }
  if (x268_PSLT < x74_curFrame && static_cast< int >(x25c_activeParticleCount) == 0) {
    return true;
  }
  return false;
}

void CElementGen::Render() {
  CStopwatch timer;

  x274_backupLightActive = CGraphics::GetLightMask();
  CGraphics::DisableAllLights();

  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    (*it)->Render();
  }

  CParticleGlobals::SParticleSystem sys('PART', this);

  if (x30_particles.size() > 0) {
    if (x28_loadedGenDesc->x48_PMDL || x28_loadedGenDesc->x31_26_PMUS) {
      RenderModels();
    }
    if (x26c_31_LINE) {
      RenderLines();
    } else {
      RenderParticles();
    }
  }

  x2d0 = timer.GetElapsedTime();
}

void CElementGen::RenderBasicParticlesNoRotTS(const CTransform4f& xf) {
  for (int i = 0; i < x30_particles.size(); ++i) {
    CParticle& particle = x30_particles[i];
    CVector3f viewPos = xf * particle.x4_pos;
    float halfSize = 0.5f * particle.x2c_lineLengthOrSize;
    uint color = particle.x34_color.GetColor_u32();
    float x = viewPos.GetX() + halfSize;
    float y = viewPos.GetY();
    float z = viewPos.GetZ() + halfSize;

    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    x -= particle.x2c_lineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    z -= particle.x2c_lineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    x += particle.x2c_lineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderBasicParticlesRotTS(const CTransform4f& xf) {
  for (int i = 0; i < x30_particles.size(); ++i) {
    CParticle& particle = x30_particles[i];
    CVector3f viewPos = xf * particle.x4_pos;
    float halfSize = 0.5f * particle.x2c_lineLengthOrSize;
    float theta = particle.x30_lineWidthOrRota * (M_PIF / 180.f);
    uint color = particle.x34_color.GetColor_u32();
    float sinT = CMath::FastSinR(theta) * halfSize;
    float cosT = CMath::FastCosR(theta) * halfSize;
    float sinPlusCos = sinT + cosT;
    float sinMinusCos = sinT - cosT;

    float x = viewPos.GetX();
    float y = viewPos.GetY();
    float z = viewPos.GetZ();

    GXPosition3f32(x + sinPlusCos, y, z - sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    GXPosition3f32(x + sinMinusCos, y, z + sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    GXPosition3f32(x - sinPlusCos, y, z + sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    GXPosition3f32(x - sinMinusCos, y, z - sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderBasicParticlesNoRotNoTS(const CTransform4f& xf) {
  for (int i = 0; i < x30_particles.size(); ++i) {
    CParticle& particle = x30_particles[i];
    CVector3f viewPos =
        xf * ((particle.x4_pos - particle.x10_prevPos) * x80_timeDeltaScale + particle.x10_prevPos);
    float halfSize = 0.5f * particle.x2c_lineLengthOrSize;
    uint color = particle.x34_color.GetColor_u32();
    float x = viewPos.GetX() + halfSize;
    float y = viewPos.GetY();
    float z = viewPos.GetZ() + halfSize;

    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    x -= particle.x2c_lineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    z -= particle.x2c_lineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    x += particle.x2c_lineLengthOrSize;
    GXPosition3f32(x, y, z);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderBasicParticlesRotNoTS(const CTransform4f& xf) {
  for (int i = 0; i < x30_particles.size(); ++i) {
    CParticle& particle = x30_particles[i];
    CVector3f viewPos =
        xf * ((particle.x4_pos - particle.x10_prevPos) * x80_timeDeltaScale + particle.x10_prevPos);
    float halfSize = 0.5f * particle.x2c_lineLengthOrSize;
    float theta = particle.x30_lineWidthOrRota * (M_PIF / 180.f);
    uint color = particle.x34_color.GetColor_u32();
    float sinT = CMath::FastSinR(theta) * halfSize;
    float cosT = CMath::FastCosR(theta) * halfSize;
    float sinPlusCos = sinT + cosT;
    float sinMinusCos = sinT - cosT;
    float x = viewPos.GetX();
    float y = viewPos.GetY();
    float z = viewPos.GetZ();

    GXPosition3f32(x + sinPlusCos, y, z - sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0202);

    GXPosition3f32(x + sinMinusCos, y, z + sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0002);

    GXPosition3f32(x - sinPlusCos, y, z + sinMinusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0000);

    GXPosition3f32(x - sinMinusCos, y, z - sinPlusCos);
    GXColor1u32(color);
    GXTexCoord1s16(0x0200);
  }
}

void CElementGen::RenderParticles() {
  CGlobalRandom gr(x27c_randState);

  if (IsIndirectTextured()) {
    RenderParticlesIndirectTexture();
    return;
  }

  if (x28_loadedGenDesc->x38_SIZE != nullptr && x28_loadedGenDesc->x38_SIZE->IsConstant()) {
    float sizeVal = 1.f;
    x28_loadedGenDesc->x38_SIZE->GetValue(0, sizeVal);
    if (sizeVal == 0.f) {
      sizeVal = 1.f;
      x28_loadedGenDesc->x38_SIZE->GetValue(1, sizeVal);
      if (sizeVal == 0.f) {
        return;
      }
    }
  }

  CGraphics::SetCullMode(kCM_None);

  bool hasModuColor = x338_moduColor.GetColor_u32() != 0xFFFFFFFF;

  CTransform4f systemViewCopy(CGraphics::GetViewMatrix());
  CTransform4f systemModelMatrix(systemViewCopy);
  systemModelMatrix.SetTranslation(CVector3f::Zero());
  CTransform4f systemCameraCopy(systemModelMatrix.GetQuickInverse() * x22c_globalOrientation);
  systemModelMatrix = CTransform4f::Translate(xe8_globalTranslation) * x10c_globalScaleTransform *
                      systemModelMatrix * x178_localScaleTransform;

  if (x26c_29_ORNT) {
    CGraphics::SetModelMatrix(systemModelMatrix * systemCameraCopy);
  } else {
    CGraphics::SetModelMatrix(systemModelMatrix);
  }

  CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);

  bool constUVs = true;
  SUVElementSet uvs;
  uvs.xMin = 0.f;
  uvs.xMax = 1.f;
  uvs.yMin = 0.f;
  uvs.yMax = 1.f;

  bool noRota = x28_loadedGenDesc->x3c_ROTA == nullptr;
  if (x28_loadedGenDesc->x3c_ROTA != nullptr && x28_loadedGenDesc->x3c_ROTA->IsConstant()) {
    float value = 1.f;
    x28_loadedGenDesc->x3c_ROTA->GetValue(0, value);
    if (0.f == value) {
      value = 1.f;
      x28_loadedGenDesc->x3c_ROTA->GetValue(1, value);
      if (0.f == value) {
        noRota = true;
      }
    }
  }

  if (x28_loadedGenDesc->x40_TEXR != nullptr) {
    int partFrame = x74_curFrame - x30_particles[0].x28_startFrame;
    TToken< CTexture > tex = x28_loadedGenDesc->x40_TEXR->GetValueTexture(partFrame);
    tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);

    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);

    if (hasModuColor) {
      static CTevCombiners::CTevPass ModulatePreviousTEVWithRasColor(
          CTevCombiners::ColorPass(CTevCombiners::kCS_Zero, CTevCombiners::kCS_PreviousColor,
                                   CTevCombiners::kCS_RasterColor, CTevCombiners::kCS_Zero),
          CTevCombiners::AlphaPass(CTevCombiners::kAS_Zero, CTevCombiners::kAS_PreviousAlpha,
                                   CTevCombiners::kAS_RasterAlpha, CTevCombiners::kAS_Zero));
      CGraphics::SetTevOp(kTS_Stage1, ModulatePreviousTEVWithRasColor);
    } else {
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    }

    x28_loadedGenDesc->x40_TEXR->GetValueUV(x74_curFrame - x30_particles[0].x28_startFrame, uvs);
    constUVs = x28_loadedGenDesc->x40_TEXR->HasConstantUV();
  } else {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  }

  CGX::SetVtxDescv(sVtxDescList);

  GXTevStageID nextStage;
  if (hasModuColor) {
    CGX::SetNumChans(2);
    nextStage = GX_TEVSTAGE2;
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);
    CGX::SetChanAmbColor(CGX::Channel1, CColor(0.f, 0.f, 0.f, 1.f).GetGXColor());
    CGX::SetChanMatColor(CGX::Channel1, x338_moduColor.GetGXColor());
    CGX::SetChanCtrl(CGX::Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                     GX_AF_NONE);
  } else {
    CGX::SetNumChans(1);
    nextStage = GX_TEVSTAGE1;
  }

  bool moveRedToAlphaBuffer = sMoveRedToAlphaBuffer;
  if (sSubtractBlend) {
    CGraphics::SetDepthWriteMode(x26c_28_zTest, kE_LEqual, false);
    CGX::SetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
    if (moveRedToAlphaBuffer) {
      CGX::SetTevColorIn(nextStage, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
      CGX::SetTevAlphaIn(nextStage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
      CGX::SetStandardTevColorAlphaOp(nextStage);
      CGX::SetTevOrder(nextStage, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
      GXSetTevSwapMode(nextStage, GX_TEV_SWAP0, GX_TEV_SWAP1);
      nextStage = static_cast< GXTevStageID >(nextStage + 1);
    }
  } else if (moveRedToAlphaBuffer) {
    CGraphics::SetDepthWriteMode(x26c_28_zTest, kE_LEqual, false);
    CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
    CGX::SetTevColorIn(nextStage, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
    CGX::SetTevAlphaIn(nextStage, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
    CGX::SetStandardTevColorAlphaOp(nextStage);
    CGX::SetTevOrder(nextStage, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevSwapMode(nextStage, GX_TEV_SWAP0, GX_TEV_SWAP1);
    nextStage = static_cast< GXTevStageID >(nextStage + 1);
  } else if (x26c_26_AAPH) {
    CGraphics::SetDepthWriteMode(x26c_28_zTest, kE_LEqual, false);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
  } else {
    CGraphics::SetDepthWriteMode(x26c_28_zTest, kE_LEqual, x26c_27_ZBUF);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  }

  CGX::SetNumTevStages(static_cast< uchar >(nextStage));
  CGX::SetNumTexGens(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);

  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  if (constUVs) {
    GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_S8, 1);
  } else {
    GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  }

  int particleCount = x30_particles.size();
  int mbspVal = rstl::max_val(1, x270_MBSP);

  if (!x26c_30_MBLR) {
    CGX::Begin(GX_QUADS, GX_VTXFMT6, static_cast< ushort >(particleCount * 4));
  } else {
    CGX::Begin(GX_QUADS, GX_VTXFMT6, static_cast< ushort >(mbspVal * (particleCount * 4)));
  }

  bool noTS = !close_enough(x80_timeDeltaScale, 1.f, 0.00001f);
  bool SORT = x28_loadedGenDesc->x30_28_SORT;
  CParticleListItem* sortItems = nullptr;

  if (SORT) {
    sortItems =
        static_cast< CParticleListItem* >(alloca(particleCount * sizeof(CParticleListItem)));
    for (int i = 0; i < particleCount; ++i) {
      CParticle& particle = x30_particles[i];
      const CVector3f& delta = particle.x4_pos - particle.x10_prevPos;
      const CVector3f& pos = delta * x80_timeDeltaScale + particle.x10_prevPos;
      // TODO: I think this is the constructor, but need the copy to be elided
      sortItems[i].x4_viewPoint = systemCameraCopy * pos;
      sortItems[i].x0_partIdx = static_cast< ushort >(i);
    }
    rstl::sort(sortItems, sortItems + particleCount, CParticleListItemViewPointComp());
  }

  CParticleGlobals::SetEmitterTime(x74_curFrame);

  if (!x26c_30_MBLR) {
    if (!SORT && constUVs && !x26c_29_ORNT) {
      if (noRota) {
        if (noTS) {
          RenderBasicParticlesNoRotNoTS(systemCameraCopy);
        } else {
          RenderBasicParticlesNoRotTS(systemCameraCopy);
        }
      } else {
        if (noTS) {
          RenderBasicParticlesRotNoTS(systemCameraCopy);
        } else {
          RenderBasicParticlesRotTS(systemCameraCopy);
        }
      }
    } else if (!x26c_29_ORNT) {
      CParticleListItem* sortIt = sortItems;
      for (int i = 0; i < particleCount; ++i) {
        CParticle& particle = SORT ? x30_particles[sortIt->x0_partIdx] : x30_particles[i];
        CVector3f viewPoint =
            systemCameraCopy *
            ((particle.x4_pos - particle.x10_prevPos) * x80_timeDeltaScale + particle.x10_prevPos);

        uint color = particle.x34_color.GetColor_u32();

        if (!constUVs) {
          int partFrame = x74_curFrame - particle.x28_startFrame - 1;
          CParticleGlobals::SetParticleLifetime(particle.x0_endFrame - particle.x28_startFrame);
          CParticleGlobals::UpdateParticleLifetimeTweenValues(partFrame);
          x28_loadedGenDesc->x40_TEXR->GetValueUV(partFrame, uvs);

          if (noRota) {
            float x = (particle.x2c_lineLengthOrSize * 0.5f) + viewPoint.GetX();
            float y = viewPoint.GetY();
            float z = (particle.x2c_lineLengthOrSize * 0.5f) + viewPoint.GetZ();

            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMax);

            x -= particle.x2c_lineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMax);

            z -= particle.x2c_lineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMin);

            x += particle.x2c_lineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMin);
          } else {
            float halfSize = 0.5f * particle.x2c_lineLengthOrSize;
            float theta = particle.x30_lineWidthOrRota * (M_PIF / 180.f);
            float sinT = CMath::FastSinR(theta) * halfSize;
            float cosT = CMath::FastCosR(theta) * halfSize;

            float sinPlusCos = sinT + cosT;
            float cosMinusSin = cosT - sinT;
            float sinMinusCos = sinT - cosT;
            float negSinPlusCos = -sinT + cosT;

            float vpX = viewPoint.GetX();
            float vpY = viewPoint.GetY();
            float vpZ = viewPoint.GetZ();

            GXPosition3f32(sinPlusCos + vpX, vpY, cosMinusSin + vpZ);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMax);

            GXPosition3f32(sinMinusCos + vpX, vpY, sinPlusCos + vpZ);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMax);

            GXPosition3f32(vpX - sinPlusCos, vpY, vpZ - cosMinusSin);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMin);

            GXPosition3f32(negSinPlusCos + vpX, vpY, (-cosT - sinT) + vpZ);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMin);
          }
        } else if (noRota) {
          float x = (particle.x2c_lineLengthOrSize * 0.5f) + viewPoint.GetX();
          float y = viewPoint.GetY();
          float z = (particle.x2c_lineLengthOrSize * 0.5f) + viewPoint.GetZ();

          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0202);

          x -= particle.x2c_lineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0002);

          z -= particle.x2c_lineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0000);

          x += particle.x2c_lineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0200);
        } else {
          float halfSize = 0.5f * particle.x2c_lineLengthOrSize;
          float theta = particle.x30_lineWidthOrRota * (M_PIF / 180.f);
          float sinT = CMath::FastSinR(theta) * halfSize;
          float cosT = CMath::FastCosR(theta) * halfSize;

          float sinPlusCos = sinT + cosT;
          float sinMinusCos = sinT - cosT;

          float vpX = viewPoint.GetX();
          float vpY = viewPoint.GetY();
          float vpZ = viewPoint.GetZ();

          GXPosition3f32(vpX + sinPlusCos, vpY, vpZ - sinMinusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0202);

          GXPosition3f32(vpX + sinMinusCos, vpY, vpZ + sinPlusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0002);

          GXPosition3f32(vpX - sinPlusCos, vpY, vpZ + sinMinusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0000);

          GXPosition3f32(vpX - sinMinusCos, vpY, vpZ - sinPlusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0200);
        }

#if NONMATCHING
        if (SORT) {
          ++sortIt;
        }
#else
        sortIt++;
#endif
      }
    } else {
      // ORNT
      CVector3f fwd = systemViewCopy.GetForward();
      CVector3f origin = systemViewCopy.GetTranslation();
      float canBeNormEps = FLT_EPSILON;

      CParticleListItem* sortIt = sortItems;
      for (int i = 0; i < particleCount; ++i) {
        CParticle* particle = SORT ? &x30_particles[sortIt->x0_partIdx] : &x30_particles[i];

        CVector3f delta = particle->x4_pos - particle->x10_prevPos;
        const CVector3f& pos = delta * x80_timeDeltaScale + particle->x10_prevPos;

        uint color = particle->x34_color.GetColor_u32();
        float rota = x28_loadedGenDesc->x3c_ROTA == nullptr ? 1.f : particle->x30_lineWidthOrRota;
        float velMagSq = particle->x1c_vel.MagSquared();

        CVector3f dir;
        if (velMagSq > canBeNormEps) {
          dir = particle->x1c_vel * frsqrte(velMagSq);
        } else {
          const CVector3f& dv = particle->x4_pos - particle->x10_prevPos;
          float dMagSq = dv.MagSquared();
          if (dMagSq > canBeNormEps) {
            dir = dv * frsqrte(dMagSq);
          } else {
            dir = CVector3f(0.f, 0.f, 1.f);
          }
        }

        CVector3f fore = dir * particle->x2c_lineLengthOrSize;

        CVector3f right;
        if (x28_loadedGenDesc->x30_31_RSOP) {
          right = CVector3f::Cross(dir, fwd);
          float crMagSq = right.MagSquared();

          if (crMagSq > canBeNormEps) {
            float invMag = frsqrte(crMagSq);
            right *= particle->x2c_lineLengthOrSize * rota * invMag;
          } else {
            CVector3f camDelta = origin - particle->x4_pos;
            right = CVector3f::Cross(dir, camDelta.AsNormalized());
            float cr2MagSq = right.MagSquared();

            if (cr2MagSq > canBeNormEps) {
              float invMag = frsqrte(cr2MagSq);
              right *= particle->x2c_lineLengthOrSize * rota * invMag;
            }
          }
        } else {
          right = CVector3f::Cross(fore, fwd) * rota;
        }

        if (!constUVs) {
          int partFrame = x74_curFrame - particle->x28_startFrame - 1;
          CParticleGlobals::SetParticleLifetime(particle->x0_endFrame - particle->x28_startFrame);
          CParticleGlobals::UpdateParticleLifetimeTweenValues(partFrame);
          x28_loadedGenDesc->x40_TEXR->GetValueUV(partFrame, uvs);

          CVector3f p = pos + right * 0.5f + fore * 0.5f;
          GXPositionVector3f(p);
          GXColor1u32(color);
          GXTexCoord2f32(uvs.xMax, uvs.yMax);

          p -= fore;
          GXPositionVector3f(p);
          GXColor1u32(color);
          GXTexCoord2f32(uvs.xMin, uvs.yMax);

          p -= right;
          GXPositionVector3f(p);
          GXColor1u32(color);
          GXTexCoord2f32(uvs.xMin, uvs.yMin);

          p += fore;
          GXPositionVector3f(p);
          GXColor1u32(color);
          GXTexCoord2f32(uvs.xMax, uvs.yMin);
        } else {
          CVector3f p = pos + right * 0.5f + fore * 0.5f;
          GXPositionVector3f(p);
          GXColor1u32(color);
          GXTexCoord1s16(0x0202);

          p -= fore;
          GXPositionVector3f(p);
          GXColor1u32(color);
          GXTexCoord1s16(0x0002);

          p -= right;
          GXPositionVector3f(p);
          GXColor1u32(color);
          GXTexCoord1s16(0x0000);

          p += fore;
          GXPositionVector3f(p);
          GXColor1u32(color);
          GXTexCoord1s16(0x0200);
        }

#if NONMATCHING
        if (SORT) {
          ++sortIt;
        }
#else
        sortIt++;
#endif
      }
    }
  } else {
    // MBLR
    float mbspFac = 1.f / static_cast< float >(mbspVal);
    for (int i = 0; i < particleCount; ++i) {
      CParticle* particle = &x30_particles[i];

      CVector3f delta = particle->x4_pos - particle->x10_prevPos;
      CVector3f vec = delta * x80_timeDeltaScale + particle->x10_prevPos;
      CVector3f mbspVec = delta * mbspFac;

      CColor col = particle->x34_color;
      col.SetAlpha(col.GetAlpha() * mbspFac);
      uint color = col.GetColor_u32();

      if (!constUVs) {
        int partFrame = x74_curFrame - particle->x28_startFrame - 1;
        CParticleGlobals::SetParticleLifetime(particle->x0_endFrame - particle->x28_startFrame);
        CParticleGlobals::UpdateParticleLifetimeTweenValues(partFrame);
        x28_loadedGenDesc->x40_TEXR->GetValueUV(partFrame, uvs);

        if (noRota) {
          for (int j = 0; j < mbspVal; ++j) {
            vec += mbspVec;
            CVector3f viewPoint = systemCameraCopy * vec;
            float x = (0.5f * particle->x2c_lineLengthOrSize) + viewPoint.GetX();
            float y = viewPoint.GetY();
            float z = (0.5f * particle->x2c_lineLengthOrSize) + viewPoint.GetZ();

            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMax);

            x -= particle->x2c_lineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMax);

            z -= particle->x2c_lineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMin);

            x += particle->x2c_lineLengthOrSize;
            GXPosition3f32(x, y, z);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMin);
          }
        } else {
          for (int j = 0; j < mbspVal; ++j) {
            vec += mbspVec;
            CVector3f viewPoint = systemCameraCopy * vec;
            float halfSize = 0.5f * particle->x2c_lineLengthOrSize;
            float theta = particle->x30_lineWidthOrRota * (M_PIF / 180.f);

            float sinT = CMath::FastSinR(theta) * halfSize;
            float cosT = CMath::FastCosR(theta) * halfSize;

            float sinPlusCos = sinT + cosT;
            float negSinPlusCos = -sinT + cosT;
            float sinMinusCos = sinT - cosT;
            float cosMinusSin = cosT - sinT;

            float vpX = viewPoint.GetX();
            float vpY = viewPoint.GetY();
            float vpZ = viewPoint.GetZ();

            GXPosition3f32(sinPlusCos + vpX, vpY, cosMinusSin + vpZ);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMax);

            GXPosition3f32(sinMinusCos + vpX, vpY, sinPlusCos + vpZ);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMax);

            GXPosition3f32(vpX - sinPlusCos, vpY, vpZ - cosMinusSin);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMin, uvs.yMin);

            GXPosition3f32(negSinPlusCos + vpX, vpY, (-cosT - sinT) + vpZ);
            GXColor1u32(color);
            GXTexCoord2f32(uvs.xMax, uvs.yMin);
          }
        }
      } else if (noRota) {
        for (int j = 0; j < mbspVal; ++j) {
          vec += mbspVec;
          CVector3f viewPoint = systemCameraCopy * vec;
          float x = (0.5f * particle->x2c_lineLengthOrSize) + viewPoint.GetX();
          float y = viewPoint.GetY();
          float z = (0.5f * particle->x2c_lineLengthOrSize) + viewPoint.GetZ();

          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0202);

          x -= particle->x2c_lineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0002);

          z -= particle->x2c_lineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0000);

          x += particle->x2c_lineLengthOrSize;
          GXPosition3f32(x, y, z);
          GXColor1u32(color);
          GXTexCoord1s16(0x0200);
        }
      } else {
        for (int j = 0; j < mbspVal; ++j) {
          vec += mbspVec;
          CVector3f viewPoint = systemCameraCopy * vec;
          float halfSize = 0.5f * particle->x2c_lineLengthOrSize;
          float theta = particle->x30_lineWidthOrRota * (M_PIF / 180.f);

          float sinT = CMath::FastSinR(theta) * halfSize;
          float cosT = CMath::FastCosR(theta) * halfSize;
          float sinPlusCos = sinT + cosT;
          float sinMinusCos = sinT - cosT;

          float vpX = viewPoint.GetX();
          float vpY = viewPoint.GetY();
          float vpZ = viewPoint.GetZ();

          GXPosition3f32(vpX + sinPlusCos, vpY, vpZ - sinMinusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0202);

          GXPosition3f32(vpX + sinMinusCos, vpY, vpZ + sinPlusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0002);

          GXPosition3f32(vpX - sinPlusCos, vpY, vpZ + sinMinusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0000);

          GXPosition3f32(vpX - sinMinusCos, vpY, vpZ - sinPlusCos);
          GXColor1u32(color);
          GXTexCoord1s16(0x0200);
        }
      }
    }
  }

  CGX::End();
  if (moveRedToAlphaBuffer) {
    GXSetTevSwapMode(static_cast< GXTevStageID >(nextStage - 1), GX_TEV_SWAP0, GX_TEV_SWAP0);
  }
  CGraphics::SetCullMode(kCM_Front);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
}

void CElementGen::RenderParticlesFlameThrower(CElementGen* const* gens, int count) {
  CTransform4f viewCopy(CGraphics::GetViewMatrix());
  CTransform4f systemModelMatrix(viewCopy);
  systemModelMatrix.SetTranslation(CVector3f::Zero());
  CTransform4f systemCameraCopy(systemModelMatrix.GetQuickInverse());

  CGraphics::SetModelMatrix(systemModelMatrix);
  CGraphics::SetCullMode(kCM_None);
  CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
  CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);

  const bool moveRedToAlpha = sMoveRedToAlphaBuffer;
  SUVElementSet uvs;

  CElementGen* const* genPtr = gens;
  for (GXTexMapID mapId = GX_TEXMAP0; mapId < count;
       ++genPtr, mapId = static_cast< GXTexMapID >(mapId + 1)) {
    const TLockedToken< CTexture >& tex =
        (*genPtr)->x28_loadedGenDesc->x40_TEXR->GetValueTexture(0);
    (*tex)->Load(mapId, CTexture::kCM_Repeat);
  }

  CGX::SetVtxDescv(sVtxDescList);

  int nStages = 1;
  CGX::SetNumChans(1);

  GXTevStageID stage = GX_TEVSTAGE0;
  if (moveRedToAlpha) {
    CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
    stage = GX_TEVSTAGE1;
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
    CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
    CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP1);
    nStages = 2;
  }
  CGX::SetNumTevStages(nStages);
  CGX::SetNumTexGens(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);

  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GXSetVtxAttrFmt(GX_VTXFMT6, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

  int totalParticles = 0;
  for (int i = 0; i < count; ++i) {
    totalParticles += gens[i]->GetParticleCount();
  }

  CTexturedParticleListItem* sortItems = static_cast< CTexturedParticleListItem* >(
      alloca(totalParticles * sizeof(CTexturedParticleListItem)));

  int activeCount = 0;
  for (int i = 0; i < count; ++i) {
    CElementGen* gen = gens[i];
    int numParts = gen->GetParticleCount();
    float timeDelta = gen->x80_timeDeltaScale;

    for (int j = 0; j < numParts; ++j) {
      CParticle& particle = gen->x30_particles[j];
      if (particle.x0_endFrame == -1)
        continue;

      // ?
      sortItems[activeCount].x4_viewPoint = systemCameraCopy * particle.x4_pos;

      // TODO: this should be the constructor
      sortItems[activeCount].x4_viewPoint =
          systemCameraCopy *
          ((particle.x4_pos - particle.x10_prevPos) * timeDelta + particle.x10_prevPos);
      sortItems[activeCount].x0_texMapIdx = i;
      sortItems[activeCount].x2_partIdx = j;
      activeCount++;
    }
  }

  rstl::sort(sortItems, sortItems + activeCount, CTexturedParticleListItemViewPointComp());

  CGenDescription* genDesc = nullptr;
  ushort lastMap = 0xFFFF;
  CElementGen* gen = nullptr;
  int emitterTime = 0;

  for (int i = 0; i < activeCount; ++i) {
    CTexturedParticleListItem* readPtr = &sortItems[i];
    ushort map = readPtr->x0_texMapIdx;

    if (lastMap != map) {
      gen = gens[map];
      emitterTime = gen->GetEmitterTime();
      CGenDescription* desc = gens[map]->x28_loadedGenDesc;
      genDesc = desc;

      if (!moveRedToAlpha) {
        if (desc->x30_26_AAPH) {
          CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
        } else {
          CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
        }
      } else {
        CGX::SetTevOrder(stage, GX_TEXCOORD0, static_cast< GXTexMapID >(map), GX_COLOR_NULL);
      }

      CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, static_cast< GXTexMapID >(map), GX_COLOR0A0);
      lastMap = map;
    }

    CParticle& particle = gen->x30_particles[readPtr->x2_partIdx];

    int elapsed = emitterTime - particle.x28_startFrame - 1;
    CParticleGlobals::SetParticleLifetime(particle.x0_endFrame - particle.x28_startFrame);
    CParticleGlobals::UpdateParticleLifetimeTweenValues(elapsed);

    genDesc->x40_TEXR->GetValueUV(elapsed, uvs);

    CGX::Begin(GX_QUADS, GX_VTXFMT6, 4);

    float halfSize = 0.5f * particle.x2c_lineLengthOrSize;
    float theta = particle.x30_lineWidthOrRota * (M_PIF / 180.f);
    uint color = particle.x34_color.GetColor_u32();
    float sinT = halfSize * CMath::FastSinR(theta);
    float cosT = halfSize * CMath::FastCosR(theta);

    float sinPlusCos = sinT + cosT;
    float cosMinusSin = cosT - sinT;
    float sinMinusCos = sinT - cosT;
    float negSinPlusCos = -sinT + cosT;
    float negCos = -cosT;

    GXPosition3f32(sinPlusCos + readPtr->x4_viewPoint.GetX(), readPtr->x4_viewPoint.GetY(),
                   cosMinusSin + readPtr->x4_viewPoint.GetZ());
    GXColor1u32(color);
    GXTexCoord2f32(uvs.xMax, uvs.yMax);

    GXPosition3f32(sinMinusCos + readPtr->x4_viewPoint.GetX(), readPtr->x4_viewPoint.GetY(),
                   sinPlusCos + readPtr->x4_viewPoint.GetZ());
    GXColor1u32(color);
    GXTexCoord2f32(uvs.xMin, uvs.yMax);

    GXPosition3f32(readPtr->x4_viewPoint.GetX() - sinPlusCos, readPtr->x4_viewPoint.GetY(),
                   readPtr->x4_viewPoint.GetZ() - cosMinusSin);
    GXColor1u32(color);
    GXTexCoord2f32(uvs.xMin, uvs.yMin);

    GXPosition3f32(negSinPlusCos + readPtr->x4_viewPoint.GetX(), readPtr->x4_viewPoint.GetY(),
                   (negCos - sinT) + readPtr->x4_viewPoint.GetZ());
    GXColor1u32(color);
    GXTexCoord2f32(uvs.xMax, uvs.yMin);

    CGX::End();
  }

  if (moveRedToAlpha) {
    GXSetTevSwapMode(stage, GX_TEV_SWAP0, GX_TEV_SWAP0);
  }

  CGraphics::SetCullMode(kCM_Front);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
}

void CElementGen::RenderParticlesIndirectTexture() {
  CTransform4f systemViewCopy(CGraphics::GetViewMatrix());
  systemViewCopy.SetTranslation(CVector3f::Zero());
  CTransform4f systemCameraCopy(systemViewCopy.GetQuickInverse() * x22c_globalOrientation);
  systemViewCopy = CTransform4f::Translate(xe8_globalTranslation) * x10c_globalScaleTransform *
                   systemViewCopy * x178_localScaleTransform;
  CGraphics::SetModelMatrix(systemViewCopy);

  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);

  if (x26c_26_AAPH) {
    CGX::SetZMode(true, GX_LEQUAL, false);
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_CLEAR);
  } else {
    CGX::SetZMode(true, GX_LEQUAL, x26c_27_ZBUF);
    CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  }

  SUVElementSet uvs;
  uvs.xMin = 0.f;
  uvs.xMax = 1.f;
  uvs.yMin = 0.f;
  uvs.yMax = 1.f;
  SUVElementSet uvsInd;
  uvsInd.xMin = 0.f;
  uvsInd.xMax = 1.f;
  uvsInd.yMin = 0.f;
  uvsInd.yMax = 1.f;

  TToken< CTexture > texToken =
      x28_loadedGenDesc->x40_TEXR->GetValueTexture(x74_curFrame - x30_particles[0].x28_startFrame);
  texToken->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
  CTexture* cachedTex = *texToken;

  bool constTexr = x28_loadedGenDesc->x40_TEXR->HasConstantTexture();
  x28_loadedGenDesc->x40_TEXR->GetValueUV(x74_curFrame - x30_particles[0].x28_startFrame, uvs);
  bool constUVs = x28_loadedGenDesc->x40_TEXR->HasConstantUV();

  TToken< CTexture > indTexToken =
      x28_loadedGenDesc->x44_TIND->GetValueTexture(x74_curFrame - x30_particles[0].x28_startFrame);
  indTexToken->Load(GX_TEXMAP2, CTexture::kCM_Repeat);
  CTexture* cachedIndTex = *indTexToken;

  bool constIndTexr = x28_loadedGenDesc->x44_TIND->HasConstantTexture();
  bool constIndUVs = x28_loadedGenDesc->x44_TIND->HasConstantUV();
  x28_loadedGenDesc->x44_TIND->GetValueUV(x74_curFrame - x30_particles[0].x28_startFrame, uvsInd);

  CGX::SetNumTexGens(3);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX1, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_TEX2, GX_IDENTITY, false, GX_PTIDENTITY);

  float indMtx[2][3] = {
      {0.5f, 0.f, 0.f},
      {0.f, 0.5f, 0.f},
  };
  GXSetIndTexMtx(GX_ITM_0, indMtx, 1);
  GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD2, GX_TEXMAP2);

  CGX::SetTevIndirect(GX_TEVSTAGE1, GX_INDTEXSTAGE0, GX_ITF_8, GX_ITB_STU, GX_ITM_0, GX_ITW_OFF,
                      GX_ITW_OFF, false, false, GX_ITBA_OFF);
  CGX::SetNumIndStages(1);

  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, CGraphics::kSpareBufferTexMapID, GX_COLOR0A0);
  CGX::SetNumTevStages(2);

  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_8_8);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_8_8);

  if (!x28_loadedGenDesc->x32_24_CIND) {
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_ZERO);
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_CPREV);
  } else {
    CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_RASC, GX_CC_ZERO);
    CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_CPREV, GX_CC_ZERO);
  }

  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE0);
  CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_APREV, GX_CA_RASA, GX_CA_ZERO);

  CGX::SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
  CGX::SetNumChans(1);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);

  CParticleListItem* sortItems = nullptr;
  int particleCount = x30_particles.size();
  bool SORT = x28_loadedGenDesc->x30_28_SORT;

  if (SORT) {
    sortItems =
        static_cast< CParticleListItem* >(alloca(particleCount * sizeof(CParticleListItem)));
    CParticleListItem* cur = sortItems;
    for (int i = 0; i < particleCount; ++i) {
      CParticle& particle = x30_particles[i];
      const CVector3f& delta = particle.x4_pos - particle.x10_prevPos;
      const CVector3f& pos = delta * x80_timeDeltaScale + particle.x10_prevPos;
      cur->x4_viewPoint = systemCameraCopy * pos;
      cur->x0_partIdx = static_cast< ushort >(i);
      cur++;
    }

    static CParticleListItemViewPointComp sComp;
    rstl::sort(sortItems, sortItems + particleCount, sComp);
  }

  static const GXVtxDescList skIndVtxDescList[] = {
      {GX_VA_POS, GX_DIRECT},  {GX_VA_CLR0, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_TEX1, GX_DIRECT}, {GX_VA_TEX2, GX_DIRECT}, {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(skIndVtxDescList);

  CParticleListItem* sortIt = sortItems;
  for (int i = 0; i < particleCount; ++i) {
    CParticle* particle = SORT ? &x30_particles[sortIt->x0_partIdx] : &x30_particles[i];
    const CVector3f& viewPoint =
        systemCameraCopy *
        ((particle->x4_pos - particle->x10_prevPos) * x80_timeDeltaScale + particle->x10_prevPos);

    float vpX = viewPoint.GetX();
    float vpY = viewPoint.GetY();
    float vpZ = viewPoint.GetZ();

    if (!constTexr) {
      TToken< CTexture > tex =
          x28_loadedGenDesc->x40_TEXR->GetValueTexture(x74_curFrame - particle->x28_startFrame);
      if (*tex != cachedTex) {
        tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
        cachedTex = *tex;
      }
    }

    if (!constIndTexr) {
      TToken< CTexture > tex =
          x28_loadedGenDesc->x44_TIND->GetValueTexture(x74_curFrame - particle->x28_startFrame);
      if (*tex != cachedIndTex) {
        tex->Load(GX_TEXMAP2, CTexture::kCM_Repeat);
        cachedIndTex = *tex;
      }
    }

    if (!constUVs) {
      x28_loadedGenDesc->x40_TEXR->GetValueUV(x74_curFrame - x30_particles[i].x28_startFrame, uvs);
    }

    if (!constIndUVs) {
      x28_loadedGenDesc->x44_TIND->GetValueUV(x74_curFrame - x30_particles[i].x28_startFrame,
                                              uvsInd);
    }

    float size = particle->x2c_lineLengthOrSize * 0.5f;

    CGraphics::CClippedScreenRect clipRect = CGraphics::ClipScreenRectFromMS(
        CVector3f(vpX - size, vpY, vpZ - size), CVector3f(size + vpX, vpY, size + vpZ), kTF_RGB565);

    int width = clipRect.GetTexWidth();
    int height = clipRect.GetHeight();
    float minU = clipRect.GetMinU();
    float maxU = clipRect.GetMaxU();
    float minV = clipRect.GetMinV();
    float maxV = clipRect.GetMaxV();

    if (clipRect.IsValid()) {
      void* dest = CGraphics::GetDolphinSpareBuffer();
      GXSetTexCopySrc(static_cast< u16 >(clipRect.GetX()), static_cast< u16 >(clipRect.GetY()),
                      static_cast< u16 >(clipRect.GetWidth()), static_cast< u16 >(height));
      GXSetTexCopyDst(static_cast< u16 >(width), static_cast< u16 >(height), GX_TF_RGB565,
                      GX_FALSE);

      size_t bufSize = CGraphics::GetSpareBufferSize();
      size_t texBufSize = GXGetTexBufferSize(width, height, GX_TF_RGB565, GX_FALSE, 0);
      if (texBufSize <= bufSize) {
        const bool useVideoFilter = CGraphics::GetUseVideoFilter();
        CGraphics::SetUseVideoFilter(false);
        GXCopyTex(dest, GX_FALSE);
        CGraphics::SetUseVideoFilter(useVideoFilter);
        GXPixModeSync();

        CGraphics::LoadDolphinSpareTexture(width, height, GX_TF_RGB565, NULL,
                                           CGraphics::kSpareBufferTexMapID);

        uint color = particle->x34_color.GetColor_u32();
        CGX::Begin(GX_QUADS, GX_VTXFMT0, 4);

        GXPosition3f32(size + vpX, vpY, size + vpZ);
        GXColor1u32(color);
        GXTexCoord2f32(uvs.xMax, uvs.yMax);
        GXTexCoord2f32(maxU, minV);
        GXTexCoord2f32(uvsInd.xMax, uvsInd.yMax);

        GXPosition3f32(vpX - size, vpY, size + vpZ);
        GXColor1u32(color);
        GXTexCoord2f32(uvs.xMin, uvs.yMax);
        GXTexCoord2f32(minU, minV);
        GXTexCoord2f32(uvsInd.xMin, uvsInd.yMax);

        GXPosition3f32(vpX - size, vpY, vpZ - size);
        GXColor1u32(color);
        GXTexCoord2f32(uvs.xMin, uvs.yMin);
        GXTexCoord2f32(minU, maxV);
        GXTexCoord2f32(uvsInd.xMin, uvsInd.yMin);

        GXPosition3f32(size + vpX, vpY, vpZ - size);
        GXColor1u32(color);
        GXTexCoord2f32(uvs.xMax, uvs.yMin);
        GXTexCoord2f32(maxU, maxV);
        GXTexCoord2f32(uvsInd.xMax, uvsInd.yMin);

        CGX::End();
      }
    }

#if NONMATCHING
    if (SORT) {
      ++sortIt;
    }
#else
    sortIt++;
#endif
  }

  CGX::SetNumIndStages(0);
  CGX::SetTevDirect(GX_TEVSTAGE1);
}

void CElementGen::RenderLines() {
  bool hasModuColor = x338_moduColor.GetColor_u32() != 0xFFFFFFFF;
  CGlobalRandom gr(x27c_randState);

  CTransform4f systemViewPointMatrix(CGraphics::GetViewMatrix());
  systemViewPointMatrix.SetTranslation(CVector3f::Zero());
  CTransform4f systemCameraMatrix(systemViewPointMatrix.GetQuickInverse() * x22c_globalOrientation);
  systemViewPointMatrix = CTransform4f::Translate(xe8_globalTranslation) *
                          x10c_globalScaleTransform * systemViewPointMatrix *
                          x178_localScaleTransform;
  CGraphics::SetModelMatrix(systemViewPointMatrix);

  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);

  if (x26c_26_AAPH) {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
  } else {
    CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
    CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
  }

  bool constUVs = true;
  SUVElementSet uvs;
  uvs.xMin = 0.f;
  uvs.xMax = 1.f;
  uvs.yMin = 0.f;
  uvs.yMax = 1.f;

  const bool widtConst =
      x28_loadedGenDesc->x18_WIDT != nullptr && x28_loadedGenDesc->x18_WIDT->IsConstant();

  if (x28_loadedGenDesc->x40_TEXR != nullptr) {
    int partFrame = x74_curFrame - x30_particles[0].x28_startFrame;
    TToken< CTexture > tex = x28_loadedGenDesc->x40_TEXR->GetValueTexture(partFrame);
    tex->Load(GX_TEXMAP0, CTexture::kCM_Repeat);

    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);

    if (hasModuColor) {
      static CTevCombiners::CTevPass ModulatePreviousTEVWithRasColor(
          CTevCombiners::ColorPass(CTevCombiners::kCS_Zero, CTevCombiners::kCS_PreviousColor,
                                   CTevCombiners::kCS_RasterColor, CTevCombiners::kCS_Zero),
          CTevCombiners::AlphaPass(CTevCombiners::kAS_Zero, CTevCombiners::kAS_PreviousAlpha,
                                   CTevCombiners::kAS_RasterAlpha, CTevCombiners::kAS_Zero));
      CGraphics::SetTevOp(kTS_Stage1, ModulatePreviousTEVWithRasColor);
    } else {
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    }

    x28_loadedGenDesc->x40_TEXR->GetValueUV(x74_curFrame - x30_particles[0].x28_startFrame, uvs);
    constUVs = x28_loadedGenDesc->x40_TEXR->HasConstantUV();
  } else {
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
  }

  static const GXVtxDescList skDescList[] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_CLR0, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetVtxDescv(skDescList);

  if (hasModuColor) {
    CGX::SetNumChans(2);
    CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR1A1);
    CGX::SetChanMatColor(CGX::Channel1, x338_moduColor.GetGXColor());
    CGX::SetChanCtrl(CGX::Channel1, false, GX_SRC_REG, GX_SRC_REG, GX_LIGHT_NULL, GX_DF_NONE,
                     GX_AF_NONE);
  } else {
    CGX::SetNumChans(1);
  }

  CGX::SetNumTexGens(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
  CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                   GX_AF_NONE);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);

  if (widtConst) {
    float lineWidth = 1.f;
    x28_loadedGenDesc->x18_WIDT->GetValue(0, lineWidth);
    if (lineWidth < 0.f) {
      lineWidth = 0.f;
    } else if (lineWidth > 42.5f) {
      lineWidth = 42.5f;
    }
    CGX::SetLineWidth(CCast::ToUint8(6.f * lineWidth), GX_TO_ZERO);
    CGX::Begin(GX_LINES, GX_VTXFMT0, static_cast< ushort >(x30_particles.size() * 2));
  }

  for (int i = 0; i < x30_particles.size(); ++i) {
    CParticle& particle = x30_particles[i];

    if (!constUVs) {
      x28_loadedGenDesc->x40_TEXR->GetValueUV(x74_curFrame - particle.x28_startFrame, uvs);
    }

    CVector3f dVec = particle.x4_pos - particle.x10_prevPos;

    if (x26d_24_FXLL) {
      float mag = dVec.Magnitude();
      if (mag > 0.f) {
        dVec *= 1.f / mag;
      }
    }

    CVector3f p1 = systemCameraMatrix * particle.x4_pos;
    CVector3f p2 = systemCameraMatrix * (particle.x4_pos + (particle.x2c_lineLengthOrSize * dVec));

    if (widtConst) {
      uint color = particle.x34_color.GetColor_u32();
      GXPosition3f32(p1[0], p1[1], p1[2]);
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMin, uvs.yMin);
      GXPosition3f32(p2[0], p2[1], p2[2]);
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMax, uvs.yMax);
    } else {
      float lineWidth = particle.x30_lineWidthOrRota;
      if (lineWidth < 0.f) {
        lineWidth = 0.f;
      } else if (lineWidth > 42.5f) {
        lineWidth = 42.5f;
      }
      CGX::SetLineWidth(CCast::ToUint8(6.f * lineWidth), GX_TO_ZERO);
      CGX::Begin(GX_LINES, GX_VTXFMT0, 2);
      uint color = particle.x34_color.GetColor_u32();
      GXPosition3f32(p1[0], p1[1], p1[2]);
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMin, uvs.yMin);
      GXPosition3f32(p2[0], p2[1], p2[2]);
      GXColor1u32(color);
      GXTexCoord2f32(uvs.xMax, uvs.yMax);
      CGX::End();
    }
  }

  if (widtConst) {
    CGX::End();
  }

  CGraphics::SetLineWidth(1.f, kTO_Zero);
}

void CElementGen::RenderModels() {
  CParticleGlobals::mParticleAccessParameters = NULL;
  if (x26d_26_modelsUseLights) {
    CGraphics::SetLightState(x274_backupLightActive);
  } else {
    CGraphics::SetAmbientColor(CColor::White());
  }

  bool moveRedToAlphaBuffer = false;
  SUVElementSet uvs;
  CGlobalRandom gr(x27c_randState);
  uvs.xMin = 0.f;
  uvs.xMax = 1.f;
  uvs.yMin = 0.f;
  uvs.yMax = 1.f;

  if (x28_loadedGenDesc->x31_26_PMUS) {
    bool enableMRTA = false;
    if (sMoveRedToAlphaBuffer && x28_loadedGenDesc->x31_25_PMAB &&
        x28_loadedGenDesc->x40_TEXR != NULL) {
      enableMRTA = true;
    }
    moveRedToAlphaBuffer = enableMRTA;

    if (x28_loadedGenDesc->x31_25_PMAB) {
      CGraphics::SetDepthWriteMode(true, kE_LEqual, false);
      if (enableMRTA) {
        CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
      } else {
        CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_One, kLO_Clear);
        CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
      }
    } else {
      CGraphics::SetDepthWriteMode(true, kE_LEqual, true);
      CGraphics::SetBlendMode(kBM_Blend, kBF_SrcAlpha, kBF_InvSrcAlpha, kLO_Clear);
      CGraphics::SetAlphaCompare(kAF_Greater, 0, kAO_And, kAF_Always, 0);
    }

    CGraphics::SetCullMode(kCM_None);

    if (x28_loadedGenDesc->x40_TEXR != NULL) {
      int partFrame = x74_curFrame - x30_particles[0].x28_startFrame;
      TToken< CTexture > tex(x28_loadedGenDesc->x40_TEXR->GetValueTexture(partFrame));
      tex.GetT()->Load(GX_TEXMAP0, CTexture::kCM_Repeat);

      CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);

      if (enableMRTA) {
        CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_CPREV, GX_CC_APREV, GX_CC_ZERO);
        CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_APREV, GX_CA_ZERO);
        CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE1);
        CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
        GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP1);
        CGX::SetNumTevStages(2);

        static const GXVtxDescList skDescList[] = {
            {GX_VA_POS, GX_DIRECT},
            {GX_VA_CLR0, GX_DIRECT},
            {GX_VA_TEX0, GX_DIRECT},
            {GX_VA_NULL, GX_NONE},
        };
        CGX::SetVtxDescv(skDescList);
        CGX::SetChanCtrl(CGX::Channel0, false, GX_SRC_REG, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE,
                         GX_AF_NONE);
        CGX::SetNumChans(1);
        CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false,
                            GX_PTIDENTITY);
        CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        CGX::SetNumTexGens(1);
      } else {
        CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
      }

      x28_loadedGenDesc->x40_TEXR->GetValueUV(x74_curFrame - x30_particles[0].x28_startFrame, uvs);
      x28_loadedGenDesc->x40_TEXR->HasConstantUV();
    } else {
      CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvPassthru);
      CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    }
  }

  CVector3f pmopVec(0.f, 0.f, 0.f);

  CTransform4f orient(CTransform4f::Identity());
  if (!x28_loadedGenDesc->x31_27_PMOO) {
    orient = x1d8_orientation;
  }
  orient = orient * x22c_globalOrientation;

  bool pmrtConst = false;
  if (x28_loadedGenDesc->x5c_PMRT != NULL && x28_loadedGenDesc->x5c_PMRT->IsFastConstant()) {
    pmrtConst = true;
  }

  CVector3f trans =
      (x13c_globalScaleTransformInverse * x1a8_localScaleTransformInverse) * xe8_globalTranslation;

  CTransform4f rot(CTransform4f::Identity());
  if (pmrtConst) {
    CVector3f pmrtVal(0.f, 0.f, 0.f);
    x28_loadedGenDesc->x5c_PMRT->GetValue(x74_curFrame, pmrtVal);
    rot = CTransform4f::RotateZ(CRelAngle::FromDegrees(pmrtVal.GetZ()));
    rot.RotateLocalY(CRelAngle::FromDegrees(pmrtVal.GetY()));
    rot.RotateLocalX(CRelAngle::FromDegrees(pmrtVal.GetX()));
  }
  rot = orient * rot;

  CParticleGlobals::SetEmitterTime(x74_curFrame);
  CColor col = x338_moduColor;

  for (int i = 0; i < x30_particles.size(); ++i) {
    CParticle& particle = x30_particles[i];
    if (particle.x0_endFrame != -1) {
      int lifetime = particle.x0_endFrame - particle.x28_startFrame;
      int partFrame = x74_curFrame - particle.x28_startFrame - 1;
      CParticleGlobals::SetParticleLifetime(lifetime);
      CParticleGlobals::UpdateParticleLifetimeTweenValues(partFrame);
      CParticleGlobals::mCurrentParticle = &particle;

      if (x26d_28_enableADV) {
        CParticleGlobals::mParticleAccessParameters = x60_advValues[i].values;
      }

      if (x28_loadedGenDesc->x58_PMOP != NULL) {
        x28_loadedGenDesc->x58_PMOP->GetValue(partFrame, pmopVec);
      }

      CTransform4f partTrans = CTransform4f::Translate(
          trans +
          ((particle.x4_pos - particle.x10_prevPos) * x80_timeDeltaScale + particle.x10_prevPos));

      if (x2c_orientType == kMOT_One) {
        CTransform4f partRot(x50_parentMatrices[i], CVector3f::Zero());
        partTrans.AddTranslation((orient * partRot) * pmopVec);
        partTrans *= partRot;
      } else {
        partTrans.AddTranslation(orient * pmopVec);
      }

      if (pmrtConst) {
        partTrans *= rot;
      } else if (x28_loadedGenDesc->x5c_PMRT != NULL) {
        CVector3f pmrtVal(0.f, 0.f, 0.f);
        x28_loadedGenDesc->x5c_PMRT->GetValue(partFrame, pmrtVal);
        rot = CTransform4f::RotateZ(CRelAngle::FromDegrees(pmrtVal.GetZ()));
        rot.RotateLocalY(CRelAngle::FromDegrees(pmrtVal.GetY()));
        rot.RotateLocalX(CRelAngle::FromDegrees(pmrtVal.GetX()));
        partTrans *= (orient * rot);
      } else {
        partTrans *= rot;
      }

      if (x28_loadedGenDesc->x60_PMSC != NULL) {
        CVector3f pmscVal(0.f, 0.f, 0.f);
        x28_loadedGenDesc->x60_PMSC->GetValue(partFrame, pmscVal);
        partTrans *= CTransform4f::Scale(pmscVal.GetX(), pmscVal.GetY(), pmscVal.GetZ());
      }

      if (x28_loadedGenDesc->x64_PMCL != NULL) {
        x28_loadedGenDesc->x64_PMCL->GetValue(partFrame, col);
        col = CColor::Modulate(col, x338_moduColor);
      }

      CGraphics::SetModelMatrix(x10c_globalScaleTransform * partTrans * x178_localScaleTransform);

      if (x28_loadedGenDesc->x31_26_PMUS) {
        if (moveRedToAlphaBuffer) {
          CGX::Begin(GX_QUADS, GX_VTXFMT0, 4);
          GXPosition3f32(0.5f, 0.f, 0.5f);
          u32 c = col.GetColor_u32();
          GXColor1u32(c);
          GXTexCoord2f32(uvs.xMax, uvs.yMax);
          GXPosition3f32(-0.5f, 0.f, 0.5f);
          GXColor1u32(c);
          GXTexCoord2f32(uvs.xMin, uvs.yMax);
          GXPosition3f32(-0.5f, 0.f, -0.5f);
          GXColor1u32(c);
          GXTexCoord2f32(uvs.xMin, uvs.yMin);
          GXPosition3f32(0.5f, 0.f, -0.5f);
          GXColor1u32(c);
          GXTexCoord2f32(uvs.xMax, uvs.yMin);
          CGX::End();
        } else {
          CGraphics::StreamBegin(kP_Quads);
          CGraphics::StreamColor(col.GetColor_u32());
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
        CModel* model = x28_loadedGenDesc->x48_PMDL->GetObject();
        if (sSubtractBlend) {
          model->Draw(CModelFlags::AlphaBlended(0.5f).DepthCompareUpdate(true, false));
        } else if (x28_loadedGenDesc->x31_25_PMAB) {
          model->Draw(CModelFlags::Additive(col).DepthCompareUpdate(true, false));
        } else if (1.f == col.GetAlpha()) {
          model->Draw(CModelFlags::Normal());
        } else {
          model->Draw(CModelFlags::AlphaBlended(col.GetAlpha()).DepthCompareUpdate(true, false));
        }
      }
    }
  }

  if (x26d_26_modelsUseLights) {
    CGraphics::DisableAllLights();
  }

  CGraphics::SetCullMode(kCM_Front);
  CTevCombiners::ResetStates();
  if (moveRedToAlphaBuffer) {
    GXSetTevSwapMode(GX_TEVSTAGE1, GX_TEV_SWAP0, GX_TEV_SWAP0);
  }
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
}

int CElementGen::GetParticleCountAll() const { return x264_recursiveParticleCount; }

int CElementGen::GetParticleCountAllInternal() const {
  int ret = x25c_activeParticleCount;
  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    CParticleGen* child = *it;
    if (child->Get4CharId() == 'PART') {
      ret += static_cast< CElementGen* >(child)->GetParticleCountAll();
    }
  }
  return ret;
}

int CElementGen::GetSystemCount() {
  int ret = static_cast< int >(x25c_activeParticleCount) > 0;
  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    CParticleGen* child = *it;
    if (child->Get4CharId() == 'PART') {
      ret += static_cast< CElementGen* >(child)->GetSystemCount();
    } else {
      ret += 1;
    }
  }
  return ret;
}

void CElementGen::AccumulateBounds(const CVector3f& pos, float size) {
  float x = pos.GetX();
  float y = pos.GetY();
  float z = pos.GetZ();
  if (x > x2e0_aabbMax.GetX())
    x2e0_aabbMax.SetX(x);
  if (x < x2d4_aabbMin.GetX())
    x2d4_aabbMin.SetX(x);
  if (y > x2e0_aabbMax.GetY())
    x2e0_aabbMax.SetY(y);
  if (y < x2d4_aabbMin.GetY())
    x2d4_aabbMin.SetY(y);
  if (z > x2e0_aabbMax.GetZ())
    x2e0_aabbMax.SetZ(z);
  if (z < x2d4_aabbMin.GetZ())
    x2d4_aabbMin.SetZ(z);
  if (size > x2ec_maxSize)
    x2ec_maxSize = size;
}

void CElementGen::BuildParticleSystemBounds() {
  CAABox childBounds = CAABox::MakeMaxInvertedBox();
  bool accumulated = false;

  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    rstl::optional_object< CAABox > chBounds = (*it)->GetBounds();
    if (chBounds) {
      accumulated = true;
      const CAABox& bounds = *chBounds;
      childBounds.AccumulateBounds(bounds.GetMinPoint());
      childBounds.AccumulateBounds(bounds.GetMaxPoint());
    }
  }

  x264_recursiveParticleCount = GetParticleCountAllInternal();

  if (GetParticleCount() > 0) {
    CVector3f scale = CVector3f::ByElementMultiply(
        CVector3f(x2ec_maxSize, x2ec_maxSize, x2ec_maxSize), x100_globalScale);

    CAABox box = CAABox(x2d4_aabbMin, x2e0_aabbMax)
                     .GetTransformedAABox((x10c_globalScaleTransform * x22c_globalOrientation) *
                                          x178_localScaleTransform);

    x2f0_systemBounds = CAABox(box.GetMinPoint() + xe8_globalTranslation - scale,
                               box.GetMaxPoint() + xe8_globalTranslation + scale);
  } else {
    x2f0_systemBounds = CAABox::MakeMaxInvertedBox();
  }

  if (accumulated) {
    x2f0_systemBounds.AccumulateBounds(childBounds.GetMinPoint());
    x2f0_systemBounds.AccumulateBounds(childBounds.GetMaxPoint());
  }
}

rstl::optional_object< CAABox > CElementGen::GetBounds() const {
  if (GetParticleCountAll() <= 0) {
    return rstl::optional_object< CAABox >();
  }
  return rstl::optional_object< CAABox >(x2f0_systemBounds);
}

int CElementGen::GetEmitterTime() const { return x74_curFrame; }

bool CElementGen::SystemHasLight() const { return x308_lightType != kLT_None; }

CLight CElementGen::GetLight() const {
  switch (x308_lightType) {
  case kLT_Directional: {
    return CLight::BuildDirectional(
        x320_LDIR.AsNormalized(),
        CColor(x30c_LCLR.GetRed() * x310_LINT, x30c_LCLR.GetGreen() * x310_LINT,
               x30c_LCLR.GetBlue() * x310_LINT, x30c_LCLR.GetAlpha() * x310_LINT));
  }
  case kLT_Spot: {
    CLight spotLight =
        CLight::BuildSpot(x314_LOFF, x320_LDIR.AsNormalized(),
                          CColor(x30c_LCLR.GetRed() * x310_LINT, x30c_LCLR.GetGreen() * x310_LINT,
                                 x30c_LCLR.GetBlue() * x310_LINT, x30c_LCLR.GetAlpha() * x310_LINT),
                          x334_LSLA);
    float quad = x32c_falloffType == kFT_Quadratic ? x330_LFOR : 0.f;
    float linear = x32c_falloffType == kFT_Linear ? x330_LFOR : 0.f;
    float constant = x32c_falloffType == kFT_Constant ? 1.f : 0.f;
    spotLight.SetAttenuation(constant, linear, quad);
    return spotLight;
  }
  default: {
    float quad = x32c_falloffType == kFT_Quadratic ? x330_LFOR : 0.f;
    float linear = x32c_falloffType == kFT_Linear ? x330_LFOR : 0.f;
    float constant = x32c_falloffType == kFT_Constant ? 1.f : 0.f;
    return CLight::BuildCustom(x314_LOFF, CVector3f(1.f, 0.f, 0.f), x30c_LCLR, constant, linear,
                               quad, x310_LINT, 0.f, 0.f);
  }
  }
}

void CElementGen::SetGeneratorRate(float rate) {
  x98_generatorRate = rstl::max_val(rate, 0.f);
  for (AUTO(it, x290_activePartChildren.begin()); it != x290_activePartChildren.end(); ++it) {
    CParticleGen* child = *it;
    if (child->Get4CharId() == 'PART') {
      child->SetGeneratorRate(x98_generatorRate);
    }
  }
}
float CElementGen::GetGeneratorRate() const { return x98_generatorRate; }



uint CElementGen::Get4CharId() const { return 'PART'; }

int CElementGen::GetNumActiveChildParticles() const { return x290_activePartChildren.size(); }

CParticleGen* CElementGen::GetActiveChildParticle(int index) const {
  return x290_activePartChildren[index];
}

void CElementGen::SetExternalVar(int index, float val) { x9c_externalVars[index] = val; }

float CElementGen::GetExternalVar(int index) const { return x9c_externalVars[index]; }

bool CElementGen::IsIndirectTextured() const {
  return x28_loadedGenDesc->x40_TEXR != nullptr && x28_loadedGenDesc->x44_TIND != nullptr;
}

bool CElementGen::GetParticleEmission() const { return x88_particleEmission; }

const CTransform4f& CElementGen::GetGlobalOrientation() const { return x22c_globalOrientation; }

const CVector3f& CElementGen::GetGlobalTranslation() const { return xe8_globalTranslation; }
