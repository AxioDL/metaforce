#ifndef _CGUIPANE
#define _CGUIPANE

#include "GuiSys/CGuiWidget.hpp"
#include "rstl/reserved_vector.hpp"
#include "Kyoto/TFunctor.hpp"

class CGuiPane : public CGuiWidget {
public:
  static CGuiWidget* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
  CGuiPane(const CGuiWidgetParms& parms, float width, float height, const CVector3f& scaleCenter);
  ~CGuiPane();

  void Draw(const CGuiWidgetDrawParms& parms) const override;
  virtual void ScaleDimensions(const CVector3f& scale);
  virtual void SetDimensions(const CVector2f& dim, bool initVBO);
  virtual CVector2f GetDimensions() const;
  virtual void InitializeBuffers();
  virtual void WriteData(COutputStream& out, bool flag) const;
  int GetCount() const { return mPanePointCount * 3; }

  const float* GetVtxBuf() const { return mPanePoints; }
  const CVector3f& GetPivot() const { return mScaleCenter; }
  float GetWidth() const { return mWidth; }
  float GetHeight() const { return mHeight; }

  FourCC GetWidgetTypeID() const override;

  CVector3f& ScaleCenter() { return mScaleCenter; }
  const CVector3f& GetScaleCenter() const { return mScaleCenter; }

  const int GetPointCount() const { return mPanePointCount; }

private:
  float mWidth;
  float mHeight;
  float* mPanePoints;
  int mPanePointCount;
  CVector3f mScaleCenter;
};
CHECK_SIZEOF(CGuiPane, 0xd4)

#endif // _CGUIPANE
