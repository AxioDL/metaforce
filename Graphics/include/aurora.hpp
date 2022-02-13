#pragma once

#include <array>
#include <cinttypes>
#include <type_traits>
#include <vector>
#ifndef ENABLE_BITWISE_ENUM
#define ENABLE_BITWISE_ENUM(type)                                                                                      \
  constexpr type operator|(type a, type b) noexcept {                                                                  \
    using T = std::underlying_type_t<type>;                                                                            \
    return type(static_cast<T>(a) | static_cast<T>(b));                                                                \
  }                                                                                                                    \
  constexpr type operator&(type a, type b) noexcept {                                                                  \
    using T = std::underlying_type_t<type>;                                                                            \
    return type(static_cast<T>(a) & static_cast<T>(b));                                                                \
  }                                                                                                                    \
  constexpr type& operator|=(type& a, type b) noexcept {                                                               \
    using T = std::underlying_type_t<type>;                                                                            \
    a = type(static_cast<T>(a) | static_cast<T>(b));                                                                   \
    return a;                                                                                                          \
  }                                                                                                                    \
  constexpr type& operator&=(type& a, type b) noexcept {                                                               \
    using T = std::underlying_type_t<type>;                                                                            \
    a = type(static_cast<T>(a) & static_cast<T>(b));                                                                   \
    return a;                                                                                                          \
  }                                                                                                                    \
  constexpr type operator~(type key) noexcept {                                                                        \
    using T = std::underlying_type_t<type>;                                                                            \
    return type(~static_cast<T>(key));                                                                                 \
  }                                                                                                                    \
  constexpr bool True(type key) noexcept {                                                                             \
    using T = std::underlying_type_t<type>;                                                                            \
    return static_cast<T>(key) != 0;                                                                                   \
  }                                                                                                                    \
  constexpr bool False(type key) noexcept {                                                                            \
    using T = std::underlying_type_t<type>;                                                                            \
    return static_cast<T>(key) == 0;                                                                                   \
  }
#endif

namespace aurora {
enum class SpecialKey : uint8_t;
enum class ModifierKey : uint16_t {
  None = 0,
  LeftShift = 1 << 0,
  RightShift = 1 << 1,
  LeftControl = 1 << 2,
  RightControl = 1 << 3,
  LeftAlt = 1 << 3,
  RightAlt = 1 << 4,
  LeftGui = 1 << 5,
  RightGui = 1 << 6,
  Num = 1 << 7,
  Caps = 1 << 8,
  Mode = 1 << 9,
  // SDL has a reserved value we don't need
};
ENABLE_BITWISE_ENUM(ModifierKey);

enum class ControllerButton : uint8_t;
enum class ControllerAxis : uint8_t;

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
  virtual void onAppWindowMoved(int32_t x, int32_t y) noexcept = 0;
  virtual void onAppExiting() noexcept = 0;

  // Input
  virtual void onCharKeyDown(uint8_t charCode, ModifierKey mods, bool isRepeat) noexcept = 0;
  virtual void onCharKeyUp(uint8_t charCode, ModifierKey mods) noexcept = 0;
  virtual void onSpecialKeyDown(SpecialKey key, ModifierKey mods, bool isRepeat) noexcept = 0;
  virtual void onSpecialKeyUp(SpecialKey key, ModifierKey mods) noexcept = 0;

  // Controller
  virtual void onControllerAdded(uint32_t which) noexcept = 0;
  virtual void onControllerRemoved(uint32_t which) noexcept = 0;
  virtual void onControllerButton(uint32_t which, ControllerButton button, bool pressed) noexcept = 0;
  virtual void onControllerAxis(uint32_t which, ControllerAxis axis, int16_t value) noexcept = 0;

  //  virtual void resized([[maybe_unused]] const WindowSize& rect, [[maybe_unused]] bool sync) noexcept {}
  //  virtual void mouseDown([[maybe_unused]] const SWindowCoord& coord, [[maybe_unused]] EMouseButton button,
  //                         [[maybe_unused]] EModifierKey mods) noexcept {}
  //  virtual void mouseUp([[maybe_unused]] const SWindowCoord& coord, [[maybe_unused]] EMouseButton button,
  //                       [[maybe_unused]] EModifierKey mods) noexcept {}
  //  virtual void mouseMove([[maybe_unused]] const SWindowCoord& coord) noexcept {}
  //  virtual void mouseEnter([[maybe_unused]] const SWindowCoord& coord) noexcept {}
  //  virtual void mouseLeave([[maybe_unused]] const SWindowCoord& coord) noexcept {}
  //  virtual void scroll([[maybe_unused]] const SWindowCoord& coord, [[maybe_unused]] const SScrollDelta& scroll)
  //  noexcept {}
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
