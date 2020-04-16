#include "Runtime/Particle/CProjectileWeaponDataFactory.hpp"

#include "Runtime/CRandom16.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/Collision/CCollisionResponseData.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"
#include "Runtime/Particle/CWeaponDescription.hpp"

namespace urde {
static logvisor::Module Log("urde::CProjectileWeaponDataFactory");

using CPF = CParticleDataFactory;

std::unique_ptr<CWeaponDescription> CProjectileWeaponDataFactory::GetGeneratorDesc(CInputStream& in,
                                                                                   CSimplePool* resPool) {
  return CreateGeneratorDescription(in, resPool);
}

std::unique_ptr<CWeaponDescription> CProjectileWeaponDataFactory::CreateGeneratorDescription(CInputStream& in,
                                                                                             CSimplePool* resPool) {
  const FourCC clsId = CPF::GetClassID(in);
  if (clsId != FOURCC('WPSM')) {
    return nullptr;
  }

  auto desc = std::make_unique<CWeaponDescription>();
  CreateWPSM(desc.get(), in, resPool);
  return desc;

}

bool CProjectileWeaponDataFactory::CreateWPSM(CWeaponDescription* desc, CInputStream& in, CSimplePool* resPool) {
  CRandom16 rand;
  CGlobalRandom gr{rand};
  FourCC clsId = CPF::GetClassID(in);

  while (clsId != SBIG('_END')) {
    switch (clsId.toUint32()) {
    case SBIG('IORN'):
      desc->x0_IORN = CPF::GetVectorElement(in);
      break;
    case SBIG('IVEC'):
      desc->x4_IVEC = CPF::GetVectorElement(in);
      break;
    case SBIG('PSOV'):
      desc->x8_PSOV = CPF::GetVectorElement(in);
      break;
    case SBIG('PSVM'):
      desc->xc_PSVM = CPF::GetModVectorElement(in);
      break;
    case SBIG('VMD2'):
      desc->x10_VMD2 = CPF::GetBool(in);
      break;
    case SBIG('PSLT'):
      desc->x14_PSLT = CPF::GetIntElement(in);
      break;
    case SBIG('PSCL'):
      desc->x18_PSCL = CPF::GetVectorElement(in);
      break;
    case SBIG('PCOL'):
      desc->x1c_PCOL = CPF::GetColorElement(in);
      break;
    case SBIG('POFS'):
      desc->x20_POFS = CPF::GetVectorElement(in);
      break;
    case SBIG('OFST'):
      desc->x24_OFST = CPF::GetVectorElement(in);
      break;
    case SBIG('APSO'):
      desc->x28_APSO = CPF::GetBool(in);
      break;
    case SBIG('HOMG'):
      desc->x29_HOMG = CPF::GetBool(in);
      break;
    case SBIG('AP11'):
      desc->x2a_AP11 = CPF::GetBool(in);
      break;
    case SBIG('AP21'):
      desc->x2b_AP21 = CPF::GetBool(in);
      break;
    case SBIG('AS11'):
      desc->x2c_AS11 = CPF::GetBool(in);
      break;
    case SBIG('AS12'):
      desc->x2d_AS12 = CPF::GetBool(in);
      break;
    case SBIG('AS13'):
      desc->x2e_AS13 = CPF::GetBool(in);
      break;
    case SBIG('TRAT'):
      desc->x30_TRAT = CPF::GetRealElement(in);
      break;
    case SBIG('APSM'): {
      std::vector<CAssetId> tracker;
      tracker.reserve(8);
      desc->x34_APSM = CPF::GetChildGeneratorDesc(in, resPool, tracker);
      break;
    }
    case SBIG('APS2'): {
      std::vector<CAssetId> tracker;
      tracker.reserve(8);
      desc->x44_APS2 = CPF::GetChildGeneratorDesc(in, resPool, tracker);
      break;
    }
    case SBIG('ASW1'):
      desc->x54_ASW1 = CPF::GetSwooshGeneratorDesc(in, resPool);
      break;
    case SBIG('ASW2'):
      desc->x64_ASW2 = CPF::GetSwooshGeneratorDesc(in, resPool);
      break;
    case SBIG('ASW3'):
      desc->x74_ASW3 = CPF::GetSwooshGeneratorDesc(in, resPool);
      break;
    case SBIG('OHEF'):
      desc->x84_OHEF = CPF::GetModel(in, resPool);
      break;
    case SBIG('COLR'): {
      FourCC cid = CPF::GetClassID(in);
      if (cid == SBIG('NONE'))
        break;
      CAssetId id(in);
      if (id.IsValid())
        desc->x94_COLR = {resPool->GetObj({FOURCC('CRSC'), id}), true};
      break;
    }
    case SBIG('EWTR'):
      desc->xa4_EWTR = CPF::GetBool(in);
      break;
    case SBIG('LWTR'):
      desc->xa5_LWTR = CPF::GetBool(in);
      break;
    case SBIG('SWTR'):
      desc->xa6_SWTR = CPF::GetBool(in);
      break;
    case SBIG('PJFX'): {
      FourCC cid = CPF::GetClassID(in);
      if (cid == FOURCC('NONE'))
        break;

      desc->xa8_PJFX = CPF::GetInt(in);
      break;
    }
    case SBIG('RNGE'):
      desc->xac_RNGE = CPF::GetRealElement(in);
      break;
    case SBIG('FOFF'):
      desc->xb0_FOFF = CPF::GetRealElement(in);
      break;
    case SBIG('SPS1'):
      desc->x28_SPS1 = CPF::GetBool(in);
      break;
    case SBIG('SPS2'):
      desc->x29_SPS2 = CPF::GetBool(in);
      break;
    case SBIG('FC60'):
      desc->x29_FC60 = CPF::GetBool(in);
      break;
    default: {
      Log.report(logvisor::Fatal, FMT_STRING("Unknown WPSM class {} @{}"), clsId, in.position());
      return false;
    }
    }
    clsId = CPF::GetClassID(in);
  }
  return true;
}

CFactoryFnReturn FProjectileWeaponDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                              CObjectReference*) {
  CSimplePool* sp = vparms.GetOwnedObj<CSimplePool*>();
  return TToken<CWeaponDescription>::GetIObjObjectFor(CProjectileWeaponDataFactory::GetGeneratorDesc(in, sp));
}
} // namespace urde
