#include "Weapons/CDecalDataFactory.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"

const CFactoryFnReturn FDecalDataFactory(const SObjectTag& tag, CInputStream& in,
                                 const CVParamTransfer& transfer) {
  rstl::rc_ptr< IVParamObj > obj = transfer.mObj;
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  CDecalDescription* ret = CDecalDataFactory::GetGeneratorDesc(in, pool);
  return ret;
}

static void hack() {
  TObjOwnerDerivedFromIObj<CDecalDescription>(nullptr);
}
CDecalDescription* CDecalDataFactory::GetGeneratorDesc(CInputStream& in, CSimplePool* pool) {
  return CreateGeneratorDescription(in, pool);
}
CDecalDescription* CDecalDataFactory::CreateGeneratorDescription(CInputStream& in,
                                                              CSimplePool* pool) {
  if (CParticleDataFactory::GetClassID(in) != 'DPSM') {
    return nullptr;
  }

  CDecalDescription* desc = rs_new CDecalDescription();
  CreateDPSM(desc, in, pool);
  return desc;
}
void CDecalDataFactory::GetQuadDecalInfo(CInputStream& in, CSimplePool* pool, uint classId,
                                      CDecalDescription::SQuadDescr& quad) {
  switch (classId) {
  case '1LFT':
  case '2LFT':
    quad.mLFT = CParticleDataFactory::GetIntElement(in);
    break;
  case '1SZE':
  case '2SZE':
    quad.mSZE = CParticleDataFactory::GetRealElement(in);
    break;
  case '1ROT':
  case '2ROT':
    quad.mROT = CParticleDataFactory::GetRealElement(in);
    break;
  case '1OFF':
  case '2OFF':
    quad.mOFF = CParticleDataFactory::GetVectorElement(in);
    break;
  case '1CLR':
  case '2CLR':
    quad.mCLR = CParticleDataFactory::GetColorElement(in);
    break;
  case '1TEX':
  case '2TEX':
    quad.mTEX = CParticleDataFactory::GetTextureElement(in, pool);
    break;
  case '1ADD':
  case '2ADD':
    quad.mADD = CParticleDataFactory::GetBool(in);
    break;
  }
}

bool CDecalDataFactory::CreateDPSM(CDecalDescription* desc, CInputStream& in, CSimplePool* pool) {
  bool done = false;
  CRandom16 random;

  while (!done) {
    CGlobalRandom globalRandom(random);
    const FourCC clsId = CParticleDataFactory::GetClassID(in);
    bool loadFirstDesc = false;
    switch (clsId) {
    case '1SZE':
    case '1LFT':
    case '1ROT':
    case '1OFF':
    case '1CLR':
    case '1TEX':
    case '1ADD':
      loadFirstDesc = true;
    case '2LFT':
    case '2SZE':
    case '2ROT':
    case '2OFF':
    case '2CLR':
    case '2TEX':
    case '2ADD': {
      CDecalDescription::SQuadDescr& quad = loadFirstDesc ? desc->mQuad1 : desc->mQuad2;
      GetQuadDecalInfo(in, pool, clsId, quad);
    } break;
    case 'DMDL': {
      rstl::optional_object< TToken< CModel > > model = CParticleDataFactory::GetModel(in, pool);
      if (model.valid()) {
        desc->mDMDL = TLockedToken< CModel >(*model);
      } else {
#if VERSION >= VERSION_R3IJ_00
        desc->mDMDL.clear();
#else
        desc->mDMDL = rstl::optional_object_null();
#endif
      }
    } break;
    case 'DLFT':
      desc->mDLFT = CParticleDataFactory::GetIntElement(in);
      break;
    case 'DMOP':
      desc->mDMOP = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'DMRT':
      desc->mDMRT = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'DMSC':
      desc->mDMSC = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'DMCL':
      desc->mDMCL = CParticleDataFactory::GetColorElement(in);
      break;
    case 'DMAB':
      desc->mDMAB = CParticleDataFactory::GetBool(in);
      break;
    case 'DMOO':
      desc->mDMOO = CParticleDataFactory::GetBool(in);
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
