#include "CBoolPOINode.hpp"
#include "CAnimSourceReader.hpp"

namespace urde
{

CBoolPOINode::CBoolPOINode()
: CPOINode("root", EPOIType::EmptyBool, CCharAnimTime(), -1, false, 1.f, -1, 0) {}

CBoolPOINode::CBoolPOINode(CInputStream& in)
: CPOINode(in), x38_val(in.readBool()) {}

CBoolPOINode CBoolPOINode::CopyNodeMinusStartTime(const CBoolPOINode& node,
                                                  const CCharAnimTime& startTime)
{
    CBoolPOINode ret = node;
    ret.x1c_time -= startTime;
    return ret;
}

u32 CBoolPOINode::_getPOIList(const CCharAnimTime& time,
                              CBoolPOINode* listOut,
                              u32 capacity, u32 iterator, u32 unk1,
                              const std::vector<CBoolPOINode>& stream,
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
