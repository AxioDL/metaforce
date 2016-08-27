#ifndef __URDE_CANIMTREEANIMREADERCONTAINER_HPP__
#define __URDE_CANIMTREEANIMREADERCONTAINER_HPP__

#include "CAnimTreeNode.hpp"

namespace urde
{

class CAnimTreeAnimReaderContainer : public CAnimTreeNode
{
    std::shared_ptr<IAnimReader> x14_reader;
    u32 x1c_animDbIdx;
public:
    CAnimTreeAnimReaderContainer(const std::string& name,
                                 std::shared_ptr<IAnimReader> reader,
                                 u32 animDbIdx);

    u32 Depth() const;
    CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const;
    u32 VGetNumChildren() const;
    std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const;
    void VGetWeightedReaders(std::vector<std::pair<float, std::weak_ptr<IAnimReader>>>& out, float w) const;

    SAdvancementResults VAdvanceView(const CCharAnimTime& a);
    CCharAnimTime VGetTimeRemaining() const;
    CSteadyStateAnimInfo VGetSteadyStateAnimInfo() const;
    bool VHasOffset(const CSegId& seg) const;
    zeus::CVector3f VGetOffset(const CSegId& seg) const;
    zeus::CQuaternion VGetRotation(const CSegId& seg) const;
    u32 VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator, u32) const;
    bool VGetBoolPOIState(const char*) const;
    s32 VGetInt32POIState(const char*) const;
    CParticleData::EParentedMode VGetParticlePOIState(const char*) const;
    void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const;
    void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut, const CCharAnimTime& time) const;
    std::shared_ptr<IAnimReader> VClone() const;
    std::shared_ptr<IAnimReader> VSimplified();
    void VSetPhase(float);
    SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const;
};

}

#endif // __URDE_CANIMTREEANIMREADERCONTAINER_HPP__
