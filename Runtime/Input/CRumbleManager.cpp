#include "Runtime/Input/CRumbleManager.hpp"

#include "Runtime/CGameState.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Input/RumbleFxTable.hpp"
#include "Runtime/World/CPlayer.hpp"

namespace metaforce {

s16 CRumbleManager::Rumble(CStateManager& mgr, const zeus::CVector3f& pos, ERumbleFxId fx, float dist,
                           ERumblePriority priority) {
  if (zeus::close_enough(dist, 0.f))
    return -1;
  zeus::CVector3f delta = mgr.GetPlayer().GetTranslation() - pos;
  if (delta.magSquared() < dist * dist)
    return Rumble(mgr, fx, 1.f - delta.magnitude() / dist, priority);
  return -1;
}

s16 CRumbleManager::Rumble(CStateManager& mgr, ERumbleFxId fx, float gain, ERumblePriority priority) {
  if (g_GameState->GameOptions().GetIsRumbleEnabled())
    return x0_rumbleGenerator.Rumble(RumbleFxTable[size_t(fx)], gain, priority, EIOPort::Player1);
  return -1;
}

} // namespace metaforce
