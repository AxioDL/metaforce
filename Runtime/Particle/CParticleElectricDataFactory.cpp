#include "CParticleElectricDataFactory.hpp"
#include "CElectricDescription.hpp"
#include "CToken.hpp"
#include "CSimplePool.hpp"
#include "CRandom16.hpp"

namespace pshag
{
static LogVisor::LogModule Log("Retro::CParticleElectricDataFactory");

using CPF = CParticleDataFactory;

CElectricDescription* CParticleElectricDataFactory::GetGeneratorDesc(CInputStream &in, CSimplePool *resPool)
{
    return CreateElectricDescription(in, resPool);
}

CElectricDescription* CParticleElectricDataFactory::CreateElectricDescription(CInputStream &in, CSimplePool *resPool)
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

bool CParticleElectricDataFactory::CreateELSM(CElectricDescription *desc, CInputStream &in, CSimplePool *resPool)
{
    CRandom16 rand{99};
    CGlobalRandom gr{rand};

    FourCC clsId = CPF::GetClassID(in);
    while (clsId != SBIG('_END'))
    {
        switch(clsId)
        {
        case SBIG('LIFE'):
            desc->x0_LIFE.reset(CPF::GetIntElement(in));
        break;
        case SBIG('SLIF'):
            desc->x4_SLIF.reset(CPF::GetIntElement(in));
        break;
        case SBIG('GRAT'):
            desc->x8_GRAT.reset(CPF::GetRealElement(in));
        break;
        case SBIG('SCNT'):
            desc->xc_SCNT.reset(CPF::GetIntElement(in));
        break;
        case SBIG('SSEG'):
            desc->x10_SSEG.reset(CPF::GetIntElement(in));
        break;
        case SBIG('COLR'):
            desc->x14_COLR.reset(CPF::GetColorElement(in));
        break;
        case SBIG('IEMT'):
            desc->x18_IEMT.reset(CPF::GetEmitterElement(in));
        break;
        case SBIG('FEMT'):
            desc->x1c_FEMT.reset(CPF::GetEmitterElement(in));
        break;
        case SBIG('AMPL'):
            desc->x20_AMPL.reset(CPF::GetRealElement(in));
        break;
        case SBIG('AMPD'):
            desc->x24_AMPD.reset(CPF::GetRealElement(in));
        break;
        case SBIG('LWD1'):
            desc->x28_LWD1.reset(CPF::GetRealElement(in));
        break;
        case SBIG('LWD2'):
            desc->x2c_LWD2.reset(CPF::GetRealElement(in));
        break;
        case SBIG('LWD3'):
            desc->x30_LWD3.reset(CPF::GetRealElement(in));
        break;
        case SBIG('LCL1'):
            desc->x34_LCL1.reset(CPF::GetColorElement(in));
        break;
        case SBIG('LCL2'):
            desc->x38_LCL2.reset(CPF::GetColorElement(in));
        break;
        case SBIG('LCL3'):
            desc->x3c_LCL3.reset(CPF::GetColorElement(in));
        break;
        case SBIG('SSWH'):
            desc->x40_SSWH = CPF::GetSwooshGeneratorDesc(in, resPool);
        break;
        case SBIG('GPSM'):
        {
            std::vector<TResId> tracker;
            tracker.reserve(8);
            desc->x50_GPSM = CPF::GetChildGeneratorDesc(in, resPool, tracker);
            break;
        }
        case SBIG('EPSM'):
        {
            std::vector<TResId> tracker;
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
            Log.report(LogVisor::FatalError, "Unknown ELSM class %.4s @%" PRIi64, &clsName, in.position());
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

std::unique_ptr<pshag::IObj> FParticleElecrticFactory(const pshag::SObjectTag &tag, pshag::CInputStream &in, const pshag::CVParamTransfer &vparms)
{
    CSimplePool* sp = static_cast<CSimplePool*>(static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam());
    return TToken<CElectricDescription>::GetIObjObjectFor(std::unique_ptr<CElectricDescription>(CParticleElectricDataFactory::GetGeneratorDesc(in, sp)));
}

}
