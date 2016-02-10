#include "CParticleSwooshDataFactory.hpp"
#include "CSwooshDescription.hpp"

#include "CSimplePool.hpp"

namespace Retro
{

std::unique_ptr<Retro::IObj> FParticleSwooshDataFactory(const SObjectTag &tag, CInputStream &in, const CVParamTransfer &vparms)
{
    CSimplePool* sp = static_cast<CSimplePool*>(static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam());
    return TToken<CSwooshDescription>::GetIObjObjectFor(std::unique_ptr<CSwooshDescription>(CParticleSwooshDataFactory::GetGeneratorDesc(in, sp)));
}
}
