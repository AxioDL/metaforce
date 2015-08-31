#include "CFrontEndUI.hpp"

namespace Retro
{
namespace MP1
{

CFrontEndUI::CFrontEndUI(CArchitectureQueue& queue)
: CIOWin("CFrontEnd")
{}
void CFrontEndUI::OnSliderSelectionChange(CGuiSliderGroup* grp, float)
{}
void CFrontEndUI::OnCheckBoxSelectionChange(CGuiTableGroup* grp)
{}
void CFrontEndUI::OnOptionSubMenuCancel(CGuiTableGroup* grp)
{}
void CFrontEndUI::OnOptionsMenuCancel(CGuiTableGroup* grp)
{}
void CFrontEndUI::OnNewGameMenuCancel(CGuiTableGroup* grp)
{}
void CFrontEndUI::OnFileMenuCancel(CGuiTableGroup* grp)
{}
void CFrontEndUI::OnGenericMenuSelectionChange(CGuiTableGroup* grp, int, int)
{}
void CFrontEndUI::OnOptionsMenuAdvance(CGuiTableGroup* grp)
{}
void CFrontEndUI::OnNewGameMenuAdvance(CGuiTableGroup* grp)
{}
void CFrontEndUI::OnFileMenuAdvance(CGuiTableGroup* grp)
{}
void CFrontEndUI::OnMainMenuAdvance(CGuiTableGroup* grp)
{}
const char* CFrontEndUI::GetAttractMovieFileName(int idx)
{return nullptr;}
const char* CFrontEndUI::GetNextAttractMovieFileName(int idx)
{return nullptr;}
void CFrontEndUI::SetCurrentMovie(EMenuMovie movie)
{}
void CFrontEndUI::StopAttractMovie()
{}
void CFrontEndUI::StartAttractMovie(int idx)
{}
void CFrontEndUI::UpdateMenuHighlights(CGuiTableGroup* grp)
{}
void CFrontEndUI::CompleteStateTransition()
{}
bool CFrontEndUI::CanBuild(const SObjectTag& tag)
{return false;}
void CFrontEndUI::StartStateTransition(EScreen screen)
{}
void CFrontEndUI::HandleDebugMenuReturnValue(CGameDebug::EReturnValue val, CArchitectureQueue& queue)
{}
void CFrontEndUI::Draw() const
{}
void CFrontEndUI::UpdateMovies(float dt)
{}
void CFrontEndUI::Update(float dt, CArchitectureQueue& queue)
{}
CIOWin::EMessageReturn CFrontEndUI::OnMessage(const CArchitectureMessage& msg, CArchitectureQueue& queue)
{return MsgRetNormal;}
void CFrontEndUI::StartGame()
{}
void CFrontEndUI::InitializeFrame()
{}

}
}
