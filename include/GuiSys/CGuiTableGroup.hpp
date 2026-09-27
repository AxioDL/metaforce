#ifndef _CGUITABLEGROUP
#define _CGUITABLEGROUP

#include "GuiSys/CGuiCompoundWidget.hpp"

#include "Kyoto/TFunctor.hpp"

class CColor;

class CGuiTableGroup : public CGuiCompoundWidget {
public:
  enum ETableSelectReturn { kTSR_Changed, kTSR_Unchanged, kTSR_WrappedAround };
  class CRepeatState {
  public:
    CRepeatState();
    const bool Update(float dt, bool state);

  private:
    float mTimer;
  };

  ~CGuiTableGroup() override;
  bool AddWorkerWidget(CGuiWidget* worker) override { return true; }
  FourCC GetWidgetTypeID() const override { return 'TBGP'; }
  void OnActivate() override;
  virtual void SetSelectionToDefault();

  static CGuiTableGroup* Create(CGuiFrame* frame, CInputStream& in, CSimplePool* pool);

  CGuiTableGroup(const CGuiWidgetParms& parms, int elementCount, int defSel, bool selectWrapAround);

  void ProcessUserInput(const CFinalInput& input) override;

  void SetMenuAdvanceCallback(const TFunctor1< CGuiTableGroup* const >& func);
  void SetMenuCancelCallback(const TFunctor1< CGuiTableGroup* const >& func);
  void SetMenuSelectionChangeCallback(const TFunctor2< CGuiTableGroup* const, const int >& func);

  void SetColors(const CColor& selected, const CColor& unselected);
  void SelectWorker(int worker);
  bool IsWorkerSelectable(int worker);
  int GetUserSelection() const { return mUserSelection; }
  int GetElementCount() const { return mElementCount; }
  void SetUserSelection(int sel) {
    mPrevUserSelection = mUserSelection;
    mUserSelection = sel;
  }
  void SetVertical(bool v) { mVertical = v; }

  bool HasMenuAdvanceCallback() const { return mDoMenuAdvance; }

private:
  bool DoAdvance();
  bool DoCancel();
  bool DoDecrement();
  bool DoIncrement();

  bool PreDecrement();
  bool PreIncrement();

  void ActivateWorker(CGuiWidget* worker);
  void DeactivateWorker(CGuiWidget* worker);
  ETableSelectReturn IncrementSelectedRow();
  ETableSelectReturn DecrementSelectedRow();
  void DoSelectNextRow();
  void DoSelectPrevRow();

  CRepeatState mDecRepeat;
  CRepeatState mIncRepeat;
  int mElementCount;
  int mUserSelection;
  int mPrevUserSelection;
  int mDefaultUserSelection;
  bool mSelectWrapAround;
  bool mVertical;
  TFunctor1< CGuiTableGroup* const > mDoMenuAdvance;
  TFunctor1< CGuiTableGroup* const > mDoMenuCancel;
  TFunctor2< CGuiTableGroup* const, const int > mDoMenuSelChange;
};

CHECK_SIZEOF(CGuiTableGroup, 0x11c)

#endif // _CGUITABLEGROUP
