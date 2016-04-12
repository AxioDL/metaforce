#include "CParticlePOINode.hpp"
#include "CAnimSourceReader.hpp"

namespace urde
{

CParticlePOINode::CParticlePOINode(CInputStream& in)
: CPOINode(in), x38_data(in) {}

CParticlePOINode CParticlePOINode::CopyNodeMinusStartTime(const CParticlePOINode& node,
                                                          const CCharAnimTime& startTime)
{
    CParticlePOINode ret = node;
    ret.x1c_time -= startTime;
    return ret;
}

u32 CParticlePOINode::_getPOIList(const CCharAnimTime& time,
                                  CParticlePOINode* listOut,
                                  u32 capacity, u32 iterator, u32 unk1,
                                  const std::vector<CParticlePOINode>& stream,
                                  const CCharAnimTime& curTime,
                                  const IAnimSourceInfo& animInfo, u32 passedCount)
{
    u32 ret = 0;
    if (animInfo.HasPOIData() && stream.size())
    {
        CCharAnimTime dur = animInfo.GetAnimationDuration();
        CCharAnimTime targetTime = curTime + time;
        if (targetTime >= dur)
            targetTime = dur;

        if (passedCount >= stream.size())
            return ret;

        CCharAnimTime nodeTime = stream[passedCount].GetTime();
        while (nodeTime <= targetTime)
        {
            u32 idx = iterator + ret;
            if (idx < capacity)
                listOut[idx] = CopyNodeMinusStartTime(stream[passedCount], curTime);
            ++passedCount;
            ++ret;
            nodeTime = stream[passedCount].GetTime();
        }
    }
    return ret;
}

}
