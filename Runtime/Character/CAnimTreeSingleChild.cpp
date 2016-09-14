#include "CAnimTreeSingleChild.hpp"

namespace urde
{

CAnimTreeSingleChild::CAnimTreeSingleChild(const std::weak_ptr<CAnimTreeNode>& node, const std::string& name)
: CAnimTreeNode(name)
, x14_child(node.lock())
{

}

SAdvancementResults CAnimTreeSingleChild::VAdvanceView(const CCharAnimTime& a)
{
    return {};
}

CCharAnimTime CAnimTreeSingleChild::VGetTimeRemaining() const
{
    return {};
}

bool CAnimTreeSingleChild::VHasOffset(const CSegId& seg) const
{
    return false;
}

zeus::CVector3f CAnimTreeSingleChild::VGetOffset(const CSegId& seg) const
{
    return {};
}

zeus::CQuaternion CAnimTreeSingleChild::VGetRotation(const CSegId& seg) const
{
    return {};
}

u32 CAnimTreeSingleChild::VGetBoolPOIList(const CCharAnimTime& time,
                                          CBoolPOINode* listOut,
                                          u32 capacity, u32 iterator, u32) const
{
    return 0;
}

u32 CAnimTreeSingleChild::VGetInt32POIList(const CCharAnimTime& time,
                                           CInt32POINode* listOut,
                                           u32 capacity, u32 iterator, u32) const
{
    return 0;
}

u32 CAnimTreeSingleChild::VGetParticlePOIList(const CCharAnimTime& time,
                                              CParticlePOINode* listOut,
                                              u32 capacity, u32 iterator, u32) const
{
    return 0;
}

u32 CAnimTreeSingleChild::VGetSoundPOIList(const CCharAnimTime& time,
                                           CSoundPOINode* listOut,
                                           u32 capacity, u32 iterator, u32) const
{
    return 0;
}

bool CAnimTreeSingleChild::VGetBoolPOIState(const char* name) const
{
    return false;
}

s32 CAnimTreeSingleChild::VGetInt32POIState(const char* name) const
{
    return -1;
}

CParticleData::EParentedMode CAnimTreeSingleChild::VGetParticlePOIState(const char* name) const
{
    return CParticleData::EParentedMode::Initial;
}

void CAnimTreeSingleChild::VGetSegStatementSet(const CSegIdList& list,
                                               CSegStatementSet& setOut) const
{
}

void CAnimTreeSingleChild::VGetSegStatementSet(const CSegIdList& list,
                                               CSegStatementSet& setOut,
                                               const CCharAnimTime& time) const
{
}

void CAnimTreeSingleChild::VSetPhase(float)
{
}

SAdvancementResults
CAnimTreeSingleChild::VGetAdvancementResults(const CCharAnimTime& a,
                                             const CCharAnimTime& b) const
{
    return {};
}

u32 CAnimTreeSingleChild::Depth() const
{
    return 1;
}

u32 CAnimTreeSingleChild::VGetNumChildren() const
{
    return 0;
}

}
