#include "CInt32POINode.hpp"
#include "CAnimSourceReader.hpp"

namespace urde
{

CInt32POINode::CInt32POINode()
: CPOINode("root", 2, CCharAnimTime(), -1, false, 1.f, -1, 0), x38_val(0), x3c_boneName("root") {}

CInt32POINode::CInt32POINode(CInputStream& in)
: CPOINode(in), x38_val(in.readUint32Big()), x3c_boneName(in.readString()) {}

CInt32POINode CInt32POINode::CopyNodeMinusStartTime(const CInt32POINode& node,
                                                    const CCharAnimTime& startTime)
{
    CInt32POINode ret = node;
    ret.x1c_time -= startTime;
    return ret;
}

u32 CInt32POINode::_getPOIList(const CCharAnimTime& time,
                               CInt32POINode* listOut,
                               u32 capacity, u32 iterator, u32 unk1,
                               const std::vector<CInt32POINode>& stream,
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
