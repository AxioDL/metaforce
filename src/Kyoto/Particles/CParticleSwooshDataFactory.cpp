#include "Kyoto/Particles/CParticleSwooshDataFactory.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

const CFactoryFnReturn FParticleSwooshDataFactory(const SObjectTag& tag, CInputStream& in,
                                            const CVParamTransfer& transfer) {
  rstl::rc_ptr< IVParamObj > obj = transfer.mObj;
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
      swoosh->mPSLT = CParticleDataFactory::GetIntElement(in);
      break;
    case 'TIME':
      swoosh->mTIME = CParticleDataFactory::GetRealElement(in);
      break;
    case 'LRAD':
      swoosh->mLRAD = CParticleDataFactory::GetRealElement(in);
      break;
    case 'RRAD':
      swoosh->mRRAD = CParticleDataFactory::GetRealElement(in);
      break;
    case 'LLRD':
      swoosh->mLLRD = CParticleDataFactory::GetBool(in);
      break;
    case 'LENG':
      swoosh->mLENG = CParticleDataFactory::GetIntElement(in);
      break;
    case 'COLR':
      swoosh->mCOLR = CParticleDataFactory::GetColorElement(in);
      break;
    case 'SIDE':
      swoosh->mSIDE = CParticleDataFactory::GetIntElement(in);
      break;
    case 'CROS':
      swoosh->mCROS = CParticleDataFactory::GetBool(in);
      break;
    case 'SROT':
      swoosh->mSROT = CParticleDataFactory::GetBool(in);
      break;
    case 'IROT':
      swoosh->mIROT = CParticleDataFactory::GetRealElement(in);
      break;
    case 'ROTM':
      swoosh->mROTM = CParticleDataFactory::GetRealElement(in);
      break;
    case 'POFS':
      swoosh->mPOFS = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'IVEL':
      swoosh->mIVEL = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'NPOS':
      swoosh->mNPOS = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'VELM':
      swoosh->mVELM = CParticleDataFactory::GetModVectorElement(in);
      break;
    case 'VLS1':
      swoosh->mVLS1 = CParticleDataFactory::GetBool(in);
      break;
    case 'VLM2':
      swoosh->mVLM2 = CParticleDataFactory::GetModVectorElement(in);
      break;
    case 'VLS2':
      swoosh->mVLS2 = CParticleDataFactory::GetBool(in);
      break;
    case 'SPLN':
      swoosh->mSPLN = CParticleDataFactory::GetIntElement(in);
      break;
    case 'WIRE':
      swoosh->mWIRE = CParticleDataFactory::GetBool(in);
      break;
    case 'TEXR':
      swoosh->mTEXR = CParticleDataFactory::GetTextureElement(in, pool);
      break;
    case 'TSPN':
      swoosh->mTSPN = CParticleDataFactory::GetIntElement(in);
      break;
    case 'AALP':
      swoosh->mAALP = CParticleDataFactory::GetBool(in);
      break;
    case 'ZBUF':
      swoosh->mZBUF = CParticleDataFactory::GetBool(in);
      break;
    case 'ORNT':
      swoosh->mORNT = CParticleDataFactory::GetBool(in);
      break;
    case 'TEXW':
      swoosh->mTEXW = CParticleDataFactory::GetBool(in);
      break;
    case 'CRND':
      swoosh->mCRND = CParticleDataFactory::GetBool(in);
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
