#include "GuiSys/CGuiTableGroup.hpp"

CGuiTableGroup::CRepeatState::CRepeatState() : mTimer(0.f) {}

const bool CGuiTableGroup::CRepeatState::Update(float dt, bool state) {
  bool ret = false;
  if (mTimer == 0.f) {
    if (state) {
      mTimer = 0.6f;
      ret = true;
    }
  } else {
    if (state) {
      mTimer -= dt;
      if (mTimer <= 0.f) {
        mTimer = 0.05f;
        ret = true;
      }
    } else {
      mTimer = 0.f;
    }
  }
  return ret;
}
