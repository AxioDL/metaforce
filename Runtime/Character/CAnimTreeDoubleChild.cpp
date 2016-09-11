#include "CAnimTreeDoubleChild.hpp"

namespace urde
{

CAnimTreeDoubleChild::CAnimTreeDoubleChild(const std::weak_ptr<CAnimTreeNode>& a,
                                           const std::weak_ptr<CAnimTreeNode>& b,
                                           const std::string& name)
: CAnimTreeNode(name), x14_a(a.lock()), x18_b(b.lock())
{
}

SAdvancementResults CAnimTreeDoubleChild::VAdvanceView(const CCharAnimTime& a)
{
    return {};
}

u32 CAnimTreeDoubleChild::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                          u32 capacity, u32 iterator, u32) const
{
    return 0;
}

u32 CAnimTreeDoubleChild::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                           u32 capacity, u32 iterator, u32) const
{
    return 0;
}

u32 CAnimTreeDoubleChild::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                              u32 capacity, u32 iterator, u32) const
{
    return 0;
}

u32 CAnimTreeDoubleChild::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                           u32 capacity, u32 iterator, u32) const
{
    return 0;
}

bool CAnimTreeDoubleChild::VGetBoolPOIState(const char* name) const
{
    return false;
}

s32 CAnimTreeDoubleChild::VGetInt32POIState(const char* name) const
{
    return 0;
}

CParticleData::EParentedMode CAnimTreeDoubleChild::VGetParticlePOIState(const char* name) const
{
    return CParticleData::EParentedMode::Initial;
}

void CAnimTreeDoubleChild::VSetPhase(float)
{
}

SAdvancementResults CAnimTreeDoubleChild::VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const
{
    return {};
}

u32 CAnimTreeDoubleChild::Depth() const
{
    return 0;
}

CAnimTreeEffectiveContribution CAnimTreeDoubleChild::VGetContributionOfHighestInfluence() const
{
    return {0.f, "", CSteadyStateAnimInfo(), CCharAnimTime(), 0};
}

u32 CAnimTreeDoubleChild::VGetNumChildren() const
{
    return 0;
}

std::shared_ptr<IAnimReader> CAnimTreeDoubleChild::VGetBestUnblendedChild() const
{
    return {};
}

void CAnimTreeDoubleChild::VGetWeightedReaders(std::vector<std::pair<float, std::weak_ptr<IAnimReader>>>& out, float w) const
{
}

}
