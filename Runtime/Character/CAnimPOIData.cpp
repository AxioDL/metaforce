#include "CAnimPOIData.hpp"
#include "CToken.hpp"

namespace urde
{

CAnimPOIData::CAnimPOIData(CInputStream& in)
{
}

CFactoryFnReturn AnimPOIDataFactory(const SObjectTag& tag, CInputStream& in,
                                    const CVParamTransfer& parms)
{
    return TToken<CAnimPOIData>::GetIObjObjectFor(std::make_unique<CAnimPOIData>(in));
}

}
