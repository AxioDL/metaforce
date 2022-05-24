#pragma once

#include <array>

#include "Runtime/CToken.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/GuiSys/CGuiWidget.hpp"

#include <zeus/CVector2f.hpp>
#include <zeus/CVector3f.hpp>

namespace zeus {
class CColor;
}

namespace metaforce {
class CSimplePool;
class CTexture;

class CAuiImagePane : public CGuiWidget {
  TLockedToken<CTexture> xb8_tex0Tok; // Used to be optional
  CAssetId xc8_tex0;
  CAssetId xcc_tex1;
  zeus::CVector2f xd0_uvBias0;
  zeus::CVector2f xd8_uvBias1;
  rstl::reserved_vector<zeus::CVector3f, 4> xe0_coords;
  rstl::reserved_vector<zeus::CVector2f, 4> x114_uvs;
  zeus::CVector2f x138_tileSize;
  float x140_interval = 0.f;
  float x144_frameTimer = 0.f;
  float x148_fadeDuration = 0.f;
  float x14c_deResFactor = 0.f;
  float x150_flashFactor = 0.f;
  void DoDrawImagePane(const zeus::CColor& color, CTexture& tex, int frame, float blurAmt, bool noBlur) const;

public:
  CAuiImagePane(const CGuiWidgetParms& parms, CSimplePool* sp, CAssetId, CAssetId,
                rstl::reserved_vector<zeus::CVector3f, 4>&& coords, rstl::reserved_vector<zeus::CVector2f, 4>&& uvs,
                bool initTex);
  FourCC GetWidgetTypeID() const override { return FOURCC('IMGP'); }
  static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);

  void Reset(ETraversalMode mode) override;
  void Update(float dt) override;
  void Draw(const CGuiWidgetDrawParms& params) override;
  bool GetIsFinishedLoadingWidgetSpecific() override;
  void SetTextureID0(CAssetId tex, CSimplePool* sp);
  void SetAnimationParms(const zeus::CVector2f& vec, float interval, float duration);
  void SetDeResFactor(float d) { x14c_deResFactor = d; }
  void SetFlashFactor(float t) { x150_flashFactor = t; }
};
} // namespace metaforce
