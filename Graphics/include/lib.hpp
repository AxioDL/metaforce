#include "aurora.hpp"
#include <cinttypes>

namespace aurora {
void App_onAppLaunched(AppDelegate& cb) noexcept;
bool App_onAppIdle(AppDelegate& cb, float dt) noexcept;
void App_onAppDraw(AppDelegate& cb) noexcept;
void App_onAppPostDraw(AppDelegate& cb) noexcept;
void App_onAppWindowResized(AppDelegate& cb, const WindowSize& size) noexcept;
void App_onAppWindowMoved(AppDelegate& cb, int32_t x, int32_t y) noexcept;
void App_onAppExiting(AppDelegate& cb) noexcept;
// Input
void App_onCharKeyDown(AppDelegate& cb, uint8_t code, bool isRepeat) noexcept;
void App_onCharKeyUp(AppDelegate& cb, uint8_t code) noexcept;
void App_onSpecialKeyDown(AppDelegate& cb, SpecialKey key, bool isRepeat) noexcept;
void App_onSpecialKeyUp(AppDelegate& cb, SpecialKey key) noexcept;
// Controller
void App_onControllerAdded(AppDelegate& cb, uint32_t which) noexcept;
void App_onControllerRemoved(AppDelegate& cb, uint32_t which) noexcept;
void App_onControllerButton(AppDelegate& cb, uint32_t which, ControllerButton button, bool pressed) noexcept;
void App_onControllerAxis(AppDelegate& cb, uint32_t which, ControllerAxis axis, int16_t value) noexcept;
} // namespace aurora
