#pragma once

#include <memory>
#include <vector>

#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/Character/IMetaAnim.hpp"

namespace metaforce {

class CMetaAnimSequence : public IMetaAnim {
  std::vector<std::shared_ptr<IMetaAnim>> x4_sequence;
  std::vector<std::shared_ptr<IMetaAnim>> CreateSequence(CInputStream& in);

public:
  explicit CMetaAnimSequence(CInputStream& in);
  EMetaAnimType GetType() const override { return EMetaAnimType::Sequence; }

  void GetUniquePrimitives(std::set<CPrimitive>& primsOut) const override;
  std::shared_ptr<CAnimTreeNode> VGetAnimationTree(const CAnimSysContext& animSys,
                                                   const CMetaAnimTreeBuildOrders& orders) const override;
};

} // namespace metaforce
