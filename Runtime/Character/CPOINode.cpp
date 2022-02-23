#include "Runtime/Character/CPOINode.hpp"

#include "Runtime/Character/CAnimSourceReader.hpp"
#include "Runtime/Character/CBoolPOINode.hpp"
#include "Runtime/Character/CInt32POINode.hpp"
#include "Runtime/Character/CParticlePOINode.hpp"
#include "Runtime/Character/CSoundPOINode.hpp"

namespace metaforce {

CPOINode::CPOINode(std::string_view name, EPOIType type, const CCharAnimTime& time, s32 index, bool unique,
                   float weight, s32 e, s32 f)
: x4_(1)
, x8_name(name)
, x18_type(type)
, x1c_time(time)
, x24_index(index)
, x28_unique(unique)
, x2c_weight(weight)
, x30_charIdx(e)
, x34_flags(f) {}

CPOINode::CPOINode(CInputStream& in)
: x4_(in.ReadShort())
, x8_name(in.Get<std::string>())
, x18_type(EPOIType(in.ReadShort()))
, x1c_time(in)
, x24_index(in.ReadInt32())
, x28_unique(in.ReadBool())
, x2c_weight(in.ReadFloat())
, x30_charIdx(in.ReadInt32())
, x34_flags(in.ReadInt32()) {}

bool CPOINode::operator>(const CPOINode& other) const { return x1c_time > other.x1c_time; }

bool CPOINode::operator<(const CPOINode& other) const { return x1c_time < other.x1c_time; }

template <class T>
size_t _getPOIList(const CCharAnimTime& time, T* listOut, size_t capacity, size_t iterator, u32 unk1,
                   const std::vector<T>& stream, const CCharAnimTime& curTime, const IAnimSourceInfo& animInfo,
                   size_t passedCount) {
  size_t ret = 0;
  if (animInfo.HasPOIData() && stream.size()) {
    const CCharAnimTime dur = animInfo.GetAnimationDuration();
    CCharAnimTime targetTime = curTime + time;
    if (targetTime >= dur) {
      targetTime = dur;
    }

    if (passedCount >= stream.size()) {
      return ret;
    }

    CCharAnimTime nodeTime = stream[passedCount].GetTime();
    while (passedCount < stream.size() && nodeTime <= targetTime) {
      const size_t idx = iterator + ret;
      if (idx < capacity) {
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
size_t _getPOIList(const CCharAnimTime& time, T* listOut, size_t capacity, size_t iterator, u32 unk1,
                   const std::vector<T>& stream, const CCharAnimTime& curTime) {
  size_t ret = 0;

  const CCharAnimTime targetTime = curTime + time;

  for (size_t it = iterator; it < stream.size(); ++it) {
    const CCharAnimTime nodeTime = stream[it].GetTime();
    if (nodeTime > targetTime) {
      return ret;
    }
    const size_t idx = iterator + ret;
    if (nodeTime >= curTime && idx < capacity) {
      listOut[idx] = T::CopyNodeMinusStartTime(stream[it], curTime);
      ++ret;
    }
  }

  return ret;
}

template size_t _getPOIList<CBoolPOINode>(const CCharAnimTime& time, CBoolPOINode* listOut, size_t capacity,
                                          size_t iterator, u32 unk1, const std::vector<CBoolPOINode>& stream,
                                          const CCharAnimTime& curTime, const IAnimSourceInfo& animInfo,
                                          size_t passedCount);
template size_t _getPOIList<CBoolPOINode>(const CCharAnimTime& time, CBoolPOINode* listOut, size_t capacity,
                                          size_t iterator, u32 unk1, const std::vector<CBoolPOINode>& stream,
                                          const CCharAnimTime& curTime);

template size_t _getPOIList<CInt32POINode>(const CCharAnimTime& time, CInt32POINode* listOut, size_t capacity,
                                           size_t iterator, u32 unk1, const std::vector<CInt32POINode>& stream,
                                           const CCharAnimTime& curTime, const IAnimSourceInfo& animInfo,
                                           size_t passedCount);
template size_t _getPOIList<CInt32POINode>(const CCharAnimTime& time, CInt32POINode* listOut, size_t capacity,
                                           size_t iterator, u32 unk1, const std::vector<CInt32POINode>& stream,
                                           const CCharAnimTime& curTime);

template size_t _getPOIList<CParticlePOINode>(const CCharAnimTime& time, CParticlePOINode* listOut, size_t capacity,
                                              size_t iterator, u32 unk1, const std::vector<CParticlePOINode>& stream,
                                              const CCharAnimTime& curTime, const IAnimSourceInfo& animInfo,
                                              size_t passedCount);
template size_t _getPOIList<CParticlePOINode>(const CCharAnimTime& time, CParticlePOINode* listOut, size_t capacity,
                                              size_t iterator, u32 unk1, const std::vector<CParticlePOINode>& stream,
                                              const CCharAnimTime& curTime);

template size_t _getPOIList<CSoundPOINode>(const CCharAnimTime& time, CSoundPOINode* listOut, size_t capacity,
                                           size_t iterator, u32 unk1, const std::vector<CSoundPOINode>& stream,
                                           const CCharAnimTime& curTime, const IAnimSourceInfo& animInfo,
                                           size_t passedCount);
template size_t _getPOIList<CSoundPOINode>(const CCharAnimTime& time, CSoundPOINode* listOut, size_t capacity,
                                           size_t iterator, u32 unk1, const std::vector<CSoundPOINode>& stream,
                                           const CCharAnimTime& curTime);

} // namespace metaforce
