#include "common.hpp"

#include "../gpu.hpp"
#include "gx.hpp"

#include <unordered_map>

namespace aurora::gfx::gx {
using namespace fmt::literals;

static logvisor::Module Log("aurora::gfx::gx");

std::unordered_map<ShaderRef, std::pair<wgpu::ShaderModule, gx::ShaderInfo>> g_gxCachedShaders;

static std::string color_arg_reg(GX::TevColorArg arg, size_t stageIdx, const STevStage& stage, ShaderInfo& info) {
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
  case GX::CC_TEXC: {
    if (stage.texMapId == GX::TEXMAP_NULL) {
      Log.report(logvisor::Fatal, FMT_STRING("unmapped texture for stage {}"), stageIdx);
      unreachable();
    } else if (stage.texMapId < GX::TEXMAP0 || stage.texMapId > GX::TEXMAP7) {
      Log.report(logvisor::Fatal, FMT_STRING("invalid texture {} for stage {}"), stage.texMapId, stageIdx);
      unreachable();
    }
    info.sampledTextures.set(stage.texMapId);
    return fmt::format(FMT_STRING("sampled{}.rgb"), stage.texMapId);
  }
  case GX::CC_TEXA: {
    if (stage.texMapId == GX::TEXMAP_NULL) {
      Log.report(logvisor::Fatal, FMT_STRING("unmapped texture for stage {}"), stageIdx);
      unreachable();
    } else if (stage.texMapId < GX::TEXMAP0 || stage.texMapId > GX::TEXMAP7) {
      Log.report(logvisor::Fatal, FMT_STRING("invalid texture {} for stage {}"), stage.texMapId, stageIdx);
      unreachable();
    }
    info.sampledTextures.set(stage.texMapId);
    return fmt::format(FMT_STRING("sampled{}.a"), stage.texMapId);
  }
  case GX::CC_RASC: {
    if (stage.channelId == GX::COLOR_NULL) {
      Log.report(logvisor::Fatal, FMT_STRING("unmapped color channel for stage {}"), stageIdx);
      unreachable();
    } else if (stage.channelId < GX::COLOR0A0 || stage.channelId > GX::COLOR1A1) {
      Log.report(logvisor::Fatal, FMT_STRING("invalid color channel {} for stage {}"), stage.channelId, stageIdx);
      unreachable();
    }
    u32 idx = stage.channelId - GX::COLOR0A0;
    info.sampledColorChannels.set(idx);
    return fmt::format(FMT_STRING("rast{}.rgb"), idx);
  }
  case GX::CC_RASA: {
    if (stage.channelId == GX::COLOR_NULL) {
      Log.report(logvisor::Fatal, FMT_STRING("unmapped color channel for stage {}"), stageIdx);
      unreachable();
    } else if (stage.channelId < GX::COLOR0A0 || stage.channelId > GX::COLOR1A1) {
      Log.report(logvisor::Fatal, FMT_STRING("invalid color channel {} for stage {}"), stage.channelId, stageIdx);
      unreachable();
    }
    u32 idx = stage.channelId - GX::COLOR0A0;
    info.sampledColorChannels.set(idx);
    return fmt::format(FMT_STRING("rast{}.a"), idx);
  }
  case GX::CC_ONE:
    return "1.0";
  case GX::CC_HALF:
    return "0.5";
  case GX::CC_KONST: {
    switch (stage.kcSel) {
    case GX::TEV_KCSEL_8_8:
      return "1.0";
    case GX::TEV_KCSEL_7_8:
      return "(7.0/8.0)";
    case GX::TEV_KCSEL_6_8:
      return "(6.0/8.0)";
    case GX::TEV_KCSEL_5_8:
      return "(5.0/8.0)";
    case GX::TEV_KCSEL_4_8:
      return "(4.0/8.0)";
    case GX::TEV_KCSEL_3_8:
      return "(3.0/8.0)";
    case GX::TEV_KCSEL_2_8:
      return "(2.0/8.0)";
    case GX::TEV_KCSEL_1_8:
      return "(1.0/8.0)";
    case GX::TEV_KCSEL_K0:
      info.sampledKcolors.set(0);
      return "ubuf.kcolor0.rgb";
    case GX::TEV_KCSEL_K1:
      info.sampledKcolors.set(1);
      return "ubuf.kcolor1.rgb";
    case GX::TEV_KCSEL_K2:
      info.sampledKcolors.set(2);
      return "ubuf.kcolor2.rgb";
    case GX::TEV_KCSEL_K3:
      info.sampledKcolors.set(3);
      return "ubuf.kcolor3.rgb";
    case GX::TEV_KCSEL_K0_R:
      info.sampledKcolors.set(0);
      return "ubuf.kcolor0.r";
    case GX::TEV_KCSEL_K1_R:
      info.sampledKcolors.set(1);
      return "ubuf.kcolor1.r";
    case GX::TEV_KCSEL_K2_R:
      info.sampledKcolors.set(2);
      return "ubuf.kcolor2.r";
    case GX::TEV_KCSEL_K3_R:
      info.sampledKcolors.set(3);
      return "ubuf.kcolor3.r";
    case GX::TEV_KCSEL_K0_G:
      info.sampledKcolors.set(0);
      return "ubuf.kcolor0.g";
    case GX::TEV_KCSEL_K1_G:
      info.sampledKcolors.set(1);
      return "ubuf.kcolor1.g";
    case GX::TEV_KCSEL_K2_G:
      info.sampledKcolors.set(2);
      return "ubuf.kcolor2.g";
    case GX::TEV_KCSEL_K3_G:
      info.sampledKcolors.set(3);
      return "ubuf.kcolor3.g";
    case GX::TEV_KCSEL_K0_B:
      info.sampledKcolors.set(0);
      return "ubuf.kcolor0.b";
    case GX::TEV_KCSEL_K1_B:
      info.sampledKcolors.set(1);
      return "ubuf.kcolor1.b";
    case GX::TEV_KCSEL_K2_B:
      info.sampledKcolors.set(2);
      return "ubuf.kcolor2.b";
    case GX::TEV_KCSEL_K3_B:
      info.sampledKcolors.set(3);
      return "ubuf.kcolor3.b";
    case GX::TEV_KCSEL_K0_A:
      info.sampledKcolors.set(0);
      return "ubuf.kcolor0.a";
    case GX::TEV_KCSEL_K1_A:
      info.sampledKcolors.set(1);
      return "ubuf.kcolor1.a";
    case GX::TEV_KCSEL_K2_A:
      info.sampledKcolors.set(2);
      return "ubuf.kcolor2.a";
    case GX::TEV_KCSEL_K3_A:
      info.sampledKcolors.set(3);
      return "ubuf.kcolor3.a";
    default:
      Log.report(logvisor::Fatal, FMT_STRING("invalid kcSel {}"), stage.kcSel);
      unreachable();
    }
  }
  case GX::CC_ZERO:
    return "0.0";
  }
}

