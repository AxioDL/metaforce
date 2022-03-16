#pragma once

#include "common.hpp"

#include <variant>

namespace aurora::gfx::gx {
constexpr u32 MaxTextures = GX::MAX_TEXMAP;
constexpr u32 MaxTevStages = GX::MAX_TEVSTAGE;
constexpr u32 MaxColorChannels = 2; // COLOR0A0, COLOR1A1
constexpr u32 MaxTevRegs = 3;       // TEVREG0-2
constexpr u32 MaxKColors = GX::MAX_KCOLOR;
constexpr u32 MaxTexMtx = 10;
constexpr u32 MaxPTTexMtx = 20;
constexpr u32 MaxTexCoord = GX::MAX_TEXCOORD;

PACK(template <typename Arg, Arg Default>
struct TevPass {
  Arg a = Default;
  Arg b = Default;
  Arg c = Default;
  Arg d = Default;
  bool operator==(const TevPass&) const = default;
});
PACK(struct TevOp {
  GX::TevOp op = GX::TevOp::TEV_ADD;
  GX::TevBias bias = GX::TevBias::TB_ZERO;
  GX::TevScale scale = GX::TevScale::CS_SCALE_1;
  GX::TevRegID outReg = GX::TevRegID::TEVPREV;
  bool clamp = true;
  bool operator==(const TevOp&) const = default;
});
PACK(struct TevStage {
  TevPass<GX::TevColorArg, GX::CC_ZERO> colorPass;
  TevPass<GX::TevAlphaArg, GX::CA_ZERO> alphaPass;
  TevOp colorOp;
  TevOp alphaOp;
  GX::TevKColorSel kcSel = GX::TEV_KCSEL_1;
  GX::TevKAlphaSel kaSel = GX::TEV_KASEL_1;
  GX::TexCoordID texCoordId = GX::TEXCOORD_NULL;
  GX::TexMapID texMapId = GX::TEXMAP_NULL;
  GX::ChannelID channelId = GX::COLOR_NULL;
  bool operator==(const TevStage&) const = default;
});
struct TextureBind {
  aurora::gfx::TextureHandle handle;
  metaforce::EClampMode clampMode;
  float lod;

