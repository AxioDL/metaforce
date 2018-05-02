#ifndef __URDE_MP1_CPARASITE_HPP__
#define __URDE_MP1_CPARASITE_HPP__

#include "World/CWallWalker.hpp"

namespace urde
{
class CModelData;
}

namespace urde::MP1
{
class CParasite : public CWallWalker
{
public:
    CParasite(TUniqueId, std::string_view, EFlavorType, const CEntityInfo&, const zeus::CTransform&, CModelData&&, const CPatternedInfo&);

    void Accept(IVisitor&);
};
}
#endif // __URDE_MP1_CPARASITE_HPP__
