#include "Runtime/MP1/CPreFrontEnd.hpp"

#include "Runtime/CResLoader.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/MP1/MP1.hpp"

namespace urde::MP1 {

CPreFrontEnd::CPreFrontEnd() : CIOWin("Pre front-end window") {}

CIOWin::EMessageReturn CPreFrontEnd::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue&) {
  if (msg.GetType() != EArchMsgType::TimerTick)
    return EMessageReturn::Normal;

  CMain* m = static_cast<CMain*>(g_Main);
  if (CResLoader* loader = g_ResFactory->GetResLoader())
    if (!loader->AreAllPaksLoaded())
      return EMessageReturn::Exit;
  if (!x14_resourceTweaksRegistered) {
    m->RegisterResourceTweaks();
    x14_resourceTweaksRegistered = true;
  }
  m->MemoryCardInitializePump();
  if (!g_MemoryCardSys)
    return EMessageReturn::Exit;
  if (!m->LoadAudio())
    return EMessageReturn::Exit;
  return EMessageReturn::RemoveIOWinAndExit;
}

} // namespace urde::MP1
