#include "Kyoto/Particles/CParticleDataFactory.hpp"

#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Particles/CColorElement.hpp"
#include "Kyoto/Particles/CEmitterElement.hpp"
#include "Kyoto/Particles/CGenDescription.hpp"
#include "Kyoto/Particles/CIntElement.hpp"
#include "Kyoto/Particles/CModVectorElement.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/Particles/CRealElement.hpp"
#include "Kyoto/Particles/CSpawnSystemKeyframeData.hpp"
#include "Kyoto/Particles/CSwooshDescription.hpp"
#include "Kyoto/Particles/CUVElement.hpp"
#include "Kyoto/Particles/CVectorElement.hpp"
#include "Kyoto/Particles/IElement.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/TToken.hpp"
#include "dolphin/types.h"
#include "rstl/algorithm.hpp"
#include "rstl/list.hpp"
#include "rstl/optional_object.hpp"
#define SBIG(v) v

CTexture* CreateTexture(int value);

rstl::list< CElementAllocationChunk > sElementAllocationChunks;
CElementAllocationChunk* IElement::CElementAllocator::sCurrentChunk = nullptr;
CElementAllocationChunk* IElement::CElementAllocator::sFreeChunk = nullptr;

void* IElement::CElementAllocator::Alloc(size_t size, const char*, const char*) {
  if (sCurrentChunk == nullptr || !sCurrentChunk->CanAllocate(size)) {
    sElementAllocationChunks.push_back(CElementAllocationChunk());
    sCurrentChunk = &sElementAllocationChunks.back();
  }

  return sCurrentChunk->Allocate(size);
}

void IElement::CElementAllocator::Free(void* ptr, size_t) {
  if (ptr == nullptr) {
    return;
  }

  if (sFreeChunk == nullptr || !sFreeChunk->Contains(ptr)) {
    sFreeChunk = nullptr;
    for (AUTO(it, sElementAllocationChunks.begin()); it != sElementAllocationChunks.end(); ++it) {
      if (it->Contains(ptr)) {
        sFreeChunk = &*it;
        break;
      }
    }
  }

  sFreeChunk->Free(ptr);
  if (sFreeChunk->GetAllocationCount() == 0) {
    for (AUTO(it, sElementAllocationChunks.begin()); it != sElementAllocationChunks.end(); ++it) {
      if (&*it == sFreeChunk) {
        sElementAllocationChunks.erase(it);
        if (sCurrentChunk == sFreeChunk) {
          sCurrentChunk = nullptr;
        }
        break;
      }
    }
    sFreeChunk = nullptr;
  }
}

const CFactoryFnReturn FParticleFactory(const SObjectTag& tag, CInputStream& in,
                                  const CVParamTransfer& xfer) {
  rstl::rc_ptr< IVParamObj > obj = xfer.x0_obj;
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  CGenDescription* desc = CParticleDataFactory::GetGeneratorDesc(in, pool, tag.GetId());
  return desc;
}

CGenDescription* CParticleDataFactory::GetGeneratorDesc(CInputStream& in, CSimplePool* pool,
                                                        uint id) {
  rstl::vector< uint > assets;
  assets.reserve(8);
  return CParticleDataFactory::CreateGeneratorDescription(in, assets, id, pool);
}

CGenDescription* CParticleDataFactory::CreateGeneratorDescription(CInputStream& in,
                                                                  rstl::vector< uint >& assets,
                                                                  const uint id,
                                                                  CSimplePool* pool) {
  if (rstl::count(assets.begin(), assets.end(), id) != 0) {
    return nullptr;
  }
  assets.push_back(id);
  FourCC clsId = GetClassID(in);
  if (clsId != SBIG('GPSM')) {
    return nullptr;
  }
  CGenDescription* desc = rs_new CGenDescription;
  CreateGPSM(desc, in, assets, pool);
  LoadGPSMTokens(desc);
  return desc;
}

void CParticleDataFactory::LoadGPSMTokens(CGenDescription* desc) {
  if (desc->x48_PMDL) {
    desc->x48_PMDL->ForceCache();
  }
  if (desc->x78_ICTS) {
    desc->x78_ICTS->ForceCache();
  }
  if (desc->x90_IDTS) {
    desc->x90_IDTS->ForceCache();
  }
  if (desc->xa4_IITS) {
    desc->xa4_IITS->ForceCache();
  }
  if (desc->xc0_SSWH) {
    desc->xc0_SSWH->ForceCache();
  }
}

