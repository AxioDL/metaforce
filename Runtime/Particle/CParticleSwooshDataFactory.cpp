#include "CParticleSwooshDataFactory.hpp"
#include "CSwooshDescription.hpp"
#include "CElectricDescription.hpp"
#include "CGenDescription.hpp"
#include "Graphics/CModel.hpp"
#include "CRandom16.hpp"
#include "CSimplePool.hpp"

namespace urde {
static logvisor::Module Log("urde::CParticleSwooshDataFactory");

using CPF = CParticleDataFactory;

CSwooshDescription* CParticleSwooshDataFactory::GetGeneratorDesc(CInputStream& in, CSimplePool* resPool) {
  return CreateGeneratorDescription(in, resPool);
}

CSwooshDescription* CParticleSwooshDataFactory::CreateGeneratorDescription(CInputStream& in, CSimplePool* resPool) {
  FourCC clsId = CPF::GetClassID(in);
  if (clsId == FOURCC('SWSH')) {
    CSwooshDescription* desc = new CSwooshDescription;
    if (CreateWPSM(desc, in, resPool))
      return desc;
    else
      delete desc;
  }
  return nullptr;
}

bool CParticleSwooshDataFactory::CreateWPSM(CSwooshDescription* desc, CInputStream& in, CSimplePool* resPool) {
  CRandom16 rand;
  FourCC clsId = CPF::GetClassID(in);
  while (clsId != SBIG('_END')) {
    CGlobalRandom gr(rand);
    switch (clsId.toUint32()) {
    case SBIG('PSLT'):
      desc->x0_PSLT = CPF::GetIntElement(in);
      break;
    case SBIG('TIME'):
      desc->x4_TIME = CPF::GetRealElement(in);
      break;
    case SBIG('LRAD'):
      desc->x8_LRAD = CPF::GetRealElement(in);
      break;
    case SBIG('RRAD'):
      desc->xc_RRAD = CPF::GetRealElement(in);
      break;
    case SBIG('LENG'):
      desc->x10_LENG = CPF::GetIntElement(in);
      break;
    case SBIG('COLR'):
      desc->x14_COLR = CPF::GetColorElement(in);
      break;
    case SBIG('SIDE'):
      desc->x18_SIDE = CPF::GetIntElement(in);
      break;
    case SBIG('IROT'):
      desc->x1c_IROT = CPF::GetRealElement(in);
      break;
    case SBIG('ROTM'):
      desc->x20_ROTM = CPF::GetRealElement(in);
      break;
    case SBIG('POFS'):
      desc->x24_POFS = CPF::GetVectorElement(in);
      break;
    case SBIG('IVEL'):
      desc->x28_IVEL = CPF::GetVectorElement(in);
      break;
    case SBIG('NPOS'):
      desc->x2c_NPOS = CPF::GetVectorElement(in);
      break;
    case SBIG('VELM'):
      desc->x30_VELM = CPF::GetModVectorElement(in);
      break;
    case SBIG('VLM2'):
      desc->x34_VLM2 = CPF::GetModVectorElement(in);
      break;
    case SBIG('SPLN'):
      desc->x38_SPLN = CPF::GetIntElement(in);
      break;
    case SBIG('TEXR'):
      desc->x3c_TEXR = CPF::GetTextureElement(in, resPool);
      break;
    case SBIG('TSPN'):
      desc->x40_TSPN = CPF::GetIntElement(in);
      break;
    case SBIG('LLRD'):
      desc->x44_24_LLRD = CPF::GetBool(in);
      break;
    case SBIG('CROS'):
      desc->x44_25_CROS = CPF::GetBool(in);
      break;
    case SBIG('VLS1'):
      desc->x44_26_VLS1 = CPF::GetBool(in);
      break;
    case SBIG('VLS2'):
      desc->x44_27_VLS2 = CPF::GetBool(in);
      break;
    case SBIG('SROT'):
      desc->x44_28_SROT = CPF::GetBool(in);
      break;
    case SBIG('WIRE'):
      desc->x44_29_WIRE = CPF::GetBool(in);
      break;
    case SBIG('TEXW'):
      desc->x44_30_TEXW = CPF::GetBool(in);
      break;
    case SBIG('AALP'):
      desc->x44_31_AALP = CPF::GetBool(in);
      break;
    case SBIG('ZBUF'):
      desc->x45_24_ZBUF = CPF::GetBool(in);
      break;
    case SBIG('ORNT'):
      desc->x45_25_ORNT = CPF::GetBool(in);
      break;
    case SBIG('CRND'):
      desc->x45_26_CRND = CPF::GetBool(in);
      break;
    default: {
      Log.report(logvisor::Fatal, fmt("Unknown SWSH class {} @{}"), clsId, in.position());
      return false;
    }
    }
    clsId = CPF::GetClassID(in);
  }

  return true;
}

CFactoryFnReturn FParticleSwooshDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                            CObjectReference*) {
  CSimplePool* sp = vparms.GetOwnedObj<CSimplePool*>();
  return TToken<CSwooshDescription>::GetIObjObjectFor(
      std::unique_ptr<CSwooshDescription>(CParticleSwooshDataFactory::GetGeneratorDesc(in, sp)));
}
} // namespace urde
