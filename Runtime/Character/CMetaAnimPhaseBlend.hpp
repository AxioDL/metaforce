#ifndef __URDE_CMETAANIMPHASEBLEND_HPP__
#define __URDE_CMETAANIMPHASEBLEND_HPP__

#include "IMetaAnim.hpp"
#include "IOStreams.hpp"

namespace urde
{

class CMetaAnimPhaseBlend : public IMetaAnim
{
    std::shared_ptr<IMetaAnim> x4_animA;
    std::shared_ptr<IMetaAnim> x8_animB;
    float xc_blend;
    bool x10_;
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

#endif // __URDE_CMETAANIMPHASEBLEND_HPP__
