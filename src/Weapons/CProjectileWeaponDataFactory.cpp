#include "Weapons/CProjectileWeaponDataFactory.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "MetroidPrime/Weapons/CWeapon.hpp"
#include "Weapons/CWeaponDescription.hpp"

#include <Kyoto/Particles/CParticleDataFactory.hpp>
#include <rstl/vector.hpp>

const CFactoryFnReturn FProjectileWeaponDataFactory(const SObjectTag& tag, CInputStream& in,
                                                    const CVParamTransfer& xfer) {
  rstl::rc_ptr< IVParamObj > obj = xfer.mObj;
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  CWeaponDescription* desc = CProjectileWeaponDataFactory::GetGeneratorDesc(in, pool);
  return desc;
}

static void hack() {
  TObjOwnerDerivedFromIObj< CWeaponDescription >(nullptr);
}
CWeaponDescription* CProjectileWeaponDataFactory::GetGeneratorDesc(CInputStream& in,
                                                                   CSimplePool* pool) {
  rstl::vector< CAssetId > assets;
  return CreateGeneratorDescription(in, pool);
}

CWeaponDescription* CProjectileWeaponDataFactory::CreateGeneratorDescription(CInputStream& in,
                                                                             CSimplePool* pool) {
  if (CParticleDataFactory::GetClassID(in) != 'WPSM') {
    return nullptr;
  }

  CWeaponDescription* desc = rs_new CWeaponDescription();
  CreateWPSM(desc, in, pool);
  return desc;
}

bool CProjectileWeaponDataFactory::CreateWPSM(CWeaponDescription* desc, CInputStream& in,
                                              CSimplePool* pool) {
  bool done = false;
  CRandom16 random;
  CGlobalRandom globalRandom(random);

  while (!done) {
    switch (CParticleDataFactory::GetClassID(in)) {
    case 'IORN':
      desc->mIORN = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'IVEC':
      desc->mIVEC = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'PSOV':
      desc->mPSOV = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'PSVM':
      desc->mPSVM = CParticleDataFactory::GetModVectorElement(in);
      break;
    case 'VMD2':
      desc->mVMD2 = CParticleDataFactory::GetBool(in);
      break;
    case 'PSLT':
      desc->mPSLT = CParticleDataFactory::GetIntElement(in);
      break;
    case 'APSM': {
      rstl::vector< CAssetId > assets;
      rstl::optional_object< TToken< CGenDescription > > child =
          CParticleDataFactory::GetChildGeneratorDesc(in, pool, assets);
      if (child) {
        desc->mAPSM = TLockedToken< CGenDescription >(*child);
      } else {
        desc->mAPSM = rstl::optional_object_null();
      }
      break;
    }
    case 'AP11':
      desc->mAP11 = CParticleDataFactory::GetBool(in);
      break;
    case 'APS2': {
      rstl::vector< CAssetId > assets;
      rstl::optional_object< TToken< CGenDescription > > child =
          CParticleDataFactory::GetChildGeneratorDesc(in, pool, assets);
      if (child) {
        desc->mAPS2 = TLockedToken< CGenDescription >(*child);
      } else {
        desc->mAPS2 = rstl::optional_object_null();
      }
      break;
    }
    case 'AP21':
      desc->mAP21 = CParticleDataFactory::GetBool(in);
      break;
    case 'APSO':
      desc->mAPSO = CParticleDataFactory::GetBool(in);
      break;
    case 'ASW1': {
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        CAssetId id = in.ReadLong();
        desc->mASW1 = TLockedToken< CSwooshDescription >(pool->GetObj(SObjectTag('SWHC', id)));
      }
      break;
    }
    case 'AS11':
      desc->mAS11 = CParticleDataFactory::GetBool(in);
      break;
    case 'ASW2': {
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        CAssetId id = in.ReadLong();
        desc->mASW2 = TLockedToken< CSwooshDescription >(pool->GetObj(SObjectTag('SWHC', id)));
      }
      break;
    }
    case 'AS12':
      desc->mAS12 = CParticleDataFactory::GetBool(in);
      break;
    case 'ASW3': {
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        CAssetId id = in.ReadLong();
        desc->mASW3 = TLockedToken< CSwooshDescription >(pool->GetObj(SObjectTag('SWHC', id)));
      }
      break;
    }
    case 'AS13':
      desc->mAS13 = CParticleDataFactory::GetBool(in);
      break;
    case 'OHEF': {
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        CAssetId id = in.ReadLong();
        desc->mOHEF = TLockedToken< CModel >(pool->GetObj(SObjectTag('CMDL', id)));
      }
      break;
    }
    case 'PSCL':
      desc->mPSCL = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'PCOL':
      desc->mPCOL = CParticleDataFactory::GetColorElement(in);
      break;
    case 'COLR': {
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        CAssetId id = in.ReadLong();
        desc->mCOLR = TLockedToken< CCollisionResponseData >(pool->GetObj(SObjectTag('CRSC', id)));
      }
      break;
    }
    case 'POFS':
      desc->mPOFS = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'OFST':
      desc->mOFST = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'HOMG':
      desc->mHOMG = CParticleDataFactory::GetBool(in);
      break;
    case 'TRAT':
      desc->mTRAT = CParticleDataFactory::GetRealElement(in);
      break;
    case 'PJFX':
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        desc->mPJFX = in.ReadLong();
      }
      break;
    case 'RNGE':
      desc->mRNGE = CParticleDataFactory::GetRealElement(in);
      break;
    case 'FOFF':
      desc->mFOFF = CParticleDataFactory::GetRealElement(in);
      break;
    case 'EWTR':
      desc->mEWTR = CParticleDataFactory::GetBool(in);
      break;
    case 'LWTR':
      desc->mLWTR = CParticleDataFactory::GetBool(in);
      break;
    case 'SWTR':
      desc->mSWTR = CParticleDataFactory::GetBool(in);
      break;
#if VERSION >= VERSION_GM8P_00
    case 'F60H':
      desc->mF60H = CParticleDataFactory::GetBool(in);
      break;
    case 'SVBD':
      desc->mSVBD = CParticleDataFactory::GetBool(in);
      break;
    case 'NDTT':
      desc->mNDTT = CParticleDataFactory::GetBool(in);
      break;
    case 'SPS1':
      desc->mSPS1 = CParticleDataFactory::GetBool(in);
      break;
    case 'SPS2':
      desc->mSPS2 = CParticleDataFactory::GetBool(in);
      break;
    case 'FC60':
      desc->mFC60 = CParticleDataFactory::GetBool(in);
      break;
#endif
    case '_END':
      done = true;
      break;
    default:
      return false;
    }
  }

  return true;
}