bool CParticleDataFactory::CreateGPSM(CGenDescription* desc, CInputStream& in,
                                      rstl::vector< CAssetId >& resources, CSimplePool* pool) {
  bool done = false;
  CRandom16 random(99);
  CGlobalRandom context(random);
  while (!done) {
    FourCC clsId = GetClassID(in);
    switch (clsId) {
    case SBIG('PSIV'):
      delete GetVectorElement(in);
      break;
    case SBIG('PSVM'):
      delete GetModVectorElement(in);
      break;
    case SBIG('PSOV'):
      delete GetVectorElement(in);
      break;
    case SBIG('PSTS'):
      desc->x8_PSTS = GetRealElement(in);
      break;
    case SBIG('PSLT'):
      desc->x0_PSLT = GetIntElement(in);
      break;
    case SBIG('PSWT'):
      desc->x4_PSWT = GetIntElement(in);
      break;
    case SBIG('LIT_'):
      desc->x30_29_LIT_ = GetBool(in);
      break;
    case SBIG('ORNT'):
      desc->x30_30_ORNT = GetBool(in);
      break;
    case SBIG('RSOP'):
      desc->x30_31_RSOP = GetBool(in);
      break;
    case SBIG('AAPH'):
      desc->x30_26_AAPH = GetBool(in);
      break;
    case SBIG('ZBUF'):
      desc->x30_27_ZBUF = GetBool(in);
      break;
    case SBIG('SORT'):
      desc->x30_28_SORT = GetBool(in);
      break;
    case SBIG('MBLR'):
      desc->x31_24_MBLR = GetBool(in);
      break;
    case SBIG('MBSP'):
      desc->x34_MBSP = GetIntElement(in);
      break;
    case SBIG('MAXP'):
      desc->x1c_MAXP = GetIntElement(in);
      break;
    case SBIG('GRTE'):
      desc->x20_GRTE = GetRealElement(in);
      break;
    case SBIG('ILOC'):
      delete GetVectorElement(in);
      break;
    case SBIG('IVEC'):
      delete GetVectorElement(in);
      break;
    case SBIG('EMTR'):
      desc->x2c_EMTR = GetEmitterElement(in);
      break;
    case SBIG('SIZE'):
      desc->x38_SIZE = GetRealElement(in);
      break;
    case SBIG('COLR'):
      desc->x24_COLR = GetColorElement(in);
      break;
    case SBIG('POFS'):
      desc->xc_POFS = GetVectorElement(in);
      break;
    case SBIG('VMD1'):
      desc->x31_28_VMD1 = GetBool(in);
      break;
    case SBIG('VMD2'):
      desc->x31_29_VMD2 = GetBool(in);
      break;
    case SBIG('VMD3'):
      desc->x31_30_VMD3 = GetBool(in);
      break;
    case SBIG('VMD4'):
      desc->x31_31_VMD4 = GetBool(in);
      break;
    case SBIG('VEL1'):
      desc->x68_VEL1 = GetModVectorElement(in);
      break;
    case SBIG('VEL2'):
      desc->x6c_VEL2 = GetModVectorElement(in);
      break;
    case SBIG('VEL3'):
      desc->x70_VEL3 = GetModVectorElement(in);
      break;
    case SBIG('VEL4'):
      desc->x74_VEL4 = GetModVectorElement(in);
      break;
    case SBIG('LTME'):
      desc->x28_LTME = GetIntElement(in);
      break;
    case SBIG('ROTA'):
      desc->x3c_ROTA = GetRealElement(in);
      break;
    case SBIG('LENG'):
      desc->x14_LENG = GetRealElement(in);
      break;
    case SBIG('WIDT'):
      desc->x18_WIDT = GetRealElement(in);
      break;
    case SBIG('TEXR'):
      desc->x40_TEXR = GetTextureElement(in, pool);
      break;
    case SBIG('TIND'):
      desc->x44_TIND = GetTextureElement(in, pool);
      break;
    case SBIG('CIND'):
      desc->x32_24_CIND = GetBool(in);
      break;
    case SBIG('PMDL'): {
      rstl::optional_object< TToken< CModel > > model(GetModel(in, pool));
      if (model) {
        desc->x48_PMDL = TCachedToken< CModel >(*model);
      } else {
        desc->x48_PMDL = rstl::optional_object_null();
      }
    } break;
    case SBIG('PMOP'):
      desc->x58_PMOP = GetVectorElement(in);
      break;
    case SBIG('PMRT'):
      desc->x5c_PMRT = GetVectorElement(in);
      break;
    case SBIG('PMSC'):
      desc->x60_PMSC = GetVectorElement(in);
      break;
    case SBIG('PMCL'):
      desc->x64_PMCL = GetColorElement(in);
      break;
    case SBIG('PMAB'):
      desc->x31_25_PMAB = GetBool(in);
      break;
    case SBIG('PMUS'):
      desc->x31_26_PMUS = GetBool(in);
      break;
    case SBIG('PMOO'):
      desc->x31_27_PMOO = GetBool(in);
      break;
    case SBIG('SEED'):
      desc->x10_SEED = GetIntElement(in);
      break;
    case SBIG('ICTS'): {
      rstl::optional_object< TToken< CGenDescription > > child(
          GetChildGeneratorDesc(in, pool, resources));
      if (child) {
        desc->x78_ICTS = TCachedToken< CGenDescription >(*child);
      } else {
        desc->x78_ICTS = rstl::optional_object_null();
      }
      break;
    }
    case SBIG('NCSY'):
      desc->x88_NCSY = GetIntElement(in);
      break;
    case SBIG('CSSD'):
      desc->x8c_CSSD = GetIntElement(in);
      break;
    case SBIG('IDTS'): {
      rstl::optional_object< TToken< CGenDescription > > child(
          GetChildGeneratorDesc(in, pool, resources));
      if (child) {
        desc->x90_IDTS = TCachedToken< CGenDescription >(*child);
      } else {
        desc->x90_IDTS = rstl::optional_object_null();
      }
      break;
    }
    case SBIG('NDSY'):
      desc->xa0_NDSY = GetIntElement(in);
      break;
    case SBIG('IITS'): {
      rstl::optional_object< TToken< CGenDescription > > child(
          GetChildGeneratorDesc(in, pool, resources));
      if (child) {
        desc->xa4_IITS = TCachedToken< CGenDescription >(*child);
      } else {
        desc->xa4_IITS = rstl::optional_object_null();
      }
      break;
    }
    case SBIG('PISY'):
      desc->xb4_PISY = GetIntElement(in);
      break;
    case SBIG('SISY'):
      desc->xb8_SISY = GetIntElement(in);
      break;
    case SBIG('SSWH'): {
      rstl::optional_object< TToken< CSwooshDescription > > child(GetSwooshGeneratorDesc(in, pool));
      if (child) {
        desc->xc0_SSWH = TCachedToken< CSwooshDescription >(*child);
      } else {
        desc->xc0_SSWH = rstl::optional_object_null();
      }
      break;
    }
    case SBIG('SSSD'):
      desc->xd0_SSSD = GetIntElement(in);
      break;
    case SBIG('SSPO'):
      desc->xd4_SSPO = GetVectorElement(in);
      break;
    case SBIG('SELC'): {
      rstl::optional_object< TToken< CElectricDescription > > electric =
          GetElectricGeneratorDesc(in, pool);
      if (electric) {
        desc->xd8_SELC = *electric;
      } else {
        desc->xd8_SELC = rstl::optional_object_null();
      }
      break;
    }
    case SBIG('SESD'):
      desc->xe4_SESD = GetIntElement(in);
      break;
    case SBIG('SEPO'):
      desc->xe8_SEPO = GetVectorElement(in);
      break;
    case SBIG('KSSM'):
      if (GetClassID(in) == SBIG('CNST')) {
        desc->xbc_KSSM = rs_new CSpawnSystemKeyframeData(in);
        desc->xbc_KSSM->LoadAllSpawnedSystemTokens(pool);
      }
      break;
    case SBIG('LINE'):
      desc->x30_24_LINE = GetBool(in);
      break;
    case SBIG('FXLL'):
      desc->x30_25_FXLL = GetBool(in);
      break;
    case SBIG('LTYP'):
      desc->xec_LTYP = GetIntElement(in);
      break;
    case SBIG('LCLR'):
      desc->xf0_LCLR = GetColorElement(in);
      break;
    case SBIG('LINT'):
      desc->xf4_LINT = GetRealElement(in);
      break;
    case SBIG('LOFF'):
      desc->xf8_LOFF = GetVectorElement(in);
      break;
    case SBIG('LDIR'):
      desc->xfc_LDIR = GetVectorElement(in);
      break;
    case SBIG('LFOT'):
      desc->x100_LFOT = GetIntElement(in);
      break;
    case SBIG('LFOR'):
      desc->x104_LFOR = GetRealElement(in);
      break;
    case SBIG('LSLA'):
      desc->x108_LSLA = GetRealElement(in);
      break;
    case SBIG('OPTS'):
      desc->x32_25_OPTS = GetBool(in);
      break;
    case SBIG('ADV1'):
      desc->x10c_ADV1 = GetRealElement(in);
      break;
    case SBIG('ADV2'):
      desc->x110_ADV2 = GetRealElement(in);
      break;
    case SBIG('ADV3'):
      desc->x114_ADV3 = GetRealElement(in);
      break;
    case SBIG('ADV4'):
      desc->x118_ADV4 = GetRealElement(in);
      break;
    case SBIG('ADV5'):
      desc->x11c_ADV5 = GetRealElement(in);
      break;
    case SBIG('ADV6'):
      desc->x120_ADV6 = GetRealElement(in);
      break;
    case SBIG('ADV7'):
      desc->x124_ADV7 = GetRealElement(in);
      break;
    case SBIG('ADV8'):
      desc->x128_ADV8 = GetRealElement(in);
      break;
    case SBIG('_END'):
      done = true;
      break;
    default:
      return false;
    }
  }

  return true;
}

