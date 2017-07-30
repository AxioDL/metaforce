#include "CPathFindArea.hpp"
#include "IVParamObj.hpp"
#include "CToken.hpp"

namespace urde
{
CPFArea::CPFArea(const std::unique_ptr<u8[]>&& buf, int len)
{
}

std::unique_ptr<IObj> FPathFindAreaFactory(const SObjectTag& /*tag*/, const std::unique_ptr<u8[]>& buf,
                                           const CVParamTransfer &xfer)
{
    return TToken<CPFArea>::GetIObjObjectFor(
                std::make_unique<CPFArea>(std::move(buf), *reinterpret_cast<int*>(xfer.GetObj())));
}
}
