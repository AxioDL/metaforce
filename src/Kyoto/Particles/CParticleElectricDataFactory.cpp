#include "Kyoto/Particles/CParticleElectricDataFactory.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Particles/CElectricDescription.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

const CFactoryFnReturn FParticleElectricDataFactory(const SObjectTag& tag, CInputStream& in,
                                              const CVParamTransfer& transfer) {
  rstl::rc_ptr< IVParamObj > obj = transfer.mObj;
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  return CParticleElectricDataFactory::GetGeneratorDesc(in, pool);
}

CElectricDescription* CParticleElectricDataFactory::GetGeneratorDesc(CInputStream& in,
                                                                     CSimplePool* pool) {
  rstl::vector< CAssetId > resources;
  return CreateGeneratorDescription(in, pool);
}

CElectricDescription* CParticleElectricDataFactory::CreateGeneratorDescription(CInputStream& in,
                                                                               CSimplePool* pool) {
  const FourCC classId = CParticleDataFactory::GetClassID(in);
  if (classId != 'ELSM') {
    return nullptr;
  }
  CElectricDescription* desc = rs_new CElectricDescription();
  CreateELSM(desc, in, pool);
  LoadELSMTokens(desc);
  return desc;
}

void CParticleElectricDataFactory::LoadELSMTokens(CElectricDescription* desc) {
  if (desc->mSSWH) {
    desc->mSSWH->ForceCache();
  }
  if (desc->mGPSM) {
    desc->mGPSM->ForceCache();
  }
  if (desc->mEPSM) {
    desc->mEPSM->ForceCache();
  }
}

bool CParticleElectricDataFactory::CreateELSM(CElectricDescription* desc, CInputStream& in,
                                              CSimplePool* pool) {
  bool done = false;
  CRandom16 random(99);
  while (!done) {
    CGlobalRandom globalRandom(random);
    const FourCC classId = CParticleDataFactory::GetClassID(in);
    switch (classId) {
    case 'LIFE':
      desc->mLIFE = CParticleDataFactory::GetIntElement(in);
      break;
    case 'SLIF':
      desc->mSLIF = CParticleDataFactory::GetIntElement(in);
      break;
    case 'GRAT':
      desc->mGRAT = CParticleDataFactory::GetRealElement(in);
      break;
    case 'SCNT':
      desc->mSCNT = CParticleDataFactory::GetIntElement(in);
      break;
    case 'SSEG':
      desc->mSSEG = CParticleDataFactory::GetIntElement(in);
      break;
    case 'COLR':
      desc->mCOLR = CParticleDataFactory::GetColorElement(in);
      break;
    case 'IEMT':
      desc->mIEMT = CParticleDataFactory::GetEmitterElement(in);
      break;
    case 'FEMT':
      desc->mFEMT = CParticleDataFactory::GetEmitterElement(in);
      break;
    case 'AMPL':
      desc->mAMPL = CParticleDataFactory::GetRealElement(in);
      break;
    case 'AMPD':
      desc->mAMPD = CParticleDataFactory::GetRealElement(in);
      break;
    case 'SSWH': {
      const FourCC childId = CParticleDataFactory::GetClassID(in);
      if (childId != 'NONE') {
        const CAssetId id = in.ReadLong();
        if (id != 0) {
          desc->mSSWH = TCachedToken< CSwooshDescription >(pool->GetObj(SObjectTag('SWHC', id)));
        }
      }
      break;
    }
    case 'GPSM': {
      rstl::vector< CAssetId > resources;
      rstl::optional_object< TToken< CGenDescription > > child =
          CParticleDataFactory::GetChildGeneratorDesc(in, pool, resources);
      if (child) {
        desc->mGPSM = TCachedToken< CGenDescription >(*child);
      }
      break;
    }
    case 'EPSM': {
      rstl::vector< CAssetId > resources;
      rstl::optional_object< TToken< CGenDescription > > child =
          CParticleDataFactory::GetChildGeneratorDesc(in, pool, resources);
      if (child) {
        desc->mEPSM = TCachedToken< CGenDescription >(*child);
      }
      break;
    }
    case 'LWD1':
      desc->mLWD1 = CParticleDataFactory::GetRealElement(in);
      break;
    case 'LWD2':
      desc->mLWD2 = CParticleDataFactory::GetRealElement(in);
      break;
    case 'LWD3':
      desc->mLWD3 = CParticleDataFactory::GetRealElement(in);
      break;
    case 'LCL1':
      desc->mLCL1 = CParticleDataFactory::GetColorElement(in);
      break;
    case 'LCL2':
      desc->mLCL2 = CParticleDataFactory::GetColorElement(in);
      break;
    case 'LCL3':
      desc->mLCL3 = CParticleDataFactory::GetColorElement(in);
      break;
    case 'ZERY':
      desc->mZERY = CParticleDataFactory::GetBool(in);
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