FourCC CParticleDataFactory::GetClassID(CInputStream& in) { return in.ReadLong(); }

bool CParticleDataFactory::GetBool(CInputStream& in) {
  GetClassID(in);
  return in.ReadBool();
}

int CParticleDataFactory::GetInt(CInputStream& in) { return in.ReadInt32(); }

float CParticleDataFactory::GetReal(CInputStream& in) { return in.ReadFloat(); }

CIntElement* CParticleDataFactory::GetIntElement(CInputStream& in) {
  FourCC clsId = GetClassID(in);
  switch (clsId) {
  case SBIG('CNST'): {
    return rs_new CIEConstant(GetInt(in));
  }
  case SBIG('KEYE'):
  case SBIG('KEYP'): {
    return rs_new CIEKeyframeEmitter(in);
  }
  case SBIG('TSCL'): {
    return rs_new CIETimescale(GetRealElement(in));
  }
  case SBIG('DETH'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new CIEDeath(a, b);
  }
  case SBIG('CHAN'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    CIntElement* c = GetIntElement(in);
    return rs_new CIETimeChain(a, b, c);
  }
  case SBIG('ADD_'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new CIEAdd(a, b);
  }
  case SBIG('MULT'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new CIEMultiply(a, b);
  }
  case SBIG('MODU'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new CIEModulo(a, b);
  }
  case SBIG('RAND'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new CIERandom(a, b);
  }
  case SBIG('IMPL'): {
    return rs_new CIEImpulse(GetIntElement(in));
  }
  case SBIG('ILPT'): {
    return rs_new CIELifetimePercent(GetIntElement(in));
  }
  case SBIG('SPAH'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    CIntElement* c = GetIntElement(in);
    return rs_new CIESampleAndHold(c, a, b);
  }
  case SBIG('IRND'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new CIEInitialRandom(a, b);
  }
  case SBIG('CLMP'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    CIntElement* c = GetIntElement(in);
    return rs_new CIEClamp(a, b, c);
  }
  case SBIG('PULS'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    CIntElement* c = GetIntElement(in);
    CIntElement* d = GetIntElement(in);
    return rs_new CIEPulse(a, b, c, d);
  }
  case SBIG('NONE'): {
    return nullptr;
  }
  case SBIG('RTOI'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new CIERealToInt(a, b);
  }
  case SBIG('SUB_'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    return rs_new CIESubtract(a, b);
  }
  case SBIG('GTCP'): {
    return rs_new CIEGetCumulativeParticleCount();
  }
  case SBIG('GAPC'): {
    return rs_new CIEGetActiveParticleCount();
  }
  case SBIG('GEMT'): {
    return rs_new CIEGetEmitterTime();
  }
  }
  return nullptr;
}

