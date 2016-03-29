#include "CProjectileWeaponDataFactory.hpp"
#include "CWeaponDescription.hpp"
#include "CCollisionResponseData.hpp"
#include "CElectricDescription.hpp"
#include "CSwooshDescription.hpp"
#include "CGenDescription.hpp"
#include "Graphics/CModel.hpp"
#include "CRandom16.hpp"
#include "CSimplePool.hpp"

namespace urde
{
static logvisor::Module Log("urde::CProjectileWeaponDataFactory");

using CPF = CParticleDataFactory;

CWeaponDescription*CProjectileWeaponDataFactory::GetGeneratorDesc(CInputStream& in, CSimplePool* resPool)
{
    return CreateGeneratorDescription(in, resPool);
}

CWeaponDescription* CProjectileWeaponDataFactory::CreateGeneratorDescription(CInputStream& in, CSimplePool* resPool)
{
    FourCC clsId = CPF::GetClassID(in);
    if (clsId == FOURCC('WPSM'))
    {
        CWeaponDescription* desc = new CWeaponDescription;
        CreateWPSM(desc, in, resPool);
    }

    return nullptr;
}

bool CProjectileWeaponDataFactory::CreateWPSM(CWeaponDescription* desc, CInputStream& in, CSimplePool* resPool)
{
    CRandom16 rand{99};
    CGlobalRandom gr{rand};
    FourCC clsId = CPF::GetClassID(in);

    while(clsId != SBIG('_END'))
    {
        switch(clsId)
        {
        case SBIG('IORN'):
            desc->x0_IORN.reset(CPF::GetVectorElement(in));
        break;
        case SBIG('IVEC'):
            desc->x4_IVEC.reset(CPF::GetVectorElement(in));
        break;
        case SBIG('PSVM'):
            desc->xc_PSVM.reset(CPF::GetModVectorElement(in));
        break;
        case SBIG('VMD2'):
            desc->x10_VMD2 = CPF::GetBool(in);
        break;
        case SBIG('PSLT'):
            desc->x14_PSLT.reset(CPF::GetIntElement(in));
        break;
        case SBIG('PCSL'):
            desc->x18_PCSL.reset(CPF::GetVectorElement(in));
        break;
        case SBIG('PCOL'):
            desc->x1c_PCOL.reset(CPF::GetColorElement(in));
        break;
        case SBIG('POFS'):
           desc->x20_POFS.reset(CPF::GetVectorElement(in));
        break;
        case SBIG('OFST'):
            desc->x24_OFST.reset(CPF::GetVectorElement(in));
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
            desc->x30_TRAT.reset(CPF::GetRealElement(in));
        break;
        case SBIG('APSM'):
        {
            std::vector<TResId> tracker;
            tracker.reserve(8);
            desc->x34_APSM = CPF::GetChildGeneratorDesc(in, resPool, tracker);
            break;
        }
        case SBIG('APS2'):
        {
            std::vector<TResId> tracker;
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
        case SBIG('COLR'):
        {
            FourCC cid = CPF::GetClassID(in);
            if (cid == SBIG('NONE'))
                break;
            TResId id = CPF::GetInt(in);
            if (id)
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
        case SBIG('PJFX'):
        {
            FourCC cid = CPF::GetClassID(in);
            if (cid == FOURCC('NONE'))
                break;

            desc->xa8_PJFX = CPF::GetInt(in);
            break;
        }
        case SBIG('RNGE'):
            desc->xac_RNGE.reset(CPF::GetRealElement(in));
        break;
        case SBIG('FOFF'):
            desc->xb0_FOFF.reset(CPF::GetRealElement(in));
        break;
        default:
        {
            uint32_t clsName = clsId.toUint32();
            Log.report(logvisor::Fatal, "Unknown WPSM class %.4s @%" PRIi64, &clsName, in.position());
            return false;
        }
        }
        clsId = CPF::GetClassID(in);
    }
    return true;
}

std::unique_ptr<IObj> FProjectileWeaponDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms)
{
    CSimplePool* sp = static_cast<CSimplePool*>(static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam());
    return TToken<CWeaponDescription>::GetIObjObjectFor(std::unique_ptr<CWeaponDescription>(CProjectileWeaponDataFactory::GetGeneratorDesc(in, sp)));
}

}