  TextureBind() noexcept = default;
  TextureBind(aurora::gfx::TextureHandle handle, metaforce::EClampMode clampMode, float lod) noexcept
  : handle(std::move(handle)), clampMode(clampMode), lod(lod) {}
  void reset() noexcept { handle.reset(); };
  [[nodiscard]] wgpu::SamplerDescriptor get_descriptor() const noexcept;
  operator bool() const noexcept { return handle; }
};
// For shader generation
PACK(struct ColorChannelConfig {
  GX::ColorSrc matSrc = GX::SRC_REG;
  GX::ColorSrc ambSrc = GX::SRC_REG;
  bool lightingEnabled = false;
  bool operator==(const ColorChannelConfig&) const = default;
});
// For uniform generation
PACK(struct ColorChannelState {
  zeus::CColor matColor = zeus::skClear;
  zeus::CColor ambColor = zeus::skClear;
  GX::LightMask lightState;
});
using LightVariant = std::variant<std::monostate, Light, zeus::CColor>;
// Mat4x4 used instead of Mat4x3 for padding purposes
using TexMtxVariant = std::variant<std::monostate, Mat4x2<float>, Mat4x4<float>>;
PACK(struct TcgConfig {
  GX::TexGenType type = GX::TG_MTX2x4;
  GX::TexGenSrc src = GX::MAX_TEXGENSRC;
  GX::TexMtx mtx = GX::IDENTITY;
  GX::PTTexMtx postMtx = GX::PTIDENTITY;
  bool normalize = false;
  bool operator==(const TcgConfig&) const = default;
});
PACK(struct FogState {
  GX::FogType type = GX::FOG_NONE;
  float startZ = 0.f;
  float endZ = 0.f;
  float nearZ = 0.f;
  float farZ = 0.f;
  zeus::CColor color;
});

struct GXState {
  zeus::CMatrix4f mv;
  zeus::CMatrix4f mvInv;
  zeus::CMatrix4f proj;
  FogState fog;
  GX::CullMode cullMode = GX::CULL_BACK;
  GX::BlendMode blendMode = GX::BM_NONE;
  GX::BlendFactor blendFacSrc = GX::BL_SRCALPHA;
  GX::BlendFactor blendFacDst = GX::BL_INVSRCALPHA;
  GX::LogicOp blendOp = GX::LO_CLEAR;
  GX::Compare depthFunc = GX::LEQUAL;
  zeus::CColor clearColor = zeus::skBlack;
  std::optional<float> dstAlpha;
  std::optional<float> alphaDiscard;
  std::array<zeus::CColor, MaxTevRegs> colorRegs;
  std::array<zeus::CColor, GX::MAX_KCOLOR> kcolors;
  std::array<ColorChannelConfig, MaxColorChannels> colorChannelConfig;
  std::array<ColorChannelState, MaxColorChannels> colorChannelState;
  std::array<LightVariant, GX::MaxLights> lights;
  std::array<TevStage, MaxTevStages> tevStages;
  std::array<TextureBind, MaxTextures> textures;
  std::array<TexMtxVariant, MaxTexMtx> texMtxs;
  std::array<Mat4x4<float>, MaxPTTexMtx> ptTexMtxs;
  std::array<TcgConfig, MaxTexCoord> tcgs;
  bool depthCompare = true;
  bool depthUpdate = true;
  bool alphaUpdate = true;
  u8 numChans = 0;
  u8 numIndStages = 0;
  u8 numTevStages = 0;
  u8 numTexGens = 0;
};
extern GXState g_gxState;

static inline Mat4x4<float> get_combined_matrix() noexcept { return g_gxState.proj * g_gxState.mv; }

void initialize() noexcept;
void shutdown() noexcept;
const TextureBind& get_texture(GX::TexMapID id) noexcept;

PACK(struct ShaderConfig {
  GX::FogType fogType;
  std::array<std::optional<TevStage>, MaxTevStages> tevStages;
  std::array<ColorChannelConfig, MaxColorChannels> colorChannels;
  std::array<TcgConfig, MaxTexCoord> tcgs;
  std::optional<float> alphaDiscard;
  bool denormalizedVertexAttributes = false;
  bool denormalizedHasNrm = false; // TODO this is a hack
  bool operator==(const ShaderConfig&) const = default;
});
PACK(struct PipelineConfig {
  ShaderConfig shaderConfig;
  GX::Primitive primitive;
  GX::Compare depthFunc;
  GX::CullMode cullMode;
  GX::BlendMode blendMode;
  GX::BlendFactor blendFacSrc, blendFacDst;
  GX::LogicOp blendOp;
  std::optional<float> dstAlpha;
  bool depthCompare, depthUpdate, alphaUpdate;
});
struct GXBindGroupLayouts {
  wgpu::BindGroupLayout uniformLayout;
  wgpu::BindGroupLayout samplerLayout;
  wgpu::BindGroupLayout textureLayout;
};
struct GXBindGroups {
  BindGroupRef uniformBindGroup;
  BindGroupRef samplerBindGroup;
  BindGroupRef textureBindGroup;
};
// Output info from shader generation
struct ShaderInfo {
  GXBindGroups bindGroups;
  std::bitset<MaxTextures> sampledTextures;
  std::bitset<MaxKColors> sampledKColors;
  std::bitset<MaxColorChannels> sampledColorChannels;
  std::bitset<MaxTevRegs> usesTevReg;
  std::bitset<MaxTexMtx> usesTexMtx;
  std::bitset<MaxPTTexMtx> usesPTTexMtx;
  std::array<GX::TexGenType, MaxTexMtx> texMtxTypes;
  u32 uniformSize = 0;
  bool usesVtxColor : 1 = false;
  bool usesNormal : 1 = false;
  bool usesFog : 1 = false;
};
struct BindGroupRanges {
  Range vtxDataRange;
  Range nrmDataRange;
  Range tcDataRange;
  Range packedTcDataRange;
};
ShaderInfo populate_pipeline_config(PipelineConfig& config, GX::Primitive primitive,
                                    const BindGroupRanges& ranges) noexcept;
wgpu::RenderPipeline build_pipeline(const PipelineConfig& config, const ShaderInfo& info,
                                    ArrayRef<wgpu::VertexBufferLayout> vtxBuffers, wgpu::ShaderModule shader,
                                    zstring_view label) noexcept;
std::pair<wgpu::ShaderModule, ShaderInfo> build_shader(const ShaderConfig& config) noexcept;
// Range build_vertex_buffer(const GXShaderInfo& info) noexcept;
Range build_uniform(const ShaderInfo& info) noexcept;
GXBindGroupLayouts build_bind_group_layouts(const ShaderInfo& info, const ShaderConfig& config) noexcept;
GXBindGroups build_bind_groups(const ShaderInfo& info, const ShaderConfig& config,
                               const BindGroupRanges& ranges) noexcept;

struct DlVert {
  s16 pos;
  s16 norm;
  // colors ignored
  std::array<s16, 7> uvs;
  // pn_mtx_idx ignored
  // tex_mtx_idxs ignored
  s16 _pad;
};
} // namespace aurora::gfx::gx

