#ifndef __PSHAG_CMETATRANSPHASETRANS_HPP__
#define __PSHAG_CMETATRANSPHASETRANS_HPP__

#include "IMetaTrans.hpp"
#include "IOStreams.hpp"

namespace urde
{

class CMetaTransPhaseTrans : public IMetaTrans
{
    float x4_;
    u32 x8_;
    bool xc_;
    bool xd_;
    u32 x10_;
public:
    CMetaTransPhaseTrans(CInputStream& in);
    EMetaTransType GetType() const {return EMetaTransType::PhaseTrans;}

    std::shared_ptr<CAnimTreeNode> VGetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                                      const std::weak_ptr<CAnimTreeNode>& b,
                                                      const CAnimSysContext& animSys) const;
};

}

#endif // __PSHAG_CMETATRANSPHASETRANS_HPP__
