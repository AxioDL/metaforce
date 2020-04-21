#pragma once

#include <memory>
#include <utility>

#include "Runtime/rstl.hpp"
#include "Runtime/Character/CAnimTreeDoubleChild.hpp"

namespace urde {

class CAnimTreeTweenBase : public CAnimTreeDoubleChild {
  static s32 sAdvancementDepth;

protected:
  int x1c_flags;
  bool x20_24_b1 : 1;
  u8 x20_25_cullSelector : 2 = 0;

public:
  CAnimTreeTweenBase(bool, const std::weak_ptr<CAnimTreeNode>& a, const std::weak_ptr<CAnimTreeNode>& b, int,
                     std::string_view name);

  virtual void SetBlendingWeight(float w) = 0;
  virtual float VGetBlendingWeight() const = 0;

  float GetBlendingWeight() const { return VGetBlendingWeight(); }

  void VGetWeightedReaders(rstl::reserved_vector<std::pair<float, std::weak_ptr<IAnimReader>>, 16>& out,
                           float w) const override;
  float VGetRightChildWeight() const override { return GetBlendingWeight(); }

  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut, const CCharAnimTime& time) const override;
  bool VHasOffset(const CSegId& seg) const override;
  zeus::CVector3f VGetOffset(const CSegId& seg) const override;
  zeus::CQuaternion VGetRotation(const CSegId& seg) const override;

  std::optional<std::unique_ptr<IAnimReader>> VSimplified() override;
  virtual std::optional<std::unique_ptr<IAnimReader>> VReverseSimplified() {
    return CAnimTreeTweenBase::VSimplified();
  }

  static bool ShouldCullTree() { return 3 <= sAdvancementDepth; }
  static void IncAdvancementDepth() { sAdvancementDepth++; }
  static void DecAdvancementDepth() { sAdvancementDepth--; }
};

} // namespace urde
