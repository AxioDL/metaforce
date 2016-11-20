#include "CCollisionResponseData.hpp"
#include "Particle/CParticleDataFactory.hpp"
#include "Particle/CDecalDescription.hpp"
#include "Particle/CSwooshDescription.hpp"
#include "Particle/CElectricDescription.hpp"
#include "Particle/CGenDescription.hpp"
#include "Graphics/CModel.hpp"
#include "CSimplePool.hpp"
#include "CRandom16.hpp"

namespace urde
{
const EWeaponCollisionResponseTypes CCollisionResponseData::skWorldMaterialTable[32] =
{
    EWeaponCollisionResponseTypes::Default,
    EWeaponCollisionResponseTypes::Unknown2,
    EWeaponCollisionResponseTypes::Metal,
    EWeaponCollisionResponseTypes::Grass,
    EWeaponCollisionResponseTypes::Ice,
    EWeaponCollisionResponseTypes::Goo,
    EWeaponCollisionResponseTypes::Metal,
    EWeaponCollisionResponseTypes::Wood,
    EWeaponCollisionResponseTypes::Grass,
    EWeaponCollisionResponseTypes::Lava,
    EWeaponCollisionResponseTypes::Lava,
    EWeaponCollisionResponseTypes::Ice,
    EWeaponCollisionResponseTypes::Mud,
    EWeaponCollisionResponseTypes::Metal,
    EWeaponCollisionResponseTypes::Default,
    EWeaponCollisionResponseTypes::Goo,
    EWeaponCollisionResponseTypes::Goo,
    EWeaponCollisionResponseTypes::Sand,
    EWeaponCollisionResponseTypes::Default,
    EWeaponCollisionResponseTypes::Default,
    EWeaponCollisionResponseTypes::Default,
    EWeaponCollisionResponseTypes::Metal,
    EWeaponCollisionResponseTypes::Default,
    EWeaponCollisionResponseTypes::Default,
    EWeaponCollisionResponseTypes::Default,
    EWeaponCollisionResponseTypes::Default,
    EWeaponCollisionResponseTypes::Default,
    EWeaponCollisionResponseTypes::Default,
    EWeaponCollisionResponseTypes::Default,
    EWeaponCollisionResponseTypes::Default,
    EWeaponCollisionResponseTypes::Default,
    EWeaponCollisionResponseTypes::Default,
};

const s32 CCollisionResponseData::kInvalidSFX = -1;

static const std::vector<FourCC> kWCRTSFXIDs =
{
    SBIG('NSFX'),SBIG('DSFX'),SBIG('CSFX'),SBIG('MSFX'),
    SBIG('GRFX'),SBIG('ICFX'),SBIG('GOFX'),SBIG('WSFX'),
    SBIG('WTFX'),SBIG('2MUD'),SBIG('2LAV'),SBIG('2SAN'),
    SBIG('2PRJ'),SBIG('DCFX'),SBIG('DSFX'),SBIG('DSHX'),
    SBIG('DEFX'),SBIG('ESFX'),SBIG('SHFX'),SBIG('BEFX'),
    SBIG('WWFX'),SBIG('TAFX'),SBIG('GTFX'),SBIG('SPFX'),
    SBIG('FPFX'),SBIG('FFFX'),SBIG('PAFX'),SBIG('BMFX'),
    SBIG('BFFX'),SBIG('PBFX'),SBIG('IBFX'),SBIG('4SVA'),
    SBIG('4RPR'),SBIG('4MTR'),SBIG('4PDS'),SBIG('4FLB'),
    SBIG('4DRN'),SBIG('4MRE'),SBIG('CZFX'),SBIG('JZAS'),
    SBIG('2ISE'),SBIG('2BSE'),SBIG('2ATB'),SBIG('2ATA'),
    SBIG('BSFX'),SBIG('WSFX'),SBIG('TSFX'),SBIG('GSFX'),
    SBIG('SSFX'),SBIG('FSFX'),SBIG('SFFX'),SBIG('PSFX'),
    SBIG('MSFX'),SBIG('SBFX'),SBIG('PBSX'),SBIG('IBSX'),
    SBIG('5SVA'),SBIG('5RPR'),SBIG('5MTR'),SBIG('5PDS'),
    SBIG('5FLB'),SBIG('5DRN'),SBIG('5MRE'),SBIG('CSFX'),
    SBIG('JZPS'),SBIG('4ISE'),SBIG('4BSE'),SBIG('4ATB'),
    SBIG('4ATA'),SBIG('BHFX'),SBIG('WHFX'),SBIG('THFX'),
    SBIG('GHFX'),SBIG('SHFX'),SBIG('FHFX'),SBIG('HFFX'),
    SBIG('PHFX'),SBIG('MHFX'),SBIG('HBFX'),SBIG('PBHX'),
    SBIG('IBHX'),SBIG('6SVA'),SBIG('6RPR'),SBIG('6MTR'),
    SBIG('6PDS'),SBIG('6FLB'),SBIG('6DRN'),SBIG('6MRE'),
    SBIG('CHFX'),SBIG('JZHS'),SBIG('6ISE'),SBIG('6BSE'),
    SBIG('6ATB'),SBIG('6ATA'),
};

static const std::vector<FourCC> kWCRTIDs =
{
    SBIG('NODP'),SBIG('DEFS'),SBIG('CRTS'),SBIG('MTLS'),
    SBIG('GRAS'),SBIG('ICEE'),SBIG('GOOO'),SBIG('WODS'),
    SBIG('WATR'),SBIG('1MUD'),SBIG('1LAV'),SBIG('1SAN'),
    SBIG('1PRJ'),SBIG('DCHR'),SBIG('DCHS'),SBIG('DCSH'),
    SBIG('DENM'),SBIG('DESP'),SBIG('DESH'),SBIG('BTLE'),
    SBIG('WASP'),SBIG('TALP'),SBIG('PTGM'),SBIG('SPIR'),
    SBIG('FPIR'),SBIG('FFLE'),SBIG('PARA'),SBIG('BMON'),
    SBIG('BFLR'),SBIG('PBOS'),SBIG('IBOS'),SBIG('1SVA'),
    SBIG('1RPR'),SBIG('1MTR'),SBIG('1PDS'),SBIG('1FLB'),
    SBIG('1DRN'),SBIG('1MRE'),SBIG('CHOZ'),SBIG('JZAP'),
    SBIG('1ISE'),SBIG('1BSE'),SBIG('1ATB'),SBIG('1ATA'),
    SBIG('BTSP'),SBIG('WWSP'),SBIG('TASP'),SBIG('TGSP'),
    SBIG('SPSP'),SBIG('FPSP'),SBIG('FFSP'),SBIG('PSSP'),
    SBIG('BMSP'),SBIG('BFSP'),SBIG('PBSP'),SBIG('IBSP'),
    SBIG('2SVA'),SBIG('2RPR'),SBIG('2MTR'),SBIG('2PDS'),
    SBIG('2FLB'),SBIG('2DRN'),SBIG('2MRE'),SBIG('CHSP'),
    SBIG('JZSP'),SBIG('3ISE'),SBIG('3BSE'),SBIG('3ATB'),
    SBIG('3ATA'),SBIG('BTSH'),SBIG('WWSH'),SBIG('TASH'),
    SBIG('TGSH'),SBIG('SPSH'),SBIG('FPSH'),SBIG('FFSH'),
    SBIG('PSSH'),SBIG('BMSH'),SBIG('BFSH'),SBIG('PBSH'),
    SBIG('IBSH'),SBIG('3SVA'),SBIG('3RPR'),SBIG('3MTR'),
    SBIG('3PDS'),SBIG('3FLB'),SBIG('3DRN'),SBIG('3MRE'),
    SBIG('CHSH'),SBIG('JZSH'),SBIG('5ISE'),SBIG('5BSE'),
    SBIG('5ATB'),SBIG('5ATA')
};

static const std::vector<FourCC> kWCRTDecalIDs =
{
    SBIG('NCDL'),SBIG('DDCL'),SBIG('CODL'),SBIG('MEDL'),
    SBIG('GRDL'),SBIG('ICDL'),SBIG('GODL'),SBIG('WODL'),
    SBIG('WTDL'),SBIG('3MUD'),SBIG('3LAV'),SBIG('3SAN'),
    SBIG('CHDL'),SBIG('ENDL')
};

using CPF = CParticleDataFactory;

void CCollisionResponseData::AddParticleSystemToResponse(EWeaponCollisionResponseTypes type, CInputStream &in, CSimplePool *resPool)
{
    int i = int(type);
    std::vector<ResId> tracker;
    tracker.resize(8);
    x0_generators[i].emplace(CPF::GetChildGeneratorDesc(in, resPool, tracker).m_token);
}

bool CCollisionResponseData::CheckAndAddDecalToResponse(FourCC clsId, CInputStream& in, CSimplePool* resPool)
{
    int i = 0;
    for (const FourCC& type : kWCRTDecalIDs)
    {
        if (type == clsId)
        {
            FourCC cls = CPF::GetClassID(in);
            if (cls == SBIG('NONE'))
                return true;

            ResId id = CPF::GetInt(in);
            if (!id)
                return true;

            x20_decals[i].emplace(resPool->GetObj({FOURCC('DPSC'), id}));
            return true;
        }
        i++;
    }
    return false;
}

bool CCollisionResponseData::CheckAndAddSoundFXToResponse(FourCC clsId, CInputStream& in)
{
    int i = 0;
    for (const FourCC& type : kWCRTSFXIDs)
    {
        if (type == clsId)
        {
            FourCC cls = CPF::GetClassID(in);
            if (cls == SBIG('NONE'))
                return true;

            x10_sfx[i] = CPF::GetInt(in);
            return true;
        }
        i++;
    }

    return false;
}

bool CCollisionResponseData::CheckAndAddParticleSystemToResponse(FourCC clsId, CInputStream& in, CSimplePool* resPool)
{
    int i = 0;
    for (const FourCC& type : kWCRTIDs)
    {
        if (type == clsId)
        {
            AddParticleSystemToResponse(EWeaponCollisionResponseTypes(i), in, resPool);
            return true;
        }
        i++;
    }
    return false;
}

bool CCollisionResponseData::CheckAndAddResourceToResponse(FourCC clsId, CInputStream& in, CSimplePool* resPool)
{
    if (CheckAndAddParticleSystemToResponse(clsId, in, resPool))
        return true;
    if (CheckAndAddSoundFXToResponse(clsId, in))
        return true;
    if (CheckAndAddDecalToResponse(clsId, in, resPool))
        return true;

    return false;
}

CCollisionResponseData::CCollisionResponseData(CInputStream& in, CSimplePool* resPool)
    : x30_RNGE(50.f), x34_FOFF(0.2f)
{
    x0_generators.resize(94);
    x10_sfx.resize(94);
    x20_decals.resize(94);
    for (s32& id : x10_sfx)
        id = kInvalidSFX;

    FourCC clsId = CPF::GetClassID(in);
    if (clsId == UncookedResType())
    {
        CRandom16 rand{99};
        CGlobalRandom gr(rand);

        while (clsId != SBIG('_END'))
        {
            clsId = CPF::GetClassID(in);
            if (CheckAndAddResourceToResponse(clsId, in, resPool))
                continue;

            if (clsId == SBIG('RNGE'))
            {
                CPF::GetClassID(in);
                x30_RNGE = CPF::GetReal(in);
            }
            else if (clsId == SBIG('FOFF'))
            {
                CPF::GetClassID(in);
                x34_FOFF = CPF::GetReal(in);
            }
        }
    }
}

const rstl::optional_object<TLockedToken<CGenDescription>>& CCollisionResponseData::GetParticleDescription(EWeaponCollisionResponseTypes type) const
{
    return x0_generators[u32(type)];
}

const rstl::optional_object<TLockedToken<CDecalDescription>>& CCollisionResponseData::GetDecalDescription(EWeaponCollisionResponseTypes type) const
{
    return x20_decals[u32(type)];
}

s32 CCollisionResponseData::GetSoundEffectId(EWeaponCollisionResponseTypes type) const
{
     if (x10_sfx[u32(type)] == kInvalidSFX)
     {
         if (ResponseTypeIsEnemyNormal(type))
             type = EWeaponCollisionResponseTypes::EnemyNormal;
         else if (ResponseTypeIsEnemySpecial(type))
             type = EWeaponCollisionResponseTypes::EnemySpecial;
         else if (ResponseTypeIsEnemyShielded(type))
             type = EWeaponCollisionResponseTypes::EnemyShielded;
         else
             type = EWeaponCollisionResponseTypes::Default;
     }

     return x10_sfx[u32(type)];
}

EWeaponCollisionResponseTypes CCollisionResponseData::GetWorldCollisionResponseType(s32 id)
{
    if (id < 0 || id >= s32(EWeaponCollisionResponseTypes::Unknown32))
        return EWeaponCollisionResponseTypes::Default;

    return skWorldMaterialTable[id];
}

bool CCollisionResponseData::ResponseTypeIsEnemyNormal(EWeaponCollisionResponseTypes type)
{
    return (type >= EWeaponCollisionResponseTypes::Unknown19 && type <= EWeaponCollisionResponseTypes::Unknown43);
}

bool CCollisionResponseData::ResponseTypeIsEnemySpecial(EWeaponCollisionResponseTypes type)
{
    return (type >= EWeaponCollisionResponseTypes::Unknown44 && type <= EWeaponCollisionResponseTypes::Unknown68);
}

bool CCollisionResponseData::ResponseTypeIsEnemyShielded(EWeaponCollisionResponseTypes type)
{
    return (type >= EWeaponCollisionResponseTypes::Unknown69 && type <= EWeaponCollisionResponseTypes::Unknown93);
}

FourCC CCollisionResponseData::UncookedResType()
{
    return SBIG('CRSM');
}

CFactoryFnReturn FCollisionResponseDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms)
{
    CSimplePool* sp = static_cast<CSimplePool*>(static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam());
    return TToken<CCollisionResponseData>::GetIObjObjectFor(std::unique_ptr<CCollisionResponseData>(new CCollisionResponseData(in, sp)));
}

}
