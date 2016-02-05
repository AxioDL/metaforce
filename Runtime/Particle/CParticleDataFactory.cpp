#include "CParticleDataFactory.hpp"
#include "CToken.hpp"
#include "CSimplePool.hpp"
#include "CGenDescription.hpp"

namespace Retro
{
static LogVisor::LogModule Log("Retro::CParticleDataFactory");

float CParticleDataFactory::GetReal(CInputStream& in)
{
    return in.readFloatBig();
}

int32_t CParticleDataFactory::GetInt(CInputStream& in)
{
    return in.readInt32Big();
}

bool CParticleDataFactory::GetBool(CInputStream& in)
{
    FourCC cid = GetClassID(in);
    if (cid != FOURCC('CNST'))
        Log.report(LogVisor::FatalError, "bool element does not begin with CNST");
    return in.readBool();
}

FourCC CParticleDataFactory::GetClassID(CInputStream& in)
{
    uint32_t val;
    in.readBytesToBuf(&val, 4);
    return val;
}

CGenDescription* CParticleDataFactory::GetGeneratorDesc(CInputStream& in, CSimplePool* resPool)
{
    std::vector<TResID> tracker;
    tracker.reserve(8);
    return CreateGeneratorDescription(in, tracker, 0, resPool);
}

CGenDescription* CParticleDataFactory::CreateGeneratorDescription(CInputStream& in, std::vector<TResID>& tracker,
                                                                  TResID resId, CSimplePool* resPool)
{
    if (std::count(tracker.cbegin(), tracker.cend(), resId) == 0)
    {
        tracker.push_back(resId);
        FourCC cid = GetClassID(in);
        if (cid == FOURCC('GPSM'))
        {
            CGenDescription* ret = new CGenDescription;
            CreateGPSM(ret, in, tracker, resPool);
            LoadGPSMTokens(ret);
            return ret;
        }
    }
    return nullptr;
}

bool CParticleDataFactory::CreateGPSM(CGenDescription* fillDesc, CInputStream& in,
                                      std::vector<TResID>& tracker, CSimplePool* resPool)
{
    FourCC clsId = GetClassID(in);
    while (clsId != SBIG('_END'))
    {
        switch (clsId)
        {
        case SBIG('PMCL'):
            fillDesc->x78_PMCL.reset(GetColorElement(in));
            break;
        case SBIG('LFOR'):
            fillDesc->x118_LFOR.reset(GetRealElement(in));
            break;
        case SBIG('IDTS'):
            fillDesc->xa4_IDTS = GetChildGeneratorDesc(in, resPool, tracker);
            break;
        case SBIG('EMTR'):
            fillDesc->x40_EMTR.reset(GetEmitterElement(in));
            break;
        case SBIG('COLR'):
            fillDesc->x30_COLR.reset(GetColorElement(in));
            break;
        case SBIG('CIND'):
            fillDesc->x45_30_CIND = GetBool(in);
            break;
        case SBIG('AAPH'):
            fillDesc->x44_26_AAPH = GetBool(in);
            break;
        case SBIG('CSSD'):
            fillDesc->xa0_CSSD.reset(GetIntElement(in));
            break;
        case SBIG('GRTE'):
            fillDesc->x2c_GRTE.reset(GetRealElement(in));
            break;
        case SBIG('FXLL'):
            fillDesc->x44_25_FXLL = GetBool(in);
            break;
        case SBIG('ICTS'):
            fillDesc->x8c_ICTS = GetChildGeneratorDesc(in, resPool, tracker);
            break;
        case SBIG('KSSM'):
        {
            fillDesc->xd0_KSSM.reset();
            FourCC cid = GetClassID(in);
            if (cid != SBIG('CNST'))
                break;
            fillDesc->xd0_KSSM.reset(new CSpawnSystemKeyframeData(in));
            fillDesc->xd0_KSSM->LoadAllSpawnedSystemTokens(resPool);
            break;
        }
        case SBIG('ILOC'):
            fillDesc->x38_ILOC.reset(GetVectorElement(in));
            break;
        case SBIG('IITS'):
            fillDesc->xb8_IITS = GetChildGeneratorDesc(in, resPool, tracker);
            break;
        case SBIG('IVEC'):
            fillDesc->x3c_IVEC.reset(GetVectorElement(in));
            break;
        case SBIG('LDIR'):
            fillDesc->x110_LDIR.reset(GetVectorElement(in));
            break;
        case SBIG('LCLR'):
            fillDesc->x104_LCLR.reset(GetColorElement(in));
            break;
        case SBIG('LENG'):
            fillDesc->x20_LENG.reset(GetRealElement(in));
            break;
        case SBIG('MAXP'):
            fillDesc->x28_MAXP.reset(GetIntElement(in));
            break;
        case SBIG('LOFF'):
            fillDesc->x10c_LOFF.reset(GetVectorElement(in));
            break;
        case SBIG('LINT'):
            fillDesc->x108_LINT.reset(GetRealElement(in));
            break;
        case SBIG('LINE'):
            fillDesc->x44_24_LINE = GetBool(in);
            break;
        case SBIG('LFOT'):
            fillDesc->x114_LFOT.reset(GetIntElement(in));
            break;
        case SBIG('LIT_'):
            fillDesc->x44_29_LIT_ = GetBool(in);
            break;
        case SBIG('LTME'):
            fillDesc->x34_LTME.reset(GetIntElement(in));
            break;
        case SBIG('LSLA'):
            fillDesc->x11c_LSLA.reset(GetRealElement(in));
            break;
        case SBIG('LTYP'):
            fillDesc->x100_LTYP.reset(GetIntElement(in));
            break;
        case SBIG('NDSY'):
            fillDesc->xb4_NDSY.reset(GetIntElement(in));
            break;
        case SBIG('MBSP'):
            fillDesc->x48_MBSP.reset(GetIntElement(in));
            break;
        case SBIG('MBLR'):
            fillDesc->x44_30_MBLR = GetBool(in);
            break;
        case SBIG('NCSY'):
            fillDesc->x9c_NCSY.reset(GetIntElement(in));
            break;
        case SBIG('PISY'):
            fillDesc->xc8_PISY.reset(GetIntElement(in));
            break;
        case SBIG('OPTS'):
            fillDesc->x45_31_OPTS = GetBool(in);
            break;
        case SBIG('PMAB'):
            fillDesc->x44_31_PMAB = GetBool(in);
            break;
        case SBIG('SESD'):
            fillDesc->xf8_SESD.reset(GetIntElement(in));
            break;
        case SBIG('SEPO'):
            fillDesc->xfc_SEPO.reset(GetVectorElement(in));
            break;
        case SBIG('PSLT'):
            fillDesc->xc_PSLT.reset(GetIntElement(in));
            break;
        case SBIG('PMSC'):
            fillDesc->x74_PMSC.reset(GetVectorElement(in));
            break;
        case SBIG('PMOP'):
            fillDesc->x6c_PMOP.reset(GetVectorElement(in));
            break;
        case SBIG('PMDL'):
            fillDesc->x5c_PMDL = GetModel(in, resPool);
            break;
        case SBIG('PMRT'):
            fillDesc->x70_PMRT.reset(GetVectorElement(in));
            break;
        case SBIG('POFS'):
            fillDesc->x18_POFS.reset(GetVectorElement(in));
            break;
        case SBIG('PMUS'):
            fillDesc->x45_24_PMUS = GetBool(in);
            break;
        case SBIG('PSIV'):
            fillDesc->x0_PSIV.reset(GetVectorElement(in));
            break;
        case SBIG('ROTA'):
            fillDesc->x50_ROTA.reset(GetRealElement(in));
            break;
        case SBIG('PSVM'):
            fillDesc->x4_PSVM.reset(GetModVectorElement(in));
            break;
        case SBIG('PSTS'):
            fillDesc->x14_PSTS.reset(GetRealElement(in));
            break;
        case SBIG('PSOV'):
            fillDesc->x8_PSOV.reset(GetVectorElement(in));
            break;
        case SBIG('PSWT'):
            fillDesc->x10_PSWT.reset(GetIntElement(in));
            break;
        case SBIG('PMLC'):
            fillDesc->xec_PMLC = GetChildGeneratorDesc(in, resPool, tracker);
            break;
        case SBIG('PMED'):
            fillDesc->x1c_PMED.reset(GetIntElement(in));
            break;
        case SBIG('PMOO'):
            fillDesc->x45_25_PMOO = GetBool(in);
            break;
        case SBIG('SSSD'):
            fillDesc->xe4_SSSD.reset(GetIntElement(in));
            break;
        case SBIG('SORT'):
            fillDesc->x44_28_SORT = GetBool(in);
            break;
        case SBIG('SIZE'):
            fillDesc->x4c_SIZE.reset(GetRealElement(in));
            break;
        case SBIG('SISY'):
            fillDesc->xcc_SISY.reset(GetIntElement(in));
            break;
        case SBIG('SSPO'):
            fillDesc->xe8_SSPO.reset(GetVectorElement(in));
            break;
        case SBIG('TEXR'):
            fillDesc->x54_TEXR.reset(GetTextureElement(in, resPool));
            break;
        case SBIG('SSWH'):
            fillDesc->xd4_SSWH = GetSwooshGeneratorDesc(in, resPool);
            break;
        case SBIG('TIND'):
            fillDesc->x58_TIND.reset(GetTextureElement(in, resPool));
            break;
        case SBIG('VMD4'):
            fillDesc->x45_29_VMD4 = GetBool(in);
            break;
        case SBIG('VMD3'):
            fillDesc->x45_28_VMD3 = GetBool(in);
            break;
        case SBIG('VMD2'):
            fillDesc->x45_27_VMD2 = GetBool(in);
            break;
        case SBIG('VMD1'):
            fillDesc->x45_26_VMD1 = GetBool(in);
            break;
        case SBIG('VEL4'):
            fillDesc->x88_VEL4.reset(GetModVectorElement(in));
            break;
        case SBIG('VEL3'):
            fillDesc->x84_VEL3.reset(GetModVectorElement(in));
            break;
        case SBIG('VEL2'):
            fillDesc->x80_VEL2.reset(GetModVectorElement(in));
            break;
        case SBIG('VEL1'):
            fillDesc->x7c_VEL1.reset(GetModVectorElement(in));
            break;
        case SBIG('ZBUF'):
            fillDesc->x44_27_ZBUF = GetBool(in);
            break;
        case SBIG('WIDT'):
            fillDesc->x24_WIDT.reset(GetRealElement(in));
            break;
        case SBIG('ORNT'):
            fillDesc->x30_30_ORNT = GetBool(in);
            break;
        case SBIG('RSOP'):
            fillDesc->x30_31_RSOP = GetBool(in);
            break;
        case SBIG('SEED'):
            fillDesc->x10_SEED.reset(GetIntElement(in));
            break;
        case SBIG('ADV1'):
            fillDesc->x10c_ADV1.reset(GetRealElement(in));
            break;
        case SBIG('ADV2'):
            fillDesc->x110_ADV2.reset(GetRealElement(in));
            break;
        case SBIG('ADV3'):
            fillDesc->x114_ADV3.reset(GetRealElement(in));
            break;
        case SBIG('ADV4'):
            fillDesc->x118_ADV4.reset(GetRealElement(in));
            break;
        case SBIG('ADV5'):
            fillDesc->x11c_ADV5.reset(GetRealElement(in));
            break;
        case SBIG('ADV6'):
            fillDesc->x120_ADV6.reset(GetRealElement(in));
            break;
        case SBIG('ADV7'):
            fillDesc->x124_ADV7.reset(GetRealElement(in));
            break;
        case SBIG('ADV8'):
            fillDesc->x128_ADV8.reset(GetRealElement(in));
            break;
        case SBIG('SELC'):
            fillDesc->xd8_SELC = GetElectricGeneratorDesc(in, resPool);
            break;
        default:
        {
            uint32_t clsName = clsId.toUint32();
            Log.report(LogVisor::FatalError, "Unknown GPSM class %.4s @%" PRIi64, &clsName, in.position());
            return false;
        }
        }
        clsId = GetClassID(in);
    }
    return true;
}

void CParticleDataFactory::LoadGPSMTokens(CGenDescription* desc)
{
}

std::unique_ptr<IObj> FParticleFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms)
{
    CSimplePool* sp = static_cast<CSimplePool*>(static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam());
    return TToken<CGenDescription>::GetIObjObjectFor(std::unique_ptr<CGenDescription>(CParticleDataFactory::GetGeneratorDesc(in, sp)));
}

}
