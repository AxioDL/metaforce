#ifndef __URDE_CANIMTREEDOUBLECHILD_HPP__
#define __URDE_CANIMTREEDOUBLECHILD_HPP__

#include "CAnimTreeNode.hpp"

namespace urde
{

class CAnimTreeDoubleChild : public CAnimTreeNode
{
public:
    SAdvancementResults VAdvanceView(const CCharAnimTime& a);
    u32 VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator, u32) const;
    bool VGetBoolPOIState(const char* name) const;
    s32 VGetInt32POIState(const char* name) const;
    CParticleData::EParentedMode VGetParticlePOIState(const char* name) const;
    void VSetPhase(float);
    SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const;
    void Depth() const;
    void VGetContributionOfHighestInfluence() const;
    u32 VGetNumChildren() const;
    std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const;
};

}

#endif // __URDE_CANIMTREEDOUBLECHILD_HPP__
