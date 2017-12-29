#ifndef __URDE_MP1_CSPACEPIRATE_HPP__
#define __URDE_MP1_CSPACEPIRATE_HPP__

#include "World/CPatterned.hpp"

namespace urde::MP1
{
class CSpacePirate : public CPatterned
{
public:
    CSpacePirate(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                 const CActorParameters&, const CPatternedInfo&, CInputStream&, u32);

    void Accept(IVisitor &visitor);
};
}
#endif // __URDE_MP1_CSPACEPIRATE_HPP__
