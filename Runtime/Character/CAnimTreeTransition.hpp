#pragma once

#include <memory>
#include <string_view>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CAnimTreeTweenBase.hpp"

namespace urde {

class CAnimTreeTransition : public CAnimTreeTweenBase {
protected:
  CCharAnimTime x24_transDur;
  CCharAnimTime x2c_timeInTrans;
  bool x34_runA;
  bool x35_loopA;
  bool x36_initialized = false;
  SAdvancementResults AdvanceViewForTransitionalPeriod(const CCharAnimTime& time);

public:
  static std::string CreatePrimitiveName(const std::weak_ptr<CAnimTreeNode>&, const std::weak_ptr<CAnimTreeNode>&,
                                         float);

  CAnimTreeTransition(bool b1, const std::weak_ptr<CAnimTreeNode>& a, const std::weak_ptr<CAnimTreeNode>& b,
                      const CCharAnimTime& transDur, const CCharAnimTime& timeInTrans, bool runA, bool loopA, int flags,
                      std::string_view name, bool initialized);
  CAnimTreeTransition(bool b1, const std::weak_ptr<CAnimTreeNode>& a, const std::weak_ptr<CAnimTreeNode>& b,
                      const CCharAnimTime& transDur, bool runA, int flags, std::string_view name);
  std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  std::unique_ptr<IAnimReader> VClone() const override;
  std::optional<std::unique_ptr<IAnimReader>> VSimplified() override;
  std::optional<std::unique_ptr<IAnimReader>> VReverseSimplified() override;
  SAdvancementResults VAdvanceView(const CCharAnimTime& a) override;
  void SetBlendingWeight(float w) override;
  float VGetBlendingWeight() const override;
};
} // namespace urde
