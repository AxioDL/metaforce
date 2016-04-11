#ifndef __PSHAG_CANIMTREEDOUBLECHILD_HPP__
#define __PSHAG_CANIMTREEDOUBLECHILD_HPP__

#include "CAnimTreeNode.hpp"

namespace urde
{

class CAnimTreeDoubleChild : public CAnimTreeNode
{
public:
    SAdvancementResults VAdvanceView(const CCharAnimTime& a);
    void VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32, u32, u32) const;
    void VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32, u32, u32) const;
    void VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32, u32, u32) const;
    void VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32, u32, u32) const;
    void VGetBoolPOIState(const char*) const;
    void VGetInt32POIState(const char*) const;
    void VGetParticlePOIState(const char*) const;
    void VSetPhase(float);
    SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const;
    void Depth() const;
    void VGetContributionOfHighestInfluence() const;
    void VGetNumChildren() const;
    void VGetBestUnblendedChild() const;
};

}

#endif // __PSHAG_CANIMTREEDOUBLECHILD_HPP__
