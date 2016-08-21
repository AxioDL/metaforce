#include "CSoundPOINode.hpp"
#include "CAnimSourceReader.hpp"

namespace urde
{

CSoundPOINode::CSoundPOINode()
: CPOINode("root", EPOIType::Sound, CCharAnimTime(), -1, false, 1.f, -1, 0),
  x38_sfxId(0),
  x3c_falloff(0.f),
  x40_maxDist(0.f)
{}

CSoundPOINode::CSoundPOINode(CInputStream& in)
: CPOINode(in),
  x38_sfxId(in.readUint32Big()),
  x3c_falloff(in.readFloatBig()),
  x40_maxDist(in.readFloatBig())
{}

CSoundPOINode::CSoundPOINode(const std::string& name, EPOIType a,
                             const CCharAnimTime& time, u32 b, bool c,
                             float d, u32 e, u32 f, u32 sfxId, float falloff, float maxDist)
: CPOINode(name, a, time, b, c, d, e, f),
  x38_sfxId(sfxId), x3c_falloff(falloff), x40_maxDist(maxDist) {}

CSoundPOINode CSoundPOINode::CopyNodeMinusStartTime(const CSoundPOINode& node,
                                                    const CCharAnimTime& startTime)
{
    CSoundPOINode ret = node;
    ret.x1c_time -= startTime;
    return ret;
}

u32 CSoundPOINode::_getPOIList(const CCharAnimTime& time,
                               CSoundPOINode* listOut,
                               u32 capacity, u32 iterator, u32 unk1,
                               const std::vector<CSoundPOINode>& stream,
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
