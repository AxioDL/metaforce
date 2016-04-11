#include "IAnimReader.hpp"
#include "CCharAnimTime.hpp"

namespace urde
{

SAdvancementResults
IAnimReader::VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const
{
    SAdvancementResults ret;
    ret.x0_remTime = a;
    return ret;
}

u32 IAnimReader::GetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut,
                                u32 capacity, u32 iterator, u32 unk) const
{
    if (time.GreaterThanZero())
        return VGetBoolPOIList(time, listOut, capacity, iterator, unk);
    return 0;
}

u32 IAnimReader::GetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut,
                                 u32 capacity, u32 iterator, u32 unk) const
{
    if (time.GreaterThanZero())
        return VGetInt32POIList(time, listOut, capacity, iterator, unk);
    return 0;
}

u32 IAnimReader::GetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut,
                                    u32 capacity, u32 iterator, u32 unk) const
{
    if (time.GreaterThanZero())
        return VGetParticlePOIList(time, listOut, capacity, iterator, unk);
    return 0;
}

u32 IAnimReader::GetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut,
                                 u32 capacity, u32 iterator, u32 unk) const
{
    if (time.GreaterThanZero())
        return VGetSoundPOIList(time, listOut, capacity, iterator, unk);
    return 0;
}

}
