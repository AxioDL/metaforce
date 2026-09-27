#ifndef _CANIMTREEBLEND
#define _CANIMTREEBLEND

#include "Kyoto/Animation/CAnimTreeTweenBase.hpp"

class CAnimTreeBlend : public CAnimTreeTweenBase {
public:
  ~CAnimTreeBlend() override {}
  CAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  rstl::ownership_transfer< IAnimReader > VClone() const override;
  void SetBlendingWeight(float weight) override;
  float VGetBlendingWeight() const override;

  CAnimTreeBlend(const bool characterSpaceBlend, const rstl::ncrc_ptr< CAnimTreeNode >& a,
                 const rstl::ncrc_ptr< CAnimTreeNode >& b, float blendWeight,
                 const rstl::string& name)
  : CAnimTreeTweenBase(characterSpaceBlend, a, b, kBlendRoot_Offset | kBlendRoot_Rotation, name)
  , mBlendWeight(blendWeight) {}

  static rstl::string CreatePrimitiveName(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                                          const rstl::ncrc_ptr< CAnimTreeNode >& b, float weight);

private:
  float mBlendWeight;
};
CHECK_SIZEOF(CAnimTreeBlend, 0x28)

#endif // _CANIMTREEBLEND
