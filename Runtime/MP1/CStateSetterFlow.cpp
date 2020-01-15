#include "Runtime/MP1/CStateSetterFlow.hpp"

#include "Runtime/CArchitectureMessage.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/MP1/MP1.hpp"

namespace urde::MP1 {

CStateSetterFlow::CStateSetterFlow() : CIOWin("") {}

CIOWin::EMessageReturn CStateSetterFlow::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) {
  if (msg.GetType() == EArchMsgType::TimerTick) {
    MP1::CMain* m = static_cast<MP1::CMain*>(g_Main);

    if (m->m_warpWorldIdx > -1) {
      CResLoader* loader = g_ResFactory->GetResLoader();
      CAssetId worldId;
      for (const auto& pak : loader->GetPaks()) {
        if (*(pak->GetPath().end() - 6) == '0' + m->m_warpWorldIdx) {
          worldId = pak->GetMLVLId();
          break;
        }
      }

      if (worldId.IsValid()) {
        m->ResetGameState();

        g_GameState->SetCurrentWorldId(worldId);
        CWorldState& ws = g_GameState->StateForWorld(worldId);
        CWorldLayerState& layers = *ws.GetLayerState();
        if (m->m_warpAreaId < layers.GetAreaCount()) {
          ws.SetAreaId(m->m_warpAreaId);
          if (m->m_warpLayerBits) {
            for (u32 i = 0; i < layers.GetAreaLayerCount(m->m_warpAreaId); ++i)
              layers.SetLayerActive(m->m_warpAreaId, i, ((m->m_warpLayerBits >> i) & 1) != 0);
          }
          CRelayTracker& relays = *ws.RelayTracker();
          for (const auto& r : m->m_warpMemoryRelays)
            relays.AddRelay(r);
        }
        g_GameState->GameOptions().ResetToDefaults();
        g_GameState->WriteBackupBuf();
        return EMessageReturn::RemoveIOWinAndExit;
      }
    }

    m->RefreshGameState();
    return EMessageReturn::RemoveIOWinAndExit;
  }
  return EMessageReturn::Exit;
}

} // namespace urde::MP1
