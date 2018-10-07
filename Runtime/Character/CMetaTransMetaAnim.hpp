#pragma once

#include "IMetaTrans.hpp"
#include "IMetaAnim.hpp"
#include "IOStreams.hpp"

namespace urde
{

class CMetaTransMetaAnim : public IMetaTrans
{
    std::shared_ptr<IMetaAnim> x4_metaAnim;
public:
    CMetaTransMetaAnim(CInputStream& in);
    EMetaTransType GetType() const {return EMetaTransType::MetaAnim;}

    std::shared_ptr<CAnimTreeNode> VGetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                                      const std::weak_ptr<CAnimTreeNode>& b,
                                                      const CAnimSysContext& animSys) const;
};

}

