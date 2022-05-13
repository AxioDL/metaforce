#pragma once

#include "common.hpp"

// TODO make this shared?
#include "../../../Runtime/Graphics/GX.hpp"

#include <bit>
#include <cstdint>
#include <utility>

#include <zeus/CAABox.hpp>
#include <zeus/CColor.hpp>
#include <zeus/CMatrix4f.hpp>
#include <zeus/CRectangle.hpp>
#include <zeus/CVector2f.hpp>
#include <zeus/CVector3f.hpp>
#include <zeus/CVector4f.hpp>

namespace metaforce {
enum class ETexelFormat {
  Invalid = -1,
  I4 = 0,
  I8 = 1,
  IA4 = 2,
  IA8 = 3,
  C4 = 4,
  C8 = 5,
  C14X2 = 6,
  RGB565 = 7,
  RGB5A3 = 8,
  RGBA8 = 9,
  CMPR = 10,
  // Metaforce addition: non-converting formats
  RGBA8PC = 11,
  R8PC = 12,
};

enum class EStreamFlagBits : u8 {
  fHasNormal = 0x1,
  fHasColor = 0x2,
  fHasTexture = 0x4,
};
using EStreamFlags = Flags<EStreamFlagBits>;
} // namespace metaforce

namespace aurora::gfx {
struct TextureRef;
using TextureHandle = std::shared_ptr<TextureRef>;

struct ClipRect {
  int32_t x;
  int32_t y;
  int32_t width;
  int32_t height;
};

struct Light {
  zeus::CVector3f pos{0.f, 0.f, 0.f};
  zeus::CVector3f dir{0.f, 0.f, -1.f};
  zeus::CColor color{0.f, 1.f};
  zeus::CVector3f cosAtt{1.f, 0.f, 0.f};
  zeus::CVector3f distAtt{1.f, 0.f, 0.f};
};

#ifndef NDEBUG
#define AURORA_GFX_DEBUG_GROUPS
#endif
void push_debug_group(zstring_view label) noexcept;
void pop_debug_group() noexcept;
struct ScopedDebugGroup {
  inline ScopedDebugGroup(zstring_view label) noexcept { push_debug_group(label); }
  inline ~ScopedDebugGroup() noexcept { pop_debug_group(); }
};

void set_viewport(float left, float top, float width, float height, float znear, float zfar) noexcept;
void set_scissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h) noexcept;

void resolve_color(const ClipRect& rect, uint32_t bindIdx, GX::TextureFormat fmt, bool clear_depth) noexcept;
void resolve_depth(const ClipRect& rect, uint32_t bindIdx, GX::TextureFormat fmt) noexcept;

void queue_movie_player(const TextureHandle& tex_y, const TextureHandle& tex_u, const TextureHandle& tex_v, float h_pad,
                        float v_pad) noexcept;

TextureHandle new_static_texture_2d(uint32_t width, uint32_t height, uint32_t mips, GX::TextureFormat format,
                                    ArrayRef<uint8_t> data, zstring_view label) noexcept;
TextureHandle new_dynamic_texture_2d(uint32_t width, uint32_t height, uint32_t mips, GX::TextureFormat format,
                                     zstring_view label) noexcept;
TextureHandle new_render_texture(uint32_t width, uint32_t height, GX::TextureFormat fmt, zstring_view label) noexcept;
void write_texture(const TextureRef& handle, ArrayRef<uint8_t> data) noexcept;
} // namespace aurora::gfx
