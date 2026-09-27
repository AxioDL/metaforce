#ifndef MP1_CSTEADYSTATEANIMINFO_HPP
#define MP1_CSTEADYSTATEANIMINFO_HPP

#include <Kyoto/Animation/CCharAnimTime.hpp>
#include <Kyoto/Math/CVector3f.hpp>

class CSteadyStateAnimInfo {
  CCharAnimTime mDuration;
  CVector3f mOffset;
  bool mLooping;

public:
  CSteadyStateAnimInfo(bool looping, const CCharAnimTime& duration, const CVector3f& offset)
  : mDuration(duration), mOffset(offset), mLooping(looping) {}

  CCharAnimTime GetDuration() const { return mDuration; }
  CVector3f GetOffset() const { return mOffset; }
  const bool IsLooping() const { return mLooping; }
};
CHECK_SIZEOF(CSteadyStateAnimInfo, 0x18)

#endif // MP1_CSTEADYSTATEANIMINFO_HPP
