#include "Runtime/GuiSys/CGuiTableGroup.hpp"
#include "Runtime/Input/CFinalInput.hpp"

namespace metaforce {

bool CGuiTableGroup::CRepeatState::Update(float dt, bool state) {
  if (x0_timer == 0.f) {
    if (state) {
      x0_timer = 0.6f;
      return true;
    }
  } else {
    if (state) {
      x0_timer -= dt;
      if (x0_timer <= 0.f) {
        x0_timer = 0.05f;
        return true;
      }
    } else {
      x0_timer = 0.f;
    }
  }
  return false;
}

CGuiTableGroup::CGuiTableGroup(const CGuiWidgetParms& parms, int elementCount, int defaultSel, bool selectWraparound)
: CGuiCompoundWidget(parms)
, xc0_elementCount(elementCount)
, xc4_userSelection(defaultSel)
, xc8_prevUserSelection(defaultSel)
, xcc_defaultUserSelection(defaultSel)
, xd0_selectWraparound(selectWraparound) {}

void CGuiTableGroup::ProcessUserInput(const CFinalInput& input) {
  if (input.PA() || input.PSpecialKey(ESpecialKey::Enter)) {
    DoAdvance();
  } else if (input.PB() || input.PSpecialKey(ESpecialKey::Esc)) {
    DoCancel();
  } else {
    bool decrement;
    if (xd1_vertical)
      decrement = (input.DLAUp() || input.DDPUp());
    else
      decrement = (input.DLALeft() || input.DDPLeft());

    bool increment;
    if (xd1_vertical)
      increment = (input.DLADown() || input.DDPDown());
    else
      increment = (input.DLARight() || input.DDPRight());

    if (xb8_decRepeat.Update(input.DeltaTime(), decrement) && decrement) {
      DoDecrement();
      return;
    }

    if (xbc_incRepeat.Update(input.DeltaTime(), increment) && increment) {
      DoIncrement();
      return;
    }
  }
}

bool CGuiTableGroup::IsWorkerSelectable(int idx) const {
  if (CGuiWidget* widget = GetWorkerWidget(idx))
    return widget->GetIsSelectable();
  return false;
}

void CGuiTableGroup::SelectWorker(int idx) {
  idx = zeus::clamp(0, idx, xc0_elementCount - 1);
  if (idx < xc4_userSelection) {
    while (idx != xc4_userSelection)
      DoSelectPrevRow();
  } else {
    while (idx != xc4_userSelection)
      DoSelectNextRow();
  }
}

void CGuiTableGroup::DoSelectWorker(int worker) {
  if (worker == xc4_userSelection)
    return;
  if (IsWorkerSelectable(worker)) {
    int oldSel = xc4_userSelection;
    SelectWorker(worker);
    if (x104_doMenuSelChange)
      x104_doMenuSelChange(this, oldSel);
  }
}

void CGuiTableGroup::SetWorkersMouseActive(bool active) {
  CGuiWidget* child = static_cast<CGuiWidget*>(GetChildObject());
  while (child) {
    if (child->GetWorkerId() != -1)
      child->SetMouseActive(active);
    child = static_cast<CGuiWidget*>(child->GetNextSibling());
  }
}

void CGuiTableGroup::DeactivateWorker(CGuiWidget* widget) { widget->SetIsActive(false); }

void CGuiTableGroup::ActivateWorker(CGuiWidget* widget) { widget->SetIsActive(true); }

CGuiTableGroup::TableSelectReturn CGuiTableGroup::DecrementSelectedRow() {
  xc8_prevUserSelection = xc4_userSelection;
  --xc4_userSelection;
  if (xc4_userSelection < 0) {
    xc4_userSelection = xd0_selectWraparound ? xc0_elementCount - 1 : 0;
    return xd0_selectWraparound ? TableSelectReturn::WrappedAround : TableSelectReturn::Unchanged;
  }
  return TableSelectReturn::Changed;
}

CGuiTableGroup::TableSelectReturn CGuiTableGroup::IncrementSelectedRow() {
  xc8_prevUserSelection = xc4_userSelection;
  ++xc4_userSelection;
  if (xc4_userSelection >= xc0_elementCount) {
    xc4_userSelection = xd0_selectWraparound ? 0 : xc0_elementCount - 1;
    return xd0_selectWraparound ? TableSelectReturn::WrappedAround : TableSelectReturn::Unchanged;
  }
  return TableSelectReturn::Changed;
}

void CGuiTableGroup::DoSelectPrevRow() {
  DecrementSelectedRow();
  DeactivateWorker(GetWorkerWidget(xc8_prevUserSelection));
  ActivateWorker(GetWorkerWidget(xc4_userSelection));
}

void CGuiTableGroup::DoSelectNextRow() {
  IncrementSelectedRow();
  DeactivateWorker(GetWorkerWidget(xc8_prevUserSelection));
  ActivateWorker(GetWorkerWidget(xc4_userSelection));
}

void CGuiTableGroup::DoCancel() {
  if (xec_doMenuCancel)
    xec_doMenuCancel(this);
}

void CGuiTableGroup::DoAdvance() {
  if (xd4_doMenuAdvance)
    xd4_doMenuAdvance(this);
}

bool CGuiTableGroup::PreDecrement() {
  if (xd0_selectWraparound) {
    for (int sel = (xc4_userSelection + xc0_elementCount - 1) % xc0_elementCount; sel != xc4_userSelection;
         sel = (sel + xc0_elementCount - 1) % xc0_elementCount) {
      if (IsWorkerSelectable(sel)) {
        SelectWorker(sel);
        return true;
      }
    }

  } else {
    for (int sel = std::max(-1, xc4_userSelection - 1); sel >= 0; --sel) {
      if (IsWorkerSelectable(sel)) {
        SelectWorker(sel);
        return true;
      }
    }
  }

  return false;
}

void CGuiTableGroup::DoDecrement() {
  int oldSel = xc4_userSelection;
  if (!PreDecrement())
    return;
  if (x104_doMenuSelChange)
    x104_doMenuSelChange(this, oldSel);
}

bool CGuiTableGroup::PreIncrement() {
  if (xd0_selectWraparound) {
    for (int sel = (xc4_userSelection + 1) % xc0_elementCount; sel != xc4_userSelection;
         sel = (sel + 1) % xc0_elementCount) {
      if (IsWorkerSelectable(sel)) {
        SelectWorker(sel);
        return true;
      }
    }

  } else {
    for (int sel = std::min(xc0_elementCount, xc4_userSelection + 1); sel < xc0_elementCount; ++sel) {
      if (IsWorkerSelectable(sel)) {
        SelectWorker(sel);
        return true;
      }
    }
  }

  return false;
}

void CGuiTableGroup::DoIncrement() {
  int oldSel = xc4_userSelection;
  if (!PreIncrement())
    return;
  if (x104_doMenuSelChange)
    x104_doMenuSelChange(this, oldSel);
}

std::shared_ptr<CGuiWidget> CGuiTableGroup::Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp) {
  CGuiWidgetParms parms = ReadWidgetHeader(frame, in);

  int elementCount = in.ReadInt16();
  in.ReadInt16();
  in.ReadLong();
  int defaultSel = in.ReadInt16();
  in.ReadInt16();
  bool selectWraparound = in.ReadBool();
  in.ReadBool();
  in.ReadFloat();
  in.ReadFloat();
  in.ReadBool();
  in.ReadFloat();
  in.ReadInt16();
  in.ReadInt16();
  in.ReadInt16();
  in.ReadInt16();

  std::shared_ptr<CGuiWidget> ret = std::make_shared<CGuiTableGroup>(parms, elementCount, defaultSel, selectWraparound);
  ret->ParseBaseInfo(frame, in, parms);
  return ret;
}

} // namespace metaforce
