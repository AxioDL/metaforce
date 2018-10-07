#ifndef __URDE_MP1_CSEEDLING_HPP__
#define __URDE_MP1_CSEEDLING_HPP__

#include "World/CWallWalker.hpp"

namespace urde
{
namespace MP1
{
class CSeedling : public CWallWalker
{
public:
    DEFINE_PATTERNED(Seedling)
    CSeedling(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&,
              CModelData&&, const CPatternedInfo&, const CActorParameters&,
              CAssetId, CAssetId, const CDamageInfo&, const CDamageInfo&,
              float, float, float, float);

    void Accept(IVisitor&);
};
}
}
#endif // __URDE_MP1_CSEEDLING_HPP__
