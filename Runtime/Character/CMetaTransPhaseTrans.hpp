#pragma once

#include "IMetaTrans.hpp"
#include "IOStreams.hpp"
#include "CCharAnimTime.hpp"

namespace urde
{

class CMetaTransPhaseTrans : public IMetaTrans
{
    CCharAnimTime x4_transDur;
    bool xc_;
    bool xd_runA;
    u32 x10_flags;
public:
    CMetaTransPhaseTrans(CInputStream& in);
    EMetaTransType GetType() const {return EMetaTransType::PhaseTrans;}

    std::shared_ptr<CAnimTreeNode> VGetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                                      const std::weak_ptr<CAnimTreeNode>& b,
                                                      const CAnimSysContext& animSys) const;
};

}

