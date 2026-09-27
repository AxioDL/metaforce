#ifndef _CAUIIMAGEPANE
#define _CAUIIMAGEPANE

#include "GuiSys/CGuiWidget.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/optional_object.hpp"

class CTexture;
class CAuiImagePane : public CGuiWidget {
public:
  ~CAuiImagePane() {}
  void Update(float dt) override;
  void Draw(const CGuiWidgetDrawParms& parms) const override;
  FourCC GetWidgetTypeID() const override { return 'IMGP'; }
  bool GetIsFinishedLoadingWidgetSpecific() const override;
  virtual void WriteData(COutputStream& out, bool) const;

  static CGuiWidget* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);

  CAuiImagePane(const CGuiWidget::CGuiWidgetParms&, CSimplePool* sp, CAssetId tex0, CAssetId tex1,
                const rstl::reserved_vector< CVector3f, 4 >& coords,
                const rstl::reserved_vector< CVector2f, 4 >& uvs, bool initTex);
  void SetFlashFactor(float factor) { mFlashFactor = factor; }
  void SetDeResFactor(float factor) { mDeResFactor = factor; }
  void SetTextureID0(CAssetId tex, CSimplePool* sp);
  void SetAnimationParms(const CVector2f& tileSize, float interval, float fadeDuration);

private:
  void DoDrawImagePane(CColor color, const CTexture& texture, int frame, float alpha,
                       bool noBlur) const;

  mutable rstl::optional_object< TCachedToken< CTexture > > mTex0Tok;
  CAssetId mTex0;
  CAssetId mTex1;
  CVector2f mUvBias0;
  CVector2f mUvBias1;
  rstl::reserved_vector< CVector3f, 4 > mCoords;
  rstl::reserved_vector< CVector2f, 4 > mUvs;
  CVector2f mTileSize;
  float mInterval;
  float mFrameTimer;
  float mFadeDuration;
  float mDeResFactor;
  float mFlashFactor;
};
CHECK_SIZEOF(CAuiImagePane, 0x154)

#endif // _CAUIIMAGEPANE
