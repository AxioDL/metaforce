#pragma once

#include "common.hpp"

// TODO make this shared?
#include "../../../Runtime/Graphics/GX.hpp"

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

enum class ERglCullMode { None = 0, Front = 1, Back = 2, All = 3 };

enum class ERglBlendMode { None = 0, Blend = 1, Logic = 2, Subtract = 3 };

enum class ERglBlendFactor {
  Zero = 0,
  One = 1,
  SrcColor = 2,
  InvSrcColor = 3,
  SrcAlpha = 4,
  InvSrcAlpha = 5,
  DstAlpha = 6,
  InvDstAlpha = 7,
  DstColor = 8,
  InvDstColor = 9,
};

enum class ERglLogicOp {
  Clear = 0,
  And = 1,
  RevAnd = 2,
  Copy = 3,
  InvAnd = 4,
  NoOp = 5,
  Xor = 6,
  Or = 7,
  Nor = 8,
  Equiv = 9,
  Inv = 10,
  RevOr = 11,
  InvCopy = 12,
  InvOr = 13,
  NAnd = 14,
  Set = 15
};

enum class ERglAlphaFunc {
  Never = 0,
  Less = 1,
  Equal = 2,
  LEqual = 3,
  Greater = 4,
  NEqual = 5,
  GEqual = 6,
  Always = 7
};

enum class ERglAlphaOp { And = 0, Or = 1, Xor = 2, XNor = 3 };

enum class ERglEnum { Never = 0, Less = 1, Equal = 2, LEqual = 3, Greater = 4, NEqual = 5, GEqual = 6, Always = 7 };

enum class ERglTevStage : u32 {
  Stage0,
  Stage1,
  Stage2,
  Stage3,
  Stage4,
  Stage5,
  Stage6,
  Stage7,
  Stage8,
  Stage9,
  Stage10,
  Stage11,
  Stage12,
  Stage13,
  Stage14,
  Stage15,
  MAX
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

namespace CTevCombiners {
struct CTevOp {
  bool x0_clamp = true;
  GX::TevOp x4_op = GX::TevOp::TEV_ADD;
  GX::TevBias x8_bias = GX::TevBias::TB_ZERO;
  GX::TevScale xc_scale = GX::TevScale::CS_SCALE_1;
  GX::TevRegID x10_regId = GX::TevRegID::TEVPREV;

  constexpr CTevOp() = default;
  constexpr CTevOp(bool clamp, GX::TevOp op, GX::TevBias bias, GX::TevScale scale, GX::TevRegID regId)
  : x0_clamp(clamp), x4_op(op), x8_bias(bias), xc_scale(scale), x10_regId(regId) {}
  constexpr CTevOp(u32 compressedDesc)
  : x0_clamp((compressedDesc >> 8 & 1) != 0)
  , x4_op(static_cast<GX::TevOp>(compressedDesc & 0xF))
  , x8_bias(static_cast<GX::TevBias>(compressedDesc >> 4 & 3))
  , xc_scale(static_cast<GX::TevScale>(compressedDesc >> 6 & 3))
  , x10_regId(static_cast<GX::TevRegID>(compressedDesc >> 9 & 3)) {}

  auto operator<=>(const CTevOp&) const = default;
};
struct ColorPass {
  GX::TevColorArg x0_a;
  GX::TevColorArg x4_b;
  GX::TevColorArg x8_c;
  GX::TevColorArg xc_d;

  constexpr ColorPass(GX::TevColorArg a, GX::TevColorArg b, GX::TevColorArg c, GX::TevColorArg d)
  : x0_a(a), x4_b(b), x8_c(c), xc_d(d) {}
  constexpr ColorPass(u32 compressedDesc)
  : x0_a(static_cast<GX::TevColorArg>(compressedDesc & 0x1F))
  , x4_b(static_cast<GX::TevColorArg>(compressedDesc >> 5 & 0x1F))
  , x8_c(static_cast<GX::TevColorArg>(compressedDesc >> 10 & 0x1F))
  , xc_d(static_cast<GX::TevColorArg>(compressedDesc >> 15 & 0x1F)) {}

  auto operator<=>(const ColorPass&) const = default;
};
struct AlphaPass {
  GX::TevAlphaArg x0_a;
  GX::TevAlphaArg x4_b;
  GX::TevAlphaArg x8_c;
  GX::TevAlphaArg xc_d;

