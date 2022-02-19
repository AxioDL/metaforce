#pragma once

#include <memory>

#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/Character/IMetaAnim.hpp"
#include "Runtime/Character/IMetaTrans.hpp"

namespace metaforce {

class CMetaTransMetaAnim : public IMetaTrans {
  std::shared_ptr<IMetaAnim> x4_metaAnim;

public:
  explicit CMetaTransMetaAnim(CInputStream& in);
  EMetaTransType GetType() const override { return EMetaTransType::MetaAnim; }

  std::shared_ptr<CAnimTreeNode> VGetTransitionTree(const std::weak_ptr<CAnimTreeNode>& a,
                                                    const std::weak_ptr<CAnimTreeNode>& b,
                                                    const CAnimSysContext& animSys) const override;
};

} // namespace metaforce
