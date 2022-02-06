#include "include/lib.hpp"

namespace aurora {
void App_onAppLaunched(AppDelegate& cb) noexcept { return cb.onAppLaunched(); }
bool App_onAppIdle(AppDelegate& cb, float dt) noexcept { return cb.onAppIdle(dt); }
void App_onAppDraw(AppDelegate& cb) noexcept { cb.onAppDraw(); }
void App_onAppPostDraw(AppDelegate& cb) noexcept { cb.onAppPostDraw(); }
void App_onAppWindowResized(AppDelegate& cb, const WindowSize& size) noexcept {
  cb.onAppWindowResized(size);
}
void App_onAppWindowMoved(AppDelegate& cb, std::int32_t x, std::int32_t y) noexcept {
  cb.onAppWindowMoved(x, y);
}
void App_onAppExiting(AppDelegate& cb) noexcept { cb.onAppExiting(); }
} // namespace aurora
