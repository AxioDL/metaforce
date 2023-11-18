#pragma once

#include <dolphin/gx.h>

#include <bitset>

#include <zeus/CColor.hpp>
#include <zeus/CVector3f.hpp>

namespace GX {
constexpr u8 MaxLights = 8;
using LightMask = std::bitset<MaxLights>;
} // namespace GX

constexpr GXColor GX_BLACK{0, 0, 0, 255};
constexpr GXColor GX_WHITE{255, 255, 255, 255};
constexpr GXColor GX_CLEAR{0, 0, 0, 0};

inline bool operator==(const GXColor& lhs, const GXColor& rhs) noexcept {
  return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}
inline bool operator!=(const GXColor& lhs, const GXColor& rhs) noexcept {
  return !(lhs == rhs);
}

static inline void GXPosition3f32(const zeus::CVector3f& v) { GXPosition3f32(v.x(), v.y(), v.z()); }
static inline void GXNormal3f32(const zeus::CVector3f& v) { GXNormal3f32(v.x(), v.y(), v.z()); }
static inline void GXTexCoord2f32(const zeus::CVector2f& v) { GXTexCoord2f32(v.x(), v.y()); }
static inline void GXColor4f32(const zeus::CColor& v) { GXColor4f32(v.r(), v.g(), v.b(), v.a()); }

static inline GXColor to_gx_color(const zeus::CColor& color) {
  return {
      static_cast<u8>(color.r() * 255.f),
      static_cast<u8>(color.g() * 255.f),
      static_cast<u8>(color.b() * 255.f),
      static_cast<u8>(color.a() * 255.f),
  };
}
static inline zeus::CColor from_gx_color(GXColor color) {
  return {
      static_cast<float>(color.r) / 255.f,
      static_cast<float>(color.g) / 255.f,
      static_cast<float>(color.b) / 255.f,
      static_cast<float>(color.a) / 255.f,
  };
}
