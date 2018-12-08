#pragma once

#include "CGuiPane.hpp"
#include "CGuiTextSupport.hpp"

namespace urde {

class CGuiTextPane : public CGuiPane {
  CGuiTextSupport xd4_textSupport;

public:
  CGuiTextPane(const CGuiWidgetParms& parms, CSimplePool* sp, const zeus::CVector2f& dim, const zeus::CVector3f& vec,
               CAssetId fontId, const CGuiTextProperties& props, const zeus::CColor& col1, const zeus::CColor& col2,
               s32 padX, s32 padY);
  FourCC GetWidgetTypeID() const { return FOURCC('TXPN'); }

  CGuiTextSupport& TextSupport() { return xd4_textSupport; }
  const CGuiTextSupport& GetTextSupport() const { return xd4_textSupport; }
  void Update(float dt);
  bool GetIsFinishedLoadingWidgetSpecific() const;
  std::vector<CAssetId> GetFontAssets() const { return {xd4_textSupport.x5c_fontId}; }
  void SetDimensions(const zeus::CVector2f& dim, bool initVBO);
  void ScaleDimensions(const zeus::CVector3f& scale);
  void Draw(const CGuiWidgetDrawParms& parms) const;

  static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
};

} // namespace urde
