#include "CCollisionResponseData.hpp"

namespace pshag
{
CCollisionResponseData::CCollisionResponseData(CInputStream& in, CSimplePool* resPool)
{
}

std::unique_ptr<IObj> FCollisionResponseDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms)
{
    CSimplePool* sp = static_cast<CSimplePool*>(static_cast<TObjOwnerParam<IObjectStore*>*>(vparms.GetObj())->GetParam());
    return TToken<CCollisionResponseData>::GetIObjObjectFor(std::unique_ptr<CCollisionResponseData>(new CCollisionResponseData(in, sp)));
}

}