CRealElement* CParticleDataFactory::GetRealElement(CInputStream& in) {
  FourCC clsId = GetClassID(in);
  switch (clsId) {
  case SBIG('CNST'): {
    return rs_new CREConstant(GetReal(in));
  }
  case SBIG('NONE'): {
    return nullptr;
  }
  case SBIG('KEYE'):
  case SBIG('KEYP'): {
    return rs_new CREKeyframeEmitter(in);
  }
  case SBIG('SCAL'): {
    return rs_new CRETimeScale(GetRealElement(in));
  }
  case SBIG('SINE'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    return rs_new CRESineWave(c, a, b);
  }
  case SBIG('ADD_'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new CREAdd(a, b);
  }
  case SBIG('MULT'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new CREMultiply(a, b);
  }
  case SBIG('DOTP'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    return rs_new CREDotProduct(a, b);
  }
  case SBIG('RAND'): {
    CRealElement* min = GetRealElement(in);
    CRealElement* max = GetRealElement(in);
    return rs_new CRERandom(min, max);
  }
  case SBIG('IRND'): {
    CRealElement* min = GetRealElement(in);
    CRealElement* max = GetRealElement(in);
    return rs_new CREInitialRandom(min, max);
  }
  case SBIG('CHAN'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CIntElement* c = GetIntElement(in);
    return rs_new CRETimeChain(a, b, c);
  }
  case SBIG('CLMP'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    return rs_new CREClamp(a, b, c);
  }
  case SBIG('PULS'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    return rs_new CREPulse(a, b, c, d);
  }
  case SBIG('RLPT'): {
    CRealElement* a = GetRealElement(in);
    return rs_new CRELifetimePercent(a);
  }
  case SBIG('LFTW'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new CRELifetimeTween(a, b);
  }
  case SBIG('PRLW'): {
    return rs_new CREParticleRotationOrLineWidth();
  }
  case SBIG('PSLL'): {
    return rs_new CREParticleSizeOrLineLength();
  }
  case SBIG('PAP1'): {
    return rs_new CREParticleAccessParameter1();
  }
  case SBIG('PAP2'): {
    return rs_new CREParticleAccessParameter2();
  }
  case SBIG('PAP3'): {
    return rs_new CREParticleAccessParameter3();
  }
  case SBIG('PAP4'): {
    return rs_new CREParticleAccessParameter4();
  }
  case SBIG('PAP5'): {
    return rs_new CREParticleAccessParameter5();
  }
  case SBIG('PAP6'): {
    return rs_new CREParticleAccessParameter6();
  }
  case SBIG('PAP7'): {
    return rs_new CREParticleAccessParameter7();
  }
  case SBIG('PAP8'): {
    return rs_new CREParticleAccessParameter8();
  }
  case SBIG('VXTR'): {
    CVectorElement* a = GetVectorElement(in);
    return rs_new CREVectorXToReal(a);
  }
  case SBIG('VYTR'): {
    CVectorElement* a = GetVectorElement(in);
    return rs_new CREVectorYToReal(a);
  }
  case SBIG('VZTR'): {
    CVectorElement* a = GetVectorElement(in);
    return rs_new CREVectorZToReal(a);
  }
  case SBIG('VMAG'): {
    CVectorElement* a = GetVectorElement(in);
    return rs_new CREVectorMagnitude(a);
  }
  case SBIG('ISWT'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new CREInitialSwitch(a, b);
  }
  case SBIG('CLTN'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    return rs_new CRECompareLessThan(a, b, c, d);
  }
  case SBIG('CEQL'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    return rs_new CRECompareEqual(a, b, c, d);
  }
  case SBIG('CRNG'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    CRealElement* e = GetRealElement(in);
    return rs_new CREConstantRange(a, b, c, d, e);
  }
  case SBIG('CEXT'): {
    CIntElement* a = GetIntElement(in);
    return rs_new CREExternalVar(a);
  }
  case SBIG('ITRL'): {
    CIntElement* a = GetIntElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new CREIntTimesReal(a, b);
  }
  case SBIG('SUB_'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    return rs_new CRESubtract(a, b);
  }
  case SBIG('GTCR'): {
    CColorElement* alpha = GetColorElement(in);
    return rs_new CREGetComponentRed(alpha);
  }
  case SBIG('GTCG'): {
    CColorElement* alpha = GetColorElement(in);
    return rs_new CREGetComponentGreen(alpha);
  }
  case SBIG('GTCB'): {
    CColorElement* alpha = GetColorElement(in);
    return rs_new CREGetComponentBlue(alpha);
  }
  case SBIG('GTCA'): {
    CColorElement* alpha = GetColorElement(in);
    return rs_new CREGetComponentAlpha(alpha);
  }
  }
  return nullptr;
}

