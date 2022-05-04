#pragma once

#include "common.hpp"

#include <type_traits>
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
constexpr u32 MaxVtxAttr = GX::VA_MAX_ATTR;
constexpr u32 MaxTevSwap = GX::MAX_TEVSWAP;

template <typename Arg, Arg Default>
struct TevPass {
  Arg a = Default;
  Arg b = Default;
  Arg c = Default;
  Arg d = Default;
  bool operator==(const TevPass&) const = default;
};
static_assert(std::has_unique_object_representations_v<TevPass<GX::TevColorArg, GX::CC_ZERO>>);
static_assert(std::has_unique_object_representations_v<TevPass<GX::TevAlphaArg, GX::CA_ZERO>>);
struct TevOp {
  GX::TevOp op = GX::TevOp::TEV_ADD;
  GX::TevBias bias = GX::TevBias::TB_ZERO;
  GX::TevScale scale = GX::TevScale::CS_SCALE_1;
  GX::TevRegID outReg = GX::TevRegID::TEVPREV;
  bool clamp = true;
  u8 _p1 = 0;
  u8 _p2 = 0;
  u8 _p3 = 0;
  bool operator==(const TevOp&) const = default;
};
static_assert(std::has_unique_object_representations_v<TevOp>);
struct TevStage {
  TevPass<GX::TevColorArg, GX::CC_ZERO> colorPass;
  TevPass<GX::TevAlphaArg, GX::CA_ZERO> alphaPass;
  TevOp colorOp;
  TevOp alphaOp;
  GX::TevKColorSel kcSel = GX::TEV_KCSEL_1;
  GX::TevKAlphaSel kaSel = GX::TEV_KASEL_1;
  GX::TexCoordID texCoordId = GX::TEXCOORD_NULL;
  GX::TexMapID texMapId = GX::TEXMAP_NULL;
  GX::ChannelID channelId = GX::COLOR_NULL;
  GX::TevSwapSel tevSwapRas = GX::TEV_SWAP0;
  GX::TevSwapSel tevSwapTex = GX::TEV_SWAP0;
  bool operator==(const TevStage&) const = default;
};
static_assert(std::has_unique_object_representations_v<TevStage>);
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
struct ColorChannelConfig {
  GX::ColorSrc matSrc = GX::SRC_REG;
  GX::ColorSrc ambSrc = GX::SRC_REG;
  bool lightingEnabled = false;
  u8 _p1 = 0;
  u8 _p2 = 0;
  u8 _p3 = 0;
  bool operator==(const ColorChannelConfig&) const = default;
};
static_assert(std::has_unique_object_representations_v<ColorChannelConfig>);
// For uniform generation
struct ColorChannelState {
  zeus::CColor matColor;
  zeus::CColor ambColor;
  GX::LightMask lightState;
};
using LightVariant = std::variant<std::monostate, Light, zeus::CColor>;
// Mat4x4 used instead of Mat4x3 for padding purposes
using TexMtxVariant = std::variant<std::monostate, Mat4x2<float>, Mat4x4<float>>;
struct TcgConfig {
  GX::TexGenType type = GX::TG_MTX2x4;
  GX::TexGenSrc src = GX::MAX_TEXGENSRC;
  GX::TexMtx mtx = GX::IDENTITY;
  GX::PTTexMtx postMtx = GX::PTIDENTITY;
  bool normalize = false;
  u8 _p1 = 0;
  u8 _p2 = 0;
  u8 _p3 = 0;
  bool operator==(const TcgConfig&) const = default;
};
static_assert(std::has_unique_object_representations_v<TcgConfig>);
struct FogState {
  GX::FogType type = GX::FOG_NONE;
  float startZ = 0.f;
  float endZ = 0.f;
  float nearZ = 0.f;
  float farZ = 0.f;
  zeus::CColor color;
};
struct TevSwap {
  GX::TevColorChan red = GX::CH_RED;
  GX::TevColorChan green = GX::CH_GREEN;
  GX::TevColorChan blue = GX::CH_BLUE;
  GX::TevColorChan alpha = GX::CH_ALPHA;
  bool operator==(const TevSwap&) const = default;
  operator bool() const { return *this != TevSwap{}; }
};
static_assert(std::has_unique_object_representations_v<TevSwap>);
struct AlphaCompare {
  GX::Compare comp0 = GX::ALWAYS;
  u32 ref0; // would be u8 but extended to avoid padding bytes
  GX::AlphaOp op = GX::AOP_AND;
  GX::Compare comp1 = GX::ALWAYS;
  u32 ref1;
  bool operator==(const AlphaCompare& other) const = default;
  operator bool() const { return *this != AlphaCompare{}; }
};
static_assert(std::has_unique_object_representations_v<AlphaCompare>);

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
  u32 dstAlpha; // u8; UINT32_MAX = disabled
  AlphaCompare alphaCompare;
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
  std::array<GX::AttrType, MaxVtxAttr> vtxDesc;
  std::array<TevSwap, MaxTevSwap> tevSwapTable{
      TevSwap{},
      TevSwap{GX::CH_RED, GX::CH_RED, GX::CH_RED, GX::CH_ALPHA},
      TevSwap{GX::CH_GREEN, GX::CH_GREEN, GX::CH_GREEN, GX::CH_ALPHA},
      TevSwap{GX::CH_BLUE, GX::CH_BLUE, GX::CH_BLUE, GX::CH_ALPHA},
  };
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

