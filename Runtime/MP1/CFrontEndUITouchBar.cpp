#include "Runtime/MP1/CFrontEndUITouchBar.hpp"

namespace metaforce {

CFrontEndUITouchBar::~CFrontEndUITouchBar() = default;
void CFrontEndUITouchBar::SetPhase(EPhase ph) { m_phase = ph; }
CFrontEndUITouchBar::EPhase CFrontEndUITouchBar::GetPhase() { return m_phase; }
void CFrontEndUITouchBar::SetFileSelectPhase(const SFileSelectDetail details[3], bool eraseGame, bool galleryActive) {
  m_phase = EPhase::FileSelect;
}
void CFrontEndUITouchBar::SetNoCardSelectPhase(bool galleryActive) { m_phase = EPhase::NoCardSelect; }
void CFrontEndUITouchBar::SetFusionBonusPhase(bool fusionSuitActive) { m_phase = EPhase::FusionBonus; }
void CFrontEndUITouchBar::SetStartOptionsPhase(bool normalBeat) { m_phase = EPhase::StartOptions; }
CFrontEndUITouchBar::EAction CFrontEndUITouchBar::PopAction() { return EAction::None; }

#ifndef __APPLE__
std::unique_ptr<CFrontEndUITouchBar> NewFrontEndUITouchBar() { return std::make_unique<CFrontEndUITouchBar>(); }
#endif

} // namespace metaforce
