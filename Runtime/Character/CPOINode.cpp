#include "CPOINode.hpp"
#include "CBoolPOINode.hpp"
#include "CInt32POINode.hpp"
#include "CParticlePOINode.hpp"
#include "CSoundPOINode.hpp"
#include "CAnimSourceReader.hpp"

namespace urde
{

CPOINode::CPOINode(std::string_view name, EPOIType type, const CCharAnimTime& time,
                   s32 index, bool unique, float weight, s32 e, s32 f)
: x4_(1),
  x8_name(name),
  x18_type(type),
  x1c_time(time),
  x24_index(index),
  x28_unique(unique),
  x2c_weight(weight),
  x30_charIdx(e),
  x34_flags(f)
{}

CPOINode::CPOINode(CInputStream& in)
: x4_(in.readUint16Big()),
  x8_name(in.readString()),
  x18_type(EPOIType(in.readUint16Big())),
  x1c_time(in),
  x24_index(in.readInt32Big()),
  x28_unique(in.readBool()),
  x2c_weight(in.readFloatBig()),
  x30_charIdx(in.readInt32Big()),
  x34_flags(in.readInt32Big())
{}

bool CPOINode::operator>(const CPOINode& other) const
{
    return x1c_time < other.x1c_time;
}

bool CPOINode::operator<(const CPOINode& other) const
{
    return x1c_time > other.x1c_time;
}

template <class T>
u32 _getPOIList(const CCharAnimTime& time,
                T* listOut,
                u32 capacity, u32 iterator, u32 unk1,
                const std::vector<T>& stream,
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
        while (passedCount < stream.size() && nodeTime <= targetTime)
        {
            u32 idx = iterator + ret;
            if (idx < capacity)
            {
                listOut[idx] = T::CopyNodeMinusStartTime(stream[passedCount], curTime);
                ++ret;
            }
            ++passedCount;
            if (passedCount < stream.size())
                nodeTime = stream[passedCount].GetTime();
        }
    }
    return ret;
}

template <class T>
u32 _getPOIList(const CCharAnimTime& time,
                T* listOut,
                u32 capacity, u32 iterator, u32 unk1,
                const std::vector<T>& stream,
                const CCharAnimTime& curTime)
{
    u32 ret = 0;

    CCharAnimTime targetTime = curTime + time;

    for (u32 it = iterator ; it < stream.size() ; ++it)
    {
        CCharAnimTime nodeTime = stream[it].GetTime();
        if (nodeTime > targetTime)
            return ret;
        u32 idx = iterator + ret;
        if (nodeTime >= curTime && idx < capacity)
        {
            listOut[idx] = T::CopyNodeMinusStartTime(stream[it], curTime);
            ++ret;
        }
    }

    return ret;
}

template u32 _getPOIList<CBoolPOINode>(const CCharAnimTime& time,
                                       CBoolPOINode* listOut,
                                       u32 capacity, u32 iterator, u32 unk1,
                                       const std::vector<CBoolPOINode>& stream,
                                       const CCharAnimTime& curTime,
                                       const IAnimSourceInfo& animInfo, u32 passedCount);
template u32 _getPOIList<CBoolPOINode>(const CCharAnimTime& time,
                                       CBoolPOINode* listOut,
                                       u32 capacity, u32 iterator, u32 unk1,
                                       const std::vector<CBoolPOINode>& stream,
                                       const CCharAnimTime& curTime);

template u32 _getPOIList<CInt32POINode>(const CCharAnimTime& time,
                                        CInt32POINode* listOut,
                                        u32 capacity, u32 iterator, u32 unk1,
                                        const std::vector<CInt32POINode>& stream,
                                        const CCharAnimTime& curTime,
                                        const IAnimSourceInfo& animInfo, u32 passedCount);
template u32 _getPOIList<CInt32POINode>(const CCharAnimTime& time,
                                        CInt32POINode* listOut,
                                        u32 capacity, u32 iterator, u32 unk1,
                                        const std::vector<CInt32POINode>& stream,
                                        const CCharAnimTime& curTime);

template u32 _getPOIList<CParticlePOINode>(const CCharAnimTime& time,
                                           CParticlePOINode* listOut,
                                           u32 capacity, u32 iterator, u32 unk1,
                                           const std::vector<CParticlePOINode>& stream,
                                           const CCharAnimTime& curTime,
                                           const IAnimSourceInfo& animInfo, u32 passedCount);
template u32 _getPOIList<CParticlePOINode>(const CCharAnimTime& time,
                                           CParticlePOINode* listOut,
                                           u32 capacity, u32 iterator, u32 unk1,
                                           const std::vector<CParticlePOINode>& stream,
                                           const CCharAnimTime& curTime);

template u32 _getPOIList<CSoundPOINode>(const CCharAnimTime& time,
                                        CSoundPOINode* listOut,
                                        u32 capacity, u32 iterator, u32 unk1,
                                        const std::vector<CSoundPOINode>& stream,
                                        const CCharAnimTime& curTime,
                                        const IAnimSourceInfo& animInfo, u32 passedCount);
template u32 _getPOIList<CSoundPOINode>(const CCharAnimTime& time,
                                        CSoundPOINode* listOut,
                                        u32 capacity, u32 iterator, u32 unk1,
                                        const std::vector<CSoundPOINode>& stream,
                                        const CCharAnimTime& curTime);

}
