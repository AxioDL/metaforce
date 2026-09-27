#ifndef _CINPUTGENERATOR
#define _CINPUTGENERATOR

#include "CArchitectureMessage.hpp"
#include "types.h"

#include "rstl/single_ptr.hpp"

#include "Kyoto/Input/IController.hpp"

class COsContext;
class IController;
class CArchitectureQueue;

class CInputGenerator {
public:
  CInputGenerator(COsContext*, float leftDiv, float rightDiv);
  void CreateUserInputMsg(CArchitectureQueue& queue, float dt, int i,
                          const CControllerGamepadData& cont);
  bool Update(float dt, CArchitectureQueue& queue);
  IController* GetController() const { return mController.get(); }

private:
  COsContext* mContext;
  rstl::single_ptr< IController > mController;
  bool mConnectedControllers[4];
  float mLeftDiv;
  float mRightDiv;
};
CHECK_SIZEOF(CInputGenerator, 0x14)

#endif // _CINPUTGENERATOR
