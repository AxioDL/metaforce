#ifndef _CAUIMETER
#define _CAUIMETER

#include <GuiSys/CGuiGroup.hpp>
#include <rstl/vector.hpp>

class CAuiMeter : public CGuiGroup {
public:
  CAuiMeter(const CGuiWidgetParms& parms, const bool b, const int w1, const int w2);
  bool AddWorkerWidget(CGuiWidget* widget) override;
  CGuiWidget* GetWorkerWidget(int idx) override;
  void SetMaxCapacity(int cap);
  void SetCapacity(int cap);
  void SetCurrValue(int rem);
  void OnVisible() override;
  void UpdateMeterWorkers();
  FourCC GetWidgetTypeID() const override { return 'METR'; }

  static CGuiWidget* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);

private:
  int test(float scale, float value) {
    if (mNoRoundUp) {
      return scale * value;
    }

    return 0.5f + scale * value;
  }
  bool mNoRoundUp;
  int mMaxCapacity;
  int mCapacity;
  int mValue;
  rstl::vector< CGuiGroup* > mWorkers;
};

#endif // _CAUIMETER
