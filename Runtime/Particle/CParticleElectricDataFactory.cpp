#include "CParticleElectricDataFactory.hpp"
#include "CSwooshDescription.hpp"
#include "CElectricDescription.hpp"
#include "CGenDescription.hpp"
#include "Graphics/CModel.hpp"
#include "CToken.hpp"
#include "CSimplePool.hpp"
#include "CRandom16.hpp"

namespace urde
{
static logvisor::Module Log("urde::CParticleElectricDataFactory");

using CPF = CParticleDataFactory;

CElectricDescription* CParticleElectricDataFactory::GetGeneratorDesc(CInputStream& in, CSimplePool* resPool)
{
    return CreateElectricDescription(in, resPool);
}

CElectricDescription* CParticleElectricDataFactory::CreateElectricDescription(CInputStream& in, CSimplePool* resPool)
{
    FourCC cid = CPF::GetClassID(in);
    if (cid == FOURCC('ELSM'))
    {
        CElectricDescription* desc = new CElectricDescription;
        CreateELSM(desc, in, resPool);
        LoadELSMTokens(desc);
        return desc;
    }

    return nullptr;
}

bool CParticleElectricDataFactory::CreateELSM(CElectricDescription* desc, CInputStream& in, CSimplePool* resPool)
{
    CRandom16 rand;
    CGlobalRandom gr{rand};

    FourCC clsId = CPF::GetClassID(in);
    while (clsId != SBIG('_END'))
    {
        switch (clsId)
        {
        case SBIG('LIFE'):
            desc->x0_LIFE = CPF::GetIntElement(in);
            break;
        case SBIG('SLIF'):
            desc->x4_SLIF = CPF::GetIntElement(in);
            break;
        case SBIG('GRAT'):
            desc->x8_GRAT = CPF::GetRealElement(in);
            break;
        case SBIG('SCNT'):
            desc->xc_SCNT = CPF::GetIntElement(in);
            break;
        case SBIG('SSEG'):
            desc->x10_SSEG = CPF::GetIntElement(in);
            break;
        case SBIG('COLR'):
            desc->x14_COLR = CPF::GetColorElement(in);
            break;
        case SBIG('IEMT'):
            desc->x18_IEMT = CPF::GetEmitterElement(in);
            break;
        case SBIG('FEMT'):
            desc->x1c_FEMT = CPF::GetEmitterElement(in);
            break;
        case SBIG('AMPL'):
            desc->x20_AMPL = CPF::GetRealElement(in);
            break;
        case SBIG('AMPD'):
            desc->x24_AMPD = CPF::GetRealElement(in);
            break;
        case SBIG('LWD1'):
            desc->x28_LWD1 = CPF::GetRealElement(in);
            break;
        case SBIG('LWD2'):
            desc->x2c_LWD2 = CPF::GetRealElement(in);
            break;
        case SBIG('LWD3'):
            desc->x30_LWD3 = CPF::GetRealElement(in);
            break;
        case SBIG('LCL1'):
            desc->x34_LCL1 = CPF::GetColorElement(in);
            break;
        case SBIG('LCL2'):
            desc->x38_LCL2 = CPF::GetColorElement(in);
            break;
        case SBIG('LCL3'):
            desc->x3c_LCL3 = CPF::GetColorElement(in);
            break;
        case SBIG('SSWH'):
            desc->x40_SSWH = CPF::GetSwooshGeneratorDesc(in, resPool);
            break;
        case SBIG('GPSM'):
        {
            std::vector<CAssetId> tracker;
            tracker.reserve(8);
            desc->x50_GPSM = CPF::GetChildGeneratorDesc(in, resPool, tracker);
            break;
        }
        case SBIG('EPSM'):
        {
            std::vector<CAssetId> tracker;
            tracker.reserve(8);
            desc->x60_EPSM = CPF::GetChildGeneratorDesc(in, resPool, tracker);
            break;
        }
        case SBIG('ZERY'):
            desc->x70_ZERY = CPF::GetBool(in);
            break;
        default:
        {
            uint32_t clsName = clsId.toUint32();
            Log.report(logvisor::Fatal, "Unknown ELSM class %.4s @%" PRIi64, &clsName, in.position());
            return false;
        }
        }
        clsId = CPF::GetClassID(in);
    }
    return true;
}

void CParticleElectricDataFactory::LoadELSMTokens(CElectricDescription* desc)
{
    if (desc->x40_SSWH.m_found)
        desc->x40_SSWH.m_swoosh = desc->x40_SSWH.m_token.GetObj();
    if (desc->x50_GPSM.m_found)
        desc->x50_GPSM.m_gen = desc->x50_GPSM.m_token.GetObj();
    if (desc->x60_EPSM.m_found)
        desc->x60_EPSM.m_gen = desc->x60_EPSM.m_token.GetObj();
}

CFactoryFnReturn FParticleElectricDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                              CObjectReference*)
{
    CSimplePool* sp = vparms.GetOwnedObj<CSimplePool*>();
    return TToken<CElectricDescription>::GetIObjObjectFor(
        std::unique_ptr<CElectricDescription>(CParticleElectricDataFactory::GetGeneratorDesc(in, sp)));
}
}
