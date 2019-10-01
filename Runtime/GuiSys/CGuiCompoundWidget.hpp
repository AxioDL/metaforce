#pragma once

#include "Runtime/GuiSys/CGuiWidget.hpp"

namespace urde {

class CGuiCompoundWidget : public CGuiWidget {
public:
  CGuiCompoundWidget(const CGuiWidgetParms& parms);
  FourCC GetWidgetTypeID() const override { return FourCC(-1); }

  void OnVisibleChange() override;
  void OnActiveChange() override;
  virtual CGuiWidget* GetWorkerWidget(int id) const;
};

} // namespace urde
