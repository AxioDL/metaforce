#pragma once

#include <memory>
#include <vector>

#include "Runtime/GuiSys/CGuiWidget.hpp"

#include <zeus/CVector2f.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {

class CGuiPane : public CGuiWidget {
  static constexpr zeus::CVector3f skDefaultNormal{0.f, -1.f, 0.f};
protected:
  zeus::CVector2f xb8_dim;

  /* Originally a vert-buffer pointer for GX */
  std::array<zeus::CVector3f, 4> xc0_verts;

  zeus::CVector3f xc8_scaleCenter;

public:
  CGuiPane(const CGuiWidgetParms& parms, const zeus::CVector2f& dim, const zeus::CVector3f& scaleCenter);
  FourCC GetWidgetTypeID() const override { return FOURCC('PANE'); }

  void Draw(const CGuiWidgetDrawParms& parms) override;
  virtual void ScaleDimensions(const zeus::CVector3f& scale);
  virtual void SetDimensions(const zeus::CVector2f& dim, bool initVBO);
  virtual zeus::CVector2f GetDimensions() const;
  virtual void InitializeBuffers();
  virtual void WriteData(COutputStream& out, bool flag) const;

  static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
};

} // namespace metaforce
