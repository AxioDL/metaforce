#pragma once

#include "RetroTypes.hpp"
#include "CAnimTreeTweenBase.hpp"

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
  std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const;
  CCharAnimTime VGetTimeRemaining() const;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const;
  std::unique_ptr<IAnimReader> VClone() const;
  std::optional<std::unique_ptr<IAnimReader>> VSimplified();
  std::optional<std::unique_ptr<IAnimReader>> VReverseSimplified();
  SAdvancementResults VAdvanceView(const CCharAnimTime& a);
  void SetBlendingWeight(float w);
  float VGetBlendingWeight() const;
};
} // namespace urde
