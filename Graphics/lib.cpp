#include "include/lib.hpp"

namespace aurora {
void App_onAppLaunched(AppDelegate& cb) noexcept { return cb.onAppLaunched(); }
bool App_onAppIdle(AppDelegate& cb, float dt) noexcept { return cb.onAppIdle(dt); }
void App_onAppDraw(AppDelegate& cb) noexcept { cb.onAppDraw(); }
void App_onAppPostDraw(AppDelegate& cb) noexcept { cb.onAppPostDraw(); }
void App_onAppWindowResized(AppDelegate& cb, const WindowSize& size) noexcept {
  cb.onAppWindowResized(size);
}
void App_onAppWindowMoved(AppDelegate& cb, int32_t x, int32_t y) noexcept {
  cb.onAppWindowMoved(x, y);
}
void App_onAppExiting(AppDelegate& cb) noexcept { cb.onAppExiting(); }

// Input
void App_onCharKeyDown(AppDelegate& cb, uint8_t code, bool isRepeat) noexcept {
  cb.onCharKeyDown(code, isRepeat);
}
void App_onCharKeyUp(AppDelegate& cb, uint8_t code) noexcept {
  cb.onCharKeyUp(code);
}
void App_onSpecialKeyDown(AppDelegate& cb, SpecialKey key, bool isRepeat) noexcept {
  cb.onSpecialKeyDown(key, isRepeat);
}
void App_onSpecialKeyUp(AppDelegate& cb, SpecialKey key) noexcept {
  cb.onSpecialKeyUp(key);
}

// Controller
void App_onControllerButton(AppDelegate& cb, uint32_t idx, ControllerButton button, bool pressed) noexcept {
  cb.onControllerButton(idx, button, pressed);
}
void App_onControllerAxis(AppDelegate& cb, uint32_t idx, ControllerAxis axis, int16_t value) noexcept {
  cb.onControllerAxis(idx, axis, value);
}
} // namespace aurora
