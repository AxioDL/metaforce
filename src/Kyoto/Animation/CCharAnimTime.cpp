#include "Kyoto/Animation/CCharAnimTime.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

#include <rstl/math.hpp>

CCharAnimTime::CCharAnimTime(CInputStream& in)
: mTime(in.Get< float >()), mType(EType(in.Get< int >())) {
}

CCharAnimTime::CCharAnimTime(float time) : mTime(time) {
  if (time == 0.f) {
    mType = kT_ZeroSteady;
  } else {
    mType = kT_NonZero;
  }
}

bool CCharAnimTime::operator<(const CCharAnimTime& other) const {
  if (mType == kT_NonZero) {
    if (other.mType == kT_NonZero) {
      return mTime < other.mTime;
    }

    return other.EqualsZero() ? mTime < 0.f : other.mTime > 0.f;
  }

  if (EqualsZero()) {
    if (other.EqualsZero()) {

      return ZeroOrdering() < other.ZeroOrdering();
    }
    if (other.mType == kT_NonZero) {
      return 0.f < other.mTime;
    }
    return other.mTime > 0.f;
  }

  if (other.mType == kT_Infinity) {
    if (mTime < 0.f && other.mTime > 0.f) {
      return true;
    }
    return false;
  }

  return mTime < 0.f;
}

bool CCharAnimTime::operator==(const CCharAnimTime& other) const {
  int iVar1;
  int iVar3;
  if (mType == kT_NonZero) {
    if (other.mType == kT_NonZero) {
      return mTime == other.mTime;
    }
    return other.EqualsZero() ? false : false;
  }

  if (EqualsZero()) {
    if (other.EqualsZero()) {
      return ZeroOrdering() == other.ZeroOrdering();
    }
    return false;
  }

  if (other.mType == kT_Infinity) {
    return mTime * other.mTime > 0.f;
  }
  return false;
}

bool CCharAnimTime::operator!=(const CCharAnimTime& other) const { return !(*this == other); }

bool CCharAnimTime::operator>(const CCharAnimTime& other) const {
  return !(*this == other) && !(*this < other);
}

float CCharAnimTime::operator/(const CCharAnimTime& other) const {
  if (EqualsZero())
    return 0.f;

  return mTime / other.mTime;
}

CCharAnimTime CCharAnimTime::operator*(const float& other) const {
  if (other == 0.f) {
    return ZeroFlat();
  }

  if (EqualsZero()) {
    return ZeroSignScale(other);
  }

  return CCharAnimTime(mTime * other);
}

CCharAnimTime CCharAnimTime::operator-(const CCharAnimTime& other) const {
  if (mType == kT_Infinity || other.mType == kT_Infinity) {
    if (mType == kT_Infinity && other.mType == kT_Infinity) {
      if (other.mTime == mTime) {
        return ZeroFlat();
      }
      return *this;
    }

    if (mType == kT_Infinity) {
      return *this;
    }

    return CCharAnimTime(kT_Infinity, -other.mTime);
  }

  if (EqualsZero() && other.EqualsZero()) {
    int ordering = ZeroOrdering() - other.ZeroOrdering();

    return CCharAnimTime(ZeroTypeFromOrdering(ordering), 0.f);
  }
  return CCharAnimTime(mTime - other.mTime);
}

CCharAnimTime CCharAnimTime::operator+(const CCharAnimTime& other) const {
  if (mType == kT_Infinity || other.mType == kT_Infinity) {
    if (mType == kT_Infinity && other.mType == kT_Infinity) {
      if (other.mTime == mTime) {
        return *this;
      }
      return ZeroFlat();
    }

    if (mType == kT_Infinity) {
      return *this;
    }

    return other;
  }

  if (EqualsZero() && other.EqualsZero()) {
    return CCharAnimTime(ZeroTypeFromOrdering(rstl::max_val(-1, rstl::min_val(ZeroOrdering() + other.ZeroOrdering(), 1))), 0.f);
  }
  return CCharAnimTime(mTime + other.mTime);
}

const CCharAnimTime& CCharAnimTime::operator+=(const CCharAnimTime& other) {
  return *this = *this + other;
}

const CCharAnimTime& CCharAnimTime::operator-=(const CCharAnimTime& other) {
  return *this = *this - other;
}

bool CCharAnimTime::operator<=(const CCharAnimTime& other) const {
  return *this == other || *this < other;
}

bool CCharAnimTime::operator>=(const CCharAnimTime& other) const {
  return *this == other || *this > other;
}

bool CCharAnimTime::GreaterThanZero() const {
  if (EqualsZero())
    return false;
  return mTime > 0.f;
}

bool CCharAnimTime::EqualsZero() const {
  return mType == kT_ZeroIncreasing || mType == kT_ZeroSteady || mType == kT_ZeroDecreasing ||
         mTime == 0.f;
}

void CCharAnimTime::PutTo(COutputStream& out) const {
  out.WriteReal32(mTime);
  out.WriteUint32(mType);
}
