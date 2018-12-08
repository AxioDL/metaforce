#include "CGameOptionsTouchBar.hpp"

namespace urde {

CGameOptionsTouchBar::EAction CGameOptionsTouchBar::PopAction() { return EAction::None; }
void CGameOptionsTouchBar::GetSelection(int& left, int& right, int& value) { left = -1, right = -1, value = -1; }
void CGameOptionsTouchBar::SetSelection(int left, int right, int value) {}

#ifndef __APPLE__
std::unique_ptr<CGameOptionsTouchBar> NewGameOptionsTouchBar() { return std::make_unique<CGameOptionsTouchBar>(); }
#endif

} // namespace urde
