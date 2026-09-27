#ifndef _CMFGAMELOADER
#define _CMFGAMELOADER

#include "Kyoto/CToken.hpp"
#include "MetroidPrime/CIOWin.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/vector.hpp"

class CStateManager;
class CInGameGuiManager;

class CMFGameLoader : public CIOWin {

public:
  CMFGameLoader();
  ~CMFGameLoader() override;
  EMessageReturn OnMessage(const CArchitectureMessage& message, CArchitectureQueue& queue) override;
  void Draw() const override;

  void MakeLoadDependancyList();

private:
  rstl::ncrc_ptr< CStateManager > mStateManager;
  rstl::ncrc_ptr< CInGameGuiManager > mGuiManager;
  rstl::vector< CToken > mLoadList;
  bool mInitialized : 1;
  bool mTransitionFinished : 1;
};
CHECK_SIZEOF(CMFGameLoader, 0x30)

#endif // _CMFGAMELOADER
