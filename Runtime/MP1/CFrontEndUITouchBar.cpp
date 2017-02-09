#include "CFrontEndUITouchBar.hpp"

namespace urde
{

CFrontEndUITouchBar::~CFrontEndUITouchBar() {}
void CFrontEndUITouchBar::SetPhase(EPhase ph) {}
void CFrontEndUITouchBar::SetFileSelectPhase(const SFileSelectDetail details[3],
                                             bool eraseGame, bool galleryActive) {}
CFrontEndUITouchBar::EAction CFrontEndUITouchBar::PopAction() { return EAction::None; }

#ifndef __APPLE__
std::unique_ptr<CFrontEndUITouchBar> NewFrontEndUITouchBar()
{
    return std::make_unique<CFrontEndUITouchBar>();
}
#endif

}
