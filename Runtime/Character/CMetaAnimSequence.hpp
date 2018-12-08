#pragma once

#include "IMetaAnim.hpp"
#include "IOStreams.hpp"

namespace urde {

class CMetaAnimSequence : public IMetaAnim {
  std::vector<std::shared_ptr<IMetaAnim>> x4_sequence;
  std::vector<std::shared_ptr<IMetaAnim>> CreateSequence(CInputStream& in);

public:
  CMetaAnimSequence(CInputStream& in);
  EMetaAnimType GetType() const { return EMetaAnimType::Sequence; }

  void GetUniquePrimitives(std::set<CPrimitive>& primsOut) const;
  std::shared_ptr<CAnimTreeNode> VGetAnimationTree(const CAnimSysContext& animSys,
                                                   const CMetaAnimTreeBuildOrders& orders) const;
};

} // namespace urde
