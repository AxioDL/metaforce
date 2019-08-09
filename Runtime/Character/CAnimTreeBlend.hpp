#pragma once

#include "CAnimTreeTweenBase.hpp"

namespace urde {

class CAnimTreeBlend : public CAnimTreeTweenBase {
  float x24_blendWeight;

public:
  static std::string CreatePrimitiveName(const std::shared_ptr<CAnimTreeNode>& a,
                                         const std::shared_ptr<CAnimTreeNode>& b, float scale);

  CAnimTreeBlend(bool, const std::shared_ptr<CAnimTreeNode>& a, const std::shared_ptr<CAnimTreeNode>& b,
                 float blendWeight, std::string_view name);

  SAdvancementResults VAdvanceView(const CCharAnimTime& dt) override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  std::unique_ptr<IAnimReader> VClone() const override;
  void SetBlendingWeight(float w) override;
  float VGetBlendingWeight() const override;
};

} // namespace urde
