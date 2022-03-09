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
    info.usesTevReg.set(0);
    return "tevreg0.rgb";
  case GX::CC_A0:
    info.usesTevReg.set(0);
    return "tevreg0.a";
  case GX::CC_C1:
    info.usesTevReg.set(1);
    return "tevreg1.rgb";
  case GX::CC_A1:
    info.usesTevReg.set(1);
    return "tevreg1.a";
  case GX::CC_C2:
    info.usesTevReg.set(2);
    return "tevreg2.rgb";
  case GX::CC_A2:
    info.usesTevReg.set(2);
    return "tevreg2.a";
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
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid color arg {}"), arg);
    unreachable();
  }
}

static std::string alpha_arg_reg(GX::TevAlphaArg arg, size_t stageIdx, const STevStage& stage, ShaderInfo& info) {
  switch (arg) {
  case GX::CA_APREV:
    return "prev.a";
  case GX::CA_A0:
    info.usesTevReg.set(0);
    return "tevreg0.a";
  case GX::CA_A1:
    info.usesTevReg.set(1);
    return "tevreg1.a";
  case GX::CA_A2:
    info.usesTevReg.set(2);
    return "tevreg2.a";
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
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid alpha arg {}"), arg);
    unreachable();
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
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid bias {}"), bias);
    unreachable();
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
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid scale {}"), scale);
    unreachable();
  }
}

