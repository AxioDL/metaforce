#include "stream/shader.hpp"

#include "../gpu.hpp"
#include "common.hpp"
#include "gx.hpp"

#include <utility>

#include <magic_enum.hpp>

namespace aurora::gfx {
using namespace fmt::literals;

static logvisor::Module Log("aurora::gfx::stream");

struct SStreamState {
  GX::Primitive primitive;
  metaforce::EStreamFlags flags;
  uint32_t vertexCount = 0;
  ByteBuffer vertexBuffer;

  explicit SStreamState(GX::Primitive primitive) noexcept : primitive(primitive) {}
};
static std::optional<SStreamState> sStreamState;

constexpr u32 maxTextures = 8;
struct STextureBind {
  aurora::gfx::TextureHandle handle;
  metaforce::EClampMode clampMode;
  float lod;

  STextureBind() noexcept = default;
  STextureBind(aurora::gfx::TextureHandle handle, metaforce::EClampMode clampMode, float lod) noexcept
  : handle(std::move(handle)), clampMode(clampMode), lod(lod) {}
  void reset() noexcept { handle.reset(); };
  wgpu::SamplerDescriptor get_descriptor() const noexcept {
    wgpu::AddressMode mode;
    switch (clampMode) {
    case metaforce::EClampMode::Clamp:
      mode = wgpu::AddressMode::ClampToEdge;
      break;
    case metaforce::EClampMode::Repeat:
      mode = wgpu::AddressMode::Repeat;
      break;
    case metaforce::EClampMode::Mirror:
      mode = wgpu::AddressMode::MirrorRepeat;
      break;
    }
    return {
        .label = "Generated Sampler",
        .addressModeU = mode,
        .addressModeV = mode,
        .addressModeW = mode,
        // TODO logic from CTexture?
        .magFilter = wgpu::FilterMode::Linear,
        .minFilter = wgpu::FilterMode::Linear,
        .mipmapFilter = wgpu::FilterMode::Linear,
        .maxAnisotropy = gpu::g_graphicsConfig.textureAnistropy,
    };
  }
  operator bool() const noexcept { return handle; }
};
static std::array<STextureBind, maxTextures> sTextures;

void bind_texture(GX::TexMapID id, metaforce::EClampMode clamp, const TextureHandle& tex, float lod) noexcept {
  sTextures[static_cast<size_t>(id)] = {tex, clamp, lod};
}

void unbind_texture(GX::TexMapID id) noexcept { sTextures[static_cast<size_t>(id)].reset(); }

void stream_begin(GX::Primitive primitive) noexcept {
  if (sStreamState) {
    Log.report(logvisor::Fatal, FMT_STRING("Stream began twice!"));
    unreachable();
  }
  sStreamState.emplace(primitive);
}

void stream_vertex(metaforce::EStreamFlags flags, const zeus::CVector3f& pos, const zeus::CVector3f& nrm,
                   const zeus::CColor& color, const zeus::CVector2f& uv) noexcept {
  if (!sStreamState) {
    Log.report(logvisor::Fatal, FMT_STRING("Stream not started!"));
    unreachable();
  }
  if (sStreamState->flags) {
    if (sStreamState->flags != flags) {
      Log.report(logvisor::Fatal, FMT_STRING("Stream changed flags?"));
      unreachable();
    }
  } else {
    sStreamState->flags = flags;
    // TODO begin shader construction
  }
  sStreamState->vertexBuffer.append(&pos, 12);
  if (flags & metaforce::EStreamFlagBits::fHasNormal) {
    sStreamState->vertexBuffer.append(&nrm, 12);
  }
  if (flags & metaforce::EStreamFlagBits::fHasColor) {
    sStreamState->vertexBuffer.append(&color, 16);
  }
  if (flags & metaforce::EStreamFlagBits::fHasTexture) {
    sStreamState->vertexBuffer.append(&uv, 8);
  }
  sStreamState->vertexCount++;
}

static std::string color_arg_reg(GX::TevColorArg arg, size_t stageIdx) {
  switch (arg) {
  case GX::CC_CPREV:
    return "prev.rgb";
  case GX::CC_APREV:
    return "prev.a";
  case GX::CC_C0:
  case GX::CC_A0:
  case GX::CC_C1:
  case GX::CC_A1:
  case GX::CC_C2:
  case GX::CC_A2:
    Log.report(logvisor::Fatal, FMT_STRING("TODO {}"), arg);
    unreachable();
  case GX::CC_TEXC:
    return fmt::format(FMT_STRING("sampled{}.rgb"), stageIdx);
  case GX::CC_TEXA:
    return fmt::format(FMT_STRING("sampled{}.a"), stageIdx);
  case GX::CC_RASC:
    return "rast.rgb";
  case GX::CC_RASA:
    return "rast.a";
  case GX::CC_ONE:
    return "1.0";
  case GX::CC_HALF:
    return "0.5)";
  case GX::CC_KONST:
    return fmt::format(FMT_STRING("ubuf.kcolor{}.rgb"), stageIdx);
  case GX::CC_ZERO:
    return "0.0";
  }
}

static std::string alpha_arg_reg(GX::TevAlphaArg arg, size_t stageIdx) {
  switch (arg) {
  case GX::CA_APREV:
    return "prev.a";
  case GX::CA_A0:
  case GX::CA_A1:
  case GX::CA_A2:
    Log.report(logvisor::Fatal, FMT_STRING("TODO {}"), arg);
    unreachable();
  case GX::CA_TEXA:
    return fmt::format(FMT_STRING("sampled{}.a"), stageIdx);
  case GX::CA_RASA:
    return "rast.a";
  case GX::CA_KONST:
    return fmt::format(FMT_STRING("ubuf.kcolor{}.a"), stageIdx);
  case GX::CA_ZERO:
    return "0.0";
  }
}

static std::string_view tev_op(GX::TevOp op) {
  switch (op) {
  case GX::TEV_ADD:
    return "+";
  case GX::TEV_SUB:
    return "-";
  default:
    Log.report(logvisor::Fatal, FMT_STRING("TODO {}"), op);
    unreachable();
  }
}

static std::string_view tev_bias(GX::TevBias bias) {
  switch (bias) {
  case GX::TB_ZERO:
    return " + 0.0";
  case GX::TB_ADDHALF:
    return " + 0.5";
  case GX::TB_SUBHALF:
    return " - 0.5";
  }
}

static std::string_view tev_scale(GX::TevScale scale) {
  switch (scale) {
  case GX::CS_SCALE_1:
    return " * 1.0";
  case GX::CS_SCALE_2:
    return " * 2.0";
  case GX::CS_SCALE_4:
    return " * 4.0";
  case GX::CS_DIVIDE_2:
    return " / 2.0";
  }
}

std::unordered_map<ShaderRef, wgpu::ShaderModule> g_streamCachedShaders;

static ShaderRef generate_shader() {
  auto flags = sStreamState->flags;
  const auto hash = hash_tev_stages(static_cast<metaforce::EStreamFlags::MaskType>(flags));
  if (g_streamCachedShaders.contains(hash)) {
    return hash;
  }

  std::string uniBufAttrs;
  if (flags & metaforce::EStreamFlagBits::fHasTexture) {
    uniBufAttrs += fmt::format(FMT_STRING("\n    tex0_lod: f32;"));
  }
  std::string sampBindings;
  if (flags & metaforce::EStreamFlagBits::fHasTexture) {
    sampBindings +=
        "\n"
        "@group(1) @binding(0)\n"
        "var tex0_samp: sampler;";
  }
  std::string texBindings;
  if (flags & metaforce::EStreamFlagBits::fHasTexture) {
    texBindings +=
        "\n"
        "@group(2) @binding(0)\n"
        "var tex0: texture_2d<f32>;";
  }
  std::string vtxOutAttrs;
  std::string vtxInAttrs;
  std::string vtxXfrAttrs;
  {
    size_t idx = 0;
    if (flags & metaforce::EStreamFlagBits::fHasNormal) {
      vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) nrm: vec3<f32>;"), idx);
      vtxInAttrs += fmt::format(FMT_STRING("\n    , @location({}) in_nrm: vec3<f32>"), ++idx);
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.nrm = in_nrm;"));
    }
    if (flags & metaforce::EStreamFlagBits::fHasColor) {
      vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) clr: vec4<f32>;"), idx);
      vtxInAttrs += fmt::format(FMT_STRING("\n    , @location({}) in_clr: vec4<f32>"), ++idx);
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.clr = in_clr;"));
    }
    if (flags & metaforce::EStreamFlagBits::fHasTexture) {
      vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) tex0_uv: vec2<f32>;"), idx);
      vtxInAttrs += fmt::format(FMT_STRING("\n    , @location({}) in_uv: vec2<f32>"), ++idx);
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.tex0_uv = in_uv;"));
    }
  }
  std::string fragmentFn;
  bool hasRast = false;
  for (size_t idx = 0; const auto& stage : g_tevStages) {
    if (!stage) {
      idx++;
      continue;
    }
    if (stage->colorPass.x0_a == GX::TevColorArg::CC_TEXC || stage->colorPass.x4_b == GX::TevColorArg::CC_TEXC ||
        stage->colorPass.x8_c == GX::TevColorArg::CC_TEXC || stage->colorPass.xc_d == GX::TevColorArg::CC_TEXC ||
        stage->alphaPass.x0_a == GX::TevAlphaArg::CA_TEXA || stage->alphaPass.x4_b == GX::TevAlphaArg::CA_TEXA ||
        stage->alphaPass.x8_c == GX::TevAlphaArg::CA_TEXA || stage->alphaPass.xc_d == GX::TevAlphaArg::CA_TEXA) {
      fragmentFn += fmt::format(
          FMT_STRING("\n    var sampled{0} = textureSampleBias(tex{0}, tex{0}_samp, in.tex{0}_uv, ubuf.tex{0}_lod);"),
          idx);
    }
    if (!hasRast) {
      if (stage->colorPass.x0_a == GX::TevColorArg::CC_RASC || stage->colorPass.x4_b == GX::TevColorArg::CC_RASC ||
          stage->colorPass.x8_c == GX::TevColorArg::CC_RASC || stage->colorPass.xc_d == GX::TevColorArg::CC_RASC ||
          stage->alphaPass.x0_a == GX::TevAlphaArg::CA_RASA || stage->alphaPass.x4_b == GX::TevAlphaArg::CA_RASA ||
          stage->alphaPass.x8_c == GX::TevAlphaArg::CA_RASA || stage->alphaPass.xc_d == GX::TevAlphaArg::CA_RASA) {
        fragmentFn += fmt::format(FMT_STRING("\n    var rast = in.clr; // TODO lighting")); // TODO lighting
        hasRast = true;
      }
    }
    idx++;
  }
  for (size_t idx = 0; const auto& stage : g_tevStages) {
    if (!stage) {
      idx++;
      continue;
    }
    {
      std::string op;
      std::string outReg;
      switch (stage->colorOp.x10_regId) {
      case GX::TevRegID::TEVPREV:
        outReg = "prev";
        break;
      default:
        Log.report(logvisor::Fatal, FMT_STRING("TODO: colorOp outReg {}"),
                   magic_enum::enum_name(stage->colorOp.x10_regId));
      }
      op = fmt::format(FMT_STRING("({3} {4} ((1.0 - {2}) * {0} + {2} * {1}){5}){6}"),
                       color_arg_reg(stage->colorPass.x0_a, idx), color_arg_reg(stage->colorPass.x4_b, idx),
                       color_arg_reg(stage->colorPass.x8_c, idx), color_arg_reg(stage->colorPass.xc_d, idx),
                       tev_op(stage->colorOp.x4_op), tev_bias(stage->colorOp.x8_bias),
                       tev_scale(stage->colorOp.xc_scale));
      fragmentFn += fmt::format(FMT_STRING("\n    {0} = vec4<f32>({1}, {0}.a);"), outReg, op);
    }
    {
      std::string op;
      std::string outReg;
      switch (stage->alphaOp.x10_regId) {
      case GX::TevRegID::TEVPREV:
        outReg = "prev.a";
        break;
      default:
        Log.report(logvisor::Fatal, FMT_STRING("TODO: alphaOp outReg {}"),
                   magic_enum::enum_name(stage->alphaOp.x10_regId));
      }
      op = fmt::format(FMT_STRING("({3} {4} ((1.0 - {2}) * {0} + {2} * {1}){5}){6}"),
                       alpha_arg_reg(stage->alphaPass.x0_a, idx), alpha_arg_reg(stage->alphaPass.x4_b, idx),
                       alpha_arg_reg(stage->alphaPass.x8_c, idx), alpha_arg_reg(stage->alphaPass.xc_d, idx),
                       tev_op(stage->alphaOp.x4_op), tev_bias(stage->alphaOp.x8_bias),
                       tev_scale(stage->alphaOp.xc_scale));
      fragmentFn += fmt::format(FMT_STRING("\n    {0} = {1};"), outReg, op);
    }
    idx++;
  }

  const auto shaderSource =
      fmt::format(FMT_STRING(R"""(
struct Uniform {{
    xf: mat4x4<f32>;{uniBufAttrs}
}};
@group(0) @binding(0)
var<uniform> ubuf: Uniform;{sampBindings}{texBindings}

struct VertexOutput {{
    @builtin(position) pos: vec4<f32>;{vtxOutAttrs}
}};

@stage(vertex)
fn vs_main(
    @location(0) in_pos: vec3<f32>{vtxInAttrs}
) -> VertexOutput {{
    var out: VertexOutput;
    out.pos = ubuf.xf * vec4<f32>(in_pos, 1.0);{vtxXfrAttrs}
    return out;
}}

@stage(fragment)
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {{
    var prev: vec4<f32>;{fragmentFn}
    return prev;
}}
)"""),
                  "uniBufAttrs"_a = uniBufAttrs, "sampBindings"_a = sampBindings, "texBindings"_a = texBindings,
                  "vtxOutAttrs"_a = vtxOutAttrs, "vtxInAttrs"_a = vtxInAttrs, "vtxXfrAttrs"_a = vtxXfrAttrs,
                  "fragmentFn"_a = fragmentFn);
  Log.report(logvisor::Info, FMT_STRING("Generated shader: {}"), shaderSource);

  wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
  wgslDescriptor.source = shaderSource.c_str();
  const auto shaderDescriptor = wgpu::ShaderModuleDescriptor{
      .nextInChain = &wgslDescriptor,
      .label = "Generated Shader",
  };
  auto shader = gpu::g_device.CreateShaderModule(&shaderDescriptor);
  g_streamCachedShaders.emplace(hash, std::move(shader));

  return hash;
}

