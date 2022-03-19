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
enum class ERglFogMode : uint32_t {
  None = 0x00,

  PerspLin = 0x02,
  PerspExp = 0x04,
  PerspExp2 = 0x05,
  PerspRevExp = 0x06,
  PerspRevExp2 = 0x07,

  OrthoLin = 0x0A,
  OrthoExp = 0x0C,
  OrthoExp2 = 0x0D,
  OrthoRevExp = 0x0E,
  OrthoRevExp2 = 0x0F
};

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

enum class EClampMode {
  Clamp,
  Repeat,
  Mirror,
};

struct CFogState {
  zeus::CColor m_color;
  float m_A = 0.f;
  float m_B = 0.5f;
  float m_C = 0.f;
  ERglFogMode m_mode;
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
struct TextureHandle {
  std::shared_ptr<TextureRef> ref;
  TextureHandle() = default;
  TextureHandle(std::shared_ptr<TextureRef>&& ref) : ref(std::move(ref)) {}
  operator bool() const { return ref.operator bool(); }
  void reset() { ref.reset(); }
};
enum class TextureFormat : uint8_t {
  RGBA8,
  R8,
  R32Float,
  DXT1,
  DXT3,
  DXT5,
  BPTC,
};

struct ClipRect {
  int32_t x;
  int32_t y;
  int32_t width;
  int32_t height;
};
enum class CameraFilterType : uint8_t {
  Passthru,
  Multiply,
  Invert,
  Add,
  Subtract,
  Blend,
  Widescreen,
  SceneAdd,
  NoColor,
  InvDstMultiply,
};
enum class ZComp : uint8_t {
  Never,
  Less,
  Equal,
  LEqual,
  Greater,
  NEqual,
  GEqual,
  Always,
};

struct Light {
  zeus::CVector3f pos{0.f, 0.f, 0.f};
  zeus::CVector3f dir{0.f, 0.f, -1.f};
  zeus::CColor color{0.f, 0.f, 0.f, 0.f};
  zeus::CVector3f linAtt{1.f, 0.f, 0.f};
  zeus::CVector3f angAtt{1.f, 0.f, 0.f};
};

[[nodiscard]] bool get_dxt_compression_supported() noexcept;

// GX state
void bind_texture(GX::TexMapID id, metaforce::EClampMode clamp, const TextureHandle& tex, float lod) noexcept;
void unbind_texture(GX::TexMapID id) noexcept;

void update_fog_state(const metaforce::CFogState& state) noexcept;
void load_light(GX::LightID id, const Light& light) noexcept;
void load_light_ambient(GX::LightID id, const zeus::CColor& ambient) noexcept;
void set_viewport(float left, float top, float width, float height, float znear, float zfar) noexcept;
void set_scissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h) noexcept;

void resolve_color(const ClipRect& rect, uint32_t bind, bool clear_depth) noexcept;
void resolve_depth(const ClipRect& rect, uint32_t bind) noexcept;

void add_material_set(/* TODO */) noexcept;
void add_model(/* TODO */) noexcept;

void queue_aabb(const zeus::CAABox& aabb, const zeus::CColor& color, bool z_only) noexcept;
void queue_fog_volume_plane(const ArrayRef<zeus::CVector4f>& verts, uint8_t pass);
void queue_fog_volume_filter(const zeus::CColor& color, bool two_way) noexcept;
void queue_textured_quad_verts(CameraFilterType filter_type, const TextureHandle& texture, ZComp z_comparison,
                               bool z_test, const zeus::CColor& color, const ArrayRef<zeus::CVector3f>& pos,
                               const ArrayRef<zeus::CVector2f>& uvs, float lod) noexcept;
void queue_textured_quad(CameraFilterType filter_type, const TextureHandle& texture, ZComp z_comparison, bool z_test,
                         const zeus::CColor& color, float uv_scale, const zeus::CRectangle& rect, float z,
                         float lod = 0) noexcept;
void queue_colored_quad_verts(CameraFilterType filter_type, ZComp z_comparison, bool z_test, const zeus::CColor& color,
                              const ArrayRef<zeus::CVector3f>& pos) noexcept;
void queue_colored_quad(CameraFilterType filter_type, ZComp z_comparison, bool z_test, const zeus::CColor& color,
                        const zeus::CRectangle& rect, float z) noexcept;
void queue_movie_player(const TextureHandle& tex_y, const TextureHandle& tex_u, const TextureHandle& tex_v, float h_pad,
                        float v_pad) noexcept;

TextureHandle new_static_texture_2d(uint32_t width, uint32_t height, uint32_t mips, metaforce::ETexelFormat format,
                                    ArrayRef<uint8_t> data, zstring_view label) noexcept;
TextureHandle new_dynamic_texture_2d(uint32_t width, uint32_t height, uint32_t mips, metaforce::ETexelFormat format,
                                     zstring_view label) noexcept;
TextureHandle new_render_texture(uint32_t width, uint32_t height, uint32_t color_bind_count, uint32_t depth_bind_count,
                                 zstring_view label) noexcept;
void write_texture(const TextureHandle& handle, ArrayRef<uint8_t> data) noexcept;
} // namespace aurora::gfx
