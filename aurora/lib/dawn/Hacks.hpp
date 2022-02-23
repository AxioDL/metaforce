#pragma once

#include <dawn/webgpu.h>

/**
 * Helpers to expose private Dawn APIs
 */
namespace aurora::gpu::hacks {
void apply_toggles(WGPUDevice device);
} // namespace aurora::gpu::hacks
