#include "Weapons/CCollisionResponseData.hpp"

#include "Kyoto/CFactoryFnReturn.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Particles/CParticleDataFactory.hpp"

const int CCollisionResponseData::kInvalidSFX = -1;
const EWeaponCollisionResponseTypes CCollisionResponseData::skWorldMaterialTable[] = {
    kWCR_Default, kWCR_Unknown2, kWCR_Metal,   kWCR_Grass,   kWCR_Ice,     kWCR_Goo,
    kWCR_Metal,   kWCR_Wood,     kWCR_Grass,   kWCR_Lava,    kWCR_Lava,    kWCR_Ice,
    kWCR_Mud,     kWCR_Metal,    kWCR_Default, kWCR_Goo,     kWCR_Goo,     kWCR_Sand,
    kWCR_Default, kWCR_Default,  kWCR_Default, kWCR_Metal,   kWCR_Default, kWCR_Default,
    kWCR_Default, kWCR_Default,  kWCR_Default, kWCR_Default, kWCR_Default, kWCR_Default,
    kWCR_Default, kWCR_Default,
};

CCollisionResponseData::CCollisionResponseData(CInputStream& in, CSimplePool* sp)
: mAudibleRange(50.f), mAudibleFallOff(0.2f) {
  bool done = false;
  mGeneratorTokens.assign(94);
  mSoundEffectIds.assign(94, kInvalidSFX);
  mDecalTokens.assign(94);

  if (CParticleDataFactory::GetClassID(in) != 'CRSM') {
    return;
  }

  CRandom16 _;
  CGlobalRandom __(_);

  while (!done) {
    const FourCC clsId = CParticleDataFactory::GetClassID(in);
    if (CheckAndAddResourcesToResponse(clsId, in, sp)) {
      continue;
    }

    switch (clsId) {
    case 'RNGE':
      CParticleDataFactory::GetClassID(in);
      mAudibleRange = CParticleDataFactory::GetReal(in);
      continue;
    case 'FOFF':
      CParticleDataFactory::GetClassID(in);
      mAudibleFallOff = CParticleDataFactory::GetReal(in);
      continue;
    case '_END':
      done = true;
      break;
    default:
      return;
    }
  }
}

bool CCollisionResponseData::CheckAndAddResourcesToResponse(const FourCC clsId, CInputStream& in,
                                                            CSimplePool* sp) {
  if (CheckAndAddParticleSystemToResponse(clsId, in, sp)) {
    return true;
  }

  if (CheckAndAddSoundFXToResponse(clsId, in)) {
    return true;
  }

  if (CheckAndAddDecalToResponse(clsId, in, sp)) {
    return true;
  }

  return false;
}

bool CCollisionResponseData::CheckAndAddParticleSystemToResponse(const FourCC clsId,
                                                                 CInputStream& in,
                                                                 CSimplePool* sp) {
  static const FourCC kWCRTIDs[] = {
      'NODP', 'DEFS', 'CRTS', 'MTLS', 'GRAS', 'ICEE', 'GOOO', 'WODS', 'WATR', '1MUD', '1LAV',
      '1SAN', '1PRJ', 'DCHR', 'DCHS', 'DCSH', 'DENM', 'DESP', 'DESH', 'BTLE', 'WASP', 'TALP',
      'PTGM', 'SPIR', 'FPIR', 'FFLE', 'PARA', 'BMON', 'BFLR', 'PBOS', 'IBOS', '1SVA', '1RPR',
      '1MTR', '1PDS', '1FLB', '1DRN', '1MRE', 'CHOZ', 'JZAP', '1ISE', '1BSE', '1ATB', '1ATA',
      'BTSP', 'WWSP', 'TASP', 'TGSP', 'SPSP', 'FPSP', 'FFSP', 'PSSP', 'BMSP', 'BFSP', 'PBSP',
      'IBSP', '2SVA', '2RPR', '2MTR', '2PDS', '2FLB', '2DRN', '2MRE', 'CHSP', 'JZSP', '3ISE',
      '3BSE', '3ATB', '3ATA', 'BTSH', 'WWSH', 'TASH', 'TGSH', 'SPSH', 'FPSH', 'FFSH', 'PSSH',
      'BMSH', 'BFSH', 'PBSH', 'IBSH', '3SVA', '3RPR', '3MTR', '3PDS', '3FLB', '3DRN', '3MRE',
      'CHSH', 'JZSH', '5ISE', '5BSE', '5ATB', '5ATA',
  };

  for (int i = 0; i < ARRAY_SIZE(kWCRTIDs); i++) {
    if (clsId != kWCRTIDs[i]) {
      continue;
    }
    AddParticleSystemToResponse(static_cast< EWeaponCollisionResponseTypes >(i), in, sp);
    return true;
  }

  return false;
}

