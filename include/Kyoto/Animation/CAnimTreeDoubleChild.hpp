#ifndef _CANIMTREEDOUBLECHILD
#define _CANIMTREEDOUBLECHILD

#include "Kyoto/Animation/CAnimTreeNode.hpp"
class CAnimTreeDoubleChild : public CAnimTreeNode {
public:
  class CDoubleChildAdvancementResult {
    CCharAnimTime mTrueAdvancement;
    CAdvancementDeltas mLeftDeltas;
    CAdvancementDeltas mRightDeltas;

  public:
    CDoubleChildAdvancementResult(const CCharAnimTime& trueAdvancement,
                                  const CAdvancementDeltas& leftDeltas,
                                  const CAdvancementDeltas& rightDeltas);
    const CAdvancementDeltas& GetLeftAdvancementDeltas() const { return mLeftDeltas; }
    const CAdvancementDeltas& GetRightAdvancementDeltas() const { return mRightDeltas; }
    const CCharAnimTime& GetTrueAdvancement() const { return mTrueAdvancement; }
  };

protected:
  CDoubleChildAdvancementResult AdvanceViewBothChildren(const CCharAnimTime& time, bool runLeft,
                                                        bool loopLeft);

public:
  CAnimTreeDoubleChild(const rstl::ncrc_ptr< CAnimTreeNode >& a,
                       const rstl::ncrc_ptr< CAnimTreeNode >& b, const rstl::string& name);
  ~CAnimTreeDoubleChild() override;
  CAdvancementResults VAdvanceView(const CCharAnimTime& a) override;
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                       uint iterator, int unk) const override;
  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                        uint iterator, int unk) const override;
  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                           uint iterator, int unk) const override;
  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                        uint iterator, int unk) const override;
  bool VGetBoolPOIState(const char* name) const override;
  s32 VGetInt32POIState(const char* name) const override;
  CParticleData::EParentedMode VGetParticlePOIState(const char* name) const override;
  void VSetPhase(float) override;
  CAdvancementResults VGetAdvancementResults(const CCharAnimTime& a,
                                             const CCharAnimTime& b) const override;
  uint Depth() const override { return rstl::max_val(mA->Depth(), mB->Depth()) + 1; }
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  uint VGetNumChildren() const override;
  rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const override;
  void VGetWeightedReaders(
      float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;

  virtual float VGetRightChildWeight() const = 0;
  void ReplaceLeftChild(const rstl::ncrc_ptr< CAnimTreeNode >& child) { mA = child; }
  void ReplaceRightChild(const rstl::ncrc_ptr< CAnimTreeNode >& child) { mB = child; }

  float GetLeftChildWeight() const { return 1.f - VGetRightChildWeight(); }
  float GetRightChildWeight() const { return VGetRightChildWeight(); }

  const rstl::rc_ptr< CAnimTreeNode >& GetLeftChild() const { return mA; }
  const rstl::rc_ptr< CAnimTreeNode >& GetRightChild() const { return mB; }

protected:
  rstl::ncrc_ptr< CAnimTreeNode > mA;
  rstl::ncrc_ptr< CAnimTreeNode > mB;
};

CHECK_SIZEOF(CAnimTreeDoubleChild, 0x1c)

#endif // _CANIMTREEDOUBLECHILD
