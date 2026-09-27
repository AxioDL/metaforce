#ifndef _CRUMBLEMANAGER
#define _CRUMBLEMANAGER

#include "types.h"

#include "Kyoto/Input/CRumbleGenerator.hpp"

class CStateManager;
class CVector3f;

class CRumbleManager {
public:
  CRumbleManager();
  ~CRumbleManager();

  short Rumble(CStateManager&, const CVector3f&, ERumbleFxId, float, ERumblePriority);
  short Rumble(CStateManager&, ERumbleFxId, float, ERumblePriority);

  void StopRumble(short id);
  void HardStopAll() { mRumbleGenerator.HardStopAll(); }
  void Update(float dt);

private:
  CRumbleGenerator mRumbleGenerator;
};

#endif // _CRUMBLEMANAGER