namespace aurora {
//template <typename Arg, Arg Default>
//inline void xxh3_update(XXH3_state_t& state, const gfx::gx::TevPass<Arg, Default>& input) {
//  XXH3_64bits_update(&state, &input.a, sizeof(Arg));
//  XXH3_64bits_update(&state, &input.b, sizeof(Arg));
//  XXH3_64bits_update(&state, &input.c, sizeof(Arg));
//  XXH3_64bits_update(&state, &input.d, sizeof(Arg));
//}
//template <>
//inline void xxh3_update(XXH3_state_t& state, const gfx::gx::TevOp& input) {
//  XXH3_64bits_update(&state, &input.op, sizeof(gfx::gx::TevOp::op));
//  XXH3_64bits_update(&state, &input.bias, sizeof(gfx::gx::TevOp::bias));
//  XXH3_64bits_update(&state, &input.scale, sizeof(gfx::gx::TevOp::scale));
//  XXH3_64bits_update(&state, &input.outReg, sizeof(gfx::gx::TevOp::outReg));
//  XXH3_64bits_update(&state, &input.clamp, sizeof(bool));
//}
//template <>
//inline void xxh3_update(XXH3_state_t& state, const gfx::gx::TevStage& input) {
//  xxh3_update(state, input.colorPass);
//  xxh3_update(state, input.alphaPass);
//  xxh3_update(state, input.colorOp);
//  xxh3_update(state, input.alphaOp);
//  XXH3_64bits_update(&state, &input.kcSel, sizeof(gfx::gx::TevStage::kcSel));
//  XXH3_64bits_update(&state, &input.kaSel, sizeof(gfx::gx::TevStage::kaSel));
//  XXH3_64bits_update(&state, &input.texCoordId, sizeof(gfx::gx::TevStage::texCoordId));
//  XXH3_64bits_update(&state, &input.texMapId, sizeof(gfx::gx::TevStage::texMapId));
//  XXH3_64bits_update(&state, &input.channelId, sizeof(gfx::gx::TevStage::channelId));
//}
//template <>
//inline void xxh3_update(XXH3_state_t& state, const gfx::gx::ColorChannelConfig& input) {
//  XXH3_64bits_update(&state, &input.lightingEnabled, sizeof(gfx::gx::ColorChannelConfig::lightingEnabled));
//  XXH3_64bits_update(&state, &input.matSrc, sizeof(gfx::gx::ColorChannelConfig::matSrc));
//  if (input.lightingEnabled) {
//    // Unused when lighting is disabled
//    XXH3_64bits_update(&state, &input.ambSrc, sizeof(gfx::gx::ColorChannelConfig::ambSrc));
//  }
//}
//template <>
//inline void xxh3_update(XXH3_state_t& state, const gfx::gx::TcgConfig& input) {
//  XXH3_64bits_update(&state, &input.type, sizeof(gfx::gx::TcgConfig::type));
//  XXH3_64bits_update(&state, &input.src, sizeof(gfx::gx::TcgConfig::src));
//  XXH3_64bits_update(&state, &input.mtx, sizeof(gfx::gx::TcgConfig::mtx));
//  XXH3_64bits_update(&state, &input.postMtx, sizeof(gfx::gx::TcgConfig::postMtx));
//  XXH3_64bits_update(&state, &input.normalize, sizeof(gfx::gx::TcgConfig::normalize));
//}
template <>
inline XXH64_hash_t xxh3_hash(const gfx::gx::ShaderConfig& input, XXH64_hash_t seed) {
  return xxh3_hash_s(&input, sizeof(gfx::gx::ShaderConfig), seed);
//  for (const auto& item : input.tevStages) {
//    if (!item) {
//      break;
//    }
//    xxh3_update(state, *item);
//  }
//  for (const auto& item : input.colorChannels) {
//    xxh3_update(state, item);
//  }
//  for (const auto& item : input.tcgs) {
//    xxh3_update(state, item);
//  }
//  if (input.alphaDiscard) {
//    XXH3_64bits_update(&state, &*input.alphaDiscard, sizeof(float));
//  }
//  XXH3_64bits_update(&state, &input.denormalizedVertexAttributes,
//                     sizeof(gfx::gx::ShaderConfig::denormalizedVertexAttributes));
//  XXH3_64bits_update(&state, &input.denormalizedHasNrm, sizeof(gfx::gx::ShaderConfig::denormalizedHasNrm));
//  XXH3_64bits_update(&state, &input.fogType, sizeof(gfx::gx::ShaderConfig::fogType));
}
} // namespace aurora