CVectorElement* CParticleDataFactory::GetVectorElement(CInputStream& in) {
  CVectorElement* ret;
  FourCC clsId = GetClassID(in);
  switch (clsId) {
  case SBIG('NONE'):
    ret = nullptr;
    break;
  case SBIG('CNST'): {
    CElementAllocationChunk* allocationContext = IElement::CElementAllocator::GetCurrentChunk();
    uint initialSize = IElement::CElementAllocator::GetCurrentAllocatedSize();
    CRealElement* x = GetRealElement(in);
    CRealElement* y = GetRealElement(in);
    CRealElement* z = GetRealElement(in);
    if (x && y && z) {
      if (x->IsConstant() && y->IsConstant() && z->IsConstant()) {
        float xf, yf, zf;
        x->GetValue(0, xf);
        y->GetValue(0, yf);
        z->GetValue(0, zf);

        delete x;
        delete y;
        delete z;

        if (allocationContext != nullptr &&
            allocationContext == IElement::CElementAllocator::GetCurrentChunk()) {
          allocationContext->Rewind(allocationContext->GetAllocatedSize() - initialSize);
        }
        ret = rs_new CVEFastConstant(xf, yf, zf);
        break;
      }
    }
    ret = rs_new CVEConstant(x, y, z);
    break;
  }
  case SBIG('KEYE'):
  case SBIG('KEYP'):
    ret = rs_new CVEKeyframeEmitter(in);
    break;
  case SBIG('ANGC'): {
    CRealElement* angleXBias = GetRealElement(in);
    CRealElement* angleYBias = GetRealElement(in);
    CRealElement* angleXRange = GetRealElement(in);
    CRealElement* angleYRange = GetRealElement(in);
    CRealElement* magnitude = GetRealElement(in);
    ret = rs_new CVEAngleCone(angleXBias, angleYBias, angleXRange, angleYRange, magnitude);
    break;
  }
  case SBIG('CONE'): {
    CVectorElement* direction = GetVectorElement(in);
    CRealElement* baseRadius = GetRealElement(in);
    ret = rs_new CVECone(direction, baseRadius);
    break;
  }
  case SBIG('CIRC'): {
    CVectorElement* circleOffset = GetVectorElement(in);
    CVectorElement* circleNormal = GetVectorElement(in);
    CRealElement* angleConstant = GetRealElement(in);
    CRealElement* angleLinear = GetRealElement(in);
    CRealElement* radius = GetRealElement(in);
    ret = rs_new CVECircle(circleOffset, circleNormal, angleConstant, angleLinear, radius);
    break;
  }
  case SBIG('CCLU'): {
    CVectorElement* circleOffset = GetVectorElement(in);
    CVectorElement* circleNormal = GetVectorElement(in);
    CIntElement* cycleFrames = GetIntElement(in);
    CRealElement* randomFactor = GetRealElement(in);

    ret = rs_new CVECircleCluster(circleOffset, circleNormal, cycleFrames, randomFactor);
    break;
  }
  case SBIG('ADD_'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    ret = rs_new CVEAdd(a, b);
    break;
  }
  case SBIG('MULT'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    ret = rs_new CVEMultiply(a, b);
    break;
  }
  case SBIG('CHAN'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    CIntElement* switchFrame = GetIntElement(in);

    ret = rs_new CVETimeChain(a, b, switchFrame);
    break;
  }
  case SBIG('PULS'): {
    CIntElement* durationA = GetIntElement(in);
    CIntElement* durationB = GetIntElement(in);
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    ret = rs_new CVEPulse(durationA, durationB, a, b);
    break;
  }
  case SBIG('RTOV'): {
    CRealElement* value = GetRealElement(in);
    ret = rs_new CVERealToVector(value);
    break;
  }
  case SBIG('PLOC'): {
    ret = rs_new CVEParticleLocation();
    break;
  }
  case SBIG('PLCO'): {
    ret = rs_new CVEParticlePreviousLocation();
    break;
  }
  case SBIG('PVEL'): {
    ret = rs_new CVEParticleVelocity();
    break;
  }
  case SBIG('PSOF'): {
    ret = rs_new CVEParticleSystemOrientationFront();
    break;
  }
  case SBIG('PSOU'): {
    ret = rs_new CVEParticleSystemOrientationUp();
    break;
  }
  case SBIG('PSOR'): {
    ret = rs_new CVEParticleSystemOrientationRight();
    break;
  }
  case SBIG('PSTR'): {
    ret = rs_new CVEParticleSystemTranslation();
    break;
  }
  case SBIG('SUB_'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    ret = rs_new CVESubtract(a, b);
    break;
  }
  case SBIG('CTVC'): {
    CColorElement* value = GetColorElement(in);
    ret = rs_new CVEColorToVector(value);
    break;
  }
  default:
    ret = nullptr;
    break;
  }
  return ret;
}

