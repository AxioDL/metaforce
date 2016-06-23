#include "CCollisionResponseData.hpp"
#include "CParticleDataFactory.hpp"
#include "CDecalDescription.hpp"
#include "CSwooshDescription.hpp"
#include "CElectricDescription.hpp"
#include "CGenDescription.hpp"
#include "Graphics/CModel.hpp"
#include "CSimplePool.hpp"
#include "CRandom16.hpp"

namespace urde
{
static const std::vector<FourCC> SFXTypes =
{
    SBIG('DSFX'),SBIG('CSFX'),SBIG('MSFX'),SBIG('GRFX'),
    SBIG('ICFX'),SBIG('GOFX'),SBIG('WSFX'),SBIG('WTFX'),
    SBIG('2MUD'),SBIG('2LAV'),SBIG('2SAN'),SBIG('2PRJ'),
    SBIG('DCFX'),SBIG('DSFX'),SBIG('DSHX'),SBIG('DEFX'),
    SBIG('ESFX'),SBIG('SHFX'),SBIG('BEFX'),SBIG('WWFX'),
    SBIG('TAFX'),SBIG('GTFX'),SBIG('SPFX'),SBIG('FPFX'),
    SBIG('FFFX'),SBIG('PAFX'),SBIG('BMFX'),SBIG('BFFX'),
    SBIG('PBFX'),SBIG('IBFX'),SBIG('4SVA'),SBIG('4RPR'),
    SBIG('4MTR'),SBIG('4PDS'),SBIG('4FLB'),SBIG('4DRN'),
    SBIG('4MRE'),SBIG('CZFX'),SBIG('JZAS'),SBIG('2ISE'),
    SBIG('2BSE'),SBIG('2ATB'),SBIG('2ATA'),SBIG('BSFX'),
    SBIG('WSFX'),SBIG('TSFX'),SBIG('GSFX'),SBIG('SSFX'),
    SBIG('FSFX'),SBIG('SFFX'),SBIG('PSFX'),SBIG('MSFX'),
    SBIG('SBFX'),SBIG('PBSX'),SBIG('IBSX'),SBIG('5SVA'),
    SBIG('5RPR'),SBIG('5MTR'),SBIG('5PDS'),SBIG('5FLB'),
    SBIG('5DRN'),SBIG('5MRE'),SBIG('CSFX'),SBIG('JZPS'),
    SBIG('4ISE'),SBIG('4BSE'),SBIG('4ATB'),SBIG('4ATA'),
    SBIG('BHFX'),SBIG('WHFX'),SBIG('THFX'),SBIG('GHFX'),
    SBIG('SHFX'),SBIG('FHFX'),SBIG('HFFX'),SBIG('PHFX'),
    SBIG('MHFX'),SBIG('HBFX'),SBIG('PBHX'),SBIG('IBHX'),
    SBIG('6SVA'),SBIG('6RPR'),SBIG('6MTR'),SBIG('6PDS'),
    SBIG('6FLB'),SBIG('6DRN'),SBIG('6MRE'),SBIG('CHFX'),
    SBIG('JZHS'),SBIG('6ISE'),SBIG('6BSE'),SBIG('6ATB'),
    SBIG('6ATA')
};
static const std::vector<FourCC> GeneratorTypes =
{
    SBIG('DEFS'),SBIG('CRTS'),SBIG('MTLS'),SBIG('GRAS'),
    SBIG('ICEE'),SBIG('GOOO'),SBIG('WODS'),SBIG('WATR'),
    SBIG('1MUD'),SBIG('1LAV'),SBIG('1SAN'),SBIG('1PRJ'),
    SBIG('DCHR'),SBIG('DCHS'),SBIG('DCSH'),SBIG('DENM'),
    SBIG('DESP'),SBIG('DESH'),SBIG('BTLE'),SBIG('WASP'),
    SBIG('TALP'),SBIG('PTGM'),SBIG('SPIR'),SBIG('FPIR'),
    SBIG('FFLE'),SBIG('PARA'),SBIG('BMON'),SBIG('BFLR'),
    SBIG('PBOS'),SBIG('IBOS'),SBIG('1SVA'),SBIG('1RPR'),
    SBIG('1MTR'),SBIG('1PDS'),SBIG('1FLB'),SBIG('1DRN'),
    SBIG('1MRE'),SBIG('CHOZ'),SBIG('JZAP'),SBIG('1ISE'),
    SBIG('1BSE'),SBIG('1ATB'),SBIG('1ATA'),SBIG('BTSP'),
    SBIG('WWSP'),SBIG('TASP'),SBIG('TGSP'),SBIG('SPSP'),
    SBIG('FPSP'),SBIG('FFSP'),SBIG('PSSP'),SBIG('BMSP'),
    SBIG('BFSP'),SBIG('PBSP'),SBIG('IBSP'),SBIG('2SVA'),
    SBIG('2RPR'),SBIG('2MTR'),SBIG('2PDS'),SBIG('2FLB'),
    SBIG('2DRN'),SBIG('2MRE'),SBIG('CHSP'),SBIG('JZSP'),
    SBIG('3ISE'),SBIG('3BSE'),SBIG('3ATB'),SBIG('3ATA'),
    SBIG('BTSH'),SBIG('WWSH'),SBIG('TASH'),SBIG('TGSH'),
    SBIG('SPSH'),SBIG('FPSH'),SBIG('FFSH'),SBIG('PSSH'),
    SBIG('BMSH'),SBIG('BFSH'),SBIG('PBSH'),SBIG('IBSH'),
    SBIG('3SVA'),SBIG('3RPR'),SBIG('3MTR'),SBIG('3PDS'),
    SBIG('3FLB'),SBIG('3DRN'),SBIG('3MRE'),SBIG('CHSH'),
    SBIG('JZSH'),SBIG('5ISE'),SBIG('5BSE'),SBIG('5ATB'),
    SBIG('5ATA')
};
static const std::vector<FourCC> DecalTypes =
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
    x0_generators[i].emplace(std::move(CPF::GetChildGeneratorDesc(in, resPool, tracker).m_token));
}

bool CCollisionResponseData::CheckAndAddDecalToResponse(FourCC clsId, CInputStream& in, CSimplePool* resPool)
{
    int i = 0;
    for (const FourCC& type : DecalTypes)
    {
        if (type == clsId)
        {
            FourCC cls = CPF::GetClassID(in);
            if (cls == SBIG('NONE'))
                return true;

            ResId id = CPF::GetInt(in);
            if (!id)
                return true;

            x20_decals[i].emplace(std::move(resPool->GetObj({FOURCC('DPSC'), id})));
            return true;
        }
        i++;
    }
    return false;
}

bool CCollisionResponseData::CheckAndAddSoundFXToResponse(FourCC clsId, CInputStream& in)
{
    int i = 0;
    for (const FourCC& type : SFXTypes)
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
    for (const FourCC& type : GeneratorTypes)
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
    for (ResId& id : x10_sfx)
        id = ~0;

    FourCC clsId = CPF::GetClassID(in);
    if (clsId == SBIG('CRSM'))
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

CFactoryFnReturn FCollisionResponseDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms)
{
    CSimplePool* sp = static_cast<CSimplePool*>(static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam());
    return TToken<CCollisionResponseData>::GetIObjObjectFor(std::unique_ptr<CCollisionResponseData>(new CCollisionResponseData(in, sp)));
}

}
