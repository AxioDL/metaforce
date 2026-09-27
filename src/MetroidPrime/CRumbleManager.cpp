#include "MetroidPrime/CRumbleManager.hpp"

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/RumbleFxTable.hpp"


#include "Kyoto/Math/CloseEnough.hpp"

CRumbleManager::CRumbleManager() {}

CRumbleManager::~CRumbleManager() { mRumbleGenerator.HardStopAll(); }

short CRumbleManager::Rumble(CStateManager& mgr, ERumbleFxId fx, float gain,
                             ERumblePriority priority) {
  if (gpGameState->GameOptions().GetIsRumbleEnabled())
    return mRumbleGenerator.Rumble(skRumbleFxTable[size_t(fx)], gain, priority, kIOP_Player1);
  return -1;
}

short CRumbleManager::Rumble(CStateManager& mgr, const CVector3f& pos, ERumbleFxId fx, float dist,
                             ERumblePriority priority) {
  if (!close_enough(dist, 0.f))
  {
    CVector3f delta = mgr.GetPlayer()->GetTranslation() - pos;
    if (delta.MagSquared() < dist * dist)
      return Rumble(mgr, fx, 1.f - delta.Magnitude() / dist, priority);
  }

  return -1;
}

void CRumbleManager::StopRumble(short id) {
  if (id == -1)
    return;
  mRumbleGenerator.Stop(id, kIOP_Player1);
}

void CRumbleManager::Update(float dt) { mRumbleGenerator.Update(dt); }
