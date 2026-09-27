#include "GuiSys/CAuiMeter.hpp"
#include "GuiSys/CGuiGroup.hpp"
#include "GuiSys/CGuiWidget.hpp"
#include <Kyoto/Math/CMath.hpp>
#include <Kyoto/Streams/CInputStream.hpp>
#include <rstl/math.hpp>

CGuiWidget* CAuiMeter::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);
  in.ReadBool();
  const bool noRoundUp = in.ReadBool();
  const int maxCapacity = in.ReadInt32();
  const int workerCount = in.ReadInt32();
  CGuiWidget* widget = rs_new CAuiMeter(parms, noRoundUp, maxCapacity, workerCount);
  widget->ParseBaseInfo(frame, in, parms);
  return widget;
}

CAuiMeter::CAuiMeter(const CGuiWidgetParms& parms, const bool noRoundUp, const int maxCapacity,
                     const int workerCount)
: CGuiGroup(parms, 0, false)
, mNoRoundUp(noRoundUp)
, mMaxCapacity(maxCapacity)
, mCapacity(mMaxCapacity)
, mValue(0) {
  mWorkers.reserve(workerCount);
}

bool CAuiMeter::AddWorkerWidget(CGuiWidget* worker) {
  CGuiGroup::AddWorkerWidget(worker);
  short id = worker->GetWorkerId();
  if (id >= mWorkers.size()) {
    for (int i = mWorkers.size(); i <= id; ++i) {
      mWorkers.push_back(nullptr);
    }
  }
  mWorkers[id] = static_cast< CGuiGroup* >(worker);
  return true;
}

CGuiWidget* CAuiMeter::GetWorkerWidget(int idx) { return mWorkers[idx]; }

// TODO: Why is the only needed for the first one?
static inline const int& hack_min(const int& a, const int& b) { return a < b ? b : a; }

void CAuiMeter::SetMaxCapacity(const int cap) {
  mMaxCapacity = hack_min(0, cap);
  mCapacity = rstl::min_val(mCapacity, mMaxCapacity);
  mValue = rstl::min_val(mValue, mCapacity);
  UpdateMeterWorkers();
}

void CAuiMeter::SetCapacity(int cap) {
  mCapacity = CMath::Clamp(0, cap, mMaxCapacity);
  mValue = rstl::min_val(mValue, mCapacity);
  UpdateMeterWorkers();
}

void CAuiMeter::SetCurrValue(int rem) {
  mValue = CMath::Clamp(0, rem, mCapacity);
  UpdateMeterWorkers();
}

void CAuiMeter::OnVisible() {
  if (GetIsVisible()) {
    UpdateMeterWorkers();
  }
}

void CAuiMeter::UpdateMeterWorkers() {
  int workerCount = mWorkers.size();
  const float scale = workerCount / float(mMaxCapacity);
  int etankCap = mNoRoundUp ? static_cast< int >(scale * mCapacity)
                               : static_cast< int >(0.5f + scale * mCapacity);
  int etankFill = mNoRoundUp ? static_cast< int >(scale * mValue)
                                : static_cast< int >(0.5f + scale * mValue);
  for (int i = 0; i < workerCount; ++i) {
    CGuiGroup* worker = mWorkers[i];
    if (!worker)
      continue;

    CGuiWidget* tanks[2];
    tanks[0] = worker->GetWorkerWidget(0);
    tanks[1] = worker->GetWorkerWidget(1);

    if (i < etankFill) {
      if (tanks[0])
        tanks[0]->SetIsVisible(true);
      if (tanks[1])
        tanks[1]->SetIsVisible(false);
    } else if (i < etankCap) {
      if (tanks[0])
        tanks[0]->SetIsVisible(false);
      if (tanks[1])
        tanks[1]->SetIsVisible(true);
    } else {
      if (tanks[0])
        tanks[0]->SetIsVisible(false);
      if (tanks[1])
        tanks[1]->SetIsVisible(false);
    }
  }
}
