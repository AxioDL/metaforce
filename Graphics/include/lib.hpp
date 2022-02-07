#include "aurora.hpp"
#include <cinttypes>

namespace aurora {
void App_onAppLaunched(AppDelegate& cb) noexcept;
bool App_onAppIdle(AppDelegate& cb, float dt) noexcept;
void App_onAppDraw(AppDelegate& cb) noexcept;
void App_onAppPostDraw(AppDelegate& cb) noexcept;
void App_onAppWindowResized(AppDelegate& cb,  const WindowSize& size) noexcept;
void App_onAppWindowMoved(AppDelegate& cb, std::int32_t x, std::int32_t y) noexcept;
void App_onAppExiting(AppDelegate& cb) noexcept;
// Input
void App_onCharKeyDown(AppDelegate& cb, std::uint8_t code, bool is_repeat) noexcept;
void App_onCharKeyUp(AppDelegate& cb, std::uint8_t code) noexcept;
void App_onSpecialKeyDown(AppDelegate& cb, const SpecialKey& key, bool is_repeat);
void App_onSpecialKeyUp(AppDelegate& cb, const SpecialKey& key);
} // namespace aurora
