#ifndef _CCONSOLEOUTPUTWINDOW
#define _CCONSOLEOUTPUTWINDOW

#include "MetroidPrime/CIOWin.hpp"

#include "Kyoto/Text/CFont.hpp"

#include <rstl/vector.hpp>

class CConsoleOutputWindow : public CIOWin {
  static CConsoleOutputWindow* mInstance;
public:
  CConsoleOutputWindow(int, float, float);

  ~CConsoleOutputWindow() override;
  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) override;
  void Update(float);
  void Draw() const override;
private:
  CFont mFont;
  float mUnk;
  rstl::vector<rstl::string> mText;
  rstl::vector<float> mUnkFloats;
  int x40_;
  int x44_;
  int x48_;
};

#endif // _CCONSOLEOUTPUTWINDOW
