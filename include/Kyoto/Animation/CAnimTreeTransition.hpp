#ifndef _CANIMTREETRANSITION
#define _CANIMTREETRANSITION

#include "Kyoto/Animation/CAnimTreeTweenBase.hpp"

class CAnimTreeTransition : public CAnimTreeTweenBase {
public:
  ~CAnimTreeTransition() override;
  CAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  rstl::ownership_transfer< IAnimReader > VClone() const override;
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified() override;
  rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const override;
  void SetBlendingWeight(float weight) override;
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VReverseSimplified() override;
  float VGetBlendingWeight() const override;

  CAnimTreeTransition(bool b1, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                      const rstl::ncrc_ptr< CAnimTreeNode >& b, const CCharAnimTime& transDur,
                      bool runA, int flags, const rstl::string& name);
  CAnimTreeTransition(bool b1, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                      const rstl::ncrc_ptr< CAnimTreeNode >& b, const CCharAnimTime& transDur,
                      const CCharAnimTime& timeInTrans, bool runA, bool loopA, int flags,
                      const rstl::string& name, bool initialized);

  static rstl::string CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                          const rstl::ncrc_ptr< CAnimTreeNode >& b, float duration);

private:
  rstl::pair< CCharAnimTime, CAdvancementDeltas >
  AdvanceViewForTransitionalPeriod(const CCharAnimTime& time);

  CCharAnimTime mTransDur;
  CCharAnimTime mTimeInTrans;
  const bool mRunA;
  bool mLoopA;
  bool mInitialized;
};
CHECK_SIZEOF(CAnimTreeTransition, 0x38)

#endif // _CANIMTREETRANSITION
