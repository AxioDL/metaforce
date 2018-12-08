#pragma once

#include "IMetaAnim.hpp"
#include "CPrimitive.hpp"
#include "IOStreams.hpp"

namespace urde {

class CMetaAnimPlay : public IMetaAnim {
  CPrimitive x4_primitive;
  CCharAnimTime x1c_startTime;

public:
  CMetaAnimPlay(CInputStream& in);
  EMetaAnimType GetType() const { return EMetaAnimType::Play; }

  void GetUniquePrimitives(std::set<CPrimitive>& primsOut) const;
  std::shared_ptr<CAnimTreeNode> VGetAnimationTree(const CAnimSysContext& animSys,
                                                   const CMetaAnimTreeBuildOrders& orders) const;
};

} // namespace urde
