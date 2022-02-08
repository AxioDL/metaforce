#pragma once

#include "hecl/hecl.hpp"

namespace hecl::blender {
std::optional<std::string> FindBlender(int& major, int& minor);
bool IsVersionSupported(int major, int minor);
std::pair<uint32_t, uint32_t> GetLatestSupportedVersion();
std::pair<uint32_t, uint32_t> GetEarliestSupportedVersion();
std::pair<uint32_t, uint32_t> GetRecommendedVersion();
void SetOverridePath(std::string_view overridePath);
} // namespace hecl::blender
