#include "CGX.hpp"

#include "Graphics/CTexture.hpp"

namespace metaforce::CGX {
SGXState sGXState{};
std::array<GX::VtxDescList, 12> sVtxDescList{};

void ResetGXStates() noexcept {
  sGXState.x48_descList = 0;
  GXClearVtxDesc();
  sGXState.x0_arrayPtrs.fill(nullptr);
  for (GX::TexMapID id = GX::TEXMAP0; id < GX::MAX_TEXMAP; id = static_cast<GX::TexMapID>(id + 1)) {
    CTexture::InvalidateTexMap(id);
  }
  for (GX::TevKColorID id = GX::KCOLOR0; const auto& item : sGXState.x58_kColors) {
    GXSetTevKColor(id, item);
    id = static_cast<GX::TevKColorID>(id + 1);
  }
  GXSetTevSwapModeTable(GX::TEV_SWAP1, GX::CH_RED, GX::CH_GREEN, GX::CH_BLUE, GX::CH_RED);
  GXSetTevSwapModeTable(GX::TEV_SWAP2, GX::CH_RED, GX::CH_GREEN, GX::CH_BLUE, GX::CH_GREEN);
  GXSetTevSwapModeTable(GX::TEV_SWAP3, GX::CH_RED, GX::CH_GREEN, GX::CH_BLUE, GX::CH_BLUE);
  SetAlphaCompare(GX::ALWAYS, 0, GX::AOP_AND, GX::ALWAYS, 0);
  // GXSetCurrentMtx(0);
  SetNumIndStages(0);
  // TODO GXSetIndTexCoordScale
  for (GX::TevStageID id = GX::TEVSTAGE0; id < GX::MAX_TEVSTAGE; id = static_cast<GX::TevStageID>(id + 1)) {
    SetTevDirect(id);
  }
  // GXSetTexCoordCylWrap
  // GXSetZTexture
}
} // namespace metaforce::CGX