CEmitterElement* CParticleDataFactory::GetEmitterElement(CInputStream& in) {
  CEmitterElement* ret;
  FourCC clsId = GetClassID(in);
  switch (clsId) {
  case SBIG('NONE'):
    ret = nullptr;
    break;
  case SBIG('SETR'): {
    FourCC prop = GetClassID(in);
    CVectorElement* pos = nullptr;
    CVectorElement* vel = nullptr;
    bool valid = false;
    if (prop == SBIG('ILOC')) {
      pos = GetVectorElement(in);
      prop = GetClassID(in);
      if (prop == SBIG('IVEC')) {
        vel = GetVectorElement(in);
        valid = true;
      }
    }
    ret = valid ? rs_new CEESimpleEmitter(pos, vel) : nullptr;
    break;
  }
  case SBIG('SEMR'): {
    CVectorElement* pos = GetVectorElement(in);
    CVectorElement* vel = GetVectorElement(in);
    ret = rs_new CEESimpleEmitter(pos, vel);
    break;
  }
  case SBIG('SPHE'): {
    CVectorElement* origin = GetVectorElement(in);
    CRealElement* radius = GetRealElement(in);
    CRealElement* velocity = GetRealElement(in);
    ret = rs_new CVESphere(origin, radius, velocity);
    break;
  }
  case SBIG('ASPH'): {
    CVectorElement* origin = GetVectorElement(in);
    CRealElement* angleXBias = GetRealElement(in);
    CRealElement* angleYBias = GetRealElement(in);
    CRealElement* angleXRange = GetRealElement(in);
    CRealElement* angleYRange = GetRealElement(in);
    CRealElement* radius = GetRealElement(in);
    CRealElement* velocity = GetRealElement(in);
    ret = rs_new CVEAngleSphere(origin, radius, velocity, angleXBias, angleYBias, angleXRange,
                                angleYRange);
    break;
  }
  default:
    ret = nullptr;
    break;
  }
  return ret;
}

