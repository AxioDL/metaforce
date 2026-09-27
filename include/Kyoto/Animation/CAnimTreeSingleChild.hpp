#ifndef _CANIMTREESINGLECHILD
#define _CANIMTREESINGLECHILD

#include "Kyoto/Animation/CAnimTreeNode.hpp"

class CAnimTreeSingleChild : public CAnimTreeNode {
public:
  CAnimTreeSingleChild(const rstl::ncrc_ptr< CAnimTreeNode >& node, const rstl::string& name)
  : CAnimTreeNode(name), mChild(node) {}

  CAdvancementResults VAdvanceView(const CCharAnimTime& dt) override;
  CCharAnimTime VGetTimeRemaining() const override;
  bool VHasOffset(const CSegId& seg) const override;
  CVector3f VGetOffset(const CSegId& seg) const override;
  CQuaternion VGetRotation(const CSegId& seg) const override;
  uint VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, uint capacity,
                       uint iterator, int) const override;
  uint VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, uint capacity,
                        uint iterator, int) const override;
  uint VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, uint capacity,
                           uint iterator, int) const override;
  uint VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, uint capacity,
                        uint iterator, int) const override;
  bool VGetBoolPOIState(const char* name) const override;
  s32 VGetInt32POIState(const char* name) const override;
  CParticleData::EParentedMode VGetParticlePOIState(const char* name) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const override;
  void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                           const CCharAnimTime& time) const override;
  void VSetPhase(float) override;
  CAdvancementResults VGetAdvancementResults(const CCharAnimTime& a,
                                             const CCharAnimTime& b) const override;
  uint Depth() const override { return mChild->Depth() + 1; }
  uint VGetNumChildren() const override;
  void VGetWeightedReaders(
      float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const override;

  void ReplaceChild(const rstl::ncrc_ptr< CAnimTreeNode >& node) { mChild = node; }

protected:
  rstl::ncrc_ptr< CAnimTreeNode > mChild;
};
CHECK_SIZEOF(CAnimTreeSingleChild, 0x18)

#endif // _CANIMTREESINGLECHILD
