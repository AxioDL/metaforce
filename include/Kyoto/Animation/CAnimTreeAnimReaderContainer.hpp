#ifndef _CANIMTREEANIMREADERCONTAINER
#define _CANIMTREEANIMREADERCONTAINER

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "rstl/object_owner.hpp"

class CAnimTreeAnimReaderContainer : public CAnimTreeNode {
public:
  ~CAnimTreeAnimReaderContainer() override {
    CCharAnimMemoryMetrics::SubtractFromTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
  }
  uint Depth() const override { return 1; }
  uint VGetNumChildren() const override { return 0; }

  CAnimTreeAnimReaderContainer(const rstl::ownership_transfer< IAnimReader >& reader,
                               const rstl::string& name, uint animDbIdx)
  : CAnimTreeNode(name)
  , mReader(reader)
  , mAnimDbIdx(animDbIdx) {
    CCharAnimMemoryMetrics::AddToTotalSize(8, CCharAnimMemoryMetrics::kASS_Two);
  }

  CAdvancementResults VAdvanceView(const CCharAnimTime& time) override;
  CCharAnimTime VGetTimeRemaining() const override;
  CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const override;
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;
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
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                           const CCharAnimTime& time) const override;
  rstl::ownership_transfer< IAnimReader > VClone() const override;
  CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const override;
  rstl::optional_object< rstl::ownership_transfer< IAnimReader > > VSimplified() override;
  void VSetPhase(float phase) override;
  CAdvancementResults VGetAdvancementResults(const CCharAnimTime& a,
                                             const CCharAnimTime& b) const override;
  rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const override;
  void VGetWeightedReaders(
      float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;

private:
  rstl::object_owner< IAnimReader, rstl::call_deep_clone< IAnimReader > > mReader;
  uint mAnimDbIdx;
};
CHECK_SIZEOF(CAnimTreeAnimReaderContainer, 0x20)

#endif // _CANIMTREEANIMREADERCONTAINER