static std::string alpha_arg_reg(GX::TevAlphaArg arg, size_t stageIdx, const STevStage& stage, ShaderInfo& info) {
  switch (arg) {
  case GX::CA_APREV:
    return "prev.a";
  case GX::CA_A0:
  case GX::CA_A1:
  case GX::CA_A2:
    Log.report(logvisor::Fatal, FMT_STRING("TODO {}"), arg);
    unreachable();
  case GX::CA_TEXA: {
    if (stage.texMapId == GX::TEXMAP_NULL) {
      Log.report(logvisor::Fatal, FMT_STRING("unmapped texture for stage {}"), stageIdx);
      unreachable();
    } else if (stage.texMapId < GX::TEXMAP0 || stage.texMapId > GX::TEXMAP7) {
      Log.report(logvisor::Fatal, FMT_STRING("invalid texture {} for stage {}"), stage.texMapId, stageIdx);
      unreachable();
    }
    info.sampledTextures.set(stage.texMapId);
    return fmt::format(FMT_STRING("sampled{}.a"), stage.texMapId);
  }
  case GX::CA_RASA: {
    if (stage.channelId == GX::COLOR_NULL) {
      Log.report(logvisor::Fatal, FMT_STRING("unmapped color channel for stage {}"), stageIdx);
      unreachable();
    } else if (stage.channelId < GX::COLOR0A0 || stage.channelId > GX::COLOR1A1) {
      Log.report(logvisor::Fatal, FMT_STRING("invalid color channel {} for stage {}"), stage.channelId, stageIdx);
      unreachable();
    }
    u32 idx = stage.channelId - GX::COLOR0A0;
    info.sampledColorChannels.set(idx);
    return fmt::format(FMT_STRING("rast{}.a"), idx);
  }
  case GX::CA_KONST: {
    switch (stage.kaSel) {
    case GX::TEV_KASEL_8_8:
      return "1.0";
    case GX::TEV_KASEL_7_8:
      return "(7.0/8.0)";
    case GX::TEV_KASEL_6_8:
      return "(6.0/8.0)";
    case GX::TEV_KASEL_5_8:
      return "(5.0/8.0)";
    case GX::TEV_KASEL_4_8:
      return "(4.0/8.0)";
    case GX::TEV_KASEL_3_8:
      return "(3.0/8.0)";
    case GX::TEV_KASEL_2_8:
      return "(2.0/8.0)";
    case GX::TEV_KASEL_1_8:
      return "(1.0/8.0)";
    case GX::TEV_KASEL_K0_R:
      info.sampledKcolors.set(0);
      return "ubuf.kcolor0.r";
    case GX::TEV_KASEL_K1_R:
      info.sampledKcolors.set(1);
      return "ubuf.kcolor1.r";
    case GX::TEV_KASEL_K2_R:
      info.sampledKcolors.set(2);
      return "ubuf.kcolor2.r";
    case GX::TEV_KASEL_K3_R:
      info.sampledKcolors.set(3);
      return "ubuf.kcolor3.r";
    case GX::TEV_KASEL_K0_G:
      info.sampledKcolors.set(0);
      return "ubuf.kcolor0.g";
    case GX::TEV_KASEL_K1_G:
      info.sampledKcolors.set(1);
      return "ubuf.kcolor1.g";
    case GX::TEV_KASEL_K2_G:
      info.sampledKcolors.set(2);
      return "ubuf.kcolor2.g";
    case GX::TEV_KASEL_K3_G:
      info.sampledKcolors.set(3);
      return "ubuf.kcolor3.g";
    case GX::TEV_KASEL_K0_B:
      info.sampledKcolors.set(0);
      return "ubuf.kcolor0.b";
    case GX::TEV_KASEL_K1_B:
      info.sampledKcolors.set(1);
      return "ubuf.kcolor1.b";
    case GX::TEV_KASEL_K2_B:
      info.sampledKcolors.set(2);
      return "ubuf.kcolor2.b";
    case GX::TEV_KASEL_K3_B:
      info.sampledKcolors.set(3);
      return "ubuf.kcolor3.b";
    case GX::TEV_KASEL_K0_A:
      info.sampledKcolors.set(0);
      return "ubuf.kcolor0.a";
    case GX::TEV_KASEL_K1_A:
      info.sampledKcolors.set(1);
      return "ubuf.kcolor1.a";
    case GX::TEV_KASEL_K2_A:
      info.sampledKcolors.set(2);
      return "ubuf.kcolor2.a";
    case GX::TEV_KASEL_K3_A:
      info.sampledKcolors.set(3);
      return "ubuf.kcolor3.a";
    default:
      Log.report(logvisor::Fatal, FMT_STRING("invalid kaSel {}"), stage.kaSel);
      unreachable();
    }
  }
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

std::pair<wgpu::ShaderModule, ShaderInfo> build_shader(const ShaderConfig& config) noexcept {
  const auto hash = xxh3_hash(config);
  if (g_gxCachedShaders.contains(hash)) {
    return g_gxCachedShaders[hash];
  }

  Log.report(logvisor::Info, FMT_STRING("Shader config (hash {:x}):"), hash);
  {
    for (int i = 0; i < config.tevStages.size(); ++i) {
      const auto& stage = config.tevStages[i];
      if (!stage) {
        break;
      }
      Log.report(logvisor::Info, FMT_STRING("  tevStages[{}]:"), i);
      Log.report(logvisor::Info, FMT_STRING("    color_a: {}"), stage->colorPass.x0_a);
      Log.report(logvisor::Info, FMT_STRING("    color_b: {}"), stage->colorPass.x4_b);
      Log.report(logvisor::Info, FMT_STRING("    color_c: {}"), stage->colorPass.x8_c);
      Log.report(logvisor::Info, FMT_STRING("    color_d: {}"), stage->colorPass.xc_d);
      Log.report(logvisor::Info, FMT_STRING("    alpha_a: {}"), stage->alphaPass.x0_a);
      Log.report(logvisor::Info, FMT_STRING("    alpha_b: {}"), stage->alphaPass.x4_b);
      Log.report(logvisor::Info, FMT_STRING("    alpha_c: {}"), stage->alphaPass.x8_c);
      Log.report(logvisor::Info, FMT_STRING("    alpha_d: {}"), stage->alphaPass.xc_d);
      Log.report(logvisor::Info, FMT_STRING("    color_op_clamp: {}"), stage->colorOp.x0_clamp);
      Log.report(logvisor::Info, FMT_STRING("    color_op_op: {}"), stage->colorOp.x4_op);
      Log.report(logvisor::Info, FMT_STRING("    color_op_bias: {}"), stage->colorOp.x8_bias);
      Log.report(logvisor::Info, FMT_STRING("    color_op_scale: {}"), stage->colorOp.xc_scale);
      Log.report(logvisor::Info, FMT_STRING("    color_op_reg_id: {}"), stage->colorOp.x10_regId);
      Log.report(logvisor::Info, FMT_STRING("    alpha_op_clamp: {}"), stage->alphaOp.x0_clamp);
      Log.report(logvisor::Info, FMT_STRING("    alpha_op_op: {}"), stage->alphaOp.x4_op);
      Log.report(logvisor::Info, FMT_STRING("    alpha_op_bias: {}"), stage->alphaOp.x8_bias);
      Log.report(logvisor::Info, FMT_STRING("    alpha_op_scale: {}"), stage->alphaOp.xc_scale);
      Log.report(logvisor::Info, FMT_STRING("    alpha_op_reg_id: {}"), stage->alphaOp.x10_regId);
      Log.report(logvisor::Info, FMT_STRING("    kc_sel: {}"), stage->kcSel);
      Log.report(logvisor::Info, FMT_STRING("    ka_sel: {}"), stage->kaSel);
      Log.report(logvisor::Info, FMT_STRING("    texCoordId: {}"), stage->texCoordId);
      Log.report(logvisor::Info, FMT_STRING("    texMapId: {}"), stage->texMapId);
      Log.report(logvisor::Info, FMT_STRING("    channelId: {}"), stage->channelId);
    }
    for (int i = 0; i < config.channelMatSrcs.size(); ++i) {
      Log.report(logvisor::Info, FMT_STRING("  channelMatSrcs[{}]: {}"), i, config.channelMatSrcs[i]);
    }
    Log.report(logvisor::Info, FMT_STRING("  alphaDiscard: {}"), config.alphaDiscard);
    Log.report(logvisor::Info, FMT_STRING("  denormalizedVertexAttributes: {}"), config.denormalizedVertexAttributes);
  }

  std::string uniBufAttrs;
  std::string sampBindings;
  std::string texBindings;
  std::string vtxOutAttrs;
  std::string vtxInAttrs;
  std::string vtxXfrAttrs;
  size_t locIdx = 0;
  // TODO
  //  if (config.denormalizedNorm) {
  //    vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) nrm: vec3<f32>;"), locIdx);
  //    vtxInAttrs += fmt::format(FMT_STRING("\n    , @location({}) in_nrm: vec3<f32>"), ++locIdx);
  //    vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.nrm = in_nrm;"));
  //  }

  ShaderInfo info{
      .uniformSize = 64, // MVP MTX
  };
  std::string fragmentFnPre;
  std::string fragmentFn;
  for (size_t idx = 0; const auto& stage : config.tevStages) {
    if (!stage) {
      break;
    }
    {
      std::string outReg;
      switch (stage->colorOp.x10_regId) {
      case GX::TevRegID::TEVPREV:
        outReg = "prev";
        break;
      default:
        Log.report(logvisor::Fatal, FMT_STRING("TODO: colorOp outReg {}"), stage->colorOp.x10_regId);
      }
      std::string op =
          fmt::format(FMT_STRING("({3} {4} ((1.0 - {2}) * {0} + {2} * {1}){5}){6}"),
                      color_arg_reg(stage->colorPass.x0_a, idx, *stage, info),
                      color_arg_reg(stage->colorPass.x4_b, idx, *stage, info),
                      color_arg_reg(stage->colorPass.x8_c, idx, *stage, info),
                      color_arg_reg(stage->colorPass.xc_d, idx, *stage, info), tev_op(stage->colorOp.x4_op),
                      tev_bias(stage->colorOp.x8_bias), tev_scale(stage->colorOp.xc_scale));
      fragmentFn += fmt::format(FMT_STRING("\n    {0} = vec4<f32>({1}, {0}.a);"), outReg, op);
    }
    {
      std::string outReg;
      switch (stage->alphaOp.x10_regId) {
      case GX::TevRegID::TEVPREV:
        outReg = "prev.a";
        break;
      default:
        Log.report(logvisor::Fatal, FMT_STRING("TODO: alphaOp outReg {}"), stage->alphaOp.x10_regId);
      }
      std::string op =
          fmt::format(FMT_STRING("({3} {4} ((1.0 - {2}) * {0} + {2} * {1}){5}){6}"),
                      alpha_arg_reg(stage->alphaPass.x0_a, idx, *stage, info),
                      alpha_arg_reg(stage->alphaPass.x4_b, idx, *stage, info),
                      alpha_arg_reg(stage->alphaPass.x8_c, idx, *stage, info),
                      alpha_arg_reg(stage->alphaPass.xc_d, idx, *stage, info), tev_op(stage->alphaOp.x4_op),
                      tev_bias(stage->alphaOp.x8_bias), tev_scale(stage->alphaOp.xc_scale));
      fragmentFn += fmt::format(FMT_STRING("\n    {0} = {1};"), outReg, op);
    }
    idx++;
  }
  for (int i = 0; i < info.sampledColorChannels.size(); ++i) {
    if (!info.sampledColorChannels.test(i)) {
      continue;
    }

    uniBufAttrs += fmt::format(FMT_STRING("\n    cc{0}_amb: vec4<f32>;"), i);
    uniBufAttrs += fmt::format(FMT_STRING("\n    cc{0}_mat: vec4<f32>;"), i); // TODO not needed for SRC_VTX
    info.uniformSize += 32;

    if (config.channelMatSrcs[i] == GX::SRC_VTX) {
      if (config.denormalizedVertexAttributes) {
        if (!info.usesVtxColor) {
          vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) clr: vec4<f32>;"), locIdx);
          vtxInAttrs += fmt::format(FMT_STRING("\n    , @location({}) in_clr: vec4<f32>"), ++locIdx);
          vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.clr = in_clr;"));
        }
        fragmentFnPre += fmt::format(FMT_STRING("\n    var rast{} = in.clr; // TODO lighting"), i);
      } else {
        Log.report(logvisor::Fatal, FMT_STRING("Don't know how to do this yet")); // TODO
      }
      info.usesVtxColor = true;
    } else {
      fragmentFnPre += fmt::format(FMT_STRING("\n    var rast{0} = ubuf.cc{0}_mat; // TODO lighting"), i);
    }
  }
  for (int i = 0; i < info.sampledKcolors.size(); ++i) {
    if (!info.sampledKcolors.test(i)) {
      continue;
    }
    uniBufAttrs += fmt::format(FMT_STRING("\n    kcolor{}: vec4<f32>;"), i);
    info.uniformSize += 16;
  }
  for (int i = 0; i < info.sampledTextures.size(); ++i) {
    if (!info.sampledTextures.test(i)) {
      continue;
    }
    uniBufAttrs += fmt::format(FMT_STRING("\n    tex{}_lod: f32;"), i);
    info.uniformSize += 4;

    sampBindings += fmt::format(FMT_STRING("\n@group(1) @binding({0})\n"
                                           "var tex{0}_samp: sampler;"),
                                i);
    texBindings += fmt::format(FMT_STRING("\n@group(2) @binding({0})\n"
                                          "var tex{0}: texture_2d<f32>;"),
                               i);

    if (config.denormalizedVertexAttributes) {
      vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) tex{}_uv: vec2<f32>;"), locIdx, i);
      vtxInAttrs += fmt::format(FMT_STRING("\n    , @location({}) in_tex{}_uv: vec2<f32>"), locIdx + 1, i);
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.tex{0}_uv = in_tex{0}_uv;"), i);
    } else {
      Log.report(logvisor::Fatal, FMT_STRING("Don't know how to do this yet")); // TODO
    }
    fragmentFnPre += fmt::format(
        FMT_STRING("\n    var sampled{0} = textureSampleBias(tex{0}, tex{0}_samp, in.tex{0}_uv, ubuf.tex{0}_lod);"), i);
    locIdx++;
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
    var prev: vec4<f32>;{fragmentFnPre}{fragmentFn}
    return prev;
}}
)"""),
                  "uniBufAttrs"_a = uniBufAttrs, "sampBindings"_a = sampBindings, "texBindings"_a = texBindings,
                  "vtxOutAttrs"_a = vtxOutAttrs, "vtxInAttrs"_a = vtxInAttrs, "vtxXfrAttrs"_a = vtxXfrAttrs,
                  "fragmentFn"_a = fragmentFn, "fragmentFnPre"_a = fragmentFnPre);
  Log.report(logvisor::Info, FMT_STRING("Generated shader: {}"), shaderSource);

  wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
  wgslDescriptor.source = shaderSource.c_str();
  const auto shaderDescriptor = wgpu::ShaderModuleDescriptor{
      .nextInChain = &wgslDescriptor,
      .label = "GX Shader",
  };
  auto shader = gpu::g_device.CreateShaderModule(&shaderDescriptor);

  info.uniformSize = align_uniform(info.uniformSize);
  auto pair = std::make_pair(std::move(shader), info);
  g_gxCachedShaders.emplace(hash, pair);

  return pair;
}
} // namespace aurora::gfx::gx