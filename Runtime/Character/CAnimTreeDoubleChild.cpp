#include "CAnimTreeDoubleChild.hpp"

namespace urde
{

CAnimTreeDoubleChild::CAnimTreeDoubleChild(const std::weak_ptr<CAnimTreeNode>& a,
                                           const std::weak_ptr<CAnimTreeNode>& b,
                                           std::string_view name)
: CAnimTreeNode(name), x14_a(a.lock()), x18_b(b.lock())
{
}

SAdvancementResults CAnimTreeDoubleChild::VAdvanceView(const CCharAnimTime& a)
{
    return {};
}

u32 CAnimTreeDoubleChild::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                          u32 capacity, u32 iterator, u32 unk) const
{
    u32 newCapacity = x14_a->GetBoolPOIList(time, listOut, capacity, iterator, unk);
    newCapacity += x18_b->GetBoolPOIList(time, listOut, capacity, newCapacity + iterator, unk);
    if (newCapacity > capacity)
        newCapacity = capacity;

    std::sort(listOut, listOut + newCapacity, CPOINode::compare);

    return newCapacity;
}

u32 CAnimTreeDoubleChild::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                           u32 capacity, u32 iterator, u32 unk) const
{
    u32 newCapacity = x14_a->GetInt32POIList(time, listOut, capacity, iterator, unk);
    newCapacity += x18_b->GetInt32POIList(time, listOut, capacity, newCapacity + iterator, unk);
    if (newCapacity > capacity)
        newCapacity = capacity;

    std::sort(listOut, listOut + newCapacity, CPOINode::compare);

    return newCapacity;
}

u32 CAnimTreeDoubleChild::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                              u32 capacity, u32 iterator, u32 unk) const
{
    u32 newCapacity = x14_a->GetParticlePOIList(time, listOut, capacity, iterator, unk);
    newCapacity += x18_b->GetParticlePOIList(time, listOut, capacity, newCapacity + iterator, unk);
    if (newCapacity > capacity)
        newCapacity = capacity;

    std::sort(listOut, listOut + newCapacity, CPOINode::compare);

    return newCapacity;
}

u32 CAnimTreeDoubleChild::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                           u32 capacity, u32 iterator, u32 unk) const
{
    u32 newCapacity = x14_a->GetSoundPOIList(time, listOut, capacity, iterator, unk);
    newCapacity += x18_b->GetSoundPOIList(time, listOut, capacity, newCapacity + iterator, unk);
    if (newCapacity > capacity)
        newCapacity = capacity;

    std::sort(listOut, listOut + newCapacity, CPOINode::compare);
    return newCapacity;
}

bool CAnimTreeDoubleChild::VGetBoolPOIState(const char* name) const
{
    return x18_b->VGetBoolPOIState(name);
}

s32 CAnimTreeDoubleChild::VGetInt32POIState(const char* name) const
{
    return x18_b->VGetBoolPOIState(name);
}

CParticleData::EParentedMode CAnimTreeDoubleChild::VGetParticlePOIState(const char* name) const
{
    return x18_b->VGetParticlePOIState(name);
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
