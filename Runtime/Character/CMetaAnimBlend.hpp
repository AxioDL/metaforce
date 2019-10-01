#pragma once

#include <memory>

#include "Runtime/IOStreams.hpp"
#include "Runtime/Character/IMetaAnim.hpp"

namespace urde {

class CMetaAnimBlend : public IMetaAnim {
  std::shared_ptr<IMetaAnim> x4_animA;
  std::shared_ptr<IMetaAnim> x8_animB;
  float xc_blend;
  bool x10_;

public:
  CMetaAnimBlend(CInputStream& in);
  EMetaAnimType GetType() const override { return EMetaAnimType::Blend; }

  void GetUniquePrimitives(std::set<CPrimitive>& primsOut) const override;
  std::shared_ptr<CAnimTreeNode> VGetAnimationTree(const CAnimSysContext& animSys,
                                                   const CMetaAnimTreeBuildOrders& orders) const override;
};

} // namespace urde
