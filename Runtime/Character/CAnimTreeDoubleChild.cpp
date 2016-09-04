#include "CAnimTreeDoubleChild.hpp"

namespace urde
{

CAnimTreeDoubleChild::CAnimTreeDoubleChild(const std::shared_ptr<CAnimTreeNode>& a,
                                           const std::shared_ptr<CAnimTreeNode>& b,
                                           const std::string& name)
: CAnimTreeNode(name), x14_a(a), x18_b(b)
{
}

SAdvancementResults CAnimTreeDoubleChild::VAdvanceView(const CCharAnimTime& a)
{
}

u32 CAnimTreeDoubleChild::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                          u32 capacity, u32 iterator, u32) const
{
}

u32 CAnimTreeDoubleChild::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                           u32 capacity, u32 iterator, u32) const
{
}

u32 CAnimTreeDoubleChild::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                              u32 capacity, u32 iterator, u32) const
{
}

u32 CAnimTreeDoubleChild::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                           u32 capacity, u32 iterator, u32) const
{
}

bool CAnimTreeDoubleChild::VGetBoolPOIState(const char* name) const
{
}

s32 CAnimTreeDoubleChild::VGetInt32POIState(const char* name) const
{
}

CParticleData::EParentedMode CAnimTreeDoubleChild::VGetParticlePOIState(const char* name) const
{
}

void CAnimTreeDoubleChild::VSetPhase(float)
{
}

SAdvancementResults CAnimTreeDoubleChild::VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const
{
}

u32 CAnimTreeDoubleChild::Depth() const
{
}

CAnimTreeEffectiveContribution CAnimTreeDoubleChild::VGetContributionOfHighestInfluence() const
{
}

u32 CAnimTreeDoubleChild::VGetNumChildren() const
{
}

std::shared_ptr<IAnimReader> CAnimTreeDoubleChild::VGetBestUnblendedChild() const
{
}

void CAnimTreeDoubleChild::VGetWeightedReaders(std::vector<std::pair<float, std::weak_ptr<IAnimReader>>>& out, float w) const
{
}

}
