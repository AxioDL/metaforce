#ifndef __PSHAG_CMETAANIMPHASEBLEND_HPP__
#define __PSHAG_CMETAANIMPHASEBLEND_HPP__

#include "IMetaAnim.hpp"
#include "IOStreams.hpp"

namespace urde
{

class CMetaAnimPhaseBlend : public IMetaAnim
{
public:
    CMetaAnimPhaseBlend(CInputStream& in);
    EMetaAnimType GetType() const {return EMetaAnimType::PhaseBlend;}

    std::shared_ptr<CAnimTreeNode> GetAnimationTree(const CAnimSysContext& animSys,
                                                    const CMetaAnimTreeBuildOrders& orders) const;
    void GetUniquePrimitives(std::set<CPrimitive>& primsOut) const;
    std::shared_ptr<CAnimTreeNode> VGetAnimationTree(const CAnimSysContext& animSys,
                                                     const CMetaAnimTreeBuildOrders& orders) const;
};

}

#endif // __PSHAG_CMETAANIMPHASEBLEND_HPP__
