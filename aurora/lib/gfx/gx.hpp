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

  STevStage(const metaforce::CTevCombiners::ColorPass& colPass, const metaforce::CTevCombiners::AlphaPass& alphaPass,
            const metaforce::CTevCombiners::CTevOp& colorOp, const metaforce::CTevCombiners::CTevOp& alphaOp)
  : colorPass(colPass), alphaPass(alphaPass), colorOp(colorOp), alphaOp(alphaOp) {}
};

constexpr u32 maxTevStages = GX::MAX_TEVSTAGE;
extern std::array<std::optional<STevStage>, maxTevStages> g_tevStages;

static inline XXH64_hash_t hash_tev_stages(XXH64_hash_t seed = 0) {
  const auto end = std::find_if(g_tevStages.cbegin(), g_tevStages.cend(), [](const auto& a) { return !a; });
  return xxh3_hash(g_tevStages.data(), (end - g_tevStages.begin()) * sizeof(STevStage), seed);
}

static inline Mat4x4<float> get_combined_matrix() { return g_proj * g_mv; }
} // namespace aurora::gfx
