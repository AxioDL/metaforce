#pragma once

#include <array>
#include <type_traits>
#include <vector>
#include <cinttypes>

namespace aurora {
enum class SpecialKey : std::uint8_t;

struct WindowSize;

struct App;
struct AppDelegate {
  AppDelegate() = default;
  virtual ~AppDelegate() noexcept = default;
  AppDelegate(const AppDelegate&) = delete;
  AppDelegate& operator=(const AppDelegate&) = delete;
  AppDelegate(AppDelegate&&) = delete;
  AppDelegate& operator=(AppDelegate&&) = delete;

  virtual void onAppLaunched() noexcept = 0;
  virtual bool onAppIdle(float dt) noexcept = 0;
  virtual void onAppDraw() noexcept = 0;
  virtual void onAppPostDraw() noexcept = 0;
  virtual void onAppWindowResized(const WindowSize& size) noexcept = 0;
  virtual void onAppWindowMoved(std::int32_t x, std::int32_t y) noexcept = 0;
  virtual void onAppExiting() noexcept = 0;

  // Input
  virtual void onCharKeyDown(std::uint8_t charCode, bool is_repeat) noexcept = 0;
  virtual void onCharKeyUp(std::uint8_t charCode) noexcept = 0;
  virtual void onSpecialKeyDown(const SpecialKey& key, bool is_repeat) noexcept = 0;
  virtual void onSpecialKeyUp(const SpecialKey& key) noexcept = 0;

//  virtual void resized([[maybe_unused]] const WindowSize& rect, [[maybe_unused]] bool sync) noexcept {}
//  virtual void mouseDown([[maybe_unused]] const SWindowCoord& coord, [[maybe_unused]] EMouseButton button,
//                         [[maybe_unused]] EModifierKey mods) noexcept {}
//  virtual void mouseUp([[maybe_unused]] const SWindowCoord& coord, [[maybe_unused]] EMouseButton button,
//                       [[maybe_unused]] EModifierKey mods) noexcept {}
//  virtual void mouseMove([[maybe_unused]] const SWindowCoord& coord) noexcept {}
//  virtual void mouseEnter([[maybe_unused]] const SWindowCoord& coord) noexcept {}
//  virtual void mouseLeave([[maybe_unused]] const SWindowCoord& coord) noexcept {}
//  virtual void scroll([[maybe_unused]] const SWindowCoord& coord, [[maybe_unused]] const SScrollDelta& scroll) noexcept {}
//
//  virtual void touchDown([[maybe_unused]] const STouchCoord& coord, [[maybe_unused]] uintptr_t tid) noexcept {}
//  virtual void touchUp([[maybe_unused]] const STouchCoord& coord, [[maybe_unused]] uintptr_t tid) noexcept {}
//  virtual void touchMove([[maybe_unused]] const STouchCoord& coord, [[maybe_unused]] uintptr_t tid) noexcept {}
//
//  virtual void charKeyDown([[maybe_unused]] unsigned long charCode, [[maybe_unused]] EModifierKey mods,
//                           [[maybe_unused]] bool isRepeat) noexcept {}
//  virtual void charKeyUp([[maybe_unused]] unsigned long charCode, [[maybe_unused]] EModifierKey mods) noexcept {}
//  virtual void specialKeyDown([[maybe_unused]] ESpecialKey key, [[maybe_unused]] EModifierKey mods,
//                              [[maybe_unused]] bool isRepeat) noexcept {}
//  virtual void specialKeyUp([[maybe_unused]] ESpecialKey key, [[maybe_unused]] EModifierKey mods) noexcept {}
//  virtual void modKeyDown([[maybe_unused]] EModifierKey mod, [[maybe_unused]] bool isRepeat) noexcept {}
//  virtual void modKeyUp([[maybe_unused]] EModifierKey mod) noexcept {}
};
} // namespace aurora
