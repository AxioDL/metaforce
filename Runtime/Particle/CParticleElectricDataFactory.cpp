#include "CParticleElectricDataFactory.hpp"
#include "CElectricDescription.hpp"
#include "CSimplePool.hpp"

namespace Retro
{

CElectricDescription* CParticleElectricDataFactory::GetGeneratorDesc(CInputStream &in, CSimplePool *resPool)
{
    return CreateElectricDescription(in, resPool);
}

CElectricDescription* CParticleElectricDataFactory::CreateElectricDescription(CInputStream &in, CSimplePool *resPool)
{
    return nullptr;
}

std::unique_ptr<Retro::IObj> FParticleElecrticFactory(const Retro::SObjectTag &tag, Retro::CInputStream &in, const Retro::CVParamTransfer &vparms)
{
    CSimplePool* sp = static_cast<CSimplePool*>(static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam());
    return TToken<CElectricDescription>::GetIObjObjectFor(std::unique_ptr<CElectricDescription>(CParticleElectricDataFactory::GetGeneratorDesc(in, sp)));
}

}
