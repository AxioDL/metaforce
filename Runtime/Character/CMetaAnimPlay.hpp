#pragma once

#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/Character/CPrimitive.hpp"
#include "Runtime/Character/IMetaAnim.hpp"

namespace metaforce {

class CMetaAnimPlay : public IMetaAnim {
  CPrimitive x4_primitive;
  CCharAnimTime x1c_startTime;

public:
  explicit CMetaAnimPlay(CInputStream& in);
  EMetaAnimType GetType() const override { return EMetaAnimType::Play; }

  void GetUniquePrimitives(std::set<CPrimitive>& primsOut) const override;
  std::shared_ptr<CAnimTreeNode> VGetAnimationTree(const CAnimSysContext& animSys,
                                                   const CMetaAnimTreeBuildOrders& orders) const override;
};

} // namespace metaforce
