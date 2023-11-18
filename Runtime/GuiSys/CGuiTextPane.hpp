#pragma once

#include <memory>
#include <vector>

#include "Runtime/GuiSys/CGuiPane.hpp"
#include "Runtime/GuiSys/CGuiTextSupport.hpp"

namespace metaforce {

class CGuiTextPane : public CGuiPane {
  static bool sDrawPaneRects;
  CGuiTextSupport xd4_textSupport;

public:
  CGuiTextPane(const CGuiWidgetParms& parms, CSimplePool* sp, const zeus::CVector2f& dim, const zeus::CVector3f& vec,
               CAssetId fontId, const CGuiTextProperties& props, const zeus::CColor& col1, const zeus::CColor& col2,
               s32 padX, s32 padY, CAssetId jpFontId, s32 jpExtentX, s32 jpExtentY);
  FourCC GetWidgetTypeID() const override { return FOURCC('TXPN'); }

  CGuiTextSupport& TextSupport() { return xd4_textSupport; }
  const CGuiTextSupport& GetTextSupport() const { return xd4_textSupport; }
  void Update(float dt) override;
  bool GetIsFinishedLoadingWidgetSpecific() override;
  std::vector<CAssetId> GetFontAssets() const { return {xd4_textSupport.x5c_fontId}; }
  void SetDimensions(const zeus::CVector2f& dim, bool initVBO) override;
  void ScaleDimensions(const zeus::CVector3f& scale) override;
  void Draw(const CGuiWidgetDrawParms& parms) override;
  bool TestCursorHit(const zeus::CMatrix4f& vp, const zeus::CVector2f& point) const override;

  static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp, u32 version);
};

} // namespace metaforce
