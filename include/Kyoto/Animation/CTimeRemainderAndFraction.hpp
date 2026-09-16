#ifndef _CTIMEREMAINDERANDFRACTION
#define _CTIMEREMAINDERANDFRACTION

#include "Kyoto/Animation/CCharAnimTime.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "rstl/math.hpp"

class CIntegerTimeAndRemainder {
public:
#if VERSION >= VERSION_GM8P_00
  CIntegerTimeAndRemainder(const CCharAnimTime& time, const CCharAnimTime& interval);
#else
  CIntegerTimeAndRemainder(const CCharAnimTime& time, const CCharAnimTime& interval)
  : x0_realTime(time.GetSeconds())
  , x4_integerTime(CCast::ToUint32(time / interval))
  , x8_remainder(rstl::max_val(x0_realTime - x4_integerTime * interval.GetSeconds(), 0.f)) {}
#endif

  const float& RealTime() const { return x0_realTime; }

  const uint& IntegerTime() const { return x4_integerTime; }

  const float& Remainder() const { return x8_remainder; }

private:
  float x0_realTime;
  uint x4_integerTime;
  float x8_remainder;
};
CHECK_SIZEOF(CIntegerTimeAndRemainder, 0xc)

class CTimeRemainderAndFraction : public CIntegerTimeAndRemainder {
public:
#if VERSION >= VERSION_GM8P_00
  CTimeRemainderAndFraction(const CCharAnimTime& time, const CCharAnimTime& interval);
#else
  CTimeRemainderAndFraction(const CCharAnimTime& time, const CCharAnimTime& interval)
  : CIntegerTimeAndRemainder(time, interval)
  , xc_fraction(Remainder() / interval.GetSeconds())
  , x10_finestSample(interval.GetSeconds()) {}
#endif

  const float& FinestSample() const { return x10_finestSample; }

private:
  float xc_fraction;
  float x10_finestSample;
};
CHECK_SIZEOF(CTimeRemainderAndFraction, 0x14)

#endif
