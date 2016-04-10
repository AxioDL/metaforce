#ifndef __PSHAG_CMETAANIMPLAY_HPP__
#define __PSHAG_CMETAANIMPLAY_HPP__

#include "IMetaAnim.hpp"
#include "IOStreams.hpp"

namespace urde
{

class CMetaAnimPlay : public IMetaAnim
{
public:
    CMetaAnimPlay(CInputStream& in);
    EMetaAnimType GetType() const {return EMetaAnimType::Primitive;}

    std::shared_ptr<CAnimTreeNode> GetAnimationTree(const CAnimSysContext& animSys,
                                                    const CMetaAnimTreeBuildOrders& orders) const;
    void GetUniquePrimitives(std::set<CPrimitive>& primsOut) const;
    std::shared_ptr<CAnimTreeNode> VGetAnimationTree(const CAnimSysContext& animSys,
                                                     const CMetaAnimTreeBuildOrders& orders) const;
};

}

#endif // __PSHAG_CMETAANIMPLAY_HPP__
