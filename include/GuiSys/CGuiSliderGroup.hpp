#ifndef _CGUISLIDERGROUP
#define _CGUISLIDERGROUP

#include "GuiSys/CGuiCompoundWidget.hpp"
#include "Kyoto/TFunctor.hpp"
#include "rstl/reserved_vector.hpp"

class CGuiFunctionDef;
class CGuiControllerInfo;

class CGuiSliderGroup : public CGuiCompoundWidget {
public:
  enum EState { kS_None, kS_Decreasing, kS_Increasing };

  ~CGuiSliderGroup() {}
  FourCC GetWidgetTypeID() const override { return 'SLGP'; }
  void Update(float dt) override;
  void ProcessUserInput(const CFinalInput& input) override;
  bool AddWorkerWidget(CGuiWidget* worker) override;
  CGuiWidget* GetWorkerWidget(int id) override;

  CGuiSliderGroup(const CGuiWidgetParms& parms, float min, float max, float cur, float increment);
  static CGuiWidget* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);

  void SetSelectionChangedCallback(
      const TFunctor2< CGuiSliderGroup* const, const float >& callback);
  int MAF_Decrement(CGuiFunctionDef* func, CGuiControllerInfo* info);
  int MAF_Increment(CGuiFunctionDef* func, CGuiControllerInfo* info);
  void SetIncrement(float increment);
  void SetMinVal(float min);
  void SetMaxVal(float max);
  void SetCurVal(float cur);
  float GetCurVal() const { return mRoundedCurVal; }
  EState GetState() const { return mState; }

private:
  float mMinVal;
  float mMaxVal;
  float mRoundedCurVal;
  float mCurVal;
  float mIncrement;
  rstl::reserved_vector< CGuiWidget*, 2 > mSliderRangeWidgets;
  TFunctor2< CGuiSliderGroup* const, const float > mChangeCallback;
  EState mState;
  bool mInputPending : 1;
};

CHECK_SIZEOF(CGuiSliderGroup, 0xf8)

#endif // _CGUISLIDERGROUP
