#ifndef __URDE_CSOUNDPOINODE_HPP__
#define __URDE_CSOUNDPOINODE_HPP__

#include "CPOINode.hpp"
#include "CCharAnimTime.hpp"

namespace urde
{
class IAnimSourceInfo;

class CSoundPOINode : public CPOINode
{
    u32 x38_sfxId;
    float x3c_falloff;
    float x40_maxDist;
public:
    CSoundPOINode(CInputStream& in);
    CSoundPOINode(const std::string& name, u16 a,
                  const CCharAnimTime& time, u32 b, bool c,
                  float d, u32 e, u32 f, u32 sfxId, float falloff, float maxDist);

    static u32 _getPOIList(const CCharAnimTime& time,
                           CSoundPOINode* listOut,
                           u32 capacity, u32 iterator, u32 unk1,
                           const std::vector<CSoundPOINode>& stream,
                           const CCharAnimTime& curTime,
                           const IAnimSourceInfo& animInfo, u32 passedCount);
    static CSoundPOINode CopyNodeMinusStartTime(const CSoundPOINode& node,
                                                const CCharAnimTime& startTime);
};

}

#endif // __URDE_CSOUNDPOINODE_HPP__
