#include "CDecalDataFactory.hpp"
#include "CDecalDescription.hpp"
#include "CGenDescription.hpp"
#include "CSwooshDescription.hpp"
#include "CElectricDescription.hpp"
#include "CParticleDataFactory.hpp"
#include "Graphics/CModel.hpp"
#include "CSimplePool.hpp"
#include "CRandom16.hpp"

namespace urde
{
static logvisor::Module Log("urde::CDecalDataFactory");

using CPF = CParticleDataFactory;
CDecalDescription* CDecalDataFactory::GetGeneratorDesc(CInputStream& in, CSimplePool* resPool)
{
    return CreateGeneratorDescription(in, resPool);
}

CDecalDescription* CDecalDataFactory::CreateGeneratorDescription(CInputStream& in, CSimplePool* resPool)
{
    FourCC clsId = CPF::GetClassID(in);
    if (clsId == FOURCC('DPSM'))
    {
        CDecalDescription* desc = new CDecalDescription;
        if (CreateDPSM(desc, in, resPool))
            return desc;
        else
            delete desc;
    }
    return nullptr;
}


bool CDecalDataFactory::CreateDPSM(CDecalDescription* desc, CInputStream& in, CSimplePool* resPool)
{
    CRandom16 rand{99};
    CGlobalRandom gr{rand};
    FourCC clsId = CPF::GetClassID(in);

    while (clsId != SBIG('_END'))
    {
        bool loadFirstDesc = false;
        switch (clsId)
        {
        case SBIG('1SZE'):
        case SBIG('1LFT'):
        case SBIG('1ROT'):
        case SBIG('1OFF'):
        case SBIG('1CLR'):
        case SBIG('1TEX'):
        case SBIG('1ADD'):
            loadFirstDesc = true;
        case SBIG('2LFT'):
        case SBIG('2SZE'):
        case SBIG('2ROT'):
        case SBIG('2OFF'):
        case SBIG('2CLR'):
        case SBIG('2TEX'):
        case SBIG('2ADD'):
            if (loadFirstDesc)
                GetQuadDecalInfo(in, resPool, clsId, desc->x0_Quad);
            else
                GetQuadDecalInfo(in, resPool, clsId, desc->x1c_Quad);
        break;

        case SBIG('DMDL'):
            desc->x38_DMDL = CPF::GetModel(in, resPool);
        break;
        case SBIG('DLFT'):
            desc->x48_DLFT.reset(CPF::GetIntElement(in));
        break;
        case SBIG('DMOP'):
            desc->x4c_DMOP.reset(CPF::GetVectorElement(in));
        break;
        case SBIG('DMRT'):
            desc->x50_DMRT.reset(CPF::GetVectorElement(in));
        break;
        case SBIG('DMSC'):
            desc->x54_DMSC.reset(CPF::GetVectorElement(in));
        break;
        case SBIG('DMCL'):
            desc->x58_DMCL.reset(CPF::GetColorElement(in));
        break;
        case SBIG('DMAB'):
            desc->x5c_24_DMAB = CPF::GetBool(in);
        break;
        case SBIG('DMOO'):
            desc->x5c_25_DMOO = CPF::GetBool(in);
        break;
        default:
        {
            uint32_t clsName = clsId.toUint32();
            Log.report(logvisor::Fatal, "Unknown DPSC class %.4s @%" PRIi64, &clsName, in.position());
            return false;
        }
        }

        clsId = CPF::GetClassID(in);
    }
    return true;
}

void CDecalDataFactory::GetQuadDecalInfo(CInputStream& in, CSimplePool* resPool, FourCC clsId, SQuadDescr& quad)
{
    switch (clsId)
    {
    case SBIG('1LFT'):
    case SBIG('2LFT'):
        quad.x0_LFT.reset(CPF::GetIntElement(in));
    break;
    case SBIG('1SZE'):
    case SBIG('2SZE'):
        quad.x4_SZE.reset(CPF::GetRealElement(in));
    break;
    case SBIG('1ROT'):
    case SBIG('2ROT'):
        quad.x8_ROT.reset(CPF::GetRealElement(in));
    break;
    case SBIG('1OFF'):
    case SBIG('2OFF'):
        quad.xc_OFF.reset(CPF::GetVectorElement(in));
    break;
    case SBIG('1CLR'):
    case SBIG('2CLR'):
        quad.x10_CLR.reset(CPF::GetColorElement(in));
    break;
    case SBIG('1TEX'):
    case SBIG('2TEX'):
        quad.x14_TEX.reset(CPF::GetTextureElement(in, resPool));
    break;
    case SBIG('1ADD'):
    case SBIG('2ADD'):
        quad.x18_ADD = CPF::GetBool(in);
    break;
    }
}

CFactoryFnReturn FDecalDataFactory(const SObjectTag &tag, CInputStream &in, const CVParamTransfer &vparms)
{
    CSimplePool* sp = static_cast<CSimplePool*>(static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam());
    return TToken<CDecalDescription>::GetIObjObjectFor(std::unique_ptr<CDecalDescription>(CDecalDataFactory::GetGeneratorDesc(in, sp)));
}

}
