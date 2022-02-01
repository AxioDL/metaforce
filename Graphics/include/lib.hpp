#include "aurora.hpp"

namespace aurora {
void App_onAppLaunched(AppDelegate& cb) noexcept;
bool App_onAppIdle(AppDelegate& cb, float dt) noexcept;
void App_onAppDraw(AppDelegate& cb) noexcept;
void App_onAppPostDraw(AppDelegate& cb) noexcept;
void App_onAppExiting(AppDelegate& cb) noexcept;
} // namespace aurora