  constexpr AlphaPass(GX::TevAlphaArg a, GX::TevAlphaArg b, GX::TevAlphaArg c, GX::TevAlphaArg d)
  : x0_a(a), x4_b(b), x8_c(c), xc_d(d) {}
  constexpr AlphaPass(u32 compressedDesc)
  : x0_a(static_cast<GX::TevAlphaArg>(compressedDesc & 0x1F))
  , x4_b(static_cast<GX::TevAlphaArg>(compressedDesc >> 5 & 0x1F))
  , x8_c(static_cast<GX::TevAlphaArg>(compressedDesc >> 10 & 0x1F))
  , xc_d(static_cast<GX::TevAlphaArg>(compressedDesc >> 15 & 0x1F)) {}

  auto operator<=>(const AlphaPass&) const = default;
};
} // namespace CTevCombiners
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

constexpr u32 MaxLights = 8;
struct Light {
  zeus::CVector3f pos{0.f, 0.f, 0.f};
  zeus::CVector3f dir{0.f, 0.f, -1.f};
  zeus::CColor color{0.f, 0.f, 0.f, 0.f};
  zeus::CVector3f linAtt{1.f, 0.f, 0.f};
  zeus::CVector3f angAtt{1.f, 0.f, 0.f};
};

[[nodiscard]] bool get_dxt_compression_supported() noexcept;

void stream_begin(GX::Primitive primitive) noexcept;
void stream_vertex(metaforce::EStreamFlags flags, const zeus::CVector3f& pos, const zeus::CVector3f& nrm,
                   const zeus::CColor& color, const zeus::CVector2f& uv) noexcept;
void stream_end() noexcept;

// GX state
void bind_texture(GX::TexMapID id, metaforce::EClampMode clamp, const TextureHandle& tex, float lod) noexcept;
void unbind_texture(GX::TexMapID id) noexcept;
void disable_tev_stage(metaforce::ERglTevStage stage) noexcept;
void update_tev_stage(metaforce::ERglTevStage stage, const metaforce::CTevCombiners::ColorPass& colPass,
                      const metaforce::CTevCombiners::AlphaPass& alphaPass,
                      const metaforce::CTevCombiners::CTevOp& colorOp,
                      const metaforce::CTevCombiners::CTevOp& alphaOp) noexcept;
void set_cull_mode(metaforce::ERglCullMode mode) noexcept;
void set_blend_mode(metaforce::ERglBlendMode mode, metaforce::ERglBlendFactor src, metaforce::ERglBlendFactor dst,
                    metaforce::ERglLogicOp op) noexcept;
void set_depth_mode(bool compare_enable, metaforce::ERglEnum func, bool update_enable) noexcept;
void set_tev_reg_color(GX::TevRegID id, const zeus::CColor& color) noexcept;
void set_tev_k_color(GX::TevKColorID id, const zeus::CColor& color) noexcept;
void set_alpha_update(bool enabled) noexcept;
void set_dst_alpha(bool enabled, float value) noexcept;
void set_clear_color(const zeus::CColor& color) noexcept;
void set_tev_order(GX::TevStageID id, GX::TexCoordID tcid, GX::TexMapID tmid, GX::ChannelID cid) noexcept;
void set_tev_k_color_sel(GX::TevStageID id, GX::TevKColorSel sel) noexcept;
void set_tev_k_alpha_sel(GX::TevStageID id, GX::TevKAlphaSel sel) noexcept;
void set_chan_amb_color(GX::ChannelID id, const zeus::CColor& color) noexcept;
void set_chan_mat_color(GX::ChannelID id, const zeus::CColor& color) noexcept;
void set_chan_mat_src(GX::ChannelID id, GX::ColorSrc src) noexcept;

// Model state
void set_alpha_discard(bool v);

void update_model_view(const zeus::CMatrix4f& mv, const zeus::CMatrix4f& mv_inv) noexcept;
void update_projection(const zeus::CMatrix4f& proj) noexcept;
void update_fog_state(const metaforce::CFogState& state) noexcept;
void load_light(GX::LightID id, const Light& light) noexcept;
void load_light_ambient(GX::LightID id, const zeus::CColor& ambient) noexcept;
void set_light_state(std::bitset<MaxLights> bits) noexcept;
void set_viewport(const zeus::CRectangle& rect, float znear, float zfar) noexcept;
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