void stream_end() noexcept {
  if (sStreamState->flags & metaforce::EStreamFlagBits::fHasTexture && !sTextures[0]) {
    Log.report(logvisor::Fatal, FMT_STRING("Stream has texture but no texture bound!"));
    unreachable();
  }
  const auto vertRange = push_verts(sStreamState->vertexBuffer.data(), sStreamState->vertexBuffer.size());

  ByteBuffer uniBuf;
  std::bitset<g_colorRegs.size()> usedColors;
  {
    const auto xf = get_combined_matrix();
    uniBuf.append(&xf, 64);
  }
  if (sStreamState->flags & metaforce::EStreamFlagBits::fHasTexture) {
    uniBuf.append(&sTextures[0].lod, 4);
  }
  const auto uniRange = push_uniform(uniBuf.data(), uniBuf.size());

  const auto shaderRef = generate_shader();

  const auto uniform_size = align_uniform(uniBuf.size());
  const auto pipeline = pipeline_ref(stream::PipelineConfig{
      .shader = shaderRef,
      .uniformSize = uniform_size,
      .primitive = sStreamState->primitive,
      .flags = sStreamState->flags,
      .depthCompare = g_depthCompare,
      .depthUpdate = g_depthUpdate,
      .depthFunc = g_depthFunc,
      .cullMode = g_cullMode,
      .blendMode = g_blendMode,
      .blendFacSrc = g_blendFacSrc,
      .blendFacDst = g_blendFacDst,
      .blendOp = g_blendOp,
      .dstAlpha = g_dstAlpha,
  });

  BindGroupRef samplerBindGroup{};
  BindGroupRef textureBindGroup{};
  if (sStreamState->flags & metaforce::EStreamFlagBits::fHasTexture) {
    const auto& state = get_state<stream::State>();
    {
      const std::array samplerEntries{wgpu::BindGroupEntry{
          .binding = 0,
          .sampler = sampler_ref(sTextures[0].get_descriptor()),
      }};
      samplerBindGroup = bind_group_ref(wgpu::BindGroupDescriptor{
          .label = "Stream Sampler Bind Group",
          .layout = state.samplerLayout,
          .entryCount = samplerEntries.size(),
          .entries = samplerEntries.data(),
      });
    }
    {
      const std::array textureEntries{wgpu::BindGroupEntry{
          .binding = 0,
          .textureView = sTextures[0].handle.ref->view,
      }};
      textureBindGroup = bind_group_ref(wgpu::BindGroupDescriptor{
          .label = "Stream Texture Bind Group",
          .layout = state.textureLayout,
          .entryCount = textureEntries.size(),
          .entries = textureEntries.data(),
      });
    }
  }

  push_draw_command(stream::DrawData{
      .pipeline = pipeline,
      .vertRange = vertRange,
      .uniformRange = uniRange,
      .vertexCount = sStreamState->vertexCount,
      .uniformSize = uniform_size,
      .samplerBindGroup = samplerBindGroup,
      .textureBindGroup = textureBindGroup,
  });

  sStreamState.reset();
}
} // namespace aurora::gfx
