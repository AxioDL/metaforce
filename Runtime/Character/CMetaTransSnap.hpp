#pragma once

#include "Runtime/IOStreams.hpp"
#include "Runtime/Character/IMetaTrans.hpp"

namespace urde {

class CMetaTransSnap : public IMetaTrans {
public:
  EMetaTransType GetType() const override { return EMetaTransType::Snap; }

  std::shared_ptr<CAnimTreeNode> VGetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                                    const std::weak_ptr<CAnimTreeNode>& b,
                                                    const CAnimSysContext& animSys) const override;
};

} // namespace urde
