#ifndef _CGUIGROUP
#define _CGUIGROUP

#include "GuiSys/CGuiCompoundWidget.hpp"

class CGuiGroup : public CGuiCompoundWidget {
public:
  CGuiGroup(const CGuiWidgetParms& parms, int defaultWorker, bool b);
  static CGuiWidget* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* sp);
  FourCC GetWidgetTypeID() const override { return 'GRUP'; }

  void SelectWorkerWidget(int workerId, bool setActive, bool setVisible);
  CGuiWidget* GetSelectedWidget();
  const CGuiWidget* GetSelectedWidget() const;
  bool AddWorkerWidget(CGuiWidget* worker) override;
  void OnActivate() override;
private:
  uint mWorkerCount;
  int mSelectedWorker;
  bool mB;
};

CHECK_SIZEOF(CGuiGroup, 0xc4)

#endif // _CGUIGROUP
