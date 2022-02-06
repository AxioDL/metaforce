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
} // namespace aurora