void shutdown() noexcept;
const TextureBind& get_texture(GX::TexMapID id) noexcept;

struct ShaderConfig {
  GX::FogType fogType;
  std::array<GX::AttrType, MaxVtxAttr> vtxAttrs;
  std::array<TevSwap, MaxTevSwap> tevSwapTable;
  std::array<TevStage, MaxTevStages> tevStages;
  u32 tevStageCount = 0;
  std::array<ColorChannelConfig, MaxColorChannels> colorChannels;
  std::array<TcgConfig, MaxTexCoord> tcgs;
  AlphaCompare alphaCompare;
  u32 indexedAttributeCount = 0;
  bool operator==(const ShaderConfig&) const = default;
};
static_assert(std::has_unique_object_representations_v<ShaderConfig>);

constexpr u32 GXPipelineConfigVersion = 1;
struct PipelineConfig {
  u32 version = GXPipelineConfigVersion;
  ShaderConfig shaderConfig;
  GX::Primitive primitive;
  GX::Compare depthFunc;
  GX::CullMode cullMode;
  GX::BlendMode blendMode;
  GX::BlendFactor blendFacSrc, blendFacDst;
  GX::LogicOp blendOp;
  u32 dstAlpha;
  bool depthCompare, depthUpdate, alphaUpdate;
  u8 _pad = 0;
};
static_assert(std::has_unique_object_representations_v<PipelineConfig>);

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
  std::bitset<MaxTextures> sampledTextures;
  std::bitset<MaxKColors> sampledKColors;
  std::bitset<MaxColorChannels> sampledColorChannels;
  std::bitset<MaxTevRegs> usesTevReg;
  std::bitset<MaxTexMtx> usesTexMtx;
  std::bitset<MaxPTTexMtx> usesPTTexMtx;
  std::array<GX::TexGenType, MaxTexMtx> texMtxTypes;
  u32 uniformSize = 0;
  bool usesFog : 1 = false;
};
struct BindGroupRanges {
  Range vtxDataRange;
  Range nrmDataRange;
  Range tcDataRange;
  Range packedTcDataRange;
};
void populate_pipeline_config(PipelineConfig& config, GX::Primitive primitive) noexcept;
wgpu::RenderPipeline build_pipeline(const PipelineConfig& config, const ShaderInfo& info,
                                    ArrayRef<wgpu::VertexBufferLayout> vtxBuffers, wgpu::ShaderModule shader,
                                    zstring_view label) noexcept;
ShaderInfo build_shader_info(const ShaderConfig& config) noexcept;
wgpu::ShaderModule build_shader(const ShaderConfig& config, const ShaderInfo& info) noexcept;
// Range build_vertex_buffer(const GXShaderInfo& info) noexcept;
Range build_uniform(const ShaderInfo& info) noexcept;
GXBindGroupLayouts build_bind_group_layouts(const ShaderInfo& info, const ShaderConfig& config) noexcept;
GXBindGroups build_bind_groups(const ShaderInfo& info, const ShaderConfig& config,
                               const BindGroupRanges& ranges) noexcept;
} // namespace aurora::gfx::gx
