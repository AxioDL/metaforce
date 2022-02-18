#pragma once
#include <aurora/gfx.hpp>

#include <utility>

#include <dawn/webgpu_cpp.h>
#include <xxhash.h>
#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)
#include <xxh_x86dispatch.h>
#endif

template <typename T>
XXH64_hash_t xxh3_hash(const T& input, XXH64_hash_t seed = 0) {
  return XXH3_64bits_withSeed(&input, sizeof(T), seed);
}

namespace aurora {
extern wgpu::Device g_Device;
extern wgpu::Queue g_Queue;
} // namespace aurora

namespace aurora::gfx {
static logvisor::Module Log("aurora::gfx");

extern zeus::CMatrix4f g_mv;
extern zeus::CMatrix4f g_mvInv;
extern zeus::CMatrix4f g_proj;
extern metaforce::CFogState g_fogState;

struct TextureRef {
  wgpu::Texture texture;
  wgpu::TextureView view;
  wgpu::Extent3D size;
  wgpu::TextureFormat format;

  TextureRef(wgpu::Texture&& texture, wgpu::TextureView&& view, wgpu::Extent3D size, wgpu::TextureFormat format)
  : texture(std::move(texture)), view(std::move(view)), size(size), format(format) {}
};

using PipelineRef = uint64_t;
using Range = std::pair<uint64_t, uint64_t>;

enum class ShaderType {
  Aabb,
  TexturedQuad,
  MoviePlayer,
};
} // namespace aurora::gfx
