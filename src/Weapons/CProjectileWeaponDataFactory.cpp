#include "Weapons/CProjectileWeaponDataFactory.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/CVParamTransfer.hpp"
#include "Weapons/CWeaponDescription.hpp"

#include <Kyoto/Particles/CParticleDataFactory.hpp>
#include <rstl/vector.hpp>

const CFactoryFnReturn FProjectileWeaponDataFactory(const SObjectTag& tag, CInputStream& in,
                                                    const CVParamTransfer& xfer) {
  rstl::rc_ptr< IVParamObj > obj = xfer.x0_obj;
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  return CProjectileWeaponDataFactory::GetGeneratorDesc(in, pool);
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
      desc->x0_IORN = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'IVEC':
      desc->x4_IVEC = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'PSOV':
      desc->x8_PSOV = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'PSVM':
      desc->xc_PSVM = CParticleDataFactory::GetModVectorElement(in);
      break;
    case 'VMD2':
      desc->x10_VMD2 = CParticleDataFactory::GetBool(in);
      break;
    case 'PSLT':
      desc->x14_PSLT = CParticleDataFactory::GetIntElement(in);
      break;
    case 'APSM': {
      rstl::vector< CAssetId > assets;
      rstl::optional_object< TToken< CGenDescription > > child =
          CParticleDataFactory::GetChildGeneratorDesc(in, pool, assets);
      if (child) {
        desc->x34_APSM = TLockedToken< CGenDescription >(*child);
      } else {
        desc->x34_APSM = rstl::optional_object_null();
      }
      break;
    }
    case 'AP11':
      desc->x2a_AP11 = CParticleDataFactory::GetBool(in);
      break;
    case 'APS2': {
      rstl::vector< CAssetId > assets;
      rstl::optional_object< TToken< CGenDescription > > child =
          CParticleDataFactory::GetChildGeneratorDesc(in, pool, assets);
      if (child) {
        desc->x44_APS2 = TLockedToken< CGenDescription >(*child);
      } else {
        desc->x44_APS2 = rstl::optional_object_null();
      }
      break;
    }
    case 'AP21':
      desc->x2b_AP21 = CParticleDataFactory::GetBool(in);
      break;
    case 'APSO':
      desc->x28_APSO = CParticleDataFactory::GetBool(in);
      break;
    case 'ASW1': {
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        CAssetId id = in.ReadLong();
        desc->x54_ASW1 = TLockedToken< CSwooshDescription >(pool->GetObj(SObjectTag('SWHC', id)));
      }
      break;
    }
    case 'AS11':
      desc->x2c_AS11 = CParticleDataFactory::GetBool(in);
      break;
    case 'ASW2': {
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        CAssetId id = in.ReadLong();
        desc->x64_ASW2 = TLockedToken< CSwooshDescription >(pool->GetObj(SObjectTag('SWHC', id)));
      }
      break;
    }
    case 'AS12':
      desc->x2d_AS12 = CParticleDataFactory::GetBool(in);
      break;
    case 'ASW3': {
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        CAssetId id = in.ReadLong();
        desc->x74_ASW3 = TLockedToken< CSwooshDescription >(pool->GetObj(SObjectTag('SWHC', id)));
      }
      break;
    }
    case 'AS13':
      desc->x2e_AS13 = CParticleDataFactory::GetBool(in);
      break;
    case 'OHEF': {
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        CAssetId id = in.ReadLong();
        desc->x84_OHEF = TLockedToken< CModel >(pool->GetObj(SObjectTag('CMDL', id)));
      }
      break;
    }
    case 'PSCL':
      desc->x18_PSCL = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'PCOL':
      desc->x1c_PCOL = CParticleDataFactory::GetColorElement(in);
      break;
    case 'COLR': {
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        CAssetId id = in.ReadLong();
        desc->x94_COLR =
            TLockedToken< CCollisionResponseData >(pool->GetObj(SObjectTag('CRSC', id)));
      }
      break;
    }
    case 'POFS':
      desc->x20_POFS = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'OFST':
      desc->x24_OFST = CParticleDataFactory::GetVectorElement(in);
      break;
    case 'HOMG':
      desc->x29_HOMG = CParticleDataFactory::GetBool(in);
      break;
    case 'TRAT':
      desc->x30_TRAT = CParticleDataFactory::GetRealElement(in);
      break;
    case 'PJFX':
      if (CParticleDataFactory::GetClassID(in) != 'NONE') {
        desc->xa8_PJFX = in.ReadLong();
      }
      break;
    case 'RNGE':
      desc->xac_RNGE = CParticleDataFactory::GetRealElement(in);
      break;
    case 'FOFF':
      desc->xb0_FOFF = CParticleDataFactory::GetRealElement(in);
      break;
    case 'EWTR':
      desc->xa4_EWTR = CParticleDataFactory::GetBool(in);
      break;
    case 'LWTR':
      desc->xa5_LWTR = CParticleDataFactory::GetBool(in);
      break;
    case 'SWTR':
      desc->xa6_SWTR = CParticleDataFactory::GetBool(in);
      break;
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
    case 'F60H':
      desc->x28_25_F60H = CParticleDataFactory::GetBool(in);
      break;
    case 'SVBD':
      desc->x28_26_SVBD = CParticleDataFactory::GetBool(in);
      break;
    case 'NDTT':
      desc->x28_27_NDTT = CParticleDataFactory::GetBool(in);
      break;
    case 'SPS1':
      desc->x28_30_SPS1 = CParticleDataFactory::GetBool(in);
      break;
    case 'SPS2':
      desc->x29_24_SPS2 = CParticleDataFactory::GetBool(in);
      break;
    case 'FC60':
      desc->x29_31_FC60 = CParticleDataFactory::GetBool(in);
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