CModVectorElement* CParticleDataFactory::GetModVectorElement(CInputStream& in) {
  CModVectorElement* ret;
  FourCC clsId = GetClassID(in);
  switch (clsId) {
  case SBIG('NONE'): {
    ret = nullptr;
    break;
  }
  case SBIG('CNST'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    if (a && b && c && a->IsConstant() && b->IsConstant() && c->IsConstant()) {
      float af, bf, cf;
#if NONMATCHING
      a->GetValue(0, af);
      b->GetValue(0, bf);
      c->GetValue(0, cf);
#else
      // BUG: Fetching from the same element for each
      a->GetValue(0, af);
      a->GetValue(0, bf);
      a->GetValue(0, cf);
#endif
      ret = rs_new CMVEFastConstant(af, bf, cf);
      delete a;
      delete b;
      delete c;
    } else {
      ret = rs_new CMVEConstant(a, b, c);
    }
    break;
  }
  case SBIG('GRAV'): {
    ret = rs_new CMVEGravity(GetVectorElement(in));
    break;
  }
  case SBIG('WIND'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    ret = rs_new CMVEWind(a, b);
    break;
  }
  case SBIG('EXPL'): {
    CRealElement* a = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    ret = rs_new CMVEExplode(a, b);
    break;
  }
  case SBIG('CHAN'): {
    CModVectorElement* a = GetModVectorElement(in);
    CModVectorElement* b = GetModVectorElement(in);
    CIntElement* c = GetIntElement(in);
    ret = rs_new CMVETimeChain(a, b, c);
    break;
  }
  case SBIG('PULS'): {
    CIntElement* a = GetIntElement(in);
    CIntElement* b = GetIntElement(in);
    CModVectorElement* c = GetModVectorElement(in);
    CModVectorElement* d = GetModVectorElement(in);
    ret = rs_new CMVEPulse(a, b, c, d);
    break;
  }
  case SBIG('IMPL'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    bool e = GetBool(in);
    ret = rs_new CMVEImplosion(a, b, c, d, e);
    break;
  }
  case SBIG('LMPL'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    bool e = GetBool(in);
    ret = rs_new CMVELinearImplosion(a, b, c, d, e);
    break;
  }
  case SBIG('EMPL'): {
    CVectorElement* a = GetVectorElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    bool e = GetBool(in);
    ret = rs_new CMVEExponentialImplosion(a, b, c, d, e);
    break;
  }
  case SBIG('SWRL'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    ret = rs_new CMVESwirl(a, b, c, d);
    break;
  }
  case SBIG('BNCE'): {
    CVectorElement* a = GetVectorElement(in);
    CVectorElement* b = GetVectorElement(in);
    CRealElement* c = GetRealElement(in);
    CRealElement* d = GetRealElement(in);
    bool e = GetBool(in);
    ret = rs_new CMVEBounce(a, b, c, d, e);
    break;
  }
  case SBIG('SPOS'): {
    ret = rs_new CMVESetPosition(GetVectorElement(in));
    break;
  }
  default:
    return nullptr;
    break;
  }
  return ret;
}

CColorElement* CParticleDataFactory::GetColorElement(CInputStream& in) {
  CColorElement* ret;
  FourCC clsId = GetClassID(in);
  switch (clsId) {
  case SBIG('CNST'): {
    CElementAllocationChunk* allocationContext = IElement::CElementAllocator::GetCurrentChunk();
    uint initialSize = IElement::CElementAllocator::GetCurrentAllocatedSize();
    CRealElement* r = GetRealElement(in);
    CRealElement* g = GetRealElement(in);
    CRealElement* b = GetRealElement(in);
    CRealElement* a = GetRealElement(in);
    if (r && g && b && a) {
      if (r->IsConstant() && g->IsConstant() && b->IsConstant() && a->IsConstant()) {
        float rf, gf, bf, af;
        r->GetValue(0, rf);
        g->GetValue(0, gf);
        b->GetValue(0, bf);
        a->GetValue(0, af);

        delete r;
        delete g;
        delete b;
        delete a;

        if (allocationContext != nullptr &&
            allocationContext == IElement::CElementAllocator::GetCurrentChunk()) {
          allocationContext->Rewind(allocationContext->GetAllocatedSize() - initialSize);
        }
        ret = rs_new CCEFastConstant(rf, gf, bf, af);
        break;
      }
    }
    ret = rs_new CCEConstant(r, g, b, a);
    break;
  }
  case SBIG('KEYE'):
  case SBIG('KEYP'):
    ret = rs_new CCEKeyframeEmitter(in);
    break;
  case SBIG('FADE'): {
    CColorElement* a = GetColorElement(in);
    CColorElement* b = GetColorElement(in);
    CRealElement* end = GetRealElement(in);
    ret = rs_new CCEFade(a, b, end);
    break;
  }
  case SBIG('CFDE'): {
    CColorElement* a = GetColorElement(in);
    CColorElement* b = GetColorElement(in);
    CRealElement* start = GetRealElement(in);
    CRealElement* end = GetRealElement(in);
    ret = rs_new CCEFadeEnd(a, b, start, end);
    break;
  }
  case SBIG('CHAN'): {
    CColorElement* a = GetColorElement(in);
    CColorElement* b = GetColorElement(in);
    CIntElement* frame = GetIntElement(in);
    ret = rs_new CCETimeChain(a, b, frame);
    break;
  }
  case SBIG('PULS'): {
    CIntElement* aDuration = GetIntElement(in);
    CIntElement* bDuration = GetIntElement(in);
    CColorElement* a = GetColorElement(in);
    CColorElement* b = GetColorElement(in);
    ret = rs_new CCEPulse(aDuration, bDuration, a, b);
    break;
  }
  case SBIG('PCOL'):
    ret = rs_new CCEParticleColor();
    break;
  case SBIG('NONE'):
    ret = nullptr;
    break;
  default:
    ret = nullptr;
    break;
  }
  return ret;
}

