#pragma once

#include <dolphin/pad.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace metaforce::input {

struct InputDevice {
  uint32_t source;
  std::string name;
  bool operator==(const InputDevice&) const = default;
};

bool Initialize();
void Shutdown();
void Update();

std::vector< InputDevice > Devices();
uint32_t DeviceSource();
void SelectDevice(uint32_t source);

bool HasAxis(const PADAxisMapping& mapping);
bool KeyboardSelected();
SDL_Gamepad* SelectedGamepad();

void ResetBindings();

template < typename Mapping >
Mapping* MappingFor(u16 target, Mapping* (*getMappings)(u32, u32*), u16 Mapping::* destination) {
  u32 count = 0;
  auto* mappings = getMappings(0, &count);
  if (!mappings) {
    return nullptr;
  }

  auto* end = mappings + count;
  auto* found = std::ranges::find(mappings, end, target, destination);
  return found == end ? nullptr : found;
}

} // namespace metaforce::input
