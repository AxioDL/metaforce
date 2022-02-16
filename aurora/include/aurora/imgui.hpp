#pragma once

#include "common.hpp"

#include <imgui.h>

namespace aurora::imgui {
ImTextureID add_texture(uint32_t width, uint32_t height, ArrayRef<uint8_t> data) noexcept;
} // namespace aurora::imgui
