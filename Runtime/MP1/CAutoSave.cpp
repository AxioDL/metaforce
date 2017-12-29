#include "CAutoSave.hpp"
#include "CSaveGameScreen.hpp"
#include "IMain.hpp"

namespace urde::MP1
{
CAutoSave::CAutoSave()
: CIOWin("")
, x14_savegameScreen(new CSaveGameScreen(ESaveContext::InGame, g_GameState->GetCardSerial()))
{
}
}
