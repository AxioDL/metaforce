#pragma once

#include "CGuiGroup.hpp"

namespace urde {
class CSimplePool;

class CAuiMeter : public CGuiGroup {
  bool xc4_noRoundUp;
  s32 xc8_maxCapacity;
  s32 xcc_capacity;
  s32 xd0_value = 0;
  std::vector<CGuiGroup*> xd4_workers;
  void UpdateMeterWorkers();

public:
  CAuiMeter(const CGuiWidgetParms& parms, bool noRoundUp, u32 maxCapacity, u32 workerCount);
  FourCC GetWidgetTypeID() const override { return FOURCC('METR'); }

  void OnVisibleChange() override;
  void SetCurrValue(s32 val);
  void SetCapacity(s32 cap);
  void SetMaxCapacity(s32 cap);
  CGuiWidget* GetWorkerWidget(int id) const override;
  bool AddWorkerWidget(CGuiWidget* worker) override;

  static std::shared_ptr<CGuiWidget> Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
};

} // namespace urde
