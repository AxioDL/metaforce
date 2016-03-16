#include "CPathFindArea.hpp"
#include "IVParamObj.hpp"
#include "CToken.hpp"

namespace urde
{
CPathFindArea::CPathFindArea(const std::unique_ptr<u8[]>&& buf, int len)
{
    x13c_data = buf.get();
}

std::unique_ptr<IObj> FPathFindAreaFactory(const SObjectTag& /*tag*/, const std::unique_ptr<u8[]>& buf, const CVParamTransfer &xfer)
{
    return TToken<CPathFindArea>::GetIObjObjectFor(std::unique_ptr<CPathFindArea>(new CPathFindArea(std::move(buf), *reinterpret_cast<int*>(xfer.GetObj()))));
}
}
