#ifndef _CANIMTREELOOPIN
#define _CANIMTREELOOPIN

#include "Kyoto/Animation/CAnimSysContext.hpp"
#include "Kyoto/Animation/CAnimTreeSingleChild.hpp"
#include "Kyoto/Animation/CSequenceFundamentals.hpp"

class CAnimTreeLoopIn : public CAnimTreeSingleChild {
public:
  ~CAnimTreeLoopIn() override {}
  CAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                       uint iterator, int unk) const override;
  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                        uint iterator, int unk) const override;
  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                           uint iterator, int unk) const override;
  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                        uint iterator, int unk) const override;
  rstl::ownership_transfer< IAnimReader > VClone() const override;
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified() override;
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const override;
  virtual bool VSupportsReverseView() const;

  CAnimTreeLoopIn(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                  const rstl::ncrc_ptr< CAnimTreeNode >& b,
                  const rstl::ncrc_ptr< CAnimTreeNode >& c, const CAnimSysContext& animCtx,
                  const rstl::string& name);
  CAnimTreeLoopIn(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                  const rstl::ncrc_ptr< CAnimTreeNode >& b, bool didLoopIn,
                  const CAnimSysContext& animCtx, const rstl::string& name,
                  const CSequenceFundamentals& fundamentals, const CCharAnimTime& time);

  static rstl::string CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                          const rstl::ncrc_ptr< CAnimTreeNode >& b,
                                          const rstl::ncrc_ptr< CAnimTreeNode >& c);

private:
  rstl::ncrc_ptr< CAnimTreeNode > mNextAnim;
  bool mDidLoopIn;
  CAnimSysContext mAnimCtx;
  CSequenceFundamentals mFundamentals;
  CCharAnimTime mCurTime;
};
CHECK_SIZEOF(CAnimTreeLoopIn, 0x90)

#endif // _CANIMTREELOOPIN
