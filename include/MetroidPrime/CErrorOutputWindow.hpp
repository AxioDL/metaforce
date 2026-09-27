#ifndef _CERROROUTPUTWINDOW
#define _CERROROUTPUTWINDOW

#include "types.h"

#include "MetroidPrime/CIOWin.hpp"

class CErrorOutputWindow : public CIOWin {
public:
  enum EFlag {
    kF_Zero,
    kF_One,
  };

  CErrorOutputWindow(EFlag);
  ~CErrorOutputWindow() override {}

  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  bool GetIsContinueDraw() const override;
  void Draw() const override;

  void UpdateWindow();
  void Update();
  void ShowMessage() const;

private:
  enum EState {
    kS_Zero,
    kS_One,
    kS_Two,
    kS_Three,
  };

  EState mState;
  bool x18_24_ : 1;
  bool x18_25_ : 1;
  bool x18_26_ : 1;
  bool x18_27_ : 1;
  bool x18_28_ : 1;
  const wchar_t* mMsg;

  void SetState(EState);
  void DrawError() const;
};

CHECK_SIZEOF(CErrorOutputWindow, 0x20)

#endif // _CERROROUTPUTWINDOW
