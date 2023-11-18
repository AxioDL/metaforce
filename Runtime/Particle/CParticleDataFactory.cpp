#include "Runtime/Particle/CParticleDataFactory.hpp"

#include "Runtime/CRandom16.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"

#include <logvisor/logvisor.hpp>

namespace metaforce {
static logvisor::Module Log("metaforce::CParticleDataFactory");

float CParticleDataFactory::GetReal(CInputStream& in) { return in.ReadFloat(); }

s32 CParticleDataFactory::GetInt(CInputStream& in) { return in.ReadInt32(); }

bool CParticleDataFactory::GetBool(CInputStream& in) {
  FourCC cid = GetClassID(in);
  if (cid != FOURCC('CNST'))
    Log.report(logvisor::Fatal, FMT_STRING("bool element does not begin with CNST"));
  return in.ReadBool();
}

FourCC CParticleDataFactory::GetClassID(CInputStream& in) {
  u32 val = 0;
  in.Get(reinterpret_cast<u8*>(&val), 4);
  return val;
}

SParticleModel CParticleDataFactory::GetModel(CInputStream& in, CSimplePool* resPool) {
  FourCC clsId = GetClassID(in);
  if (clsId == SBIG('NONE'))
    return {};
  CAssetId id = in.Get<CAssetId>();
  if (!id.IsValid())
    return {};
  return resPool->GetObj({FOURCC('CMDL'), id});
}

SChildGeneratorDesc CParticleDataFactory::GetChildGeneratorDesc(CAssetId res, CSimplePool* resPool,
                                                                const std::vector<CAssetId>& tracker) {
  if (std::count(tracker.cbegin(), tracker.cend(), res) == 0)
    return resPool->GetObj({FOURCC('PART'), res});
  return {};
}

SChildGeneratorDesc CParticleDataFactory::GetChildGeneratorDesc(CInputStream& in, CSimplePool* resPool,
                                                                const std::vector<CAssetId>& tracker) {
  FourCC clsId = GetClassID(in);
  if (clsId == SBIG('NONE'))
    return {};
  CAssetId id = in.Get<CAssetId>();
  if (!id.IsValid())
    return {};
  return GetChildGeneratorDesc(id, resPool, tracker);
}

SSwooshGeneratorDesc CParticleDataFactory::GetSwooshGeneratorDesc(CInputStream& in, CSimplePool* resPool) {
  FourCC clsId = GetClassID(in);
  if (clsId == SBIG('NONE'))
    return {};
  CAssetId id = in.Get<CAssetId>();
  if (!id.IsValid())
    return {};
  return resPool->GetObj({FOURCC('SWHC'), id});
}

SElectricGeneratorDesc CParticleDataFactory::GetElectricGeneratorDesc(CInputStream& in, CSimplePool* resPool) {
  FourCC clsId = GetClassID(in);
  if (clsId == SBIG('NONE'))
    return {};
  CAssetId id = in.Get<CAssetId>();
  if (!id.IsValid())
    return {};
  return resPool->GetObj({FOURCC('ELSC'), id});
}

static std::unique_ptr<CTexture> CreateTexture(u32 value) {
  auto tex = std::make_unique<CTexture>(ETexelFormat::RGBA8, 4, 4, 1, "CUVElement Fallback Texture"sv);
  auto* data = reinterpret_cast<u32*>(tex->Lock());
  for (int i = 0; i < 4 * 4; ++i) {
    data[i] = value;
  }
  tex->UnLock();
  return tex;
}

std::unique_ptr<CUVElement> CParticleDataFactory::GetTextureElement(CInputStream& in, CSimplePool* resPool) {
  FourCC clsId = GetClassID(in);
  switch (clsId.toUint32()) {
  case SBIG('CNST'): {
    CAssetId id;
    const auto subId = GetClassID(in);
    if (subId != SBIG('NONE')) {
      id = in.Get<CAssetId>();
    }
    TToken<CTexture> txtr;
    if (id.IsValid()) {
      txtr = resPool->GetObj({FOURCC('TXTR'), id});
    } else {
      txtr = CreateTexture(0xFFFFFFFF);
    }
    return std::make_unique<CUVEConstant>(std::move(txtr));
  }
  case SBIG('ATEX'): {
    CAssetId id;
    const auto subId = GetClassID(in);
    if (subId != SBIG('NONE')) {
      id = in.Get<CAssetId>();
    }
    auto a = GetIntElement(in);
    auto b = GetIntElement(in);
    auto c = GetIntElement(in);
    auto d = GetIntElement(in);
    auto e = GetIntElement(in);
    const bool f = GetBool(in);
    TToken<CTexture> txtr;
    if (id.IsValid()) {
      txtr = resPool->GetObj({FOURCC('TXTR'), id});
    } else {
      txtr = CreateTexture(0xFFFFFFFF);
    }
    return std::make_unique<CUVEAnimTexture>(std::move(txtr), std::move(std::move(a)), std::move(b), std::move(c),
                                             std::move(d), std::move(e), f);
  }
  default:
    break;
  }
  return nullptr;
}

std::unique_ptr<CColorElement> CParticleDataFactory::GetColorElement(CInputStream& in) {
  FourCC clsId = GetClassID(in);
  switch (clsId.toUint32()) {
  case SBIG('KEYE'):
  case SBIG('KEYP'): {
    return std::make_unique<CCEKeyframeEmitter>(in);
  }
  case SBIG('CNST'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    auto c = GetRealElement(in);
    auto d = GetRealElement(in);
    if (a->IsConstant() && b->IsConstant() && c->IsConstant() && d->IsConstant()) {
      float af, bf, cf, df;
      a->GetValue(0, af);
      b->GetValue(0, bf);
      c->GetValue(0, cf);
      d->GetValue(0, df);
      return std::make_unique<CCEFastConstant>(af, bf, cf, df);
    } else {
      return std::make_unique<CCEConstant>(std::move(std::move(a)), std::move(b), std::move(c), std::move(d));
    }
  }
  case SBIG('CHAN'): {
    auto a = GetColorElement(in);
    auto b = GetColorElement(in);
    auto c = GetIntElement(in);
    return std::make_unique<CCETimeChain>(std::move(std::move(a)), std::move(b), std::move(c));
  }
  case SBIG('CFDE'): {
    auto a = GetColorElement(in);
    auto b = GetColorElement(in);
    auto c = GetRealElement(in);
    auto d = GetRealElement(in);
    return std::make_unique<CCEFadeEnd>(std::move(std::move(a)), std::move(b), std::move(c), std::move(d));
  }
  case SBIG('FADE'): {
    auto a = GetColorElement(in);
    auto b = GetColorElement(in);
    auto c = GetRealElement(in);
    return std::make_unique<CCEFade>(std::move(std::move(a)), std::move(b), std::move(c));
  }
  case SBIG('PULS'): {
    auto a = GetIntElement(in);
    auto b = GetIntElement(in);
    auto c = GetColorElement(in);
    auto d = GetColorElement(in);
    return std::make_unique<CCEPulse>(std::move(std::move(a)), std::move(b), std::move(c), std::move(d));
  }
  case SBIG('PCOL'): {
    return std::make_unique<CCEParticleColor>();
  }
  default:
    break;
  }
  return nullptr;
}

std::unique_ptr<CModVectorElement> CParticleDataFactory::GetModVectorElement(CInputStream& in) {
  FourCC clsId = GetClassID(in);
  switch (clsId.toUint32()) {
  case SBIG('IMPL'): {
    auto a = GetVectorElement(in);
    auto b = GetRealElement(in);
    auto c = GetRealElement(in);
    auto d = GetRealElement(in);
    const bool e = GetBool(in);
    return std::make_unique<CMVEImplosion>(std::move(a), std::move(b), std::move(c), std::move(d), e);
  }
  case SBIG('EMPL'): {
    auto a = GetVectorElement(in);
    auto b = GetRealElement(in);
    auto c = GetRealElement(in);
    auto d = GetRealElement(in);
    const bool e = GetBool(in);
    return std::make_unique<CMVEExponentialImplosion>(std::move(a), std::move(b), std::move(c), std::move(d), e);
  }
  case SBIG('CHAN'): {
    auto a = GetModVectorElement(in);
    auto b = GetModVectorElement(in);
    auto c = GetIntElement(in);
    return std::make_unique<CMVETimeChain>(std::move(a), std::move(b), std::move(c));
  }
  case SBIG('BNCE'): {
    auto a = GetVectorElement(in);
    auto b = GetVectorElement(in);
    auto c = GetRealElement(in);
    auto d = GetRealElement(in);
    const bool e = GetBool(in);
    return std::make_unique<CMVEBounce>(std::move(a), std::move(b), std::move(c), std::move(d), e);
  }
  case SBIG('CNST'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    auto c = GetRealElement(in);
    if (a->IsConstant() && b->IsConstant() && c->IsConstant()) {
      float af, bf, cf;
      a->GetValue(0, af);
      b->GetValue(0, bf);
      c->GetValue(0, cf);
      // NOTE: 0-00 bug uses the value of a for each element
      // Other versions unknown
      return std::make_unique<CMVEFastConstant>(af, bf, cf);
    } else {
      return std::make_unique<CMVEConstant>(std::move(a), std::move(b), std::move(c));
    }
  }
  case SBIG('GRAV'): {
    auto a = GetVectorElement(in);
    return std::make_unique<CMVEGravity>(std::move(a));
  }
  case SBIG('EXPL'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    return std::make_unique<CMVEExplode>(std::move(a), std::move(b));
  }
  case SBIG('SPOS'): {
    auto a = GetVectorElement(in);
    return std::make_unique<CMVESetPosition>(std::move(a));
  }
  case SBIG('LMPL'): {
    auto a = GetVectorElement(in);
    auto b = GetRealElement(in);
    auto c = GetRealElement(in);
    auto d = GetRealElement(in);
    const bool e = GetBool(in);
    return std::make_unique<CMVELinearImplosion>(std::move(a), std::move(b), std::move(c), std::move(d), e);
  }
  case SBIG('PULS'): {
    auto a = GetIntElement(in);
    auto b = GetIntElement(in);
    auto c = GetModVectorElement(in);
    auto d = GetModVectorElement(in);
    return std::make_unique<CMVEPulse>(std::move(a), std::move(b), std::move(c), std::move(d));
  }
  case SBIG('WIND'): {
    auto a = GetVectorElement(in);
    auto b = GetRealElement(in);
    return std::make_unique<CMVEWind>(std::move(a), std::move(b));
  }
  case SBIG('SWRL'): {
    auto a = GetVectorElement(in);
    auto b = GetVectorElement(in);
    auto c = GetRealElement(in);
    auto d = GetRealElement(in);
    return std::make_unique<CMVESwirl>(std::move(a), std::move(b), std::move(c), std::move(d));
  }
  default:
    break;
  }
  return nullptr;
}

std::unique_ptr<CEmitterElement> CParticleDataFactory::GetEmitterElement(CInputStream& in) {
  FourCC clsId = GetClassID(in);
  switch (clsId.toUint32()) {
  case SBIG('SETR'): {
    FourCC prop = GetClassID(in);
    if (prop == SBIG('ILOC')) {
      auto a = GetVectorElement(in);
      prop = GetClassID(in);
      if (prop == SBIG('IVEC')) {
        auto b = GetVectorElement(in);
        return std::make_unique<CEESimpleEmitter>(std::move(a), std::move(b));
      }
    }
    return nullptr;
  }
  case SBIG('SEMR'): {
    auto a = GetVectorElement(in);
    auto b = GetVectorElement(in);
    return std::make_unique<CEESimpleEmitter>(std::move(a), std::move(b));
  }
  case SBIG('SPHE'): {
    auto a = GetVectorElement(in);
    auto b = GetRealElement(in);
    auto c = GetRealElement(in);
    return std::make_unique<CVESphere>(std::move(a), std::move(b), std::move(c));
  }
  case SBIG('ASPH'): {
    auto a = GetVectorElement(in);
    auto b = GetRealElement(in);
    auto c = GetRealElement(in);
    auto d = GetRealElement(in);
    auto e = GetRealElement(in);
    auto f = GetRealElement(in);
    auto g = GetRealElement(in);
    return std::make_unique<CVEAngleSphere>(std::move(a), std::move(f), std::move(g), std::move(b), std::move(c),
                                            std::move(d), std::move(e));
  }
  default:
    break;
  }
  return nullptr;
}

std::unique_ptr<CVectorElement> CParticleDataFactory::GetVectorElement(CInputStream& in) {
  FourCC clsId = GetClassID(in);
  switch (clsId.toUint32()) {
  case SBIG('CONE'): {
    auto a = GetVectorElement(in);
    auto b = GetRealElement(in);
    return std::make_unique<CVECone>(std::move(a), std::move(b));
  }
  case SBIG('CHAN'): {
    auto a = GetVectorElement(in);
    auto b = GetVectorElement(in);
    auto c = GetIntElement(in);
    return std::make_unique<CVETimeChain>(std::move(a), std::move(b), std::move(c));
  }
  case SBIG('ANGC'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    auto c = GetRealElement(in);
    auto d = GetRealElement(in);
    auto e = GetRealElement(in);
    return std::make_unique<CVEAngleCone>(std::move(a), std::move(b), std::move(c), std::move(d), std::move(e));
  }
  case SBIG('ADD_'): {
    auto a = GetVectorElement(in);
    auto b = GetVectorElement(in);
    return std::make_unique<CVEAdd>(std::move(a), std::move(b));
  }
  case SBIG('CCLU'): {
    auto a = GetVectorElement(in);
    auto b = GetVectorElement(in);
    auto c = GetIntElement(in);
    auto d = GetRealElement(in);
    return std::make_unique<CVECircleCluster>(std::move(a), std::move(b), std::move(c), std::move(d));
  }
  case SBIG('CNST'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    auto c = GetRealElement(in);
    if (a->IsConstant() && b->IsConstant() && c->IsConstant()) {
      float af, bf, cf;
      a->GetValue(0, af);
      b->GetValue(0, bf);
      c->GetValue(0, cf);
      return std::make_unique<CVEFastConstant>(af, bf, cf);
    } else {
      return std::make_unique<CVEConstant>(std::move(a), std::move(b), std::move(c));
    }
  }
  case SBIG('CIRC'): {
    auto a = GetVectorElement(in);
    auto b = GetVectorElement(in);
    auto c = GetRealElement(in);
    auto d = GetRealElement(in);
    auto e = GetRealElement(in);
    return std::make_unique<CVECircle>(std::move(a), std::move(b), std::move(c), std::move(d), std::move(e));
  }
  case SBIG('KEYE'):
  case SBIG('KEYP'): {
    return std::make_unique<CVEKeyframeEmitter>(in);
  }
  case SBIG('MULT'): {
    auto a = GetVectorElement(in);
    auto b = GetVectorElement(in);
    return std::make_unique<CVEMultiply>(std::move(a), std::move(b));
  }
  case SBIG('RTOV'): {
    auto a = GetRealElement(in);
    return std::make_unique<CVERealToVector>(std::move(a));
  }
  case SBIG('PULS'): {
    auto a = GetIntElement(in);
    auto b = GetIntElement(in);
    auto c = GetVectorElement(in);
    auto d = GetVectorElement(in);
    return std::make_unique<CVEPulse>(std::move(a), std::move(b), std::move(c), std::move(d));
  }
  case SBIG('PVEL'): {
    return std::make_unique<CVEParticleVelocity>();
  }
  case SBIG('PLCO'): {
    return std::make_unique<CVEParticleColor>();
  }
  case SBIG('PLOC'): {
    return std::make_unique<CVEParticleLocation>();
  }
  case SBIG('PSOF'): {
    return std::make_unique<CVEParticleSystemOrientationFront>();
  }
  case SBIG('PSOU'): {
    return std::make_unique<CVEParticleSystemOrientationUp>();
  }
  case SBIG('PSOR'): {
    return std::make_unique<CVEParticleSystemOrientationRight>();
  }
  case SBIG('PSTR'): {
    return std::make_unique<CVEParticleSystemTranslation>();
  }
  case SBIG('SUB_'): {
    auto a = GetVectorElement(in);
    auto b = GetVectorElement(in);
    return std::make_unique<CVESubtract>(std::move(a), std::move(b));
  }
  case SBIG('CTVC'): {
    auto a = GetColorElement(in);
    return std::make_unique<CVEColorToVector>(std::move(a));
  }
  default:
    break;
  }
  return nullptr;
}

std::unique_ptr<CRealElement> CParticleDataFactory::GetRealElement(CInputStream& in) {
  FourCC clsId = GetClassID(in);
  switch (clsId.toUint32()) {
  case SBIG('LFTW'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    return std::make_unique<CRELifetimeTween>(std::move(a), std::move(b));
  }
  case SBIG('CNST'): {
    const float a = GetReal(in);
    return std::make_unique<CREConstant>(a);
  }
  case SBIG('CHAN'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    auto c = GetIntElement(in);
    return std::make_unique<CRETimeChain>(std::move(a), std::move(b), std::move(c));
  }
  case SBIG('ADD_'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    return std::make_unique<CREAdd>(std::move(a), std::move(b));
  }
  case SBIG('CLMP'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    auto c = GetRealElement(in);
    return std::make_unique<CREClamp>(std::move(a), std::move(b), std::move(c));
  }
  case SBIG('KEYE'):
  case SBIG('KEYP'): {
    return std::make_unique<CREKeyframeEmitter>(in);
  }
  case SBIG('IRND'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    return std::make_unique<CREInitialRandom>(std::move(a), std::move(b));
  }
  case SBIG('RAND'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    return std::make_unique<CRERandom>(std::move(a), std::move(b));
  }
  case SBIG('DOTP'): {
    auto a = GetVectorElement(in);
    auto b = GetVectorElement(in);
    return std::make_unique<CREDotProduct>(std::move(a), std::move(b));
  }
  case SBIG('MULT'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    return std::make_unique<CREMultiply>(std::move(a), std::move(b));
  }
  case SBIG('PULS'): {
    auto a = GetIntElement(in);
    auto b = GetIntElement(in);
    auto c = GetRealElement(in);
    auto d = GetRealElement(in);
    return std::make_unique<CREPulse>(std::move(a), std::move(b), std::move(c), std::move(d));
  }
  case SBIG('SCAL'): {
    auto a = GetRealElement(in);
    return std::make_unique<CRETimeScale>(std::move(a));
  }
  case SBIG('RLPT'): {
    auto a = GetRealElement(in);
    return std::make_unique<CRELifetimePercent>(std::move(a));
  }
  case SBIG('SINE'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    auto c = GetRealElement(in);
    return std::make_unique<CRESineWave>(std::move(a), std::move(b), std::move(c));
  }
  case SBIG('ISWT'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    return std::make_unique<CREInitialSwitch>(std::move(a), std::move(b));
  }
  case SBIG('CLTN'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    auto c = GetRealElement(in);
    auto d = GetRealElement(in);
    return std::make_unique<CRECompareLessThan>(std::move(a), std::move(b), std::move(c), std::move(d));
  }
  case SBIG('CEQL'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    auto c = GetRealElement(in);
    auto d = GetRealElement(in);
    return std::make_unique<CRECompareEquals>(std::move(a), std::move(b), std::move(c), std::move(d));
  }
  case SBIG('PAP1'): {
    return std::make_unique<CREParticleAccessParam1>();
  }
  case SBIG('PAP2'): {
    return std::make_unique<CREParticleAccessParam2>();
  }
  case SBIG('PAP3'): {
    return std::make_unique<CREParticleAccessParam3>();
  }
  case SBIG('PAP4'): {
    return std::make_unique<CREParticleAccessParam4>();
  }
  case SBIG('PAP5'): {
    return std::make_unique<CREParticleAccessParam5>();
  }
  case SBIG('PAP6'): {
    return std::make_unique<CREParticleAccessParam6>();
  }
  case SBIG('PAP7'): {
    return std::make_unique<CREParticleAccessParam7>();
  }
  case SBIG('PAP8'): {
    return std::make_unique<CREParticleAccessParam8>();
  }
  case SBIG('PSLL'): {
    return std::make_unique<CREParticleSizeOrLineLength>();
  }
  case SBIG('PRLW'): {
    return std::make_unique<CREParticleRotationOrLineWidth>();
  }
  case SBIG('SUB_'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    return std::make_unique<CRESubtract>(std::move(a), std::move(b));
  }
  case SBIG('VMAG'): {
    auto a = GetVectorElement(in);
    return std::make_unique<CREVectorMagnitude>(std::move(a));
  }
  case SBIG('VXTR'): {
    auto a = GetVectorElement(in);
    return std::make_unique<CREVectorXToReal>(std::move(a));
  }
  case SBIG('VYTR'): {
    auto a = GetVectorElement(in);
    return std::make_unique<CREVectorYToReal>(std::move(a));
  }
  case SBIG('VZTR'): {
    auto a = GetVectorElement(in);
    return std::make_unique<CREVectorZToReal>(std::move(a));
  }
  case SBIG('CEXT'): {
    auto a = GetIntElement(in);
    return std::make_unique<CREExternalVar>(std::move(a));
  }
  case SBIG('ITRL'): {
    auto a = GetIntElement(in);
    auto b = GetRealElement(in);
    return std::make_unique<CREIntTimesReal>(std::move(a), std::move(b));
  }
  case SBIG('CRNG'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    auto c = GetRealElement(in);
    auto d = GetRealElement(in);
    auto e = GetRealElement(in);
    return std::make_unique<CREConstantRange>(std::move(a), std::move(b), std::move(c), std::move(d), std::move(e));
  }
  case SBIG('GTCR'): {
    auto a = GetColorElement(in);
    return std::make_unique<CREGetComponentRed>(std::move(a));
  }
  case SBIG('GTCG'): {
    auto a = GetColorElement(in);
    return std::make_unique<CREGetComponentGreen>(std::move(a));
  }
  case SBIG('GTCB'): {
    auto a = GetColorElement(in);
    return std::make_unique<CREGetComponentBlue>(std::move(a));
  }
  case SBIG('GTCA'): {
    auto a = GetColorElement(in);
    return std::make_unique<CREGetComponentAlpha>(std::move(a));
  }
  default:
    break;
  }
  return nullptr;
}

std::unique_ptr<CIntElement> CParticleDataFactory::GetIntElement(CInputStream& in) {
  FourCC clsId = GetClassID(in);
  switch (clsId.toUint32()) {
  case SBIG('KEYE'):
  case SBIG('KEYP'): {
    return std::make_unique<CIEKeyframeEmitter>(in);
  }
  case SBIG('DETH'): {
    auto a = GetIntElement(in);
    auto b = GetIntElement(in);
    return std::make_unique<CIEDeath>(std::move(a), std::move(b));
  }
  case SBIG('CLMP'): {
    auto a = GetIntElement(in);
    auto b = GetIntElement(in);
    auto c = GetIntElement(in);
    return std::make_unique<CIEClamp>(std::move(a), std::move(b), std::move(c));
  }
  case SBIG('CHAN'): {
    auto a = GetIntElement(in);
    auto b = GetIntElement(in);
    auto c = GetIntElement(in);
    return std::make_unique<CIETimeChain>(std::move(a), std::move(b), std::move(c));
  }
  case SBIG('ADD_'): {
    auto a = GetIntElement(in);
    auto b = GetIntElement(in);
    return std::make_unique<CIEAdd>(std::move(a), std::move(b));
  }
  case SBIG('CNST'): {
    const int a = GetInt(in);
    return std::make_unique<CIEConstant>(a);
  }
  case SBIG('IMPL'): {
    auto a = GetIntElement(in);
    return std::make_unique<CIEImpulse>(std::move(a));
  }
  case SBIG('ILPT'): {
    auto a = GetIntElement(in);
    return std::make_unique<CIELifetimePercent>(std::move(a));
  }
  case SBIG('IRND'): {
    auto a = GetIntElement(in);
    auto b = GetIntElement(in);
    return std::make_unique<CIEInitialRandom>(std::move(a), std::move(b));
  }
  case SBIG('PULS'): {
    auto a = GetIntElement(in);
    auto b = GetIntElement(in);
    auto c = GetIntElement(in);
    auto d = GetIntElement(in);
    return std::make_unique<CIEPulse>(std::move(a), std::move(b), std::move(c), std::move(d));
  }
  case SBIG('MULT'): {
    auto a = GetIntElement(in);
    auto b = GetIntElement(in);
    return std::make_unique<CIEMultiply>(std::move(a), std::move(b));
  }
  case SBIG('SPAH'): {
    auto a = GetIntElement(in);
    auto b = GetIntElement(in);
    auto c = GetIntElement(in);
    return std::make_unique<CIESampleAndHold>(std::move(c), std::move(a), std::move(b));
  }
  case SBIG('RAND'): {
    auto a = GetIntElement(in);
    auto b = GetIntElement(in);
    return std::make_unique<CIERandom>(std::move(a), std::move(b));
  }
  case SBIG('RTOI'): {
    auto a = GetRealElement(in);
    auto b = GetRealElement(in);
    return std::make_unique<CIERealToInt>(std::move(a), std::move(b));
  }
  case SBIG('TSCL'): {
    auto a = GetRealElement(in);
    return std::make_unique<CIETimeScale>(std::move(a));
  }
  case SBIG('GAPC'): {
    return std::make_unique<CIEGetActiveParticleCount>();
  }
  case SBIG('GTCP'): {
    return std::make_unique<CIEGetCumulativeParticleCount>();
  }
  case SBIG('GEMT'): {
    return std::make_unique<CIEGetEmitterTime>();
  }
  case SBIG('MODU'): {
    auto a = GetIntElement(in);
    auto b = GetIntElement(in);
    return std::make_unique<CIEModulo>(std::move(a), std::move(b));
  }
  case SBIG('SUB_'): {
    auto a = GetIntElement(in);
    auto b = GetIntElement(in);
    return std::make_unique<CIESubtract>(std::move(a), std::move(b));
  }
  default:
    break;
  }
  return nullptr;
}

std::unique_ptr<CGenDescription> CParticleDataFactory::GetGeneratorDesc(CInputStream& in, CSimplePool* resPool) {
  std::vector<CAssetId> tracker;
  tracker.reserve(8);
  return CreateGeneratorDescription(in, tracker, {}, resPool);
}

std::unique_ptr<CGenDescription> CParticleDataFactory::CreateGeneratorDescription(CInputStream& in,
                                                                                  std::vector<CAssetId>& tracker,
                                                                                  CAssetId resId,
                                                                                  CSimplePool* resPool) {
  if (std::count(tracker.cbegin(), tracker.cend(), resId) == 0) {
    tracker.push_back(resId);
    FourCC cid = GetClassID(in);
    if (cid == FOURCC('GPSM')) {
      auto ret = std::make_unique<CGenDescription>();
      CreateGPSM(ret.get(), in, tracker, resPool);
      LoadGPSMTokens(ret.get());
      return ret;
    }
  }
  return nullptr;
}

bool CParticleDataFactory::CreateGPSM(CGenDescription* fillDesc, CInputStream& in, std::vector<CAssetId>& tracker,
                                      CSimplePool* resPool) {
  CRandom16 rand;
  CGlobalRandom gr(rand);
  FourCC clsId = GetClassID(in);
  while (clsId != SBIG('_END')) {
    switch (clsId.toUint32()) {
    case SBIG('PMCL'):
      fillDesc->x78_x64_PMCL = GetColorElement(in);
      break;
    case SBIG('LFOR'):
      fillDesc->x118_x104_LFOR = GetRealElement(in);
      break;
    case SBIG('IDTS'):
      fillDesc->xa4_x90_IDTS = GetChildGeneratorDesc(in, resPool, tracker);
      break;
    case SBIG('EMTR'):
      fillDesc->x40_x2c_EMTR = GetEmitterElement(in);
      break;
    case SBIG('COLR'):
      fillDesc->x30_x24_COLR = GetColorElement(in);
      break;
    case SBIG('CIND'):
      fillDesc->x45_30_x32_24_CIND = GetBool(in);
      break;
    case SBIG('AAPH'):
      fillDesc->x44_26_x30_26_AAPH = GetBool(in);
      break;
    case SBIG('CSSD'):
      fillDesc->xa0_x8c_CSSD = GetIntElement(in);
      break;
    case SBIG('GRTE'):
      fillDesc->x2c_x20_GRTE = GetRealElement(in);
      break;
    case SBIG('FXLL'):
      fillDesc->x44_25_x30_25_FXLL = GetBool(in);
      break;
    case SBIG('ICTS'):
      fillDesc->x8c_x78_ICTS = GetChildGeneratorDesc(in, resPool, tracker);
      break;
    case SBIG('KSSM'): {
      fillDesc->xd0_xbc_KSSM.reset();
      FourCC cid = GetClassID(in);
      if (cid != SBIG('CNST'))
        break;
      fillDesc->xd0_xbc_KSSM = std::make_unique<CSpawnSystemKeyframeData>(in);
      fillDesc->xd0_xbc_KSSM->LoadAllSpawnedSystemTokens(resPool);
      break;
    }
    case SBIG('ILOC'):
      GetVectorElement(in);
      break;
    case SBIG('IITS'):
      fillDesc->xb8_xa4_IITS = GetChildGeneratorDesc(in, resPool, tracker);
      break;
    case SBIG('IVEC'):
      GetVectorElement(in);
      break;
    case SBIG('LDIR'):
      fillDesc->x110_xfc_LDIR = GetVectorElement(in);
      break;
    case SBIG('LCLR'):
      fillDesc->x104_xf0_LCLR = GetColorElement(in);
      break;
    case SBIG('LENG'):
      fillDesc->x20_x14_LENG = GetRealElement(in);
      break;
    case SBIG('MAXP'):
      fillDesc->x28_x1c_MAXP = GetIntElement(in);
      break;
    case SBIG('LOFF'):
      fillDesc->x10c_xf8_LOFF = GetVectorElement(in);
      break;
    case SBIG('LINT'):
      fillDesc->x108_xf4_LINT = GetRealElement(in);
      break;
    case SBIG('LINE'):
      fillDesc->x44_24_x30_24_LINE = GetBool(in);
      break;
    case SBIG('LFOT'):
      fillDesc->x114_x100_LFOT = GetIntElement(in);
      break;
    case SBIG('LIT_'):
      fillDesc->x44_29_x30_29_LIT_ = GetBool(in);
      break;
    case SBIG('LTME'):
      fillDesc->x34_x28_LTME = GetIntElement(in);
      break;
    case SBIG('LSLA'):
      fillDesc->x11c_x108_LSLA = GetRealElement(in);
      break;
    case SBIG('LTYP'):
      fillDesc->x100_xec_LTYP = GetIntElement(in);
      break;
    case SBIG('NDSY'):
      fillDesc->xb4_xa0_NDSY = GetIntElement(in);
      break;
    case SBIG('MBSP'):
      fillDesc->x48_x34_MBSP = GetIntElement(in);
      break;
    case SBIG('MBLR'):
      fillDesc->x44_30_x31_24_MBLR = GetBool(in);
      break;
    case SBIG('NCSY'):
      fillDesc->x9c_x88_NCSY = GetIntElement(in);
      break;
    case SBIG('PISY'):
      fillDesc->xc8_xb4_PISY = GetIntElement(in);
      break;
    case SBIG('OPTS'):
      fillDesc->x45_31_x32_25_OPTS = GetBool(in);
      break;
    case SBIG('PMAB'):
      fillDesc->x44_31_x31_25_PMAB = GetBool(in);
      break;
    case SBIG('SESD'):
      fillDesc->xf8_xe4_SESD = GetIntElement(in);
      break;
    case SBIG('SEPO'):
      fillDesc->xfc_xe8_SEPO = GetVectorElement(in);
      break;
    case SBIG('PSLT'):
      fillDesc->xc_x0_PSLT = GetIntElement(in);
      break;
    case SBIG('PMSC'):
      fillDesc->x74_x60_PMSC = GetVectorElement(in);
      break;
    case SBIG('PMOP'):
      fillDesc->x6c_x58_PMOP = GetVectorElement(in);
      break;
    case SBIG('PMDL'):
      fillDesc->x5c_x48_PMDL = GetModel(in, resPool);
      break;
    case SBIG('PMRT'):
      fillDesc->x70_x5c_PMRT = GetVectorElement(in);
      break;
    case SBIG('POFS'):
      fillDesc->x18_xc_POFS = GetVectorElement(in);
      break;
    case SBIG('PMUS'):
      fillDesc->x45_24_x31_26_PMUS = GetBool(in);
      break;
    case SBIG('PSIV'):
      GetVectorElement(in);
      break;
    case SBIG('ROTA'):
      fillDesc->x50_x3c_ROTA = GetRealElement(in);
      break;
    case SBIG('PSVM'):
      GetModVectorElement(in);
      break;
    case SBIG('PSTS'):
      fillDesc->x14_x8_PSTS = GetRealElement(in);
      break;
    case SBIG('PSOV'):
      GetVectorElement(in);
      break;
    case SBIG('PSWT'):
      fillDesc->x10_x4_PSWT = GetIntElement(in);
      break;
    case SBIG('SEED'):
      fillDesc->x1c_x10_SEED = GetIntElement(in);
      break;
    case SBIG('PMOO'):
      fillDesc->x45_25_x31_27_PMOO = GetBool(in);
      break;
    case SBIG('SSSD'):
      fillDesc->xe4_xd0_SSSD = GetIntElement(in);
      break;
    case SBIG('SORT'):
      fillDesc->x44_28_x30_28_SORT = GetBool(in);
      break;
    case SBIG('SIZE'):
      fillDesc->x4c_x38_SIZE = GetRealElement(in);
      break;
    case SBIG('SISY'):
      fillDesc->xcc_xb8_SISY = GetIntElement(in);
      break;
    case SBIG('SSPO'):
      fillDesc->xe8_xd4_SSPO = GetVectorElement(in);
      break;
    case SBIG('TEXR'):
      fillDesc->x54_x40_TEXR = GetTextureElement(in, resPool);
      break;
    case SBIG('SSWH'):
      fillDesc->xd4_xc0_SSWH = GetSwooshGeneratorDesc(in, resPool);
      break;
    case SBIG('TIND'):
      fillDesc->x58_x44_TIND = GetTextureElement(in, resPool);
      break;
    case SBIG('VMD4'):
      fillDesc->x45_29_x31_31_VMD4 = GetBool(in);
      break;
    case SBIG('VMD3'):
      fillDesc->x45_28_x31_30_VMD3 = GetBool(in);
      break;
    case SBIG('VMD2'):
      fillDesc->x45_27_x31_29_VMD2 = GetBool(in);
      break;
    case SBIG('VMD1'):
      fillDesc->x45_26_x31_28_VMD1 = GetBool(in);
      break;
    case SBIG('VEL4'):
      fillDesc->x88_x74_VEL4 = GetModVectorElement(in);
      break;
    case SBIG('VEL3'):
      fillDesc->x84_x70_VEL3 = GetModVectorElement(in);
      break;
    case SBIG('VEL2'):
      fillDesc->x80_x6c_VEL2 = GetModVectorElement(in);
      break;
    case SBIG('VEL1'):
      fillDesc->x7c_x68_VEL1 = GetModVectorElement(in);
      break;
    case SBIG('ZBUF'):
      fillDesc->x44_27_x30_27_ZBUF = GetBool(in);
      break;
    case SBIG('WIDT'):
      fillDesc->x24_x18_WIDT = GetRealElement(in);
      break;
    case SBIG('ORNT'):
      fillDesc->x30_30_ORNT = GetBool(in);
      break;
    case SBIG('RSOP'):
      fillDesc->x30_31_RSOP = GetBool(in);
      break;
    case SBIG('ADV1'):
      fillDesc->x10c_ADV1 = GetRealElement(in);
      break;
    case SBIG('ADV2'):
      fillDesc->x110_ADV2 = GetRealElement(in);
      break;
    case SBIG('ADV3'):
      fillDesc->x114_ADV3 = GetRealElement(in);
      break;
    case SBIG('ADV4'):
      fillDesc->x118_ADV4 = GetRealElement(in);
      break;
    case SBIG('ADV5'):
      fillDesc->x11c_ADV5 = GetRealElement(in);
      break;
    case SBIG('ADV6'):
      fillDesc->x120_ADV6 = GetRealElement(in);
      break;
    case SBIG('ADV7'):
      fillDesc->x124_ADV7 = GetRealElement(in);
      break;
    case SBIG('ADV8'):
      fillDesc->x128_ADV8 = GetRealElement(in);
      break;
    case SBIG('SELC'):
      fillDesc->xec_xd8_SELC = GetElectricGeneratorDesc(in, resPool);
      break;
    default: {
      Log.report(logvisor::Fatal, FMT_STRING("Unknown GPSM class {} @{}"), clsId, in.GetReadPosition());
      return false;
    }
    }
    clsId = GetClassID(in);
  }
#if 0
  /* Now for our custom additions, if available */
  if (!in.atEnd() && (in.position() + 4) < in.length()) {
    clsId = GetClassID(in);
    if (clsId == 0xFFFFFFFF)
      return true;

    while (clsId != SBIG('_END') && !in.atEnd()) {
      switch (clsId.toUint32()) {
      case SBIG('BGCL'):
        fillDesc->m_bevelGradient = GetColorElement(in);
        break;
      default:
        break;
      }
      clsId = GetClassID(in);
    }
  }
#endif

  return true;
}

void CParticleDataFactory::LoadGPSMTokens(CGenDescription* desc) {
  desc->x5c_x48_PMDL.Load();
  desc->x8c_x78_ICTS.Load();
  desc->xa4_x90_IDTS.Load();
  desc->xb8_xa4_IITS.Load();
  desc->xd4_xc0_SSWH.Load();
}

CFactoryFnReturn FParticleFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                  CObjectReference* selfRef) {
  auto* const sp = vparms.GetOwnedObj<CSimplePool*>();
  return TToken<CGenDescription>::GetIObjObjectFor(CParticleDataFactory::GetGeneratorDesc(in, sp));
}

} // namespace metaforce
