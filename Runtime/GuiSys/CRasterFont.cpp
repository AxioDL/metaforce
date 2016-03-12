#include "GuiSys/CRasterFont.hpp"

namespace urde
{
CRasterFont::CRasterFont(urde::CInputStream& in, urde::IObjectStore& store)
{
}

std::unique_ptr<IObj> FRasterFontFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms)
{
    return TToken<CRasterFont>::GetIObjObjectFor(std::make_unique<CRasterFont>(in, *(reinterpret_cast<IObjectStore*>(vparms.GetObj()))));
}

}
