#include "Kyoto/Animation/CTimeRemainderAndFraction.hpp"

#if VERSION >= VERSION_GM8P_00
CIntegerTimeAndRemainder::CIntegerTimeAndRemainder(const CCharAnimTime& time,
                                                   const CCharAnimTime& interval)
: x0_realTime(time.GetSeconds())
, x4_integerTime(CCast::ToUint32(time / interval))
, x8_remainder(rstl::max_val(x0_realTime - x4_integerTime * interval.GetSeconds(), 0.f)) {}

CTimeRemainderAndFraction::CTimeRemainderAndFraction(const CCharAnimTime& time,
                                                     const CCharAnimTime& interval)
: CIntegerTimeAndRemainder(time, interval)
, xc_fraction(Remainder() / interval.GetSeconds())
, x10_finestSample(interval.GetSeconds()) {}
#endif
