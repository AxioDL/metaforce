#pragma once

#include "common.hpp"

#include <cstdint>

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

struct CFogState {
  zeus::CColor m_color;
  float m_A = 0.f;
  float m_B = 0.5f;
  float m_C = 0.f;
  ERglFogMode m_mode;
};
} // namespace metaforce

namespace aurora::gfx {
struct TextureRef {
  uint32_t id;
  bool render;
};
struct TextureHandle {
  TextureRef ref;
  explicit TextureHandle(TextureRef ref) : ref(ref) {}
  ~TextureHandle() noexcept;
  TextureHandle(const TextureHandle&) = delete;
  TextureHandle& operator=(const TextureHandle&) = delete;
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
enum class ZTest : uint8_t {
  Never,
  Less,
  Equal,
  LEqual,
  Greater,
  NEqual,
  GEqual,
  Always,
};

[[nodiscard]] bool get_dxt_compression_supported() noexcept;

void update_model_view(const zeus::CMatrix4f& mv, const zeus::CMatrix4f& mv_inv) noexcept;
void update_projection(const zeus::CMatrix4f& proj) noexcept;
void update_fog_state(const metaforce::CFogState& state) noexcept;
void set_viewport(const zeus::CRectangle& rect, float znear, float zfar) noexcept;
void set_scissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h) noexcept;

void resolve_color(const ClipRect& rect, uint32_t bind, bool clear_depth) noexcept;
void resolve_depth(const ClipRect& rect, uint32_t bind) noexcept;

void add_material_set(/* TODO */) noexcept;
void add_model(/* TODO */) noexcept;

void queue_aabb(const zeus::CAABox& aabb, const zeus::CColor& color, bool z_only) noexcept;
void queue_fog_volume_plane(const ArrayRef<zeus::CVector4f>& verts, uint8_t pass);
void queue_fog_volume_filter(const zeus::CColor& color, bool two_way) noexcept;
void queue_textured_quad_verts(CameraFilterType filter_type, TextureRef texture, ZTest z_comparison, bool z_test,
                               const zeus::CColor& color, const ArrayRef<zeus::CVector3f>& pos,
                               const ArrayRef<zeus::CVector2f>& uvs, float lod) noexcept;
void queue_textured_quad(CameraFilterType filter_type, TextureRef texture, ZTest z_comparison, bool z_test,
                         const zeus::CColor& color, float uv_scale, const zeus::CRectangle& rect, float z) noexcept;
void queue_colored_quad_verts(CameraFilterType filter_type, ZTest z_comparison, bool z_test, const zeus::CColor& color,
                              const ArrayRef<zeus::CVector3f>& pos) noexcept;
void queue_colored_quad(CameraFilterType filter_type, ZTest z_comparison, bool z_test, const zeus::CColor& color,
                        const zeus::CRectangle& rect, float z) noexcept;
void queue_movie_player(TextureRef tex_y, TextureRef tex_u, TextureRef tex_v, const zeus::CColor& color, float h_pad,
                        float v_pad) noexcept;

std::shared_ptr<TextureHandle> new_static_texture_2d(uint32_t width, uint32_t height, uint32_t mips,
                                                     TextureFormat format, ArrayRef<uint8_t> data,
                                                     std::string_view label) noexcept;
std::shared_ptr<TextureHandle> new_dynamic_texture_2d(uint32_t width, uint32_t height, uint32_t mips,
                                                      TextureFormat format, std::string_view label) noexcept;
std::shared_ptr<TextureHandle> new_render_texture(uint32_t width, uint32_t height, uint32_t color_bind_count,
                                                  uint32_t depth_bind_count, std::string_view label) noexcept;
void write_texture(TextureRef ref, ArrayRef<uint8_t> data) noexcept;
} // namespace aurora::gfx
