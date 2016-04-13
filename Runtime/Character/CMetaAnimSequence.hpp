#ifndef __URDE_CMETAANIMSEQUENCE_HPP__
#define __URDE_CMETAANIMSEQUENCE_HPP__

#include "IMetaAnim.hpp"
#include "IOStreams.hpp"

namespace urde
{

class CMetaAnimSequence : public IMetaAnim
{
    std::vector<std::shared_ptr<IMetaAnim>> x4_sequence;
    std::vector<std::shared_ptr<IMetaAnim>> CreateSequence(CInputStream& in);
public:
    CMetaAnimSequence(CInputStream& in);
    EMetaAnimType GetType() const {return EMetaAnimType::Sequence;}

    std::shared_ptr<CAnimTreeNode> GetAnimationTree(const CAnimSysContext& animSys,
                                                    const CMetaAnimTreeBuildOrders& orders) const;
    void GetUniquePrimitives(std::set<CPrimitive>& primsOut) const;
    std::shared_ptr<CAnimTreeNode> VGetAnimationTree(const CAnimSysContext& animSys,
                                                     const CMetaAnimTreeBuildOrders& orders) const;
};

}

#endif // __URDE_CMETAANIMSEQUENCE_HPP__
