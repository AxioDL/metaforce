#include "Kyoto/Particles/CParticleSwooshDataFactory.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

const CFactoryFnReturn FParticleSwooshDataFactory(const SObjectTag& tag, CInputStream& in,
                                            const CVParamTransfer& transfer) {
  rstl::rc_ptr< IVParamObj > obj = transfer.x0_obj;
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  return CParticleSwooshDataFactory::GetGeneratorDesc(in, pool);
}

// Force function ordering
static void hack() {
  TObjOwnerDerivedFromIObj<CSwooshDescription>::GetNewDerivedObject(nullptr);;
}

CSwooshDescription* CParticleSwooshDataFactory::GetGeneratorDesc(CInputStream& in,
                                                                 CSimplePool* pool) {
  rstl::vector< CAssetId > ids;
  return CreateGeneratorDescription(in, pool);
}

CSwooshDescription* CParticleSwooshDataFactory::CreateGeneratorDescription(CInputStream& in,
                                                                           CSimplePool* pool) {
  const FourCC classId = CParticleDataFactory::GetClassID(in);
  if (classId != 'SWSH') {
    return nullptr;
  }
  CSwooshDescription* desc = rs_new CSwooshDescription();

  CreateWPSM(desc, in, pool);

  return desc;
}

// This is actually what it was named, must be a copy+paste job
bool CParticleSwooshDataFactory::CreateWPSM(CSwooshDescription* swoosh, CInputStream& in,
                                            CSimplePool* pool) {
  bool done = false;
  CRandom16 random(99);
  CGlobalRandom globalRandom(random);

  while (!done) {
    const FourCC classId = CParticleDataFactory::GetClassID(in);
    switch (classId) {
    case 'PSLT':
      swoosh->x0_PSLT = CParticleDataFactory::GetIntElement(in);
      break;
    case 'TIME':
      swoosh->x4_TIME = CParticleDataFactory::GetRealElement(in);
      break;
    case 'LRAD':
      swoosh->x8_LRAD = CParticleDataFactory::GetRealElement(in);
      break;
    case 'RRAD':
      swoosh->xc_RRAD = CParticleDataFactory::GetRealElement(in);
      break;
    case 'LLRD':
      swoosh->x44_24_LLRD = CParticleDataFactory::GetBool(in);
      break;
    case 'LENG':
      swoosh->x10_LENG = CParticleDataFactory::GetIntElement(in);
      break;
    case 'COLR':
      swoosh->x14_COLR = CParticleDataFactory::GetColorElement(in);
      break;
    case 'SIDE':
      swoosh->x18_SIDE = CParticleDataFactory::GetIntElement(in);
      break;
    case 'CROS':
      swoosh->x44_25_CROS = CParticleDataFactory::GetBool(in);
      break;
    case 'SROT':
      swoosh->x44_28_SROT = CParticleDataFactory::GetBool(in);
      break;
    case 'IROT':
      swoosh->x1c_IROT = CParticleDataFactory::GetRealElement(in);
      break;
    case 'ROTM':
      swoosh->x20_ROTM = CParticleDataFactory::GetRealElement(in);
      break;
    case 'POFS':
      swoosh->x24_POFS = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'IVEL':
      swoosh->x28_IVEL = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'NPOS':
      swoosh->x2c_NPOS = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'VELM':
      swoosh->x30_VELM = CParticleDataFactory::GetModVectorElement(in);
      break;
    case 'VLS1':
      swoosh->x44_26_VLS1 = CParticleDataFactory::GetBool(in);
      break;
    case 'VLM2':
      swoosh->x34_VLM2 = CParticleDataFactory::GetModVectorElement(in);
      break;
    case 'VLS2':
      swoosh->x44_27_VLS2 = CParticleDataFactory::GetBool(in);
      break;
    case 'SPLN':
      swoosh->x38_SPLN = CParticleDataFactory::GetIntElement(in);
      break;
    case 'WIRE':
      swoosh->x44_29_WIRE = CParticleDataFactory::GetBool(in);
      break;
    case 'TEXR':
      swoosh->x3c_TEXR = CParticleDataFactory::GetTextureElement(in, pool);
      break;
    case 'TSPN':
      swoosh->x40_TSPN = CParticleDataFactory::GetIntElement(in);
      break;
    case 'AALP':
      swoosh->x44_31_AALP = CParticleDataFactory::GetBool(in);
      break;
    case 'ZBUF':
      swoosh->x45_24_ZBUF = CParticleDataFactory::GetBool(in);
      break;
    case 'ORNT':
      swoosh->x45_25_ORNT = CParticleDataFactory::GetBool(in);
      break;
    case 'TEXW':
      swoosh->x44_30_TEXW = CParticleDataFactory::GetBool(in);
      break;
    case 'CRND':
      swoosh->x45_26_CRND = CParticleDataFactory::GetBool(in);
      break;
    case '_END':
      done = true;
      break;
    default:
      return false;
    }
  }

  return true;
}
