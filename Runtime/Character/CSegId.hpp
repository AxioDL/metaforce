#pragma once

#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/RetroTypes.hpp"

namespace metaforce {

class CSegId {
  u8 x0_segId = 0xFF;

public:
  constexpr CSegId() noexcept = default;
  constexpr CSegId(u8 id) noexcept : x0_segId(id) {}
  explicit CSegId(CInputStream& in) : x0_segId(in.ReadLong()) {}
  constexpr CSegId& operator++() noexcept {
    ++x0_segId;
    return *this;
  }
  constexpr CSegId& operator--() noexcept {
    --x0_segId;
    return *this;
  }
  constexpr operator u8() const noexcept { return x0_segId; }

  constexpr bool IsValid() const noexcept { return !IsInvalid(); }
  constexpr bool IsInvalid() const noexcept { return x0_segId == 0xFF; }
};

} // namespace metaforce
