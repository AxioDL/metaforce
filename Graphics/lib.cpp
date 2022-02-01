#include "include/lib.hpp"

namespace aurora {
void App_onAppLaunched(AppDelegate& cb) noexcept { return cb.onAppLaunched(); }
bool App_onAppIdle(AppDelegate& cb, float dt) noexcept { return cb.onAppIdle(dt); }
void App_onAppDraw(AppDelegate& cb) noexcept { cb.onAppDraw(); }
void App_onAppPostDraw(AppDelegate& cb) noexcept { cb.onAppPostDraw(); }
void App_onAppExiting(AppDelegate& cb) noexcept { cb.onAppExiting(); }
} // namespace aurora