bool CCollisionResponseData::CheckAndAddSoundFXToResponse(const FourCC clsId, CInputStream& in) {
  static const FourCC kCRTSFXIDs[] = {
      'NSFX', 'DSFX', 'CSFX', 'MSFX', 'GRFX', 'ICFX', 'GOFX', 'WSFX', 'WTFX', '2MUD', '2LAV',
      '2SAN', '2PRJ', 'DCFX', 'DSFX', 'DSHX', 'DEFX', 'ESFX', 'SHFX', 'BEFX', 'WWFX', 'TAFX',
      'GTFX', 'SPFX', 'FPFX', 'FFFX', 'PAFX', 'BMFX', 'BFFX', 'PBFX', 'IBFX', '4SVA', '4RPR',
      '4MTR', '4PDS', '4FLB', '4DRN', '4MRE', 'CZFX', 'JZAS', '2ISE', '2BSE', '2ATB', '2ATA',
      'BSFX', 'WSFX', 'TSFX', 'GSFX', 'SSFX', 'FSFX', 'SFFX', 'PSFX', 'MSFX', 'SBFX', 'PBSX',
      'IBSX', '5SVA', '5RPR', '5MTR', '5PDS', '5FLB', '5DRN', '5MRE', 'CSFX', 'JZPS', '4ISE',
      '4BSE', '4ATB', '4ATA', 'BHFX', 'WHFX', 'THFX', 'GHFX', 'SHFX', 'FHFX', 'HFFX', 'PHFX',
      'MHFX', 'HBFX', 'PBHX', 'IBHX', '6SVA', '6RPR', '6MTR', '6PDS', '6FLB', '6DRN', '6MRE',
      'CHFX', 'JZHS', '6ISE', '6BSE', '6ATB', '6ATA',
  };

  for (int i = 0; i < ARRAY_SIZE(kCRTSFXIDs); i++) {
    if (clsId != kCRTSFXIDs[i]) {
      continue;
    }
    if (CParticleDataFactory::GetClassID(in) != 'NONE') {
      mSoundEffectIds[i] = in.Get< int >();
    }
    return true;
  }

  return false;
}

bool CCollisionResponseData::CheckAndAddDecalToResponse(const FourCC clsId, CInputStream& in,
                                                        CSimplePool* sp) {
  static const FourCC kCRTDecalIDs[] = {
      'NDCL', 'DDCL', 'CODL', 'MEDL', 'GRDL', 'ICDL', 'GODL',
      'WODL', 'WTDL', '3MUD', '3LAV', '3SAN', 'CHDL', 'ENDL',
  };

  for (int i = 0; i < ARRAY_SIZE(kCRTDecalIDs); i++) {
    if (clsId != kCRTDecalIDs[i]) {
      continue;
    }

    if (CParticleDataFactory::GetClassID(in) != 'NONE') {
      CAssetId id = in.Get< CAssetId >();
      mDecalTokens[i] = TToken< CDecalDescription >(sp->GetObj(SObjectTag('DPSC', id)));
    }

    return true;
  }

  return false;
}

