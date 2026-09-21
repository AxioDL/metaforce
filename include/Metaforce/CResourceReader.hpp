#pragma once

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Metaforce/Endian.hpp"

#include <array>
#include <borealis/log.hpp>
#include <limits>
#include <span>

class CResourceReader {
  std::span< const uchar > mRemaining;
  borealis::Log mLog;

public:
  CResourceReader(std::span< const uchar > data, const char* module)
  : mRemaining{data}, mLog{module} {}

  size_t Remaining() const { return mRemaining.size(); }

  std::span< const uchar > Take(size_t size) {
    if (size > Remaining()) {
      mLog.fatal("Truncated resource: need {} bytes, have {}", size, Remaining());
    }
    const auto result = mRemaining.first(size);
    mRemaining = mRemaining.subspan(size);
    return result;
  }

  template < typename T >
  T Read() {
    return read_bits< T >(Take(sizeof(T)).data());
  }

  uint ReadCount(size_t stride) {
    const uint count = Read< uint >();
    if (count > std::numeric_limits< int >::max() || count > Remaining() / stride) {
      mLog.fatal("Invalid resource count {} for {}-byte records ({} bytes remain)", count, stride,
                 Remaining());
    }
    return count;
  }

  CVector3f ReadVector3f() {
    const float x = Read< float >();
    const float y = Read< float >();
    const float z = Read< float >();
    return CVector3f(x, y, z);
  }

  CAABox ReadAABox() {
    const CVector3f min = ReadVector3f();
    const CVector3f max = ReadVector3f();
    return CAABox(min.GetX(), min.GetY(), min.GetZ(), max.GetX(), max.GetY(), max.GetZ());
  }

  CTransform4f ReadTransform() {
    std::array< float, 12 > values;
    for (auto& value : values) {
      value = Read< float >();
    }
    return CTransform4f(values[0], values[1], values[2], values[3], values[4], values[5], values[6],
                        values[7], values[8], values[9], values[10], values[11]);
  }
};
