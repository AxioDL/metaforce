#include "common.hpp"

#include "../gpu.hpp"
#include "gx.hpp"

namespace aurora::gfx {
using namespace fmt::literals;

static logvisor::Module Log("aurora::gfx::gx");

std::unordered_map<ShaderRef, wgpu::ShaderModule> g_gxCachedShaders;

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
    return "0.5";
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

wgpu::ShaderModule build_shader(const GXShaderConfig& config) {
  const auto hash = xxh3_hash(config);
  if (g_gxCachedShaders.contains(hash)) {
    return g_gxCachedShaders[hash];
  }

  std::string uniBufAttrs;
  std::string sampBindings;
  std::string texBindings;
  std::string vtxOutAttrs;
  std::string vtxInAttrs;
  std::string vtxXfrAttrs;
  size_t locIdx = 0;
  if (config.denormalizedNorm) {
    vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) nrm: vec3<f32>;"), locIdx);
    vtxInAttrs += fmt::format(FMT_STRING("\n    , @location({}) in_nrm: vec3<f32>"), ++locIdx);
    vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.nrm = in_nrm;"));
  }
  if (config.denormalizedColor) {
    vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) clr: vec4<f32>;"), locIdx);
    vtxInAttrs += fmt::format(FMT_STRING("\n    , @location({}) in_clr: vec4<f32>"), ++locIdx);
    vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.clr = in_clr;"));
  }
  for (int i = 0; i < maxTextures; ++i) {
    if (!config.boundTextures.test(i)) {
      continue;
    }
    uniBufAttrs += fmt::format(FMT_STRING("\n    tex{}_lod: f32;"), i);
    sampBindings += fmt::format(FMT_STRING("\n@group(1) @binding({0})\n"
                                           "var tex{0}_samp: sampler;"), i);
    texBindings += fmt::format(FMT_STRING("\n@group(2) @binding({0})\n"
                                          "var tex{0}: texture_2d<f32>;"), i);
    vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) tex{}_uv: vec2<f32>;"), locIdx, i);
    vtxInAttrs += fmt::format(FMT_STRING("\n    , @location({}) in_tex{}_uv: vec2<f32>"), locIdx + 1, i);
    vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.tex{0}_uv = in_tex{0}_uv;"), i);
    locIdx++;
  }

  std::string fragmentFn;
  bool hasRast = false;
  for (size_t idx = 0; const auto& stage : config.tevStages) {
    if (!stage) {
      break;
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
  for (size_t idx = 0; const auto& stage : config.tevStages) {
    if (!stage) {
      break;
    }
    {
      std::string op;
      std::string outReg;
      switch (stage->colorOp.x10_regId) {
      case GX::TevRegID::TEVPREV:
        outReg = "prev";
        break;
      default:
        Log.report(logvisor::Fatal, FMT_STRING("TODO: colorOp outReg {}"), stage->colorOp.x10_regId);
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
        Log.report(logvisor::Fatal, FMT_STRING("TODO: alphaOp outReg {}"), stage->alphaOp.x10_regId);
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
      .label = "GX Shader",
  };
  auto shader = gpu::g_device.CreateShaderModule(&shaderDescriptor);
  g_gxCachedShaders.emplace(hash, shader);

  return shader;
}
} // namespace aurora::gfx