std::pair<wgpu::ShaderModule, ShaderInfo> build_shader(const ShaderConfig& config) noexcept {
  const auto hash = xxh3_hash(config);
  if (g_gxCachedShaders.contains(hash)) {
    return g_gxCachedShaders[hash];
  }

  Log.report(logvisor::Info, FMT_STRING("Shader config (hash {:x}):"), hash);
  ShaderInfo info{
      .uniformSize = 64 * 3, // mv, mvInv, proj
  };

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

  std::string uniformPre;
  std::string uniBufAttrs;
  std::string uniformBindings;
  std::string sampBindings;
  std::string texBindings;
  std::string vtxOutAttrs;
  std::string vtxInAttrs;
  std::string vtxXfrAttrsPre;
  std::string vtxXfrAttrs;
  size_t locIdx = 0;
  if (config.denormalizedVertexAttributes) {
    vtxInAttrs += "\n    @location(0) in_pos: vec3<f32>";
    vtxOutAttrs += "\n    @builtin(position) pos: vec4<f32>;";
    vtxXfrAttrsPre += "\n    var obj_pos = vec4<f32>(in_pos, 1.0);"
        "\n    var mv_pos = ubuf.mv * obj_pos;"
        "\n    out.pos = ubuf.proj * mv_pos;";
    if (config.denormalizedHasNrm) {
      vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) nrm: vec3<f32>;"), locIdx);
      vtxInAttrs += fmt::format(FMT_STRING("\n    , @location({}) in_nrm: vec3<f32>"), ++locIdx);
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.nrm = in_nrm;"));
      vtxXfrAttrsPre += "\n    var obj_norm = vec4<f32>(in_nrm, 0.0);"
          "\n    var mv_norm = ubuf.mv_inv * obj_norm;";
      info.usesNormal = true;
    }
  } else {
    uniformBindings += R"""(
struct Vec3Block {
    data: array<vec4<f32>>;
};
struct Vec2Block {
    data: array<vec2<f32>>;
};
@group(0) @binding(1)
var<storage, read> v_verts: Vec3Block;
@group(0) @binding(2)
var<storage, read> v_norms: Vec3Block;
@group(0) @binding(3)
var<storage, read> v_uvs: Vec2Block;
@group(0) @binding(4)
var<storage, read> v_packed_uvs: Vec2Block;
)""";
    vtxInAttrs +=
        "\n    @location(0) in_pos_nrm_idx: vec2<i32>"
        "\n    , @location(1) in_uv_0_4_idx: vec4<i32>"
        "\n    , @location(2) in_uv_5_7_idx: vec4<i32>";
    vtxOutAttrs += "\n    @builtin(position) pos: vec4<f32>;";
    vtxXfrAttrsPre += "\n    var obj_pos = vec4<f32>(v_verts.data[in_pos_nrm_idx[0]].xyz, 1.0);"
        "\n    var obj_norm = vec4<f32>(v_verts.data[in_pos_nrm_idx[1]].xyz, 0.0);"
        "\n    var mv_pos = ubuf.mv * obj_pos;"
        "\n    var mv_norm = ubuf.mv_inv * obj_norm;"
        "\n    out.pos = ubuf.proj * mv_pos;";
  }

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
      case GX::TEVREG0:
        outReg = "tevreg0";
        info.usesTevReg.set(0);
        break;
      case GX::TEVREG1:
        outReg = "tevreg1";
        info.usesTevReg.set(1);
        break;
      case GX::TEVREG2:
        outReg = "tevreg2";
        info.usesTevReg.set(2);
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
      if (stage->colorOp.x0_clamp) {
        op = fmt::format(FMT_STRING("clamp(vec3<f32>({}), vec3<f32>(0.0), vec3<f32>(1.0))"), op);
      }
      fragmentFn += fmt::format(FMT_STRING("\n    {0} = vec4<f32>({1}, {0}.a);"), outReg, op);
    }
    {
      std::string outReg;
      switch (stage->alphaOp.x10_regId) {
      case GX::TevRegID::TEVPREV:
        outReg = "prev.a";
        break;
      case GX::TEVREG0:
        outReg = "tevreg0.a";
        info.usesTevReg.set(0);
        break;
      case GX::TEVREG1:
        outReg = "tevreg1.a";
        info.usesTevReg.set(1);
        break;
      case GX::TEVREG2:
        outReg = "tevreg2.a";
        info.usesTevReg.set(2);
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
      if (stage->alphaOp.x0_clamp) {
        op = fmt::format(FMT_STRING("clamp({}, 0.0, 1.0)"), op);
      }
      fragmentFn += fmt::format(FMT_STRING("\n    {0} = {1};"), outReg, op);
    }
    idx++;
  }
  for (int i = 0; i < info.usesTevReg.size(); ++i) {
    if (!info.usesTevReg.test(i)) {
      continue;
    }
    uniBufAttrs += fmt::format(FMT_STRING("\n    tevreg{}: vec4<f32>;"), i);
    fragmentFnPre += fmt::format(FMT_STRING("\n    var tevreg{0} = ubuf.tevreg{0};"), i);
    info.uniformSize += 16;
  }
  if (info.sampledColorChannels.any()) {
    uniformPre += "\n"
        "struct Light {\n"
        "    pos: vec3<f32>;\n"
        "    dir: vec3<f32>;\n"
        "    color: vec4<f32>;\n"
        "    lin_att: vec3<f32>;\n"
        "    ang_att: vec3<f32>;\n"
        "};";
    uniBufAttrs += fmt::format(FMT_STRING("\n    lights: array<Light, {}>;"), MaxLights);
    uniBufAttrs += "\n    lighting_ambient: vec4<f32>;";
    info.uniformSize += (80 * MaxLights) + 16;
  }
  for (int i = 0; i < info.sampledColorChannels.size(); ++i) {
    if (!info.sampledColorChannels.test(i)) {
      continue;
    }

    uniBufAttrs += fmt::format(FMT_STRING("\n    cc{0}_amb: vec4<f32>;"), i);
    uniBufAttrs += fmt::format(FMT_STRING("\n    cc{0}_mat: vec4<f32>;"), i);
    info.uniformSize += 32;

    vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) cc{}: vec4<f32>;"), locIdx++, i);

    if (config.channelMatSrcs[i] == GX::SRC_VTX) {
      if (config.denormalizedVertexAttributes) {
        if (!info.usesVtxColor) {
          vtxInAttrs += fmt::format(FMT_STRING("\n    , @location({}) in_clr: vec4<f32>"), locIdx);
          vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.cc{} = in_clr;"), i);
        }
        fragmentFnPre += fmt::format(FMT_STRING("\n    var rast{0} = in.cc{0};"), i);
      } else {
        Log.report(logvisor::Fatal, FMT_STRING("SRC_VTX unsupported with normalized vertex attributes"));
      }
      info.usesVtxColor = true;
    } else {
      // TODO only perform lighting on CC0 when enabled
      vtxXfrAttrs += fmt::format(FMT_STRING(R"""(
    {{
      var lighting = ubuf.lighting_ambient + ubuf.cc{0}_amb;
      for (var i = 0; i < {1}; i = i + 1) {{
          var light = ubuf.lights[i];
          var delta = mv_pos.xyz - light.pos;
          var dist = length(delta);
          var delta_norm = delta / dist;
          var ang_dot = max(dot(delta_norm, light.dir), 0.0);
          var lin_att = light.lin_att;
          var att = 1.0 / (lin_att.z * dist * dist * lin_att.y * dist + lin_att.x);
          var ang_att = light.ang_att;
          var ang_att_d = ang_att.z * ang_dot * ang_dot * ang_att.y * ang_dot + ang_att.x;
          var this_color = light.color.xyz * ang_att_d * att * max(dot(-delta_norm, mv_norm.xyz), 0.0);
//          if (i == 0 && c_traits.shader.world_shadow == 1u) {{
//              // TODO ExtTex0 sample
//          }}
          lighting = lighting + vec4<f32>(this_color, 0.0);
      }}
      out.cc{0} = clamp(lighting, vec4<f32>(0.0), vec4<f32>(1.0));
    }})"""), i, MaxLights);
      fragmentFnPre += fmt::format(FMT_STRING("\n    var rast{0} = in.cc{0};"), i);
    }
  }
  for (int i = 0; i < info.sampledKcolors.size(); ++i) {
    if (!info.sampledKcolors.test(i)) {
      continue;
    }
    uniBufAttrs += fmt::format(FMT_STRING("\n    kcolor{}: vec4<f32>;"), i);
    info.uniformSize += 16;
  }
  size_t texBindIdx = 0;
  for (int i = 0; i < info.sampledTextures.size(); ++i) {
    if (!info.sampledTextures.test(i)) {
      continue;
    }
    uniBufAttrs += fmt::format(FMT_STRING("\n    tex{}_lod: f32;"), i);
    info.uniformSize += 4;

    sampBindings += fmt::format(FMT_STRING("\n@group(1) @binding({})\n"
                                           "var tex{}_samp: sampler;"),
                                texBindIdx, i);
    texBindings += fmt::format(FMT_STRING("\n@group(2) @binding({})\n"
                                          "var tex{}: texture_2d<f32>;"),
                               texBindIdx, i);
    ++texBindIdx;

    if (config.denormalizedVertexAttributes) {
      vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) tex{}_uv: vec2<f32>;"), locIdx, i);
      vtxInAttrs += fmt::format(FMT_STRING("\n    , @location({}) in_tex{}_uv: vec2<f32>"), locIdx + 1, i);
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.tex{0}_uv = in_tex{0}_uv;"), i);
    } else {
      vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) tex{}_uv: vec2<f32>;"), locIdx, i);
      if (i < 4) {
        if (i == 0) {
          vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.tex{}_uv = v_packed_uvs.data[in_uv_0_4_idx[{}]];"), i, i);
        } else {
          vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.tex{}_uv = v_uvs.data[in_uv_0_4_idx[{}]];"), i, i);
        }
      } else {
        vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.tex{}_uv = v_uvs.data[in_uv_5_7_idx[{}]];"), i, i - 4);
      }
    }
    fragmentFnPre += fmt::format(
        FMT_STRING("\n    var sampled{0} = textureSampleBias(tex{0}, tex{0}_samp, in.tex{0}_uv, ubuf.tex{0}_lod);"), i);
    locIdx++;
  }

  const auto shaderSource =
      fmt::format(FMT_STRING(R"""({uniformPre}
struct Uniform {{
    mv: mat4x4<f32>;
    mv_inv: mat4x4<f32>;
    proj: mat4x4<f32>;{uniBufAttrs}
}};
@group(0) @binding(0)
var<uniform> ubuf: Uniform;{uniformBindings}{sampBindings}{texBindings}

struct VertexOutput {{{vtxOutAttrs}
}};

@stage(vertex)
fn vs_main({vtxInAttrs}
) -> VertexOutput {{
    var out: VertexOutput;{vtxXfrAttrsPre}{vtxXfrAttrs}
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
                  "fragmentFn"_a = fragmentFn, "fragmentFnPre"_a = fragmentFnPre, "vtxXfrAttrsPre"_a = vtxXfrAttrsPre,
                  "uniformBindings"_a = uniformBindings, "uniformPre"_a = uniformPre);
  Log.report(logvisor::Info, FMT_STRING("Generated shader: {}"), shaderSource);

  wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
  wgslDescriptor.source = shaderSource.c_str();
  const auto label = fmt::format(FMT_STRING("GX Shader {:x}"), hash);
  const auto shaderDescriptor = wgpu::ShaderModuleDescriptor{
      .nextInChain = &wgslDescriptor,
      .label = label.c_str(),
  };
  auto shader = gpu::g_device.CreateShaderModule(&shaderDescriptor);

  info.uniformSize = align_uniform(info.uniformSize);
  auto pair = std::make_pair(std::move(shader), info);
  g_gxCachedShaders.emplace(hash, pair);

  return pair;
}
} // namespace aurora::gfx::gx
