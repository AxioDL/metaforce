#include "CGX.hpp"

#include "Graphics/CTexture.hpp"

namespace metaforce::CGX {
SGXState sGXState;

void ResetGXStates() noexcept {
  sGXState.x48_descList = nullptr;
  // GXClearVtxDesc();
  sGXState.x0_arrayPtrs.fill(nullptr);
  for (GX::TexMapID id = GX::TEXMAP0; id < GX::MAX_TEXMAP; id = static_cast<GX::TexMapID>(id + 1)) {
    CTexture::InvalidateTexMap(id);
  }
  for (GX::TevKColorID id = GX::KCOLOR0; const auto& item : sGXState.x58_kColors) {
    GXSetTevKColor(id, item);
    id = static_cast<GX::TevKColorID>(id + 1);
  }
  // GXSetTevSwapModeTable
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