CUVElement* CParticleDataFactory::GetTextureElement(CInputStream& in, CSimplePool* resPool) {
  CUVElement* ret;
  FourCC clsId = GetClassID(in);
  switch (clsId) {
  case SBIG('NONE'):
    ret = nullptr;
    break;
  case SBIG('CNST'): {
    CAssetId id = 0;
    FourCC subId = GetClassID(in);
    if (subId != SBIG('NONE')) {
      id = in.ReadLong();
    }
    if (id == 0) {
      TToken< CTexture > tex = CreateTexture(-1);
      ret = rs_new CUVEConstant(tex);
    } else {
      TToken< CTexture > tex = resPool->GetObj(SObjectTag(SBIG('TXTR'), id));
      ret = rs_new CUVEConstant(tex);
    }
    break;
  }
  case SBIG('ATEX'): {
    CAssetId id = 0;
    FourCC subId = GetClassID(in);
    if (subId != SBIG('NONE')) {
      id = in.ReadLong();
    }
    CIntElement* tileW = GetIntElement(in);
    CIntElement* tileH = GetIntElement(in);
    CIntElement* strideW = GetIntElement(in);
    CIntElement* strideH = GetIntElement(in);
    CIntElement* cycleFrames = GetIntElement(in);
    bool loop = GetBool(in);
    if (id == 0) {
      TToken< CTexture > tex = CreateTexture(-1);
      ret = rs_new CUVEAnimTexture(tex, tileW, tileH, strideW, strideH, cycleFrames, loop);
    } else {
      TToken< CTexture > tex = resPool->GetObj(SObjectTag(SBIG('TXTR'), id));
      ret = rs_new CUVEAnimTexture(tex, tileW, tileH, strideW, strideH, cycleFrames, loop);
    }
    break;
  }
  default:
    return nullptr;
  }
  return ret;
}

rstl::optional_object< TToken< CGenDescription > >
CParticleDataFactory::GetChildGeneratorDesc(CInputStream& in, CSimplePool* pool,
                                            const rstl::vector< CAssetId >& resources) {
  FourCC clsId = GetClassID(in);
  CAssetId id;
  if (clsId != SBIG('NONE')) {
    id = in.Get< CAssetId >();
  } else {
    return rstl::optional_object_null();
  }
  if (id == 0) {
    return rstl::optional_object_null();
  }
  return GetChildGeneratorDesc(id, pool, resources);
}

rstl::optional_object< TToken< CGenDescription > >
CParticleDataFactory::GetChildGeneratorDesc(CAssetId id, CSimplePool* pool,
                                            const rstl::vector< CAssetId >& resources) {
  if (rstl::count(resources.begin(), resources.end(), id) == 0) {
    return TToken< CGenDescription >(pool->GetObj(SObjectTag(CParticleGen::ResType(), id)));
  }
  return rstl::optional_object_null();
}

rstl::optional_object< TToken< CSwooshDescription > >
CParticleDataFactory::GetSwooshGeneratorDesc(CInputStream& in, CSimplePool* pool) {
  FourCC clsId = GetClassID(in);
  CAssetId id;
  if (clsId != SBIG('NONE')) {
    id = in.Get< CAssetId >();
  } else {
    return rstl::optional_object_null();
  }
  if (id == 0) {
    return rstl::optional_object_null();
  }
  return TToken< CSwooshDescription >(pool->GetObj(SObjectTag(SBIG('SWHC'), id)));
}

rstl::optional_object< TToken< CElectricDescription > >
CParticleDataFactory::GetElectricGeneratorDesc(CInputStream& in, CSimplePool* pool) {
  FourCC clsId = GetClassID(in);
  CAssetId id;
  if (clsId != SBIG('NONE')) {
    id = in.Get< CAssetId >();
  } else {
    return rstl::optional_object_null();
  }
  if (id == 0) {
    return rstl::optional_object_null();
  }
  return TToken< CElectricDescription >(pool->GetObj(SObjectTag(SBIG('ELSC'), id)));
}

rstl::optional_object< TToken< CModel > > CParticleDataFactory::GetModel(CInputStream& in,
                                                                         CSimplePool* pool) {
  FourCC clsId = GetClassID(in);
  CAssetId id;
  if (clsId != SBIG('NONE')) {
    id = in.Get< CAssetId >();
  } else {
    return rstl::optional_object_null();
  }
  if (id == 0) {
    return rstl::optional_object_null();
  }
  return TToken< CModel >(pool->GetObj(SObjectTag(SBIG('CMDL'), id)));
}

CTexture* CreateTexture(int value) {
  CTexture* texture = rs_new CTexture(kTF_RGBA8, 4, 4, 1);
  int* data = static_cast< int* >(texture->Lock());
  for (int i = 1; i <= 16; ++i) {
    data[i - 1] = value;
  }
  texture->UnLock();
  return texture;
}
