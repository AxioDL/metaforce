#pragma once

#include "common.hpp"

#include <variant>

namespace aurora::gfx::gx {
extern zeus::CMatrix4f g_mv;
extern zeus::CMatrix4f g_mvInv;
extern zeus::CMatrix4f g_proj;
extern metaforce::CFogState g_fogState;
extern metaforce::ERglCullMode g_cullMode;
extern metaforce::ERglBlendMode g_blendMode;
extern metaforce::ERglBlendFactor g_blendFacSrc;
extern metaforce::ERglBlendFactor g_blendFacDst;
extern metaforce::ERglLogicOp g_blendOp;
extern bool g_depthCompare;
extern bool g_depthUpdate;
extern metaforce::ERglEnum g_depthFunc;
extern std::array<zeus::CColor, 3> g_colorRegs;
extern std::array<zeus::CColor, GX::MAX_KCOLOR> g_kcolors;
extern bool g_alphaUpdate;
extern std::optional<float> g_dstAlpha;
extern zeus::CColor g_clearColor;
extern bool g_alphaDiscard;

struct SChannelState {
  zeus::CColor matColor;
  zeus::CColor ambColor;
  GX::ColorSrc matSrc;
};
extern std::array<SChannelState, 2> g_colorChannels;
using LightVariant = std::variant<std::monostate, Light, zeus::CColor>;
extern std::array<LightVariant, MaxLights> g_lights;
extern std::bitset<MaxLights> g_lightState;

struct STevStage {
  metaforce::CTevCombiners::ColorPass colorPass;
  metaforce::CTevCombiners::AlphaPass alphaPass;
  metaforce::CTevCombiners::CTevOp colorOp;
  metaforce::CTevCombiners::CTevOp alphaOp;
  GX::TevKColorSel kcSel = GX::TEV_KCSEL_1;
  GX::TevKAlphaSel kaSel = GX::TEV_KASEL_1;
  GX::TexCoordID texCoordId = GX::TEXCOORD_NULL;
  GX::TexMapID texMapId = GX::TEXMAP_NULL;
  GX::ChannelID channelId = GX::COLOR_NULL;

  constexpr STevStage(const metaforce::CTevCombiners::ColorPass& colPass,
                      const metaforce::CTevCombiners::AlphaPass& alphaPass,
                      const metaforce::CTevCombiners::CTevOp& colorOp,
                      const metaforce::CTevCombiners::CTevOp& alphaOp) noexcept
  : colorPass(colPass), alphaPass(alphaPass), colorOp(colorOp), alphaOp(alphaOp) {}
};
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

constexpr u32 maxTevStages = GX::MAX_TEVSTAGE;
extern std::array<std::optional<STevStage>, maxTevStages> g_tevStages;
constexpr u32 maxTextures = 8;
extern std::array<TextureBind, maxTextures> g_textures;

static inline Mat4x4<float> get_combined_matrix() noexcept { return g_proj * g_mv; }

void shutdown() noexcept;
const TextureBind& get_texture(GX::TexMapID id) noexcept;

struct ShaderConfig {
  std::array<std::optional<STevStage>, maxTevStages> tevStages;
  std::array<GX::ColorSrc, 2> channelMatSrcs;
  bool alphaDiscard;
  bool denormalizedVertexAttributes;
};
struct PipelineConfig {
  ShaderConfig shaderConfig;
  GX::Primitive primitive;
  metaforce::ERglEnum depthFunc;
  metaforce::ERglCullMode cullMode;
  metaforce::ERglBlendMode blendMode;
  metaforce::ERglBlendFactor blendFacSrc, blendFacDst;
  metaforce::ERglLogicOp blendOp;
  std::optional<float> dstAlpha;
  bool depthCompare, depthUpdate, alphaUpdate;
};
// Output info from shader generation
struct ShaderInfo {
  std::bitset<maxTextures> sampledTextures;
  std::bitset<4> sampledKcolors;
  std::bitset<2> sampledColorChannels;
  u32 uniformSize = 0;
  bool usesVtxColor : 1 = false;
  bool usesNormal : 1 = false;
};
ShaderInfo populate_pipeline_config(PipelineConfig& config, GX::Primitive primitive) noexcept;
wgpu::RenderPipeline build_pipeline(const PipelineConfig& config, const ShaderInfo& info,
                                    ArrayRef<wgpu::VertexBufferLayout> vtxBuffers, wgpu::ShaderModule shader,
                                    zstring_view label) noexcept;
std::pair<wgpu::ShaderModule, ShaderInfo> build_shader(const ShaderConfig& config) noexcept;

// Range build_vertex_buffer(const GXShaderInfo& info) noexcept;
Range build_uniform(const ShaderInfo& info) noexcept;

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
GXBindGroupLayouts build_bind_group_layouts(const ShaderInfo& info) noexcept;
GXBindGroups build_bind_groups(const ShaderInfo& info) noexcept;
} // namespace aurora::gfx::gx

namespace aurora {
template <>
inline void xxh3_update(XXH3_state_t& state, const gfx::gx::STevStage& input) {
  XXH3_64bits_update(&state, &input.colorPass, sizeof(metaforce::CTevCombiners::ColorPass));
  XXH3_64bits_update(&state, &input.alphaPass, sizeof(metaforce::CTevCombiners::AlphaPass));
  XXH3_64bits_update(&state, &input.colorOp, sizeof(metaforce::CTevCombiners::CTevOp));
  XXH3_64bits_update(&state, &input.alphaOp, sizeof(metaforce::CTevCombiners::CTevOp));
  XXH3_64bits_update(&state, &input.kcSel, sizeof(GX::TevKColorSel));
  XXH3_64bits_update(&state, &input.kaSel, sizeof(GX::TevKAlphaSel));
  XXH3_64bits_update(&state, &input.texCoordId, sizeof(GX::TexCoordID));
  XXH3_64bits_update(&state, &input.texMapId, sizeof(GX::TexMapID));
  XXH3_64bits_update(&state, &input.channelId, sizeof(GX::ChannelID));
}
template <>
inline XXH64_hash_t xxh3_hash(const gfx::gx::ShaderConfig& input, XXH64_hash_t seed) {
  XXH3_state_t state;
  XXH3_INITSTATE(&state);
  XXH3_64bits_reset_withSeed(&state, seed);
  for (const auto& item : input.tevStages) {
    if (!item) {
      break;
    }
    xxh3_update(state, *item);
  }
  XXH3_64bits_update(&state, input.channelMatSrcs.data(), input.channelMatSrcs.size() * sizeof(GX::ColorSrc));
  XXH3_64bits_update(&state, &input.alphaDiscard, sizeof(bool));
  XXH3_64bits_update(&state, &input.denormalizedVertexAttributes, sizeof(bool));
  return XXH3_64bits_digest(&state);
}
} // namespace aurora