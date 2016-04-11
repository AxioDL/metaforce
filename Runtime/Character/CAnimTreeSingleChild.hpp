#ifndef __PSHAG_CANIMTREESINGLECHILD_HPP__
#define __PSHAG_CANIMTREESINGLECHILD_HPP__

#include "CAnimTreeNode.hpp"

namespace urde
{

class CAnimTreeSingleChild : public CAnimTreeNode
{
    std::shared_ptr<CAnimTreeNode> x14_child;
public:
    CAnimTreeSingleChild(const std::weak_ptr<CAnimTreeNode>& node, const std::string& name);

    SAdvancementResults VAdvanceView(const CCharAnimTime& a);
    CCharAnimTime VGetTimeRemaining() const;
    bool VHasOffset(const CSegId& seg) const;
    void VGetOffset(const CSegId& seg) const;
    void VGetRotation(const CSegId& seg) const;
    u32 VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity, u32 iterator, u32) const;
    u32 VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator, u32) const;
    void VGetBoolPOIState(const char*) const;
    void VGetInt32POIState(const char*) const;
    void VGetParticlePOIState(const char*) const;
    void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const;
    void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut, const CCharAnimTime& time) const;
    void VSetPhase(float);
    SAdvancementResults VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const;
    void Depth() const;
    void VGetNumChildren() const;
};

}

#endif // __PSHAG_CANIMTREESINGLECHILD_HPP__
