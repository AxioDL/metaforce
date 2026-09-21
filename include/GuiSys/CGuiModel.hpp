#ifndef _CGUIMODEL
#define _CGUIMODEL

#include "GuiSys/CGuiWidget.hpp"

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"

class CModel;

class CGuiModel : public CGuiWidget {
public:
  CGuiModel(const CGuiWidgetParms& parms, CSimplePool* sp, CAssetId modelId, uint lightMask,
            bool flag);
  ~CGuiModel();

  const rstl::optional_object< TCachedToken< CModel > >& GetModel() const { return xb8_model; }
  bool GetIsFinishedLoadingWidgetSpecific() const override;
  void Touch() const override;
  void Draw(const CGuiWidgetDrawParms& parms) const override;
  // bool TestCursorHit(const CMatrix4f& vp, const CVector2f& point) const override;

  static CGuiModel* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);

  virtual rstl::vector< CAssetId > GetModelAssets() const;
  FourCC GetWidgetTypeID() const override;

private:
  mutable rstl::optional_object< TCachedToken< CModel > > xb8_model;
  CAssetId xc8_modelId;
  uint xcc_lightMask;
};
CHECK_SIZEOF(CGuiModel, 0xd0)

#endif // _CGUIMODEL
