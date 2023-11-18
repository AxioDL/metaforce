#include "Runtime/Character/CMetaAnimRandom.hpp"

#include "Runtime/Character/CAnimSysContext.hpp"
#include "Runtime/Character/CMetaAnimFactory.hpp"

namespace metaforce {

CMetaAnimRandom::RandomData CMetaAnimRandom::CreateRandomData(CInputStream& in) {
  CMetaAnimRandom::RandomData ret;
  u32 randCount = in.ReadLong();
  ret.reserve(randCount);

  for (u32 i = 0; i < randCount; ++i) {
    std::shared_ptr<IMetaAnim> metaAnim = CMetaAnimFactory::CreateMetaAnim(in);
    ret.emplace_back(std::move(metaAnim), in.ReadLong());
  }

  return ret;
}

CMetaAnimRandom::CMetaAnimRandom(CInputStream& in) : x4_randomData(CreateRandomData(in)) {}

void CMetaAnimRandom::GetUniquePrimitives(std::set<CPrimitive>& primsOut) const {
  for (const auto& pair : x4_randomData)
    pair.first->GetUniquePrimitives(primsOut);
}

std::shared_ptr<CAnimTreeNode> CMetaAnimRandom::VGetAnimationTree(const CAnimSysContext& animSys,
                                                                  const CMetaAnimTreeBuildOrders& orders) const {
  const u32 r = animSys.x8_random->Range(1, 100);
  const std::pair<std::shared_ptr<IMetaAnim>, u32>* useRd = nullptr;
  for (const auto& rd : x4_randomData) {
    useRd = &rd;
    if (r <= rd.second) {
      break;
    }
  }

  return useRd->first->GetAnimationTree(animSys, orders);
}

} // namespace metaforce
