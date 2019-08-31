#pragma once

#include "IMetaAnim.hpp"
#include "IOStreams.hpp"

namespace urde {

class CMetaAnimSequence : public IMetaAnim {
  std::vector<std::shared_ptr<IMetaAnim>> x4_sequence;
  std::vector<std::shared_ptr<IMetaAnim>> CreateSequence(CInputStream& in);

public:
  CMetaAnimSequence(CInputStream& in);
  EMetaAnimType GetType() const override { return EMetaAnimType::Sequence; }

  void GetUniquePrimitives(std::set<CPrimitive>& primsOut) const override;
  std::shared_ptr<CAnimTreeNode> VGetAnimationTree(const CAnimSysContext& animSys,
                                                   const CMetaAnimTreeBuildOrders& orders) const override;
};

} // namespace urde
