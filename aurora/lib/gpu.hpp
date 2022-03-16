#pragma once

#include <array>
#include <cstdint>
#include <dawn/webgpu_cpp.h>
#include <optick.h>

#ifdef __GNUC__
[[noreturn]] inline __attribute__((always_inline)) void unreachable() { __builtin_unreachable(); }
#elif defined(_MSC_VER)
[[noreturn]] __forceinline void unreachable() { __assume(false); }
#else
#error Unknown compiler
#endif

struct SDL_Window;

namespace aurora::gpu {
struct GraphicsConfig {
  uint32_t width;
  uint32_t height;
  wgpu::TextureFormat colorFormat;
  wgpu::TextureFormat depthFormat;
  uint32_t msaaSamples;
  uint16_t textureAnistropy;
};
struct TextureWithSampler {
  wgpu::Texture texture;
  wgpu::TextureView view;
  wgpu::Extent3D size;
  wgpu::TextureFormat format;
  wgpu::Sampler sampler;
};

#ifdef DAWN_ENABLE_BACKEND_D3D12
static const wgpu::BackendType preferredBackendType = wgpu::BackendType::D3D12;
#elif DAWN_ENABLE_BACKEND_VULKAN
static const wgpu::BackendType preferredBackendType = wgpu::BackendType::Vulkan;
#elif DAWN_ENABLE_BACKEND_METAL
static const wgpu::BackendType preferredBackendType = wgpu::BackendType::Metal;
#else
static const wgpu::BackendType preferredBackendType = wgpu::BackendType::OpenGL;
#endif
extern wgpu::Device g_device;
extern wgpu::Queue g_queue;
extern wgpu::SwapChain g_swapChain;
extern wgpu::BackendType g_backendType;
extern GraphicsConfig g_graphicsConfig;
extern TextureWithSampler g_frameBuffer;
extern TextureWithSampler g_frameBufferResolved;
extern TextureWithSampler g_depthBuffer;

void initialize(SDL_Window* window);
void shutdown();
void resize_swapchain(uint32_t width, uint32_t height);

#if USE_OPTICK
void* get_native_swapchain();
Optick::GPUContext begin_cmdlist();
void end_cmdlist(Optick::GPUContext ctx);
#endif
} // namespace aurora::gpu

namespace aurora::gpu::utils {
template <auto N>
static consteval std::array<wgpu::VertexAttribute, N>
make_vertex_attributes(std::array<wgpu::VertexFormat, N> formats) {
  std::array<wgpu::VertexAttribute, N> attributes;
  uint64_t offset = 0;
  for (uint32_t i = 0; i < N; ++i) {
    auto format = formats[i];
    attributes[i] = wgpu::VertexAttribute{
        .format = format,
        .offset = offset,
        .shaderLocation = i,
    };
    switch (format) {
    case wgpu::VertexFormat::Uint8x2:
    case wgpu::VertexFormat::Sint8x2:
    case wgpu::VertexFormat::Unorm8x2:
    case wgpu::VertexFormat::Snorm8x2:
      offset += 2;
      break;
    case wgpu::VertexFormat::Uint8x4:
    case wgpu::VertexFormat::Sint8x4:
    case wgpu::VertexFormat::Unorm8x4:
    case wgpu::VertexFormat::Snorm8x4:
    case wgpu::VertexFormat::Uint16x2:
    case wgpu::VertexFormat::Sint16x2:
    case wgpu::VertexFormat::Unorm16x2:
    case wgpu::VertexFormat::Snorm16x2:
    case wgpu::VertexFormat::Float16x2:
    case wgpu::VertexFormat::Float32:
    case wgpu::VertexFormat::Uint32:
    case wgpu::VertexFormat::Sint32:
      offset += 4;
      break;
    case wgpu::VertexFormat::Uint16x4:
    case wgpu::VertexFormat::Sint16x4:
    case wgpu::VertexFormat::Unorm16x4:
    case wgpu::VertexFormat::Snorm16x4:
    case wgpu::VertexFormat::Float16x4:
    case wgpu::VertexFormat::Float32x2:
    case wgpu::VertexFormat::Uint32x2:
    case wgpu::VertexFormat::Sint32x2:
      offset += 8;
      break;
    case wgpu::VertexFormat::Float32x3:
    case wgpu::VertexFormat::Uint32x3:
    case wgpu::VertexFormat::Sint32x3:
      offset += 12;
      break;
    case wgpu::VertexFormat::Float32x4:
    case wgpu::VertexFormat::Uint32x4:
    case wgpu::VertexFormat::Sint32x4:
      offset += 16;
      break;
    case wgpu::VertexFormat::Undefined:
      break;
    }
  }
  return attributes;
}

template <auto N>
static inline wgpu::VertexBufferLayout
make_vertex_buffer_layout(uint64_t arrayStride, const std::array<wgpu::VertexAttribute, N>& attributes,
                          wgpu::VertexStepMode stepMode = wgpu::VertexStepMode::Vertex) {
  return {
      .arrayStride = arrayStride,
      .stepMode = stepMode,
      .attributeCount = static_cast<uint32_t>(attributes.size()),
      .attributes = attributes.data(),
  };
}

template <auto N>
static inline wgpu::VertexState make_vertex_state(const wgpu::ShaderModule& module,
                                                  const std::array<wgpu::VertexBufferLayout, N>& buffers,
                                                  const char* entryPoint = "vs_main") {
  return {
      .module = module,
      .entryPoint = entryPoint,
      .bufferCount = static_cast<uint32_t>(buffers.size()),
      .buffers = buffers.data(),
  };
}
} // namespace aurora::gpu::utils
