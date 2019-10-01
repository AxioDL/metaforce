#pragma once

#include <memory>
#include <vector>

#include "Runtime/Character/CAnimationDatabase.hpp"

namespace urde {
class CAnimation;

class CAnimationDatabaseGame final : public CAnimationDatabase {
  std::vector<std::shared_ptr<IMetaAnim>> x10_anims;

public:
  CAnimationDatabaseGame(const std::vector<CAnimation>& anims);
  const std::shared_ptr<IMetaAnim>& GetMetaAnim(s32 idx) const override;
  u32 GetNumMetaAnims() const override;
  const char* GetMetaAnimName(s32 idx) const override;
  void GetAllUniquePrimitives(std::vector<CPrimitive>& primsOut) const override;
  void GetUniquePrimitivesFromMetaAnim(std::set<CPrimitive>& primsOut, std::string_view name) const override;
};

} // namespace urde
