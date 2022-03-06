#include "gx.hpp"

#include "common.hpp"
#include "../gpu.hpp"

#include <magic_enum.hpp>

namespace aurora::gfx {
static logvisor::Module Log("aurora::gfx::gx");

zeus::CMatrix4f g_mv;
zeus::CMatrix4f g_mvInv;
zeus::CMatrix4f g_proj;
metaforce::CFogState g_fogState;
metaforce::ERglCullMode g_cullMode;
metaforce::ERglBlendMode g_blendMode;
metaforce::ERglBlendFactor g_blendFacSrc;
metaforce::ERglBlendFactor g_blendFacDst;
metaforce::ERglLogicOp g_blendOp;
bool g_depthCompare;
bool g_depthUpdate;
metaforce::ERglEnum g_depthFunc;
std::array<zeus::CColor, 3> g_colorRegs;
std::array<zeus::CColor, GX::MAX_KCOLOR> g_kcolors;
bool g_alphaUpdate;
std::optional<float> g_dstAlpha;
zeus::CColor g_clearColor = zeus::skClear;

std::array<SChannelState, 2> g_colorChannels;
std::array<LightVariant, MaxLights> g_lights;
std::bitset<MaxLights> g_lightState;

std::array<std::optional<STevStage>, maxTevStages> g_tevStages;

// GX state
void set_cull_mode(metaforce::ERglCullMode mode) noexcept { g_cullMode = mode; }
void set_blend_mode(metaforce::ERglBlendMode mode, metaforce::ERglBlendFactor src, metaforce::ERglBlendFactor dst,
                    metaforce::ERglLogicOp op) noexcept {
  g_blendMode = mode;
  g_blendFacSrc = src;
  g_blendFacDst = dst;
  g_blendOp = op;
}
void set_depth_mode(bool compare_enable, metaforce::ERglEnum func, bool update_enable) noexcept {
  g_depthCompare = compare_enable;
  g_depthFunc = func;
  g_depthUpdate = update_enable;
}
void set_tev_reg_color(GX::TevRegID id, const zeus::CColor& color) noexcept {
  if (id < GX::TEVREG0 || id > GX::TEVREG2) {
    Log.report(logvisor::Fatal, FMT_STRING("set_tev_reg_color: bad reg {}"), id);
    unreachable();
  }
  g_colorRegs[id - 1] = color;
}
void set_tev_k_color(GX::TevKColorID id, const zeus::CColor& color) noexcept {
  if (id >= GX::MAX_KCOLOR) {
    Log.report(logvisor::Fatal, FMT_STRING("set_tev_k_color: bad reg {}"), id);
    unreachable();
  }
  g_kcolors[id] = color;
}
void set_alpha_update(bool enabled) noexcept { g_alphaUpdate = enabled; }
void set_dst_alpha(bool enabled, float value) noexcept {
  if (enabled) {
    g_dstAlpha = value;
  } else {
    g_dstAlpha.reset();
  }
}
void set_clear_color(const zeus::CColor& color) noexcept { g_clearColor = color; }
void set_alpha_discard(bool v) {
  // TODO
}

void update_model_view(const zeus::CMatrix4f& mv, const zeus::CMatrix4f& mv_inv) noexcept {
  g_mv = mv;
  g_mvInv = mv_inv;
}
constexpr zeus::CMatrix4f DepthCorrect{
    // clang-format off
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 0.5f, 0.5f,
    0.f, 0.f, 0.f, 1.f,
    // clang-format on
};
void update_projection(const zeus::CMatrix4f& proj) noexcept { g_proj = DepthCorrect * proj; }
void update_fog_state(const metaforce::CFogState& state) noexcept { g_fogState = state; }

void disable_tev_stage(metaforce::ERglTevStage stage) noexcept { g_tevStages[static_cast<size_t>(stage)].reset(); }
void update_tev_stage(metaforce::ERglTevStage stage, const metaforce::CTevCombiners::ColorPass& colPass,
                      const metaforce::CTevCombiners::AlphaPass& alphaPass,
                      const metaforce::CTevCombiners::CTevOp& colorOp,
                      const metaforce::CTevCombiners::CTevOp& alphaOp) noexcept {
  g_tevStages[static_cast<size_t>(stage)] = {colPass, alphaPass, colorOp, alphaOp};
}

void set_tev_order(GX::TevStageID id, GX::TexCoordID tcid, GX::TexMapID tmid, GX::ChannelID cid) noexcept {
  auto& stage = g_tevStages[id];
  if (!stage) {
    Log.report(logvisor::Fatal, FMT_STRING("set_tev_order: disabled stage {}"), id);
    unreachable();
  }
  stage->texCoordId = tcid;
  stage->texMapId = tmid;
  stage->channelId = cid;
}
void set_tev_k_color_sel(GX::TevStageID id, GX::TevKColorSel sel) noexcept {
  auto& stage = g_tevStages[id];
  if (!stage) {
    Log.report(logvisor::Fatal, FMT_STRING("set_tev_k_color_sel: disabled stage {}"), id);
    unreachable();
  }
  stage->kcSel = sel;
}
void set_tev_k_alpha_sel(GX::TevStageID id, GX::TevKAlphaSel sel) noexcept {
  auto& stage = g_tevStages[id];
  if (!stage) {
    Log.report(logvisor::Fatal, FMT_STRING("set_tev_k_alpha_sel: disabled stage {}"), id);
    unreachable();
  }
  stage->kaSel = sel;
}

void set_chan_amb_color(GX::ChannelID id, const zeus::CColor& color) noexcept {
  if (id < GX::COLOR0A0 || id > GX::COLOR1A1) {
    Log.report(logvisor::Fatal, FMT_STRING("set_chan_amb_color: invalid channel {}"), id);
    unreachable();
  }
  g_colorChannels[id - GX::COLOR0A0].ambColor = color;
}
void set_chan_mat_color(GX::ChannelID id, const zeus::CColor& color) noexcept {
  if (id < GX::COLOR0A0 || id > GX::COLOR1A1) {
    Log.report(logvisor::Fatal, FMT_STRING("set_chan_mat_color: invalid channel {}"), id);
    unreachable();
  }
  g_colorChannels[id - GX::COLOR0A0].matColor = color;
}

void load_light(GX::LightID id, const Light& light) noexcept {
  g_lights[id] = light;
}
void load_light_ambient(GX::LightID id, const zeus::CColor& ambient) noexcept {
  g_lights[id] = ambient;
}
void set_light_state(std::bitset<MaxLights> bits) noexcept {
  g_lightState = bits;
}
} // namespace aurora::gfx
