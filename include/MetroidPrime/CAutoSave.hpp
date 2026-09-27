#ifndef _CAUTOSAVE
#define _CAUTOSAVE

#include "MetroidPrime/CIOWin.hpp"
#include "rstl/single_ptr.hpp"

class CSaveGameScreen;

class CAutoSave : public CIOWin {
public:
  ~CAutoSave() override;
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  bool GetIsContinueDraw() const override;
  void Draw() const override;

  CAutoSave();

private:
  rstl::single_ptr< CSaveGameScreen > mSaveGameScreen;
};
CHECK_SIZEOF(CAutoSave, 0x18)

#endif // _CAUTOSAVE
