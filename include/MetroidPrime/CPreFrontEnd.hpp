#ifndef _CPREFRONTEND
#define _CPREFRONTEND

#include "MetroidPrime/CIOWin.hpp"

class CPreFrontEnd : public CIOWin {
public:
  CPreFrontEnd();
  ~CPreFrontEnd();

  EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&);
  void Draw() const;

private:
  bool mResourceTweaksRegistered;
};

#endif // _CPREFRONTEND
