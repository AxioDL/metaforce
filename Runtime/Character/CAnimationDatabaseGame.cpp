#include "CAnimationDatabaseGame.hpp"
#include "IMetaAnim.hpp"
#include "CPrimitive.hpp"
#include "CAnimation.hpp"

namespace urde {

CAnimationDatabaseGame::CAnimationDatabaseGame(const std::vector<CAnimation>& anims) {
  x10_anims.reserve(anims.size());
  for (const CAnimation& anim : anims)
    x10_anims.emplace_back(anim.GetMetaAnim());
}

const std::shared_ptr<IMetaAnim>& CAnimationDatabaseGame::GetMetaAnim(u32 idx) const { return x10_anims[idx]; }

u32 CAnimationDatabaseGame::GetNumMetaAnims() const { return x10_anims.size(); }

const char* CAnimationDatabaseGame::GetMetaAnimName(u32 idx) const {
  return "Meta-animation name unavailable in Release mode.";
}

void CAnimationDatabaseGame::GetAllUniquePrimitives(std::vector<CPrimitive>& primsOut) const {
  std::set<CPrimitive> primitives;
  for (const std::shared_ptr<IMetaAnim>& anim : x10_anims)
    anim->GetUniquePrimitives(primitives);
  primsOut.reserve(primitives.size());
  for (const CPrimitive& prim : primitives)
    primsOut.push_back(prim);
}

void CAnimationDatabaseGame::GetUniquePrimitivesFromMetaAnim(std::set<CPrimitive>& primsOut,
                                                             std::string_view name) const {
  // Empty
}

} // namespace urde
