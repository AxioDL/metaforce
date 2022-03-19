#include "common.hpp"

#include "../gpu.hpp"
#include "gx.hpp"

#include <absl/container/flat_hash_map.h>

namespace aurora::gfx::gx {
using namespace fmt::literals;

static logvisor::Module Log("aurora::gfx::gx");

absl::flat_hash_map<ShaderRef, std::pair<wgpu::ShaderModule, gx::ShaderInfo>> g_gxCachedShaders;
#ifndef NDEBUG
static absl::flat_hash_map<ShaderRef, gx::ShaderConfig> g_gxCachedShaderConfigs;
#endif

static inline std::string_view chan_comp(GX::TevColorChan chan) noexcept {
  switch (chan) {
  case GX::CH_RED:
    return "r";
  case GX::CH_GREEN:
    return "g";
  case GX::CH_BLUE:
    return "b";
  case GX::CH_ALPHA:
    return "a";
  }
}

static std::string color_arg_reg(GX::TevColorArg arg, size_t stageIdx, const ShaderConfig& config,
                                 const TevStage& stage, ShaderInfo& info) {
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
    const auto swap = config.tevSwapTable[stage.tevSwapTex];
    return fmt::format(FMT_STRING("sampled{}.{}{}{}"), stage.texMapId, chan_comp(swap.red), chan_comp(swap.green),
                       chan_comp(swap.blue));
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
    const auto swap = config.tevSwapTable[stage.tevSwapTex];
    return fmt::format(FMT_STRING("sampled{}.{}"), stage.texMapId, chan_comp(swap.alpha));
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
    const auto swap = config.tevSwapTable[stage.tevSwapRas];
    return fmt::format(FMT_STRING("rast{}.{}{}{}"), idx, chan_comp(swap.red), chan_comp(swap.green),
                       chan_comp(swap.blue));
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
    const auto swap = config.tevSwapTable[stage.tevSwapRas];
    return fmt::format(FMT_STRING("rast{}.{}"), idx, chan_comp(swap.alpha));
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
      info.sampledKColors.set(0);
      return "ubuf.kcolor0.rgb";
    case GX::TEV_KCSEL_K1:
      info.sampledKColors.set(1);
      return "ubuf.kcolor1.rgb";
    case GX::TEV_KCSEL_K2:
      info.sampledKColors.set(2);
      return "ubuf.kcolor2.rgb";
    case GX::TEV_KCSEL_K3:
      info.sampledKColors.set(3);
      return "ubuf.kcolor3.rgb";
    case GX::TEV_KCSEL_K0_R:
      info.sampledKColors.set(0);
      return "ubuf.kcolor0.r";
    case GX::TEV_KCSEL_K1_R:
      info.sampledKColors.set(1);
      return "ubuf.kcolor1.r";
    case GX::TEV_KCSEL_K2_R:
      info.sampledKColors.set(2);
      return "ubuf.kcolor2.r";
    case GX::TEV_KCSEL_K3_R:
      info.sampledKColors.set(3);
      return "ubuf.kcolor3.r";
    case GX::TEV_KCSEL_K0_G:
      info.sampledKColors.set(0);
      return "ubuf.kcolor0.g";
    case GX::TEV_KCSEL_K1_G:
      info.sampledKColors.set(1);
      return "ubuf.kcolor1.g";
    case GX::TEV_KCSEL_K2_G:
      info.sampledKColors.set(2);
      return "ubuf.kcolor2.g";
    case GX::TEV_KCSEL_K3_G:
      info.sampledKColors.set(3);
      return "ubuf.kcolor3.g";
    case GX::TEV_KCSEL_K0_B:
      info.sampledKColors.set(0);
      return "ubuf.kcolor0.b";
    case GX::TEV_KCSEL_K1_B:
      info.sampledKColors.set(1);
      return "ubuf.kcolor1.b";
    case GX::TEV_KCSEL_K2_B:
      info.sampledKColors.set(2);
      return "ubuf.kcolor2.b";
    case GX::TEV_KCSEL_K3_B:
      info.sampledKColors.set(3);
      return "ubuf.kcolor3.b";
    case GX::TEV_KCSEL_K0_A:
      info.sampledKColors.set(0);
      return "ubuf.kcolor0.a";
    case GX::TEV_KCSEL_K1_A:
      info.sampledKColors.set(1);
      return "ubuf.kcolor1.a";
    case GX::TEV_KCSEL_K2_A:
      info.sampledKColors.set(2);
      return "ubuf.kcolor2.a";
    case GX::TEV_KCSEL_K3_A:
      info.sampledKColors.set(3);
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

static std::string alpha_arg_reg(GX::TevAlphaArg arg, size_t stageIdx, const ShaderConfig& config,
                                 const TevStage& stage, ShaderInfo& info) {
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
    const auto swap = config.tevSwapTable[stage.tevSwapTex];
    return fmt::format(FMT_STRING("sampled{}.{}"), stage.texMapId, chan_comp(swap.alpha));
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
    const auto swap = config.tevSwapTable[stage.tevSwapRas];
    return fmt::format(FMT_STRING("rast{}.{}"), idx, chan_comp(swap.alpha));
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
      info.sampledKColors.set(0);
      return "ubuf.kcolor0.r";
    case GX::TEV_KASEL_K1_R:
      info.sampledKColors.set(1);
      return "ubuf.kcolor1.r";
    case GX::TEV_KASEL_K2_R:
      info.sampledKColors.set(2);
      return "ubuf.kcolor2.r";
    case GX::TEV_KASEL_K3_R:
      info.sampledKColors.set(3);
      return "ubuf.kcolor3.r";
    case GX::TEV_KASEL_K0_G:
      info.sampledKColors.set(0);
      return "ubuf.kcolor0.g";
    case GX::TEV_KASEL_K1_G:
      info.sampledKColors.set(1);
      return "ubuf.kcolor1.g";
    case GX::TEV_KASEL_K2_G:
      info.sampledKColors.set(2);
      return "ubuf.kcolor2.g";
    case GX::TEV_KASEL_K3_G:
      info.sampledKColors.set(3);
      return "ubuf.kcolor3.g";
    case GX::TEV_KASEL_K0_B:
      info.sampledKColors.set(0);
      return "ubuf.kcolor0.b";
    case GX::TEV_KASEL_K1_B:
      info.sampledKColors.set(1);
      return "ubuf.kcolor1.b";
    case GX::TEV_KASEL_K2_B:
      info.sampledKColors.set(2);
      return "ubuf.kcolor2.b";
    case GX::TEV_KASEL_K3_B:
      info.sampledKColors.set(3);
      return "ubuf.kcolor3.b";
    case GX::TEV_KASEL_K0_A:
      info.sampledKColors.set(0);
      return "ubuf.kcolor0.a";
    case GX::TEV_KASEL_K1_A:
      info.sampledKColors.set(1);
      return "ubuf.kcolor1.a";
    case GX::TEV_KASEL_K2_A:
      info.sampledKColors.set(2);
      return "ubuf.kcolor2.a";
    case GX::TEV_KASEL_K3_A:
      info.sampledKColors.set(3);
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

static std::string alpha_compare(GX::Compare comp, float ref, bool& valid) {
  switch (comp) {
  case GX::NEVER:
    return "false";
  case GX::LESS:
    return fmt::format(FMT_STRING("(prev.a < {}f)"), ref);
  case GX::LEQUAL:
    return fmt::format(FMT_STRING("(prev.a <= {}f)"), ref);
  case GX::EQUAL:
    return fmt::format(FMT_STRING("(prev.a == {}f)"), ref);
  case GX::NEQUAL:
    return fmt::format(FMT_STRING("(prev.a != {}f)"), ref);
  case GX::GEQUAL:
    return fmt::format(FMT_STRING("(prev.a >= {}f)"), ref);
  case GX::GREATER:
    return fmt::format(FMT_STRING("(prev.a < {}f)"), ref);
  case GX::ALWAYS:
    valid = false;
    return "true";
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid compare {}"), comp);
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

static inline std::string vtx_attr(const ShaderConfig& config, GX::Attr attr) {
  const auto type = config.vtxAttrs[attr];
  if (type == GX::NONE) {
    if (attr == GX::VA_NRM) {
      // Default normal
      return "vec3<f32>(1.0, 0.0, 0.0)";
    }
    Log.report(logvisor::Fatal, FMT_STRING("unmapped attr {}"), attr);
    unreachable();
  }
  if (attr == GX::VA_POS) {
    if (type == GX::DIRECT) {
      return "in_pos";
    }
    return "v_verts.data[in_pos_nrm_idx[0]].xyz";
  }
  if (attr == GX::VA_NRM) {
    if (type == GX::DIRECT) {
      return "in_nrm";
    }
    return "v_norms.data[in_pos_nrm_idx[1]].xyz";
  }
  if (attr == GX::VA_CLR0 || attr == GX::VA_CLR1) {
    const auto idx = attr - GX::VA_CLR0;
    if (type == GX::DIRECT) {
      return fmt::format(FMT_STRING("in_clr{}"), idx);
    }
    Log.report(logvisor::Fatal, FMT_STRING("indexed color unsupported"));
    unreachable();
  }
  if (attr >= GX::VA_TEX0 && attr <= GX::VA_TEX7) {
    const auto idx = attr - GX::VA_TEX0;
    if (type == GX::DIRECT) {
      return fmt::format(FMT_STRING("in_tex{}_uv"), idx);
    }
    if (idx == 0) {
      return "v_packed_uvs.data[in_uv_0_4_idx[0]]";
    }
    if (idx < 4) {
      return fmt::format(FMT_STRING("v_uvs.data[in_uv_0_4_idx[{}]]"), idx);
    }
    return fmt::format(FMT_STRING("v_uvs.data[in_uv_5_7_idx[{}]]"), idx - 4);
  }
  Log.report(logvisor::Fatal, FMT_STRING("unhandled attr {}"), attr);
  unreachable();
}

std::pair<wgpu::ShaderModule, ShaderInfo> build_shader(const ShaderConfig& config) noexcept {
  const auto hash = xxh3_hash(config);
  const auto it = g_gxCachedShaders.find(hash);
  if (it != g_gxCachedShaders.end()) {
#ifndef NDEBUG
    if (g_gxCachedShaderConfigs[hash] != config) {
      Log.report(logvisor::Fatal, FMT_STRING("Shader collision!"));
      unreachable();
    }
#endif
    return it->second;
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
      Log.report(logvisor::Info, FMT_STRING("    color_a: {}"), stage->colorPass.a);
      Log.report(logvisor::Info, FMT_STRING("    color_b: {}"), stage->colorPass.b);
      Log.report(logvisor::Info, FMT_STRING("    color_c: {}"), stage->colorPass.c);
      Log.report(logvisor::Info, FMT_STRING("    color_d: {}"), stage->colorPass.d);
      Log.report(logvisor::Info, FMT_STRING("    alpha_a: {}"), stage->alphaPass.a);
      Log.report(logvisor::Info, FMT_STRING("    alpha_b: {}"), stage->alphaPass.b);
      Log.report(logvisor::Info, FMT_STRING("    alpha_c: {}"), stage->alphaPass.c);
      Log.report(logvisor::Info, FMT_STRING("    alpha_d: {}"), stage->alphaPass.d);
      Log.report(logvisor::Info, FMT_STRING("    color_op_clamp: {}"), stage->colorOp.clamp);
      Log.report(logvisor::Info, FMT_STRING("    color_op_op: {}"), stage->colorOp.op);
      Log.report(logvisor::Info, FMT_STRING("    color_op_bias: {}"), stage->colorOp.bias);
      Log.report(logvisor::Info, FMT_STRING("    color_op_scale: {}"), stage->colorOp.scale);
      Log.report(logvisor::Info, FMT_STRING("    color_op_reg_id: {}"), stage->colorOp.outReg);
      Log.report(logvisor::Info, FMT_STRING("    alpha_op_clamp: {}"), stage->alphaOp.clamp);
      Log.report(logvisor::Info, FMT_STRING("    alpha_op_op: {}"), stage->alphaOp.op);
      Log.report(logvisor::Info, FMT_STRING("    alpha_op_bias: {}"), stage->alphaOp.bias);
      Log.report(logvisor::Info, FMT_STRING("    alpha_op_scale: {}"), stage->alphaOp.scale);
      Log.report(logvisor::Info, FMT_STRING("    alpha_op_reg_id: {}"), stage->alphaOp.outReg);
      Log.report(logvisor::Info, FMT_STRING("    kc_sel: {}"), stage->kcSel);
      Log.report(logvisor::Info, FMT_STRING("    ka_sel: {}"), stage->kaSel);
      Log.report(logvisor::Info, FMT_STRING("    texCoordId: {}"), stage->texCoordId);
      Log.report(logvisor::Info, FMT_STRING("    texMapId: {}"), stage->texMapId);
      Log.report(logvisor::Info, FMT_STRING("    channelId: {}"), stage->channelId);
    }
    for (int i = 0; i < config.colorChannels.size(); ++i) {
      const auto& chan = config.colorChannels[i];
      Log.report(logvisor::Info, FMT_STRING("  colorChannels[{}]: enabled {} mat {} amb {}"), i, chan.lightingEnabled,
                 chan.matSrc, chan.ambSrc);
    }
    for (int i = 0; i < config.tcgs.size(); ++i) {
      const auto& tcg = config.tcgs[i];
      if (tcg.src != GX::MAX_TEXGENSRC) {
        Log.report(logvisor::Info, FMT_STRING("  tcg[{}]: src {} mtx {} post {} type {} norm {}"), i, tcg.src, tcg.mtx,
                   tcg.postMtx, tcg.type, tcg.normalize);
      }
    }
    Log.report(logvisor::Info, FMT_STRING("  alphaCompare: comp0 {} ref0 {} op {} comp1 {} ref1 {}"),
               config.alphaCompare.comp0, config.alphaCompare.ref0, config.alphaCompare.op, config.alphaCompare.comp1,
               config.alphaCompare.ref1);
    Log.report(logvisor::Info, FMT_STRING("  hasIndexedAttributes: {}"), config.hasIndexedAttributes);
    Log.report(logvisor::Info, FMT_STRING("  fogType: {}"), config.fogType);
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
  size_t vtxOutIdx = 0;
  if (config.hasIndexedAttributes) {
    // Display list attributes
    vtxInAttrs +=
        "\n    @location(0) in_pos_nrm_idx: vec2<i32>"
        "\n    , @location(1) in_uv_0_4_idx: vec4<i32>"
        "\n    , @location(2) in_uv_5_7_idx: vec4<i32>";
    locIdx += 3;
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
  }
  for (GX::Attr attr{}; attr < MaxVtxAttr; attr = GX::Attr(attr + 1)) {
    // Direct attributes
    if (config.vtxAttrs[attr] != GX::DIRECT) {
      continue;
    }
    if (locIdx > 0) {
      vtxInAttrs += "\n    , ";
    } else {
      vtxInAttrs += "\n    ";
    }
    if (attr == GX::VA_POS) {
      vtxInAttrs += fmt::format(FMT_STRING("@location({}) in_pos: vec3<f32>"), locIdx++);
    } else if (attr == GX::VA_NRM) {
      vtxInAttrs += fmt::format(FMT_STRING("@location({}) in_nrm: vec3<f32>"), locIdx++);
    } else if (attr == GX::VA_CLR0 || attr == GX::VA_CLR1) {
      vtxInAttrs += fmt::format(FMT_STRING("@location({}) in_clr{}: vec4<f32>"), locIdx++, attr - GX::VA_CLR0);
    } else if (attr >= GX::VA_TEX0 && attr <= GX::VA_TEX7) {
      vtxInAttrs += fmt::format(FMT_STRING("@location({}) in_tex{}_uv: vec2<f32>"), locIdx++, attr - GX::VA_TEX0);
    }
  }
  vtxXfrAttrsPre += fmt::format(FMT_STRING("\n    var obj_pos = vec4<f32>({}, 1.0);"
                                           "\n    var obj_norm = vec4<f32>({}, 0.0);"
                                           "\n    var mv_pos = ubuf.mv * obj_pos;"
                                           "\n    var mv_norm = ubuf.mv_inv * obj_norm;"
                                           "\n    out.pos = ubuf.proj * mv_pos;"),
                                vtx_attr(config, GX::VA_POS), vtx_attr(config, GX::VA_NRM));

  std::string fragmentFnPre;
  std::string fragmentFn;
  for (size_t idx = 0; const auto& stage : config.tevStages) {
    if (!stage) {
      break;
    }
    {
      std::string outReg;
      switch (stage->colorOp.outReg) {
      case GX::TEVPREV:
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
        Log.report(logvisor::Fatal, FMT_STRING("invalid colorOp outReg {}"), stage->colorOp.outReg);
      }
      std::string op =
          fmt::format(FMT_STRING("({3} {4} ((1.0 - {2}) * {0} + {2} * {1}){5}){6}"),
                      color_arg_reg(stage->colorPass.a, idx, config, *stage, info),
                      color_arg_reg(stage->colorPass.b, idx, config, *stage, info),
                      color_arg_reg(stage->colorPass.c, idx, config, *stage, info),
                      color_arg_reg(stage->colorPass.d, idx, config, *stage, info), tev_op(stage->colorOp.op),
                      tev_bias(stage->colorOp.bias), tev_scale(stage->colorOp.scale));
      if (stage->colorOp.clamp) {
        op = fmt::format(FMT_STRING("clamp(vec3<f32>({}), vec3<f32>(0.0), vec3<f32>(1.0))"), op);
      }
      fragmentFn += fmt::format(FMT_STRING("\n    {0} = vec4<f32>({1}, {0}.a);"), outReg, op);
    }
    {
      std::string outReg;
      switch (stage->alphaOp.outReg) {
      case GX::TEVPREV:
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
        Log.report(logvisor::Fatal, FMT_STRING("invalid alphaOp outReg {}"), stage->alphaOp.outReg);
      }
      std::string op =
          fmt::format(FMT_STRING("({3} {4} ((1.0 - {2}) * {0} + {2} * {1}){5}){6}"),
                      alpha_arg_reg(stage->alphaPass.a, idx, config, *stage, info),
                      alpha_arg_reg(stage->alphaPass.b, idx, config, *stage, info),
                      alpha_arg_reg(stage->alphaPass.c, idx, config, *stage, info),
                      alpha_arg_reg(stage->alphaPass.d, idx, config, *stage, info), tev_op(stage->alphaOp.op),
                      tev_bias(stage->alphaOp.bias), tev_scale(stage->alphaOp.scale));
      if (stage->alphaOp.clamp) {
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
  bool addedLightStruct = false;
  for (int i = 0; i < info.sampledColorChannels.size(); ++i) {
    if (!info.sampledColorChannels.test(i)) {
      continue;
    }

    uniBufAttrs += fmt::format(FMT_STRING("\n    cc{0}_amb: vec4<f32>;"), i);
    uniBufAttrs += fmt::format(FMT_STRING("\n    cc{0}_mat: vec4<f32>;"), i);
    info.uniformSize += 32;

    if (config.colorChannels[i].lightingEnabled) {
      if (!addedLightStruct) {
        uniformPre +=
            "\n"
            "struct Light {\n"
            "    pos: vec3<f32>;\n"
            "    dir: vec3<f32>;\n"
            "    color: vec4<f32>;\n"
            "    lin_att: vec3<f32>;\n"
            "    ang_att: vec3<f32>;\n"
            "};";
        addedLightStruct = true;
      }

      uniBufAttrs += fmt::format(FMT_STRING("\n    lights{}: array<Light, {}>;"), i, GX::MaxLights);
      uniBufAttrs += fmt::format(FMT_STRING("\n    lighting_ambient{}: vec4<f32>;"), i);
      info.uniformSize += (80 * GX::MaxLights) + 16;

      vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) cc{}: vec4<f32>;"), vtxOutIdx++, i);
      vtxXfrAttrs += fmt::format(FMT_STRING(R"""(
    {{
      var lighting = ubuf.lighting_ambient{0} + ubuf.cc{0}_amb;
      for (var i = 0; i < {1}; i = i + 1) {{
          var light = ubuf.lights{0}[i];
          var delta = mv_pos.xyz - light.pos;
          var dist = length(delta);
          var delta_norm = delta / dist;
          var ang_dot = max(dot(delta_norm, light.dir), 0.0);
          var att = 1.0 / (light.lin_att.z * dist * dist +
                           light.lin_att.y * dist +
                           light.lin_att.x);
          var ang_att = light.ang_att.z * ang_dot * ang_dot +
                        light.ang_att.y * ang_dot +
                        light.ang_att.x;
          var this_color = light.color.xyz * ang_att * att * max(dot(-delta_norm, mv_norm.xyz), 0.0);
          lighting = lighting + vec4<f32>(this_color, 0.0);
      }}
      out.cc{0} = clamp(lighting, vec4<f32>(0.0), vec4<f32>(1.0));
    }})"""),
                                 i, GX::MaxLights);
      fragmentFnPre += fmt::format(FMT_STRING("\n    var rast{0} = in.cc{0};"), i);
    } else if (config.colorChannels[i].matSrc == GX::SRC_VTX) {
      vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) cc{}: vec4<f32>;"), vtxOutIdx++, i);
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.cc{} = {};"), i, vtx_attr(config, GX::Attr(GX::VA_CLR0 + i)));
      fragmentFnPre += fmt::format(FMT_STRING("\n    var rast{0} = in.cc{0};"), i);
    } else {
      fragmentFnPre += fmt::format(FMT_STRING("\n    var rast{0} = ubuf.cc{0}_mat;"), i);
    }
  }
  for (int i = 0; i < info.sampledKColors.size(); ++i) {
    if (!info.sampledKColors.test(i)) {
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
    const auto& tcg = config.tcgs[i];
    vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) tex{}_uv: vec2<f32>;"), vtxOutIdx++, i);
    if (tcg.src >= GX::TG_TEX0 && tcg.src <= GX::TG_TEX7) {
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    var tc{} = vec4<f32>({}, 0.0, 1.0);"), i,
                                 vtx_attr(config, GX::Attr(GX::VA_TEX0 + (tcg.src - GX::TG_TEX0))));
    } else if (tcg.src == GX::TG_POS) {
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    var tc{} = vec4<f32>(obj_pos.xyz, 1.0);"), i);
    } else if (tcg.src == GX::TG_NRM) {
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    var tc{} = vec4<f32>(obj_norm.xyz, 1.0);"), i);
    } else {
      Log.report(logvisor::Fatal, FMT_STRING("unhandled tcg src {}"), tcg.src);
      unreachable();
    }
    // TODO this all assumes MTX3x4 currently
    if (tcg.mtx == GX::IDENTITY) {
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    var tc{0}_tmp = tc{0}.xyz;"), i);
    } else {
      u32 texMtxIdx = (tcg.mtx - GX::TEXMTX0) / 3;
      info.usesTexMtx.set(texMtxIdx);
      info.texMtxTypes[texMtxIdx] = tcg.type;
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    var tc{0}_tmp = ubuf.texmtx{1} * tc{0};"), i, texMtxIdx);
    }
    if (tcg.normalize) {
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    tc{0}_tmp = normalize(tc{0}_tmp);"), i);
    }
    if (tcg.postMtx == GX::PTIDENTITY) {
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    var tc{0}_proj = tc{0}_tmp;"), i);
    } else {
      u32 postMtxIdx = (tcg.postMtx - GX::PTTEXMTX0) / 3;
      info.usesPTTexMtx.set(postMtxIdx);
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    var tc{0}_proj = ubuf.postmtx{1} * vec4<f32>(tc{0}_tmp.xyz, 1.0);"),
                                 i, postMtxIdx);
    }
    vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.tex{0}_uv = tc{0}_proj.xy;"), i);
    fragmentFnPre += fmt::format(
        FMT_STRING("\n    var sampled{0} = textureSampleBias(tex{0}, tex{0}_samp, in.tex{0}_uv, ubuf.tex{0}_lod);"), i);
  }
  for (int i = 0; i < info.usesTexMtx.size(); ++i) {
    if (!info.usesTexMtx.test(i)) {
      continue;
    }
    switch (info.texMtxTypes[i]) {
    case GX::TG_MTX2x4:
      uniBufAttrs += fmt::format(FMT_STRING("\n    texmtx{}: mat4x2<f32>;"), i);
      info.uniformSize += 32;
      break;
    case GX::TG_MTX3x4:
      uniBufAttrs += fmt::format(FMT_STRING("\n    texmtx{}: mat4x3<f32>;"), i);
      info.uniformSize += 64;
      break;
    default:
      Log.report(logvisor::Fatal, FMT_STRING("unhandled tex mtx type {}"), info.texMtxTypes[i]);
      unreachable();
    }
  }
  for (int i = 0; i < info.usesPTTexMtx.size(); ++i) {
    if (!info.usesPTTexMtx.test(i)) {
      continue;
    }
    uniBufAttrs += fmt::format(FMT_STRING("\n    postmtx{}: mat4x3<f32>;"), i);
    info.uniformSize += 64;
  }
  if (config.fogType != GX::FOG_NONE) {
    info.usesFog = true;

    uniformPre +=
        "\n"
        "struct Fog {\n"
        "    color: vec4<f32>;\n"
        "    a: f32;\n"
        "    b: f32;\n"
        "    c: f32;\n"
        "    pad: f32;\n"
        "}";
    uniBufAttrs += "\n    fog: Fog;";
    info.uniformSize += 32;

    fragmentFn += "\n    var fogF = clamp((ubuf.fog.a / (ubuf.fog.b - in.pos.z)) - ubuf.fog.c, 0.0, 1.0);";
    switch (config.fogType) {
    case GX::FOG_PERSP_LIN:
    case GX::FOG_ORTHO_LIN:
      fragmentFn += "\n    var fogZ = fogF;";
      break;
    case GX::FOG_PERSP_EXP:
    case GX::FOG_ORTHO_EXP:
      fragmentFn += "\n    var fogZ = 1.0 - exp2(-8.0 * fogF);";
      break;
    case GX::FOG_PERSP_EXP2:
    case GX::FOG_ORTHO_EXP2:
      fragmentFn += "\n    var fogZ = 1.0 - exp2(-8.0 * fogF * fogF);";
      break;
    case GX::FOG_PERSP_REVEXP:
    case GX::FOG_ORTHO_REVEXP:
      fragmentFn += "\n    var fogZ = exp2(-8.0 * (1.0 - fogF));";
      break;
    case GX::FOG_PERSP_REVEXP2:
    case GX::FOG_ORTHO_REVEXP2:
      fragmentFn +=
          "\n    fogF = 1.0 - fogF;"
          "\n    var fogZ = exp2(-8.0 * fogF * fogF);";
      break;
    default:
      Log.report(logvisor::Fatal, FMT_STRING("invalid fog type {}"), config.fogType);
      unreachable();
    }
    fragmentFn += "\n    prev = vec4<f32>(mix(prev.rgb, ubuf.fog.color.rgb, clamp(fogZ, 0.0, 1.0)), prev.a);";
  }
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
  }

  if (config.alphaCompare) {
    bool comp0Valid = true;
    bool comp1Valid = true;
    std::string comp0 = alpha_compare(config.alphaCompare.comp0, config.alphaCompare.ref0, comp0Valid);
    std::string comp1 = alpha_compare(config.alphaCompare.comp1, config.alphaCompare.ref1, comp1Valid);
    if (comp0Valid || comp1Valid) {
      switch (config.alphaCompare.op) {
      case GX::AOP_AND:
        fragmentFn += fmt::format(FMT_STRING("\n    if (!({} && {})) {{ discard; }}"), comp0, comp1);
        break;
      case GX::AOP_OR:
        fragmentFn += fmt::format(FMT_STRING("\n    if (!({} || {})) {{ discard; }}"), comp0, comp1);
        break;
      case GX::AOP_XOR:
        fragmentFn += fmt::format(FMT_STRING("\n    if (!({} ^^ {})) {{ discard; }}"), comp0, comp1);
        break;
      case GX::AOP_XNOR:
        fragmentFn += fmt::format(FMT_STRING("\n    if (({} ^^ {})) {{ discard; }}"), comp0, comp1);
        break;
      default:
        Log.report(logvisor::Fatal, FMT_STRING("invalid alpha compare op {}"), config.alphaCompare.op);
        unreachable();
      }
    }
  }
  // if (config.alphaDiscard) {
  //   fragmentFn += fmt::format(FMT_STRING("\n    if (prev.a < {}f) {{ discard; }}"), *config.alphaDiscard);
  // }

  const auto shaderSource =
      fmt::format(FMT_STRING(R"""({uniformPre}
struct Uniform {{
    mv: mat4x4<f32>;
    mv_inv: mat4x4<f32>;
    proj: mat4x4<f32>;{uniBufAttrs}
}};
@group(0) @binding(0)
var<uniform> ubuf: Uniform;{uniformBindings}{sampBindings}{texBindings}

struct VertexOutput {{
    @builtin(position) pos: vec4<f32>;{vtxOutAttrs}
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
#ifndef NDEBUG
  g_gxCachedShaderConfigs.emplace(hash, config);
#endif

  return pair;
}
} // namespace aurora::gfx::gx
