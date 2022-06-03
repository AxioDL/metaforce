#include "common.hpp"

#include "../gpu.hpp"
#include "gx.hpp"

#include <absl/container/flat_hash_map.h>

constexpr bool EnableNormalVisualization = false;
constexpr bool EnableDebugPrints = false;
constexpr bool UsePerPixelLighting = true;

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
  default:
    return "?";
  }
}

static void color_arg_reg_info(GX::TevColorArg arg, const TevStage& stage, ShaderInfo& info) {
  switch (arg) {
  case GX::CC_CPREV:
  case GX::CC_APREV:
    if (!info.writesTevReg.test(GX::TEVPREV)) {
      info.loadsTevReg.set(GX::TEVPREV);
    }
    break;
  case GX::CC_C0:
  case GX::CC_A0:
    if (!info.writesTevReg.test(GX::TEVREG0)) {
      info.loadsTevReg.set(GX::TEVREG0);
    }
    break;
  case GX::CC_C1:
  case GX::CC_A1:
    if (!info.writesTevReg.test(GX::TEVREG1)) {
      info.loadsTevReg.set(GX::TEVREG1);
    }
    break;
  case GX::CC_C2:
  case GX::CC_A2:
    if (!info.writesTevReg.test(GX::TEVREG2)) {
      info.loadsTevReg.set(GX::TEVREG2);
    }
    break;
  case GX::CC_TEXC:
  case GX::CC_TEXA:
    if (stage.texCoordId == GX::TEXCOORD_NULL) {
      Log.report(logvisor::Fatal, FMT_STRING("texCoord not bound"));
    }
    if (stage.texMapId == GX::TEXMAP_NULL) {
      Log.report(logvisor::Fatal, FMT_STRING("texMap not bound"));
    }
    info.sampledTexCoords.set(stage.texCoordId);
    info.sampledTextures.set(stage.texMapId);
    break;
  case GX::CC_RASC:
  case GX::CC_RASA:
    if (stage.channelId >= GX::COLOR0A0 && stage.channelId <= GX::COLOR1A1) {
      info.sampledColorChannels.set(stage.channelId - GX::COLOR0A0);
    }
    break;
  case GX::CC_KONST:
    switch (stage.kcSel) {
    case GX::TEV_KCSEL_K0:
    case GX::TEV_KCSEL_K0_R:
    case GX::TEV_KCSEL_K0_G:
    case GX::TEV_KCSEL_K0_B:
    case GX::TEV_KCSEL_K0_A:
      info.sampledKColors.set(0);
      break;
    case GX::TEV_KCSEL_K1:
    case GX::TEV_KCSEL_K1_R:
    case GX::TEV_KCSEL_K1_G:
    case GX::TEV_KCSEL_K1_B:
    case GX::TEV_KCSEL_K1_A:
      info.sampledKColors.set(1);
      break;
    case GX::TEV_KCSEL_K2:
    case GX::TEV_KCSEL_K2_R:
    case GX::TEV_KCSEL_K2_G:
    case GX::TEV_KCSEL_K2_B:
    case GX::TEV_KCSEL_K2_A:
      info.sampledKColors.set(2);
      break;
    case GX::TEV_KCSEL_K3:
    case GX::TEV_KCSEL_K3_R:
    case GX::TEV_KCSEL_K3_G:
    case GX::TEV_KCSEL_K3_B:
    case GX::TEV_KCSEL_K3_A:
      info.sampledKColors.set(3);
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

static bool formatHasAlpha(GX::TextureFormat format) {
  switch (format) {
  case GX::TF_IA4:
  case GX::TF_IA8:
  case GX::TF_RGB5A3:
  case GX::TF_RGBA8:
  case GX::TF_CMPR:
  case GX::CTF_RA4:
  case GX::CTF_RA8:
  case GX::CTF_YUVA8:
  case GX::CTF_A8:
    return true;
  default:
    return false;
  }
}

static std::string color_arg_reg(GX::TevColorArg arg, size_t stageIdx, const ShaderConfig& config,
                                 const TevStage& stage) {
  switch (arg) {
  case GX::CC_CPREV:
    return "prev.rgb";
  case GX::CC_APREV:
    return "vec3<f32>(prev.a)";
  case GX::CC_C0:
    return "tevreg0.rgb";
  case GX::CC_A0:
    return "vec3<f32>(tevreg0.a)";
  case GX::CC_C1:
    return "tevreg1.rgb";
  case GX::CC_A1:
    return "vec3<f32>(tevreg1.a)";
  case GX::CC_C2:
    return "tevreg2.rgb";
  case GX::CC_A2:
    return "vec3<f32>(tevreg2.a)";
  case GX::CC_TEXC: {
    if (stage.texMapId == GX::TEXMAP_NULL) {
      Log.report(logvisor::Fatal, FMT_STRING("unmapped texture for stage {}"), stageIdx);
      unreachable();
    } else if (stage.texMapId < GX::TEXMAP0 || stage.texMapId > GX::TEXMAP7) {
      Log.report(logvisor::Fatal, FMT_STRING("invalid texture {} for stage {}"), stage.texMapId, stageIdx);
      unreachable();
    }
    const auto swap = config.tevSwapTable[stage.tevSwapTex];
    // TODO check for CH_ALPHA + config.texHasAlpha
    return fmt::format(FMT_STRING("sampled{}.{}{}{}"), stageIdx, chan_comp(swap.red), chan_comp(swap.green),
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
    const auto swap = config.tevSwapTable[stage.tevSwapTex];
    return fmt::format(FMT_STRING("vec3<f32>(sampled{}.{})"), stageIdx, chan_comp(swap.alpha));
  }
  case GX::CC_RASC: {
    if (stage.channelId == GX::COLOR_NULL) {
      Log.report(logvisor::Fatal, FMT_STRING("unmapped color channel for stage {}"), stageIdx);
      unreachable();
    } else if (stage.channelId == GX::COLOR_ZERO) {
      return "vec3<f32>(0.0)";
    } else if (stage.channelId < GX::COLOR0A0 || stage.channelId > GX::COLOR1A1) {
      Log.report(logvisor::Fatal, FMT_STRING("invalid color channel {} for stage {}"), stage.channelId, stageIdx);
      unreachable();
    }
    u32 idx = stage.channelId - GX::COLOR0A0;
    const auto swap = config.tevSwapTable[stage.tevSwapRas];
    return fmt::format(FMT_STRING("rast{}.{}{}{}"), idx, chan_comp(swap.red), chan_comp(swap.green),
                       chan_comp(swap.blue));
  }
  case GX::CC_RASA: {
    if (stage.channelId == GX::COLOR_NULL) {
      Log.report(logvisor::Fatal, FMT_STRING("unmapped color channel for stage {}"), stageIdx);
      unreachable();
    } else if (stage.channelId == GX::COLOR_ZERO) {
      return "vec3<f32>(0.0)";
    } else if (stage.channelId < GX::COLOR0A0 || stage.channelId > GX::COLOR1A1) {
      Log.report(logvisor::Fatal, FMT_STRING("invalid color channel {} for stage {}"), stage.channelId, stageIdx);
      unreachable();
    }
    u32 idx = stage.channelId - GX::COLOR0A0;
    const auto swap = config.tevSwapTable[stage.tevSwapRas];
    return fmt::format(FMT_STRING("vec3<f32>(rast{}.{})"), idx, chan_comp(swap.alpha));
  }
  case GX::CC_ONE:
    return "vec3<f32>(1.0)";
  case GX::CC_HALF:
    return "vec3<f32>(0.5)";
  case GX::CC_KONST: {
    switch (stage.kcSel) {
    case GX::TEV_KCSEL_8_8:
      return "vec3<f32>(1.0)";
    case GX::TEV_KCSEL_7_8:
      return "vec3<f32>(7.0/8.0)";
    case GX::TEV_KCSEL_6_8:
      return "vec3<f32>(6.0/8.0)";
    case GX::TEV_KCSEL_5_8:
      return "vec3<f32>(5.0/8.0)";
    case GX::TEV_KCSEL_4_8:
      return "vec3<f32>(4.0/8.0)";
    case GX::TEV_KCSEL_3_8:
      return "vec3<f32>(3.0/8.0)";
    case GX::TEV_KCSEL_2_8:
      return "vec3<f32>(2.0/8.0)";
    case GX::TEV_KCSEL_1_8:
      return "vec3<f32>(1.0/8.0)";
    case GX::TEV_KCSEL_K0:
      return "ubuf.kcolor0.rgb";
    case GX::TEV_KCSEL_K1:
      return "ubuf.kcolor1.rgb";
    case GX::TEV_KCSEL_K2:
      return "ubuf.kcolor2.rgb";
    case GX::TEV_KCSEL_K3:
      return "ubuf.kcolor3.rgb";
    case GX::TEV_KCSEL_K0_R:
      return "vec3<f32>(ubuf.kcolor0.r)";
    case GX::TEV_KCSEL_K1_R:
      return "vec3<f32>(ubuf.kcolor1.r)";
    case GX::TEV_KCSEL_K2_R:
      return "vec3<f32>(ubuf.kcolor2.r)";
    case GX::TEV_KCSEL_K3_R:
      return "vec3<f32>(ubuf.kcolor3.r)";
    case GX::TEV_KCSEL_K0_G:
      return "vec3<f32>(ubuf.kcolor0.g)";
    case GX::TEV_KCSEL_K1_G:
      return "vec3<f32>(ubuf.kcolor1.g)";
    case GX::TEV_KCSEL_K2_G:
      return "vec3<f32>(ubuf.kcolor2.g)";
    case GX::TEV_KCSEL_K3_G:
      return "vec3<f32>(ubuf.kcolor3.g)";
    case GX::TEV_KCSEL_K0_B:
      return "vec3<f32>(ubuf.kcolor0.b)";
    case GX::TEV_KCSEL_K1_B:
      return "vec3<f32>(ubuf.kcolor1.b)";
    case GX::TEV_KCSEL_K2_B:
      return "vec3<f32>(ubuf.kcolor2.b)";
    case GX::TEV_KCSEL_K3_B:
      return "vec3<f32>(ubuf.kcolor3.b)";
    case GX::TEV_KCSEL_K0_A:
      return "vec3<f32>(ubuf.kcolor0.a)";
    case GX::TEV_KCSEL_K1_A:
      return "vec3<f32>(ubuf.kcolor1.a)";
    case GX::TEV_KCSEL_K2_A:
      return "vec3<f32>(ubuf.kcolor2.a)";
    case GX::TEV_KCSEL_K3_A:
      return "vec3<f32>(ubuf.kcolor3.a)";
    default:
      Log.report(logvisor::Fatal, FMT_STRING("invalid kcSel {}"), stage.kcSel);
      unreachable();
    }
  }
  case GX::CC_ZERO:
    return "vec3<f32>(0.0)";
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid color arg {}"), arg);
    unreachable();
  }
}

static void alpha_arg_reg_info(GX::TevAlphaArg arg, const TevStage& stage, ShaderInfo& info) {
  switch (arg) {
  case GX::CA_APREV:
    if (!info.writesTevReg.test(GX::TEVPREV)) {
      info.loadsTevReg.set(GX::TEVPREV);
    }
    break;
  case GX::CA_A0:
    if (!info.writesTevReg.test(GX::TEVREG0)) {
      info.loadsTevReg.set(GX::TEVREG0);
    }
    break;
  case GX::CA_A1:
    if (!info.writesTevReg.test(GX::TEVREG1)) {
      info.loadsTevReg.set(GX::TEVREG1);
    }
    break;
  case GX::CA_A2:
    if (!info.writesTevReg.test(GX::TEVREG2)) {
      info.loadsTevReg.set(GX::TEVREG2);
    }
    break;
  case GX::CA_TEXA:
    if (stage.texCoordId == GX::TEXCOORD_NULL) {
      Log.report(logvisor::Fatal, FMT_STRING("texCoord not bound"));
    }
    if (stage.texMapId == GX::TEXMAP_NULL) {
      Log.report(logvisor::Fatal, FMT_STRING("texMap not bound"));
    }
    info.sampledTexCoords.set(stage.texCoordId);
    info.sampledTextures.set(stage.texMapId);
    break;
  case GX::CA_RASA:
    if (stage.channelId >= GX::COLOR0A0 && stage.channelId <= GX::COLOR1A1) {
      info.sampledColorChannels.set(stage.channelId - GX::COLOR0A0);
    }
    break;
  case GX::CA_KONST:
    switch (stage.kaSel) {
    case GX::TEV_KASEL_K0_R:
    case GX::TEV_KASEL_K0_G:
    case GX::TEV_KASEL_K0_B:
    case GX::TEV_KASEL_K0_A:
      info.sampledKColors.set(0);
      break;
    case GX::TEV_KASEL_K1_R:
    case GX::TEV_KASEL_K1_G:
    case GX::TEV_KASEL_K1_B:
    case GX::TEV_KASEL_K1_A:
      info.sampledKColors.set(1);
      break;
    case GX::TEV_KASEL_K2_R:
    case GX::TEV_KASEL_K2_G:
    case GX::TEV_KASEL_K2_B:
    case GX::TEV_KASEL_K2_A:
      info.sampledKColors.set(2);
      break;
    case GX::TEV_KASEL_K3_R:
    case GX::TEV_KASEL_K3_G:
    case GX::TEV_KASEL_K3_B:
    case GX::TEV_KASEL_K3_A:
      info.sampledKColors.set(3);
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

static std::string alpha_arg_reg(GX::TevAlphaArg arg, size_t stageIdx, const ShaderConfig& config,
                                 const TevStage& stage) {
  switch (arg) {
  case GX::CA_APREV:
    return "prev.a";
  case GX::CA_A0:
    return "tevreg0.a";
  case GX::CA_A1:
    return "tevreg1.a";
  case GX::CA_A2:
    return "tevreg2.a";
  case GX::CA_TEXA: {
    if (stage.texMapId == GX::TEXMAP_NULL) {
      Log.report(logvisor::Fatal, FMT_STRING("unmapped texture for stage {}"), stageIdx);
      unreachable();
    } else if (stage.texMapId < GX::TEXMAP0 || stage.texMapId > GX::TEXMAP7) {
      Log.report(logvisor::Fatal, FMT_STRING("invalid texture {} for stage {}"), stage.texMapId, stageIdx);
      unreachable();
    }
    const auto swap = config.tevSwapTable[stage.tevSwapTex];
    return fmt::format(FMT_STRING("sampled{}.{}"), stageIdx, chan_comp(swap.alpha));
  }
  case GX::CA_RASA: {
    if (stage.channelId == GX::COLOR_NULL) {
      Log.report(logvisor::Fatal, FMT_STRING("unmapped color channel for stage {}"), stageIdx);
      unreachable();
    } else if (stage.channelId == GX::COLOR_ZERO) {
      return "0.0";
    } else if (stage.channelId < GX::COLOR0A0 || stage.channelId > GX::COLOR1A1) {
      Log.report(logvisor::Fatal, FMT_STRING("invalid color channel {} for stage {}"), stage.channelId, stageIdx);
      unreachable();
    }
    u32 idx = stage.channelId - GX::COLOR0A0;
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
      return "ubuf.kcolor0.r";
    case GX::TEV_KASEL_K1_R:
      return "ubuf.kcolor1.r";
    case GX::TEV_KASEL_K2_R:
      return "ubuf.kcolor2.r";
    case GX::TEV_KASEL_K3_R:
      return "ubuf.kcolor3.r";
    case GX::TEV_KASEL_K0_G:
      return "ubuf.kcolor0.g";
    case GX::TEV_KASEL_K1_G:
      return "ubuf.kcolor1.g";
    case GX::TEV_KASEL_K2_G:
      return "ubuf.kcolor2.g";
    case GX::TEV_KASEL_K3_G:
      return "ubuf.kcolor3.g";
    case GX::TEV_KASEL_K0_B:
      return "ubuf.kcolor0.b";
    case GX::TEV_KASEL_K1_B:
      return "ubuf.kcolor1.b";
    case GX::TEV_KASEL_K2_B:
      return "ubuf.kcolor2.b";
    case GX::TEV_KASEL_K3_B:
      return "ubuf.kcolor3.b";
    case GX::TEV_KASEL_K0_A:
      return "ubuf.kcolor0.a";
    case GX::TEV_KASEL_K1_A:
      return "ubuf.kcolor1.a";
    case GX::TEV_KASEL_K2_A:
      return "ubuf.kcolor2.a";
    case GX::TEV_KASEL_K3_A:
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
    return ""sv;
  case GX::TEV_SUB:
    return "-"sv;
  default:
    Log.report(logvisor::Fatal, FMT_STRING("TODO {}"), op);
    unreachable();
  }
}

static std::string_view tev_bias(GX::TevBias bias) {
  switch (bias) {
  case GX::TB_ZERO:
    return ""sv;
  case GX::TB_ADDHALF:
    return " + 0.5"sv;
  case GX::TB_SUBHALF:
    return " - 0.5"sv;
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid bias {}"), bias);
    unreachable();
  }
}

static std::string alpha_compare(GX::Compare comp, u8 ref, bool& valid) {
  const float fref = ref / 255.f;
  switch (comp) {
  case GX::NEVER:
    return "false"s;
  case GX::LESS:
    return fmt::format(FMT_STRING("(prev.a < {}f)"), fref);
  case GX::LEQUAL:
    return fmt::format(FMT_STRING("(prev.a <= {}f)"), fref);
  case GX::EQUAL:
    return fmt::format(FMT_STRING("(prev.a == {}f)"), fref);
  case GX::NEQUAL:
    return fmt::format(FMT_STRING("(prev.a != {}f)"), fref);
  case GX::GEQUAL:
    return fmt::format(FMT_STRING("(prev.a >= {}f)"), fref);
  case GX::GREATER:
    return fmt::format(FMT_STRING("(prev.a > {}f)"), fref);
  case GX::ALWAYS:
    valid = false;
    return "true"s;
  default:
    Log.report(logvisor::Fatal, FMT_STRING("invalid compare {}"), comp);
    unreachable();
  }
}

static std::string_view tev_scale(GX::TevScale scale) {
  switch (scale) {
  case GX::CS_SCALE_1:
    return ""sv;
  case GX::CS_SCALE_2:
    return " * 2.0"sv;
  case GX::CS_SCALE_4:
    return " * 4.0"sv;
  case GX::CS_DIVIDE_2:
    return " / 2.0"sv;
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
      return "vec3<f32>(1.0, 0.0, 0.0)"s;
    }
    Log.report(logvisor::Fatal, FMT_STRING("unmapped attr {}"), attr);
    unreachable();
  }
  if (attr == GX::VA_POS) {
    return "in_pos"s;
  }
  if (attr == GX::VA_NRM) {
    return "in_nrm"s;
  }
  if (attr == GX::VA_CLR0 || attr == GX::VA_CLR1) {
    const auto idx = attr - GX::VA_CLR0;
    return fmt::format(FMT_STRING("in_clr{}"), idx);
  }
  if (attr >= GX::VA_TEX0 && attr <= GX::VA_TEX7) {
    const auto idx = attr - GX::VA_TEX0;
    return fmt::format(FMT_STRING("in_tex{}_uv"), idx);
  }
  Log.report(logvisor::Fatal, FMT_STRING("unhandled attr {}"), attr);
  unreachable();
}

static inline std::string texture_conversion(const TextureConfig& tex, u32 stageIdx, u32 texMapId) {
  std::string out;
  if (tex.renderTex)
  switch (tex.copyFmt) {
  default:
    break;
  case GX::TF_RGB565:
    // Set alpha channel to 1.0
    out += fmt::format(FMT_STRING("\n    sampled{0}.a = 1.0;"), stageIdx);
    break;
  case GX::TF_I4:
  case GX::TF_I8:
    // FIXME HACK
    if (tex.loadFmt != GX::TF_C4 && tex.loadFmt != GX::TF_C8 && tex.loadFmt != GX::TF_C14X2) {
      // Perform intensity conversion
      out += fmt::format(FMT_STRING("\n    sampled{0} = vec4<f32>(intensityF32(sampled{0}.rgb), 0.f, 0.f, 1.f);"),
                         stageIdx);
    }
    break;
  }
  switch (tex.loadFmt) {
  default:
    break;
  case GX::TF_I4:
  case GX::TF_I8:
    // Splat R to RGBA
    out += fmt::format(FMT_STRING("\n    sampled{0} = vec4<f32>(sampled{0}.r);"), stageIdx);
    break;
  }
  return out;
}

constexpr std::array<std::string_view, GX::TevColorArg::CC_ZERO + 1> TevColorArgNames{
    "CPREV"sv, "APREV"sv, "C0"sv,   "A0"sv,   "C1"sv,  "A1"sv,   "C2"sv,    "A2"sv,
    "TEXC"sv,  "TEXA"sv,  "RASC"sv, "RASA"sv, "ONE"sv, "HALF"sv, "KONST"sv, "ZERO"sv,
};
constexpr std::array<std::string_view, GX::TevAlphaArg::CA_ZERO + 1> TevAlphaArgNames{
    "APREV"sv, "A0"sv, "A1"sv, "A2"sv, "TEXA"sv, "RASA"sv, "KONST"sv, "ZERO"sv,
};

constexpr std::array<std::string_view, MaxVtxAttr> VtxAttributeNames{
    "pn_mtx",        "tex0_mtx",      "tex1_mtx",      "tex2_mtx",    "tex3_mtx", "tex4_mtx", "tex5_mtx",
    "tex6_mtx",      "tex7_mtx",      "pos",           "nrm",         "clr0",     "clr1",     "tex0_uv",
    "tex1_uv",       "tex2_uv",       "tex3_uv",       "tex4_uv",     "tex5_uv",  "tex6_uv",  "tex7_uv",
    "pos_mtx_array", "nrm_mtx_array", "tex_mtx_array", "light_array", "nbt",
};

ShaderInfo build_shader_info(const ShaderConfig& config) noexcept {
  //  const auto hash = xxh3_hash(config);
  //  const auto it = g_gxCachedShaders.find(hash);
  //  if (it != g_gxCachedShaders.end()) {
  //    return it->second.second;
  //  }

  ShaderInfo info{
      .uniformSize = 64 * 3, // mv, mvInv, proj
  };
  for (int i = 0; i < config.tevStageCount; ++i) {
    const auto& stage = config.tevStages[i];
    // Color pass
    color_arg_reg_info(stage.colorPass.a, stage, info);
    color_arg_reg_info(stage.colorPass.b, stage, info);
    color_arg_reg_info(stage.colorPass.c, stage, info);
    color_arg_reg_info(stage.colorPass.d, stage, info);
    info.writesTevReg.set(stage.colorOp.outReg);

    // Alpha pass
    alpha_arg_reg_info(stage.alphaPass.a, stage, info);
    alpha_arg_reg_info(stage.alphaPass.b, stage, info);
    alpha_arg_reg_info(stage.alphaPass.c, stage, info);
    alpha_arg_reg_info(stage.alphaPass.d, stage, info);
    if (!info.writesTevReg.test(stage.alphaOp.outReg)) {
      // If we're writing alpha to a register that's not been
      // written to in the shader, load from uniform buffer
      info.loadsTevReg.set(stage.alphaOp.outReg);
      info.writesTevReg.set(stage.alphaOp.outReg);
    }
  }
  info.uniformSize += info.loadsTevReg.count() * 16;
  for (int i = 0; i < info.sampledColorChannels.size(); ++i) {
    if (info.sampledColorChannels.test(i)) {
      info.uniformSize += 32;
      if (config.colorChannels[i].lightingEnabled) {
        info.uniformSize += 16 + (80 * GX::MaxLights);
      }
    }
  }
  info.uniformSize += info.sampledKColors.count() * 16;
  for (int i = 0; i < info.sampledTextures.size(); ++i) {
    if (!info.sampledTextures.test(i)) {
      continue;
    }
    const auto& tcg = config.tcgs[i];
    if (tcg.mtx != GX::IDENTITY) {
      u32 texMtxIdx = (tcg.mtx - GX::TEXMTX0) / 3;
      info.usesTexMtx.set(texMtxIdx);
      info.texMtxTypes[texMtxIdx] = tcg.type;
    }
    if (tcg.postMtx != GX::PTIDENTITY) {
      u32 postMtxIdx = (tcg.postMtx - GX::PTTEXMTX0) / 3;
      info.usesPTTexMtx.set(postMtxIdx);
    }
  }
  for (int i = 0; i < info.usesTexMtx.size(); ++i) {
    if (info.usesTexMtx.test(i)) {
      switch (info.texMtxTypes[i]) {
      case GX::TG_MTX2x4:
        info.uniformSize += 32;
        break;
      case GX::TG_MTX3x4:
        info.uniformSize += 64;
        break;
      default:
        break;
      }
    }
  }
  info.uniformSize += info.usesPTTexMtx.count() * 64;
  if (config.fogType != GX::FOG_NONE) {
    info.usesFog = true;
    info.uniformSize += 32;
  }
  info.uniformSize += info.sampledTextures.count() * 4;
  info.uniformSize = align_uniform(info.uniformSize);
  return info;
}

wgpu::ShaderModule build_shader(const ShaderConfig& config, const ShaderInfo& info) noexcept {
  const auto hash = xxh3_hash(config);
  const auto it = g_gxCachedShaders.find(hash);
  if (it != g_gxCachedShaders.end()) {
#ifndef NDEBUG
    if (g_gxCachedShaderConfigs[hash] != config) {
      Log.report(logvisor::Fatal, FMT_STRING("Shader collision!"));
      unreachable();
    }
#endif
    return it->second.first;
  }

  if (EnableDebugPrints) {
    Log.report(logvisor::Info, FMT_STRING("Shader config (hash {:x}):"), hash);
    {
      for (int i = 0; i < config.tevStageCount; ++i) {
        const auto& stage = config.tevStages[i];
        Log.report(logvisor::Info, FMT_STRING("  tevStages[{}]:"), i);
        Log.report(logvisor::Info, FMT_STRING("    color_a: {}"), TevColorArgNames[stage.colorPass.a]);
        Log.report(logvisor::Info, FMT_STRING("    color_b: {}"), TevColorArgNames[stage.colorPass.b]);
        Log.report(logvisor::Info, FMT_STRING("    color_c: {}"), TevColorArgNames[stage.colorPass.c]);
        Log.report(logvisor::Info, FMT_STRING("    color_d: {}"), TevColorArgNames[stage.colorPass.d]);
        Log.report(logvisor::Info, FMT_STRING("    alpha_a: {}"), TevAlphaArgNames[stage.alphaPass.a]);
        Log.report(logvisor::Info, FMT_STRING("    alpha_b: {}"), TevAlphaArgNames[stage.alphaPass.b]);
        Log.report(logvisor::Info, FMT_STRING("    alpha_c: {}"), TevAlphaArgNames[stage.alphaPass.c]);
        Log.report(logvisor::Info, FMT_STRING("    alpha_d: {}"), TevAlphaArgNames[stage.alphaPass.d]);
        Log.report(logvisor::Info, FMT_STRING("    color_op_clamp: {}"), stage.colorOp.clamp);
        Log.report(logvisor::Info, FMT_STRING("    color_op_op: {}"), stage.colorOp.op);
        Log.report(logvisor::Info, FMT_STRING("    color_op_bias: {}"), stage.colorOp.bias);
        Log.report(logvisor::Info, FMT_STRING("    color_op_scale: {}"), stage.colorOp.scale);
        Log.report(logvisor::Info, FMT_STRING("    color_op_reg_id: {}"), stage.colorOp.outReg);
        Log.report(logvisor::Info, FMT_STRING("    alpha_op_clamp: {}"), stage.alphaOp.clamp);
        Log.report(logvisor::Info, FMT_STRING("    alpha_op_op: {}"), stage.alphaOp.op);
        Log.report(logvisor::Info, FMT_STRING("    alpha_op_bias: {}"), stage.alphaOp.bias);
        Log.report(logvisor::Info, FMT_STRING("    alpha_op_scale: {}"), stage.alphaOp.scale);
        Log.report(logvisor::Info, FMT_STRING("    alpha_op_reg_id: {}"), stage.alphaOp.outReg);
        Log.report(logvisor::Info, FMT_STRING("    kc_sel: {}"), stage.kcSel);
        Log.report(logvisor::Info, FMT_STRING("    ka_sel: {}"), stage.kaSel);
        Log.report(logvisor::Info, FMT_STRING("    texCoordId: {}"), stage.texCoordId);
        Log.report(logvisor::Info, FMT_STRING("    texMapId: {}"), stage.texMapId);
        Log.report(logvisor::Info, FMT_STRING("    channelId: {}"), stage.channelId);
      }
      for (int i = 0; i < config.colorChannels.size(); ++i) {
        const auto& chan = config.colorChannels[i];
        Log.report(logvisor::Info, FMT_STRING("  colorChannels[{}]: enabled {} mat {} amb {}"), i, chan.lightingEnabled,
                   chan.matSrc, chan.ambSrc);
      }
      for (int i = 0; i < config.tcgs.size(); ++i) {
        const auto& tcg = config.tcgs[i];
        if (tcg.src != GX::MAX_TEXGENSRC) {
          Log.report(logvisor::Info, FMT_STRING("  tcg[{}]: src {} mtx {} post {} type {} norm {}"), i, tcg.src,
                     tcg.mtx, tcg.postMtx, tcg.type, tcg.normalize);
        }
      }
      Log.report(logvisor::Info, FMT_STRING("  alphaCompare: comp0 {} ref0 {} op {} comp1 {} ref1 {}"),
                 config.alphaCompare.comp0, config.alphaCompare.ref0, config.alphaCompare.op, config.alphaCompare.comp1,
                 config.alphaCompare.ref1);
      Log.report(logvisor::Info, FMT_STRING("  indexedAttributeCount: {}"), config.indexedAttributeCount);
      Log.report(logvisor::Info, FMT_STRING("  fogType: {}"), config.fogType);
    }
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
  size_t uniBindingIdx = 1;
  if (config.indexedAttributeCount > 0) {
    // Display list attributes
    int currAttrIdx = 0;
    bool addedTex1Uv = false;
    for (GX::Attr attr{}; attr < MaxVtxAttr; attr = GX::Attr(attr + 1)) {
      // Indexed attributes
      if (config.vtxAttrs[attr] != GX::INDEX8 && config.vtxAttrs[attr] != GX::INDEX16) {
        continue;
      }
      const auto [div, rem] = std::div(currAttrIdx, 4);
      std::string_view attrName;
      bool addUniformBinding = true;
      // TODO: this is a hack to only have to bind tex0_uv and tex1_uv for MP
      // should figure out a more generic approach
      if (attr >= GX::VA_TEX1 && attr <= GX::VA_TEX7) {
        attrName = VtxAttributeNames[GX::VA_TEX1];
        addUniformBinding = !addedTex1Uv;
        addedTex1Uv = true;
      } else {
        attrName = VtxAttributeNames[attr];
      }
      vtxXfrAttrsPre +=
          fmt::format(FMT_STRING("\n    var {} = v_arr_{}[in_dl{}[{}]];"), vtx_attr(config, attr), attrName, div, rem);
      if (addUniformBinding) {
        std::string_view arrType;
        if (attr == GX::VA_POS || attr == GX::VA_NRM) {
          arrType = "vec3<f32>";
        } else if (attr >= GX::VA_TEX0 && attr <= GX::VA_TEX7) {
          arrType = "vec2<f32>";
        }
        uniformBindings += fmt::format(FMT_STRING("\n@group(0) @binding({})"
                                                  "\nvar<storage, read> v_arr_{}: array<{}>;"),
                                       uniBindingIdx++, attrName, arrType);
      }
      ++currAttrIdx;
    }
    auto [num4xAttrArrays, rem] = std::div(currAttrIdx, 4);
    u32 num2xAttrArrays = 0;
    if (rem > 2) {
      ++num4xAttrArrays;
    } else if (rem > 0) {
      num2xAttrArrays = 1;
    }
    for (u32 i = 0; i < num4xAttrArrays; ++i) {
      if (locIdx > 0) {
        vtxInAttrs += "\n    , ";
      } else {
        vtxInAttrs += "\n    ";
      }
      vtxInAttrs += fmt::format(FMT_STRING("@location({}) in_dl{}: vec4<i32>"), locIdx++, i);
    }
    for (u32 i = 0; i < num2xAttrArrays; ++i) {
      if (locIdx > 0) {
        vtxInAttrs += "\n    , ";
      } else {
        vtxInAttrs += "\n    ";
      }
      vtxInAttrs += fmt::format(FMT_STRING("@location({}) in_dl{}: vec2<i32>"), locIdx++, num4xAttrArrays + i);
    }
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
  vtxXfrAttrsPre += fmt::format(FMT_STRING("\n    var mv_pos = ubuf.pos_mtx * vec4<f32>({}, 1.0);"
                                           "\n    var mv_nrm = ubuf.nrm_mtx * vec4<f32>({}, 0.0);"
                                           "\n    out.pos = ubuf.proj * vec4<f32>(mv_pos, 1.0);"),
                                vtx_attr(config, GX::VA_POS), vtx_attr(config, GX::VA_NRM));
  if constexpr (EnableNormalVisualization) {
    vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) nrm: vec3<f32>,"), vtxOutIdx++);
    vtxXfrAttrsPre += "\n    out.nrm = mv_nrm;";
  }

  std::string fragmentFnPre;
  std::string fragmentFn;
  for (u32 idx = 0; idx < config.tevStageCount; ++idx) {
    const auto& stage = config.tevStages[idx];
    {
      std::string outReg;
      switch (stage.colorOp.outReg) {
      case GX::TEVPREV:
        outReg = "prev";
        break;
      case GX::TEVREG0:
        outReg = "tevreg0";
        break;
      case GX::TEVREG1:
        outReg = "tevreg1";
        break;
      case GX::TEVREG2:
        outReg = "tevreg2";
        break;
      default:
        Log.report(logvisor::Fatal, FMT_STRING("invalid colorOp outReg {}"), stage.colorOp.outReg);
      }
      std::string op = fmt::format(
          FMT_STRING("(({4}mix({0}, {1}, {2}) + {3}){5}){6}"), color_arg_reg(stage.colorPass.a, idx, config, stage),
          color_arg_reg(stage.colorPass.b, idx, config, stage), color_arg_reg(stage.colorPass.c, idx, config, stage),
          color_arg_reg(stage.colorPass.d, idx, config, stage), tev_op(stage.colorOp.op), tev_bias(stage.colorOp.bias),
          tev_scale(stage.colorOp.scale));
      if (stage.colorOp.clamp) {
        op = fmt::format(FMT_STRING("clamp({}, vec3<f32>(0.0), vec3<f32>(1.0))"), op);
      }
      fragmentFn +=
          fmt::format(FMT_STRING("\n    // TEV stage {2}\n    {0} = vec4<f32>({1}, {0}.a);"), outReg, op, idx);
    }
    {
      std::string outReg;
      switch (stage.alphaOp.outReg) {
      case GX::TEVPREV:
        outReg = "prev.a";
        break;
      case GX::TEVREG0:
        outReg = "tevreg0.a";
        break;
      case GX::TEVREG1:
        outReg = "tevreg1.a";
        break;
      case GX::TEVREG2:
        outReg = "tevreg2.a";
        break;
      default:
        Log.report(logvisor::Fatal, FMT_STRING("invalid alphaOp outReg {}"), stage.alphaOp.outReg);
      }
      std::string op = fmt::format(
          FMT_STRING("(({4}mix({0}, {1}, {2}) + {3}){5}){6}"), alpha_arg_reg(stage.alphaPass.a, idx, config, stage),
          alpha_arg_reg(stage.alphaPass.b, idx, config, stage), alpha_arg_reg(stage.alphaPass.c, idx, config, stage),
          alpha_arg_reg(stage.alphaPass.d, idx, config, stage), tev_op(stage.alphaOp.op), tev_bias(stage.alphaOp.bias),
          tev_scale(stage.alphaOp.scale));
      if (stage.alphaOp.clamp) {
        op = fmt::format(FMT_STRING("clamp({}, 0.0, 1.0)"), op);
      }
      fragmentFn += fmt::format(FMT_STRING("\n    {0} = {1};"), outReg, op);
    }
  }
  if (info.loadsTevReg.test(0)) {
    uniBufAttrs += "\n    tevprev: vec4<f32>,";
    fragmentFnPre += "\n    var prev = ubuf.tevprev;";
  } else {
    fragmentFnPre += "\n    var prev: vec4<f32>;";
  }
  for (int i = 1 /* Skip TEVPREV */; i < info.loadsTevReg.size(); ++i) {
    if (info.loadsTevReg.test(i)) {
      uniBufAttrs += fmt::format(FMT_STRING("\n    tevreg{}: vec4<f32>,"), i - 1);
      fragmentFnPre += fmt::format(FMT_STRING("\n    var tevreg{0} = ubuf.tevreg{0};"), i - 1);
    } else if (info.writesTevReg.test(i)) {
      fragmentFnPre += fmt::format(FMT_STRING("\n    var tevreg{0}: vec4<f32>;"), i - 1);
    }
  }
  bool addedLightStruct = false;
  for (int i = 0; i < info.sampledColorChannels.size(); ++i) {
    if (!info.sampledColorChannels.test(i)) {
      continue;
    }

    uniBufAttrs += fmt::format(FMT_STRING("\n    cc{0}_amb: vec4<f32>,"), i);
    uniBufAttrs += fmt::format(FMT_STRING("\n    cc{0}_mat: vec4<f32>,"), i);

    const auto& cc = config.colorChannels[i];
    if (cc.lightingEnabled) {
      if (!addedLightStruct) {
        uniformPre +=
            "\n"
            "struct Light {\n"
            "    pos: vec3<f32>,\n"
            "    dir: vec3<f32>,\n"
            "    color: vec4<f32>,\n"
            "    cos_att: vec3<f32>,\n"
            "    dist_att: vec3<f32>,\n"
            "};";
        if (UsePerPixelLighting) {
          vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) mv_pos: vec3<f32>,"), vtxOutIdx++);
          vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) mv_nrm: vec3<f32>,"), vtxOutIdx++);
          vtxXfrAttrs += fmt::format(FMT_STRING(R"""(
    out.mv_pos = mv_pos;
    out.mv_nrm = mv_nrm;)"""));
        }
        addedLightStruct = true;
      }

      uniBufAttrs += fmt::format(FMT_STRING("\n    lightState{}: u32,"), i);
      uniBufAttrs += fmt::format(FMT_STRING("\n    lights{}: array<Light, {}>,"), i, GX::MaxLights);

      std::string ambSrc, matSrc, lightAttnFn, lightDiffFn;
      if (cc.ambSrc == GX::SRC_VTX) {
        ambSrc = vtx_attr(config, static_cast<GX::Attr>(GX::VA_CLR0 + i));
      } else if (cc.ambSrc == GX::SRC_REG) {
        ambSrc = fmt::format(FMT_STRING("ubuf.cc{0}_amb"), i);
      }
      if (cc.matSrc == GX::SRC_VTX) {
        matSrc = vtx_attr(config, static_cast<GX::Attr>(GX::VA_CLR0 + i));
      } else if (cc.matSrc == GX::SRC_REG) {
        matSrc = fmt::format(FMT_STRING("ubuf.cc{0}_mat"), i);
      }
      GX::DiffuseFn diffFn = cc.diffFn;
      if (cc.attnFn == GX::AF_NONE) {
        lightAttnFn = "attn = 1.0;";
      } else if (cc.attnFn == GX::AF_SPOT) {
        lightAttnFn = fmt::format(FMT_STRING(R"""(
          var cosine = max(0.0, dot(ldir, light.dir));
          var cos_attn = dot(light.cos_att, vec3<f32>(1.0, cosine, cosine * cosine));
          var dist_attn = dot(light.dist_att, vec3<f32>(1.0, dist, dist2));
          attn = max(0.0, cos_attn / dist_attn);)"""));
      } else if (cc.attnFn == GX::AF_SPEC) {
        diffFn = GX::DF_NONE;
        Log.report(logvisor::Fatal, FMT_STRING("AF_SPEC unimplemented"));
      }
      if (diffFn == GX::DF_NONE) {
        lightDiffFn = "1.0";
      } else if (diffFn == GX::DF_SIGN) {
        if (UsePerPixelLighting) {
          lightDiffFn = "dot(ldir, in.mv_nrm)";
        } else {
          lightDiffFn = "dot(ldir, mv_nrm)";
        }
      } else if (diffFn == GX::DF_CLAMP) {
        if (UsePerPixelLighting) {
          lightDiffFn = "max(0.0, dot(ldir, in.mv_nrm))";
        } else {
          lightDiffFn = "max(0.0, dot(ldir, mv_nrm))";
        }
      }
      std::string outVar, posVar;
      if (UsePerPixelLighting) {
        outVar = fmt::format(FMT_STRING("rast{}"), i);
        posVar = "in.mv_pos";
      } else {
        outVar = fmt::format(FMT_STRING("out.cc{}"), i);
        posVar = "mv_pos";
      }
      auto lightFunc = fmt::format(FMT_STRING(R"""(
    {{
      var lighting = {5};
      for (var i = 0u; i < {1}u; i++) {{
          if ((ubuf.lightState{0} & (1u << i)) == 0u) {{ continue; }}
          var light = ubuf.lights{0}[i];
          var ldir = light.pos - {7};
          var dist2 = dot(ldir, ldir);
          var dist = sqrt(dist2);
          ldir = ldir / dist;
          var attn: f32;{2}
          var diff = {3};
          lighting = lighting + (attn * diff * light.color);
      }}
      {6} = {4} * clamp(lighting, vec4<f32>(0.0), vec4<f32>(1.0));
    }})"""),
                                   i, GX::MaxLights, lightAttnFn, lightDiffFn, matSrc, ambSrc, outVar, posVar);
      if (UsePerPixelLighting) {
        fragmentFnPre += fmt::format(FMT_STRING("\n    var rast{}: vec4<f32>;"), i);
        fragmentFnPre += lightFunc;
      } else {
        vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) cc{}: vec4<f32>,"), vtxOutIdx++, i);
        vtxXfrAttrs += lightFunc;
        fragmentFnPre += fmt::format(FMT_STRING("\n    var rast{0} = in.cc{0};"), i);
      }
    } else if (cc.matSrc == GX::SRC_VTX) {
      vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) cc{}: vec4<f32>,"), vtxOutIdx++, i);
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.cc{} = {};"), i, vtx_attr(config, GX::Attr(GX::VA_CLR0 + i)));
      fragmentFnPre += fmt::format(FMT_STRING("\n    var rast{0} = in.cc{0};"), i);
    } else {
      fragmentFnPre += fmt::format(FMT_STRING("\n    var rast{0} = ubuf.cc{0}_mat;"), i);
    }
  }
  for (int i = 0; i < info.sampledKColors.size(); ++i) {
    if (info.sampledKColors.test(i)) {
      uniBufAttrs += fmt::format(FMT_STRING("\n    kcolor{}: vec4<f32>,"), i);
    }
  }
  for (int i = 0; i < info.sampledTexCoords.size(); ++i) {
    if (!info.sampledTexCoords.test(i)) {
      continue;
    }
    const auto& tcg = config.tcgs[i];
    vtxOutAttrs += fmt::format(FMT_STRING("\n    @location({}) tex{}_uv: vec2<f32>,"), vtxOutIdx++, i);
    if (tcg.src >= GX::TG_TEX0 && tcg.src <= GX::TG_TEX7) {
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    var tc{} = vec4<f32>({}, 0.0, 1.0);"), i,
                                 vtx_attr(config, GX::Attr(GX::VA_TEX0 + (tcg.src - GX::TG_TEX0))));
    } else if (tcg.src == GX::TG_POS) {
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    var tc{} = vec4<f32>(in_pos, 1.0);"), i);
    } else if (tcg.src == GX::TG_NRM) {
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    var tc{} = vec4<f32>(in_nrm, 1.0);"), i);
    } else {
      Log.report(logvisor::Fatal, FMT_STRING("unhandled tcg src {} for "), tcg.src);
      unreachable();
    }
    if (tcg.mtx == GX::IDENTITY) {
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    var tc{0}_tmp = tc{0}.xyz;"), i);
    } else {
      u32 texMtxIdx = (tcg.mtx - GX::TEXMTX0) / 3;
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    var tc{0}_tmp = ubuf.texmtx{1} * tc{0};"), i, texMtxIdx);
    }
    if (tcg.normalize) {
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    tc{0}_tmp = normalize(tc{0}_tmp);"), i);
    }
    if (tcg.postMtx == GX::PTIDENTITY) {
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    var tc{0}_proj = tc{0}_tmp;"), i);
    } else {
      u32 postMtxIdx = (tcg.postMtx - GX::PTTEXMTX0) / 3;
      vtxXfrAttrs += fmt::format(FMT_STRING("\n    var tc{0}_proj = ubuf.postmtx{1} * vec4<f32>(tc{0}_tmp.xyz, 1.0);"),
                                 i, postMtxIdx);
    }
    vtxXfrAttrs += fmt::format(FMT_STRING("\n    out.tex{0}_uv = tc{0}_proj.xy;"), i);
  }
  for (int i = 0; i < config.tevStages.size(); ++i) {
    const auto& stage = config.tevStages[i];
    if (stage.texMapId == GX::TEXMAP_NULL ||
        stage.texCoordId == GX::TEXCOORD_NULL
        // TODO should check this per-stage probably
        || !info.sampledTextures.test(stage.texMapId)) {
      continue;
    }
    std::string uvIn;
    const auto& texConfig = config.textureConfig[stage.texMapId];
    // TODO
    // if (texConfig.flipUV) {
    //   uvIn = fmt::format(FMT_STRING("vec2<f32>(in.tex{0}_uv.x, -in.tex{0}_uv.y)"), stage.texCoordId);
    // } else {
    uvIn = fmt::format(FMT_STRING("in.tex{0}_uv"), stage.texCoordId);
    // }
    if (is_palette_format(texConfig.loadFmt)) {
      fragmentFnPre +=
          fmt::format(FMT_STRING("\n    var sampled{0} = textureSamplePalette{3}(tex{1}, tex{1}_samp, {2}, tlut{1});"),
                      i, stage.texMapId, uvIn, is_palette_format(texConfig.copyFmt) ? ""sv : "RGB"sv);
    } else {
      fragmentFnPre += fmt::format(
          FMT_STRING("\n    var sampled{0} = textureSampleBias(tex{1}, tex{1}_samp, {2}, ubuf.tex{1}_lod);"), i,
          stage.texMapId, uvIn);
    }
    fragmentFnPre += texture_conversion(texConfig, i, stage.texMapId);
  }
  for (int i = 0; i < info.usesTexMtx.size(); ++i) {
    if (info.usesTexMtx.test(i)) {
      switch (info.texMtxTypes[i]) {
      case GX::TG_MTX2x4:
        uniBufAttrs += fmt::format(FMT_STRING("\n    texmtx{}: mat4x2<f32>,"), i);
        break;
      case GX::TG_MTX3x4:
        uniBufAttrs += fmt::format(FMT_STRING("\n    texmtx{}: mat4x3<f32>,"), i);
        break;
      default:
        Log.report(logvisor::Fatal, FMT_STRING("unhandled tex mtx type {}"), info.texMtxTypes[i]);
        unreachable();
      }
    }
  }
  for (int i = 0; i < info.usesPTTexMtx.size(); ++i) {
    if (info.usesPTTexMtx.test(i)) {
      uniBufAttrs += fmt::format(FMT_STRING("\n    postmtx{}: mat4x3<f32>,"), i);
    }
  }
  if (info.usesFog) {
    uniformPre +=
        "\n"
        "struct Fog {\n"
        "    color: vec4<f32>,\n"
        "    a: f32,\n"
        "    b: f32,\n"
        "    c: f32,\n"
        "    pad: f32,\n"
        "}";
    uniBufAttrs += "\n    fog: Fog,";

    fragmentFn += "\n    // Fog\n    var fogF = clamp((ubuf.fog.a / (ubuf.fog.b - in.pos.z)) - ubuf.fog.c, 0.0, 1.0);";
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
  size_t texBindIdx = 0;
  for (int i = 0; i < info.sampledTextures.size(); ++i) {
    if (!info.sampledTextures.test(i)) {
      continue;
    }
    uniBufAttrs += fmt::format(FMT_STRING("\n    tex{}_lod: f32,"), i);

    sampBindings += fmt::format(FMT_STRING("\n@group(1) @binding({})\n"
                                           "var tex{}_samp: sampler;"),
                                texBindIdx, i);

    const auto& texConfig = config.textureConfig[i];
    if (is_palette_format(texConfig.loadFmt)) {
      texBindings += fmt::format(FMT_STRING("\n@group(2) @binding({})\n"
                                            "var tex{}: texture_2d<{}>;"),
                                 texBindIdx, i, is_palette_format(texConfig.copyFmt) ? "i32"sv : "f32"sv);
      ++texBindIdx;
      texBindings += fmt::format(FMT_STRING("\n@group(2) @binding({})\n"
                                            "var tlut{}: texture_2d<f32>;"),
                                 texBindIdx, i);
    } else {
      texBindings += fmt::format(FMT_STRING("\n@group(2) @binding({})\n"
                                            "var tex{}: texture_2d<f32>;"),
                                 texBindIdx, i);
    }
    ++texBindIdx;
  }

  if (config.alphaCompare) {
    bool comp0Valid = true;
    bool comp1Valid = true;
    std::string comp0 = alpha_compare(config.alphaCompare.comp0, config.alphaCompare.ref0, comp0Valid);
    std::string comp1 = alpha_compare(config.alphaCompare.comp1, config.alphaCompare.ref1, comp1Valid);
    if (comp0Valid || comp1Valid) {
      fragmentFn += "\n    // Alpha compare";
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
  if constexpr (EnableNormalVisualization) {
    fragmentFn += "\n    prev = vec4<f32>(in.nrm, prev.a);";
  }

  const auto shaderSource = fmt::format(FMT_STRING(R"""({10}
struct Uniform {{
    pos_mtx: mat4x3<f32>,
    nrm_mtx: mat4x3<f32>,
    proj: mat4x4<f32>,{0}
}};
@group(0) @binding(0)
var<uniform> ubuf: Uniform;{3}{1}{2}

struct VertexOutput {{
    @builtin(position) pos: vec4<f32>,{4}
}};

fn intensityF32(rgb: vec3<f32>) -> f32 {{
    // RGB to intensity conversion
    // https://github.com/dolphin-emu/dolphin/blob/4cd48e609c507e65b95bca5afb416b59eaf7f683/Source/Core/VideoCommon/TextureConverterShaderGen.cpp#L237-L241
    return dot(rgb, vec3(0.257, 0.504, 0.098)) + 16.0 / 255.0;
}}
fn intensityI32(rgb: vec3<f32>) -> i32 {{
    return i32(dot(rgb, vec3(0.257, 0.504, 0.098)) * 255.f);
}}
fn textureSamplePalette(tex: texture_2d<i32>, samp: sampler, uv: vec2<f32>, tlut: texture_2d<f32>) -> vec4<f32> {{
    // Gather index values
    var i = textureGather(0, tex, samp, uv);
    // Load palette colors
    var c0 = textureLoad(tlut, vec2<i32>(i[0], 0), 0);
    var c1 = textureLoad(tlut, vec2<i32>(i[1], 0), 0);
    var c2 = textureLoad(tlut, vec2<i32>(i[2], 0), 0);
    var c3 = textureLoad(tlut, vec2<i32>(i[3], 0), 0);
    // Perform bilinear filtering
    var f = fract(uv * vec2<f32>(textureDimensions(tex)) + 0.5);
    var t0 = mix(c3, c2, f.x);
    var t1 = mix(c0, c1, f.x);
    return mix(t0, t1, f.y);
}}
fn textureSamplePaletteRGB(tex: texture_2d<f32>, samp: sampler, uv: vec2<f32>, tlut: texture_2d<f32>) -> vec4<f32> {{
    // Gather RGB channels
    var iR = textureGather(0, tex, samp, uv);
    var iG = textureGather(1, tex, samp, uv);
    var iB = textureGather(2, tex, samp, uv);
    // Perform intensity conversion
    var i0 = intensityI32(vec3<f32>(iR[0], iG[0], iB[0]));
    var i1 = intensityI32(vec3<f32>(iR[1], iG[1], iB[1]));
    var i2 = intensityI32(vec3<f32>(iR[2], iG[2], iB[2]));
    var i3 = intensityI32(vec3<f32>(iR[3], iG[3], iB[3]));
    // Load palette colors
    var c0 = textureLoad(tlut, vec2<i32>(i0, 0), 0);
    var c1 = textureLoad(tlut, vec2<i32>(i1, 0), 0);
    var c2 = textureLoad(tlut, vec2<i32>(i2, 0), 0);
    var c3 = textureLoad(tlut, vec2<i32>(i3, 0), 0);
    // Perform bilinear filtering
    var f = fract(uv * vec2<f32>(textureDimensions(tex)) + 0.5);
    var t0 = mix(c3, c2, f.x);
    var t1 = mix(c0, c1, f.x);
    return mix(t0, t1, f.y);
}}

@stage(vertex)
fn vs_main({5}
) -> VertexOutput {{
    var out: VertexOutput;{9}{6}
    return out;
}}

@stage(fragment)
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {{{8}{7}
    return prev;
}}
)"""),
                                        uniBufAttrs, sampBindings, texBindings, uniformBindings, vtxOutAttrs,
                                        vtxInAttrs, vtxXfrAttrs, fragmentFn, fragmentFnPre, vtxXfrAttrsPre, uniformPre);
  if (EnableDebugPrints) {
    Log.report(logvisor::Info, FMT_STRING("Generated shader: {}"), shaderSource);
  }

  wgpu::ShaderModuleWGSLDescriptor wgslDescriptor{};
  wgslDescriptor.source = shaderSource.c_str();
  const auto label = fmt::format(FMT_STRING("GX Shader {:x}"), hash);
  const auto shaderDescriptor = wgpu::ShaderModuleDescriptor{
      .nextInChain = &wgslDescriptor,
      .label = label.c_str(),
  };
  auto shader = gpu::g_device.CreateShaderModule(&shaderDescriptor);

  auto pair = std::make_pair(std::move(shader), info);
  g_gxCachedShaders.emplace(hash, pair);
#ifndef NDEBUG
  g_gxCachedShaderConfigs.emplace(hash, config);
#endif

  return pair.first;
}
} // namespace aurora::gfx::gx
