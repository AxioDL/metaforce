#include "Kyoto/Animation/CTimeRemainderAndFraction.hpp"

#if VERSION >= VERSION_GM8P_00
CIntegerTimeAndRemainder::CIntegerTimeAndRemainder(const CCharAnimTime& time,
                                                   const CCharAnimTime& interval)
: mRealTime(time.GetSeconds())
, mIntegerTime(CCast::ToUint32(time / interval))
, mRemainder(rstl::max_val(mRealTime - mIntegerTime * interval.GetSeconds(), 0.f)) {}

CTimeRemainderAndFraction::CTimeRemainderAndFraction(const CCharAnimTime& time,
                                                     const CCharAnimTime& interval)
: CIntegerTimeAndRemainder(time, interval)
, mFraction(Remainder() / interval.GetSeconds())
, mFinestSample(interval.GetSeconds()) {}
#endif
