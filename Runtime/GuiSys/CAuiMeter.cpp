#include "Runtime/GuiSys/CAuiMeter.hpp"

#include <algorithm>
#include <memory>

#include <zeus/Math.hpp>

namespace urde {

CAuiMeter::CAuiMeter(const CGuiWidgetParms& parms, bool noRoundUp, u32 maxCapacity, u32 workerCount)
: CGuiGroup(parms, 0, false), xc4_noRoundUp(noRoundUp), xc8_maxCapacity(maxCapacity), xcc_capacity(maxCapacity) {
  xd4_workers.reserve(workerCount);
}

void CAuiMeter::UpdateMeterWorkers() {
  float scale = xd4_workers.size() / float(xc8_maxCapacity);

  size_t etankCap;
  if (xc4_noRoundUp)
    etankCap = xcc_capacity * scale;
  else
    etankCap = xcc_capacity * scale + 0.5f;

  size_t etankFill;
  if (xc4_noRoundUp)
    etankFill = xd0_value * scale;
  else
    etankFill = xd0_value * scale + 0.5f;

  for (size_t i = 0; i < xd4_workers.size(); ++i) {
    CGuiGroup* worker = xd4_workers[i];
    if (!worker)
      continue;

    CGuiWidget* fullTank = worker->GetWorkerWidget(0);
    CGuiWidget* emptyTank = worker->GetWorkerWidget(1);

    if (i < etankFill) {
      if (fullTank)
        fullTank->SetIsVisible(true);
      if (emptyTank)
        emptyTank->SetIsVisible(false);
    } else if (i < etankCap) {
      if (fullTank)
        fullTank->SetIsVisible(false);
      if (emptyTank)
        emptyTank->SetIsVisible(true);
    } else {
      if (fullTank)
        fullTank->SetIsVisible(false);
      if (emptyTank)
        emptyTank->SetIsVisible(false);
    }
  }
}

void CAuiMeter::OnVisibleChange() {
  if (GetIsVisible())
    UpdateMeterWorkers();
}

void CAuiMeter::SetCurrValue(s32 val) {
  xd0_value = zeus::clamp(0, val, xcc_capacity);
  UpdateMeterWorkers();
}

void CAuiMeter::SetCapacity(s32 cap) {
  xcc_capacity = zeus::clamp(0, cap, xc8_maxCapacity);
  xd0_value = std::min(xcc_capacity, xd0_value);
  UpdateMeterWorkers();
}

void CAuiMeter::SetMaxCapacity(s32 cap) {
  xc8_maxCapacity = std::max(0, cap);
  xcc_capacity = std::min(xc8_maxCapacity, xcc_capacity);
  xd0_value = std::min(xcc_capacity, xd0_value);
  UpdateMeterWorkers();
}

CGuiWidget* CAuiMeter::GetWorkerWidget(int id) const { return xd4_workers[id]; }

bool CAuiMeter::AddWorkerWidget(CGuiWidget* worker) {
  CGuiGroup::AddWorkerWidget(worker);
  if (worker->GetWorkerId() >= xd4_workers.size())
    xd4_workers.resize(worker->GetWorkerId() + 1);
  xd4_workers[worker->GetWorkerId()] = static_cast<CGuiGroup*>(worker);
  return true;
}

std::shared_ptr<CGuiWidget> CAuiMeter::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  in.readBool();
  bool noRoundUp = in.readBool();
  u32 maxCapacity = in.readUint32Big();
  u32 workerCount = in.readUint32Big();
  std::shared_ptr<CAuiMeter> ret = std::make_shared<CAuiMeter>(parms, noRoundUp, maxCapacity, workerCount);
  ret->ParseBaseInfo(frame, in, parms);
  return ret;
}

} // namespace urde
