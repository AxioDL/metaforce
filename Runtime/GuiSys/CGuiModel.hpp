#pragma once

#include <memory>

#include "Runtime/CToken.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/GuiSys/CGuiWidget.hpp"

namespace urde {
class CSimplePool;

class CGuiModel : public CGuiWidget {
  TLockedToken<CModel> xb8_model;
  CAssetId xc8_modelId;
  u32 xcc_lightMask;

public:
  CGuiModel(const CGuiWidgetParms& parms, CSimplePool* sp, CAssetId modelId, u32 lightMask, bool flag);
  FourCC GetWidgetTypeID() const override { return FOURCC('MODL'); }

  std::vector<CAssetId> GetModelAssets() const { return {xc8_modelId}; }
  const TLockedToken<CModel>& GetModel() const { return xb8_model; }
  bool GetIsFinishedLoadingWidgetSpecific() const override;
  void Touch() const override;
  void Draw(const CGuiWidgetDrawParms& parms) const override;
  bool TestCursorHit(const zeus::CMatrix4f& vp, const zeus::CVector2f& point) const override;

  static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
};

} // namespace urde
