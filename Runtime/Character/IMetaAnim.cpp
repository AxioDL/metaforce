#include "Runtime/Character/IMetaAnim.hpp"

#include <array>

#include "Runtime/Character/CAnimTreeNode.hpp"
#include "Runtime/Character/CBoolPOINode.hpp"
#include "Runtime/Character/CCharAnimTime.hpp"
#include "Runtime/Character/IAnimReader.hpp"

namespace metaforce {

std::shared_ptr<CAnimTreeNode> IMetaAnim::GetAnimationTree(const CAnimSysContext& animSys,
                                                           const CMetaAnimTreeBuildOrders& orders) const {
  if (orders.x44_singleAdvance) {
    std::shared_ptr<CAnimTreeNode> tree = VGetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders());
    if (orders.x44_singleAdvance->IsTime() || orders.x44_singleAdvance->IsString()) {
      CCharAnimTime time = GetTime(*orders.x44_singleAdvance, *tree);
      AdvanceAnim(*tree, time);
    }
    return tree;
  }
  return VGetAnimationTree(animSys, CMetaAnimTreeBuildOrders::NoSpecialOrders());
}

void IMetaAnim::AdvanceAnim(IAnimReader& anim, const CCharAnimTime& dt) {
  CCharAnimTime remDt = dt;
  while (remDt > CCharAnimTime()) {
    SAdvancementResults res = anim.VAdvanceView(remDt);
    remDt = res.x0_remTime;
  }
}

CCharAnimTime IMetaAnim::GetTime(const CPreAdvanceIndicator& ind, const IAnimReader& anim) {
  if (ind.IsTime()) {
    return ind.GetTime();
  }

  std::array<CBoolPOINode, 64> nodes;
  const CCharAnimTime rem = anim.VGetTimeRemaining();
  const size_t count = anim.VGetBoolPOIList(rem, nodes.data(), nodes.size(), 0, 0);
  const char* cmpStr = ind.GetString();
  for (size_t i = 0; i < count; ++i) {
    const CBoolPOINode& node = nodes[i];
    if (node.GetString() != cmpStr || !node.GetValue()) {
      continue;
    }
    return node.GetTime();
  }

  return {};
}

} // namespace metaforce
