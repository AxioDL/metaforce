#include "CGX.hpp"

#include "Graphics/CTexture.hpp"

namespace metaforce::CGX {
SGXState sGXState{};
std::array<GXVtxDescList, 12> sVtxDescList{};

void SetLineWidth(u8 width, GXTexOffset offset) noexcept {
  u16 flags = width | offset << 8;
  if (flags != sGXState.x54_lineWidthAndOffset) {
    sGXState.x54_lineWidthAndOffset = flags;
    GXSetLineWidth(width, offset);
  }
}

void ResetGXStates() noexcept {
  sGXState.x48_descList = 0;
  GXClearVtxDesc();
  sGXState.x0_arrayPtrs.fill(nullptr);
  for (GXTexMapID id = GX_TEXMAP0; id < GX_MAX_TEXMAP; id = static_cast<GXTexMapID>(id + 1)) {
    CTexture::InvalidateTexMap(id);
  }
  for (GXTevKColorID id = GX_KCOLOR0; const auto& item : sGXState.x58_kColors) {
    GXSetTevKColor(id, item);
    id = static_cast<GXTevKColorID>(id + 1);
  }
  GXSetTevSwapModeTable(GX_TEV_SWAP1, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_RED);
  GXSetTevSwapModeTable(GX_TEV_SWAP2, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_GREEN);
  GXSetTevSwapModeTable(GX_TEV_SWAP3, GX_CH_RED, GX_CH_GREEN, GX_CH_BLUE, GX_CH_BLUE);
  SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
  GXSetCurrentMtx(GX_PNMTX0);
  SetNumIndStages(0);
  for (int i = 0; i < GX_MAX_INDTEXSTAGE; i++) {
    GXSetIndTexCoordScale(static_cast<GXIndTexStageID>(i), GX_ITS_1, GX_ITS_1);
  }
  for (int i = 0; i < GX_MAX_TEVSTAGE; i++) {
    SetTevDirect(static_cast<GXTevStageID>(i));
  }
  for (int i = 0; i < GX_MAX_TEXCOORD; i++) {
    GXSetTexCoordScaleManually(static_cast<GXTexCoordID>(i), false, 0, 0);
  }
  GXSetZTexture(GX_ZT_DISABLE, GX_TF_Z8, 0);
}
} // namespace metaforce::CGX