bool CCollisionResponseData::AddParticleSystemToResponse(EWeaponCollisionResponseTypes type,
                                                         CInputStream& in, CSimplePool* sp) {
  rstl::vector< CAssetId > tracker;
  rstl::optional_object< TToken< CGenDescription > > tok =
      CParticleDataFactory::GetChildGeneratorDesc(in, sp, tracker);
  if (tok) {
    mGeneratorTokens[type] = *tok;
    return true;
  }
  return false;
}
CCollisionResponseData::~CCollisionResponseData() {}

rstl::optional_object< TLockedToken< CGenDescription > >
CCollisionResponseData::GetParticleDescription(const EWeaponCollisionResponseTypes type) const {
  int idx = type;
  if (!mGeneratorTokens.at(idx).valid()) {
    bool found = false;
    if (ResponseTypeIsEnemyNormal(type)) {
      idx = kWCR_EnemyNormal;
      found = true;
    } else if (ResponseTypeIsEnemySpecial(type)) {
      idx = kWCR_EnemySpecial;
      found = true;
    } else if (ResponseTypeIsEnemyShielded(type)) {
      idx = kWCR_EnemyShielded;
      found = true;
    }

    if (found && !mGeneratorTokens.at(idx).valid()) {
      idx = kWCR_EnemyNormal;
    }

    if (!mGeneratorTokens.at(idx).valid() && idx != kWCR_None) {
      idx = kWCR_Default;
    }
  }

  return mGeneratorTokens.at(idx);
}

uint CCollisionResponseData::GetSoundEffectId(EWeaponCollisionResponseTypes type) const {
  int idx = type;
  if (mSoundEffectIds.at(idx) == kInvalidSFX) {
    bool found = false;
    if (ResponseTypeIsEnemyNormal(type)) {
      idx = kWCR_EnemyNormal;
      found = true;
    } else if (ResponseTypeIsEnemySpecial(type)) {
      idx = kWCR_EnemySpecial;
      found = true;
    } else if (ResponseTypeIsEnemyShielded(type)) {
      idx = kWCR_EnemyShielded;
      found = true;
    }

    if (found && !mGeneratorTokens.at(idx).valid()) {
      idx = kWCR_EnemyNormal;
    }

    if (!mGeneratorTokens.at(idx).valid()) {
      idx = kWCR_Default;
    }
  }

  return mSoundEffectIds.at(idx);
}

rstl::optional_object< TLockedToken< CDecalDescription > >
CCollisionResponseData::GetDecalDescription(const EWeaponCollisionResponseTypes type) const {
  return mDecalTokens.at(type);
}

bool CCollisionResponseData::ResponseTypeIsEnemyShielded(const EWeaponCollisionResponseTypes type) {
  if (type >= kWCR_Unknown69 && type <= kWCR_AtomicAlphaReflect) {
    return true;
  }
  
  return false;
}

bool CCollisionResponseData::ResponseTypeIsEnemyNormal(const EWeaponCollisionResponseTypes type) {
  if (type >= kWCR_Unknown19 && type <= kWCR_AtomicAlpha) {
    return true;
  }
  
  return false;
}

bool CCollisionResponseData::ResponseTypeIsEnemySpecial(const EWeaponCollisionResponseTypes type) {
  if (type >= kWCR_Unknown44 && type <= kWCR_Unknown68) {
    return true;
  }
  
  return false;
}

float CCollisionResponseData::GetAudibleRange() const { return mAudibleRange; }
float CCollisionResponseData::GetAudibleFallOff() const { return mAudibleFallOff; }

EWeaponCollisionResponseTypes CCollisionResponseData::GetWorldCollisionResponseType(const int idx) {
  if (idx >= 0 && idx < ARRAY_SIZE(skWorldMaterialTable)) {
    return skWorldMaterialTable[idx];
  }
  return kWCR_Default;
}


const CFactoryFnReturn FCollisionResponseDataFactory(const SObjectTag& tag, CInputStream& in,
                                               const CVParamTransfer& xfer) {
  rstl::rc_ptr< IVParamObj > obj = xfer.mObj;
  CSimplePool* pool = static_cast< TObjOwnerParam< CSimplePool* >* >(obj.GetPtr())->GetData();
  return rs_new CCollisionResponseData(in, pool);
}
