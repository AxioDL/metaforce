#include "CAutoSave.hpp"
#include "CSaveGameScreen.hpp"
#include "Runtime/MP1/MP1.hpp"

namespace urde::MP1 {
CAutoSave::CAutoSave()
: CIOWin("AutoSave"sv), x14_savegameScreen(new CSaveGameScreen(ESaveContext::InGame, g_GameState->GetCardSerial())) {
  static_cast<MP1::CMain*>(g_Main)->RefreshGameState();
}
void CAutoSave::Draw() {
  x14_savegameScreen->Draw();
}
CIOWin::EMessageReturn CAutoSave::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue) {
  if (g_GameState->GetCardSerial() == 0ull) {
    return EMessageReturn ::RemoveIOWinAndExit;
  }

  if (msg.GetType() == EArchMsgType::UserInput) {
    x14_savegameScreen->ProcessUserInput(MakeMsg::GetParmUserInput(msg).x4_parm);
  } else if (msg.GetType() == EArchMsgType::TimerTick){
    auto ret = x14_savegameScreen->Update(MakeMsg::GetParmTimerTick(msg).x4_parm);
    if (ret != EMessageReturn::Exit) {
      return EMessageReturn::RemoveIOWinAndExit;
    }
  }

  return EMessageReturn::Exit;
}
} // namespace urde::MP1
