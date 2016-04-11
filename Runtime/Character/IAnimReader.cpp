#include "IAnimReader.hpp"
#include "CCharAnimTime.hpp"

namespace urde
{

SAdvancementResults IAnimReader::VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const
{
    SAdvancementResults ret;
    ret.x0_remTime = a;
    return ret;
}

void IAnimReader::GetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32, u32, u32) const
{
}

void IAnimReader::GetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32, u32, u32) const
{
}

void IAnimReader::GetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32, u32, u32) const
{
}

void IAnimReader::GetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32, u32, u32) const
{
}

}
