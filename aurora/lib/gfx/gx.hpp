#pragma once

#include "common.hpp"

#include <variant>

namespace aurora::gfx {
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
struct STextureBind {
  aurora::gfx::TextureHandle handle;
  metaforce::EClampMode clampMode;
  float lod;

  STextureBind() noexcept = default;
  STextureBind(aurora::gfx::TextureHandle handle, metaforce::EClampMode clampMode, float lod) noexcept
  : handle(std::move(handle)), clampMode(clampMode), lod(lod) {}
  void reset() noexcept { handle.reset(); };
  [[nodiscard]] wgpu::SamplerDescriptor get_descriptor() const noexcept;
  operator bool() const noexcept { return handle; }
};

constexpr u32 maxTevStages = GX::MAX_TEVSTAGE;
extern std::array<std::optional<STevStage>, maxTevStages> g_tevStages;
constexpr u32 maxTextures = 8;
extern std::array<STextureBind, maxTextures> g_textures;

static inline Mat4x4<float> get_combined_matrix() noexcept { return g_proj * g_mv; }

const STextureBind& get_texture(GX::TexMapID id) noexcept;

struct GXShaderConfig {
  std::array<std::optional<STevStage>, maxTevStages> tevStages;
  bool alphaDiscard;
  bool denormalizedVertexAttributes;
  bool denormalizedNorm;
  bool denormalizedColor;
  std::bitset<maxTextures> boundTextures;
};
struct GXPipelineConfig {
  GXShaderConfig shaderConfig;
  GX::Primitive primitive;
  metaforce::ERglEnum depthFunc;
  metaforce::ERglCullMode cullMode;
  metaforce::ERglBlendMode blendMode;
  metaforce::ERglBlendFactor blendFacSrc, blendFacDst;
  metaforce::ERglLogicOp blendOp;
  std::optional<float> dstAlpha;
  bool depthCompare, depthUpdate, alphaUpdate;
};
void populate_gx_pipeline_config(GXPipelineConfig& config, GX::Primitive primitive,
                                 std::bitset<maxTextures> enabledTextures) noexcept;
wgpu::ShaderModule build_shader(const GXShaderConfig& config);
wgpu::RenderPipeline build_pipeline(wgpu::PipelineLayout layout, ArrayRef<wgpu::VertexBufferLayout> vtxBuffers,
                                    const GXPipelineConfig& config, zstring_view label) noexcept;
} // namespace aurora::gfx
