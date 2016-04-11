#ifndef __PSHAG_CMETAANIMPLAY_HPP__
#define __PSHAG_CMETAANIMPLAY_HPP__

#include "IMetaAnim.hpp"
#include "CPrimitive.hpp"
#include "IOStreams.hpp"

namespace urde
{

class CMetaAnimPlay : public IMetaAnim
{
    CPrimitive x4_primitive;
    float x1c_;
    u32 x20_;
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
