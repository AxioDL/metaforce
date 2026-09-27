#ifndef _CCHARANIMTIME
#define _CCHARANIMTIME

#include "types.h"

class COutputStream;
class CInputStream;
class CCharAnimTime {
public:
  enum EType {
    kT_NonZero,
    kT_ZeroIncreasing,
    kT_ZeroSteady,
    kT_ZeroDecreasing,
    kT_Infinity,
  };
  const float GetSeconds() const { return mTime; }

  explicit CCharAnimTime(CInputStream& in);
  explicit CCharAnimTime(float time = 0.f);
  explicit CCharAnimTime(const EType& type, const float& time) : mTime(time), mType(type) {}

  bool operator>(const CCharAnimTime& other) const;
  bool operator==(const CCharAnimTime& other) const;
  bool operator!=(const CCharAnimTime& other) const;
  bool operator<(const CCharAnimTime& other) const;
  float operator/(const CCharAnimTime& other) const;
  CCharAnimTime operator*(const float& other) const;
  CCharAnimTime operator-(const CCharAnimTime& other) const;
  CCharAnimTime operator+(const CCharAnimTime& other) const;
  const CCharAnimTime& operator+=(const CCharAnimTime& other);
  const CCharAnimTime& operator-=(const CCharAnimTime& other);
  bool operator<=(const CCharAnimTime& other) const;
  bool operator>=(const CCharAnimTime& other) const;
  bool GreaterThanZero() const;
  bool EqualsZero() const;
  void PutTo(COutputStream& out) const;
  static CCharAnimTime Infinity() { return CCharAnimTime(kT_Infinity, 1.0f); }
  static CCharAnimTime ZeroPlus() { return CCharAnimTime(kT_ZeroIncreasing, 0.f); }
  static CCharAnimTime ZeroFlat() { return CCharAnimTime(kT_ZeroSteady, 0.f); }
  static CCharAnimTime ZeroMinus() { return CCharAnimTime(kT_ZeroDecreasing, 0.f); }

  int ZeroOrdering() const {
    if (mType == kT_ZeroDecreasing) {
      return -1;
    }
    if (mType == kT_ZeroSteady) {
      return 0;
    }
    return 1;
  }

  static EType ZeroTypeFromOrdering(int ordering) {
    if (ordering == -1) {
      return kT_ZeroDecreasing;
    }
    if (ordering == 0) {
      return kT_ZeroSteady;
    }

    return kT_ZeroIncreasing;
  }

  CCharAnimTime ZeroSignScale(float other) const {
    if (other > 0.f) {
      return *this;
    } else if (other < 0.f) {
      return CCharAnimTime(ZeroTypeFromOrdering(-ZeroOrdering()), 0.f);
    }
    return ZeroFlat();
  }

private:
  float mTime;
  EType mType;
};
CHECK_SIZEOF(CCharAnimTime, 0x8)

#endif // _CCHARANIMTIME
