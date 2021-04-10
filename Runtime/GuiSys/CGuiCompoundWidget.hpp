#pragma once

#include "Runtime/GuiSys/CGuiWidget.hpp"

namespace metaforce {

class CGuiCompoundWidget : public CGuiWidget {
public:
  explicit CGuiCompoundWidget(const CGuiWidgetParms& parms);
  FourCC GetWidgetTypeID() const override { return FourCC(-1); }

  void OnVisibleChange() override;
  void OnActiveChange() override;
  virtual CGuiWidget* GetWorkerWidget(int id) const;
};

} // namespace metaforce
