#include <aurora/aurora.hpp>
#include <memory>

namespace aurora {
void app_run(std::unique_ptr<AppDelegate> app, Icon icon) noexcept {}
std::vector<std::string> get_args() noexcept {
  return {}; // TODO
}
WindowSize get_window_size() noexcept {
  return {}; // TODO
}
void set_window_title(std::string_view title) noexcept {
  // TODO
}
Backend get_backend() noexcept {
  return Backend::Vulkan; // TODO
}
std::string_view get_backend_string() noexcept {
  return {}; // TODO
}
void set_fullscreen(bool fullscreen) noexcept {
  // TODO
}
int32_t get_controller_player_index(uint32_t which) noexcept {
  return -1; // TODO
}
void set_controller_player_index(uint32_t which, int32_t index) noexcept {
  // TODO
}
bool is_controller_gamecube(uint32_t which) noexcept {
  return true; // TODO
}
std::string get_controller_name(uint32_t which) noexcept {
  return ""; // TODO
}
} // namespace aurora
