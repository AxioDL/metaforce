#include "include/lib.hpp"

namespace aurora {
void App_onAppLaunched(AppDelegate& cb) noexcept { return cb.onAppLaunched(); }
bool App_onAppIdle(AppDelegate& cb, float dt) noexcept { return cb.onAppIdle(dt); }
void App_onAppDraw(AppDelegate& cb) noexcept { cb.onAppDraw(); }
void App_onAppPostDraw(AppDelegate& cb) noexcept { cb.onAppPostDraw(); }
void App_onAppWindowResized(AppDelegate& cb, const WindowSize& size) noexcept { cb.onAppWindowResized(size); }
void App_onAppWindowMoved(AppDelegate& cb, int32_t x, int32_t y) noexcept { cb.onAppWindowMoved(x, y); }
void App_onAppExiting(AppDelegate& cb) noexcept { cb.onAppExiting(); }

// Input
void App_onCharKeyDown(AppDelegate& cb, uint8_t code, std::uint16_t mods, bool isRepeat) noexcept {
  cb.onCharKeyDown(code, static_cast<ModifierKey>(mods), isRepeat);
}
void App_onCharKeyUp(AppDelegate& cb, uint8_t code, std::uint16_t mods) noexcept {
  cb.onCharKeyUp(code, static_cast<ModifierKey>(mods));
}
void App_onSpecialKeyDown(AppDelegate& cb, SpecialKey key, std::uint16_t mods, bool isRepeat) noexcept {
  cb.onSpecialKeyDown(key, static_cast<ModifierKey>(mods), isRepeat);
}
void App_onSpecialKeyUp(AppDelegate& cb, SpecialKey key, std::uint16_t mods) noexcept {
  cb.onSpecialKeyUp(key, static_cast<ModifierKey>(mods));
}

// Controller
void App_onControllerAdded(AppDelegate& cb, uint32_t which) noexcept { cb.onControllerAdded(which); }
void App_onControllerRemoved(AppDelegate& cb, uint32_t which) noexcept { cb.onControllerRemoved(which); }
void App_onControllerButton(AppDelegate& cb, uint32_t which, ControllerButton button, bool pressed) noexcept {
  cb.onControllerButton(which, button, pressed);
}
void App_onControllerAxis(AppDelegate& cb, uint32_t which, ControllerAxis axis, int16_t value) noexcept {
  cb.onControllerAxis(which, axis, value);
}
} // namespace aurora
