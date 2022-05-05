#include "gpu.hpp"

#include <aurora/aurora.hpp>

#include <SDL.h>
#include <dawn/native/DawnNative.h>
#include <dawn/webgpu_cpp.h>
#include <logvisor/logvisor.hpp>
#include <magic_enum.hpp>
#include <memory>

#include "dawn/BackendBinding.hpp"

// FIXME hack to avoid crash on Windows
namespace aurora {
extern WindowSize g_windowSize;
} // namespace aurora

namespace aurora::gpu {
static logvisor::Module Log("aurora::gpu");

wgpu::Device g_device;
wgpu::Queue g_queue;
wgpu::SwapChain g_swapChain;
wgpu::BackendType g_backendType;
GraphicsConfig g_graphicsConfig;
TextureWithSampler g_frameBuffer;
TextureWithSampler g_frameBufferResolved;
TextureWithSampler g_depthBuffer;

static std::unique_ptr<dawn::native::Instance> g_Instance;
static dawn::native::Adapter g_Adapter;
static wgpu::AdapterProperties g_AdapterProperties;
static std::unique_ptr<utils::BackendBinding> g_BackendBinding;

static TextureWithSampler create_render_texture(bool multisampled) {
  const auto size = wgpu::Extent3D{
      .width = g_graphicsConfig.width,
      .height = g_graphicsConfig.height,
  };
  const auto format = g_graphicsConfig.colorFormat;
  uint32_t sampleCount = 1;
  if (multisampled) {
    sampleCount = g_graphicsConfig.msaaSamples;
  }
  const auto textureDescriptor = wgpu::TextureDescriptor{
      .label = "Render texture",
      .usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
      .size = size,
      .format = format,
      .sampleCount = sampleCount,
  };
  auto texture = g_device.CreateTexture(&textureDescriptor);

  const auto viewDescriptor = wgpu::TextureViewDescriptor{};
  auto view = texture.CreateView(&viewDescriptor);

  const auto samplerDescriptor = wgpu::SamplerDescriptor{
      .label = "Render sampler",
      .magFilter = wgpu::FilterMode::Linear,
      .minFilter = wgpu::FilterMode::Linear,
      .mipmapFilter = wgpu::FilterMode::Linear,
  };
  auto sampler = g_device.CreateSampler(&samplerDescriptor);

  return {
      .texture = std::move(texture),
      .view = std::move(view),
      .size = size,
      .format = format,
      .sampler = std::move(sampler),
  };
}

static TextureWithSampler create_depth_texture() {
  const auto size = wgpu::Extent3D{
      .width = g_graphicsConfig.width,
      .height = g_graphicsConfig.height,
  };
  const auto format = g_graphicsConfig.depthFormat;
  const auto textureDescriptor = wgpu::TextureDescriptor{
      .label = "Depth texture",
      .usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding,
      .size = size,
      .format = format,
      .sampleCount = g_graphicsConfig.msaaSamples,
  };
  auto texture = g_device.CreateTexture(&textureDescriptor);

  const auto viewDescriptor = wgpu::TextureViewDescriptor{};
  auto view = texture.CreateView(&viewDescriptor);

  const auto samplerDescriptor = wgpu::SamplerDescriptor{
      .label = "Depth sampler",
      .magFilter = wgpu::FilterMode::Linear,
      .minFilter = wgpu::FilterMode::Linear,
      .mipmapFilter = wgpu::FilterMode::Linear,
  };
  auto sampler = g_device.CreateSampler(&samplerDescriptor);

  return {
      .texture = std::move(texture),
      .view = std::move(view),
      .size = size,
      .format = format,
      .sampler = std::move(sampler),
  };
}

static void error_callback(WGPUErrorType type, char const* message, void* userdata) {
  Log.report(logvisor::Fatal, FMT_STRING("Dawn error {}: {}"),
             magic_enum::enum_name(static_cast<wgpu::ErrorType>(type)), message);
}

void initialize(SDL_Window* window) {
  Log.report(logvisor::Info, FMT_STRING("Creating Dawn instance"));
  g_Instance = std::make_unique<dawn::native::Instance>();
#if !defined(NDEBUG)
  // D3D12's debug layer is very slow
  if (preferredBackendType != wgpu::BackendType::D3D12) {
    g_Instance->EnableBackendValidation(true);
  }
#endif
  utils::DiscoverAdapter(g_Instance.get(), window, preferredBackendType);

  {
    std::vector<dawn::native::Adapter> adapters = g_Instance->GetAdapters();
    const auto adapterIt = std::find_if(adapters.begin(), adapters.end(), [](const auto& adapter) -> bool {
      wgpu::AdapterProperties properties;
      adapter.GetProperties(&properties);
      return properties.backendType == preferredBackendType;
    });
    if (adapterIt == adapters.end()) {
      Log.report(logvisor::Fatal, FMT_STRING("Failed to find usable graphics backend"));
      unreachable();
    }
    g_Adapter = *adapterIt;
  }
  g_Adapter.GetProperties(&g_AdapterProperties);
  g_backendType = g_AdapterProperties.backendType;
  const auto backendName = magic_enum::enum_name(g_backendType);
  Log.report(logvisor::Info, FMT_STRING("Graphics adapter information\n  API: {}\n  Device: {} ({})\n  Driver: {}"),
             backendName, g_AdapterProperties.name, magic_enum::enum_name(g_AdapterProperties.adapterType),
             g_AdapterProperties.driverDescription);

  {
    WGPUSupportedLimits supportedLimits;
    g_Adapter.GetLimits(&supportedLimits);
    const wgpu::RequiredLimits requiredLimits{
        .limits =
            {
                // Use "best" supported alignments
                .minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment == 0
                                                       ? static_cast<uint32_t>(WGPU_LIMIT_U32_UNDEFINED)
                                                       : supportedLimits.limits.minUniformBufferOffsetAlignment,
                .minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment == 0
                                                       ? static_cast<uint32_t>(WGPU_LIMIT_U32_UNDEFINED)
                                                       : supportedLimits.limits.minStorageBufferOffsetAlignment,
            },
    };
    const std::array<wgpu::FeatureName, 1> requiredFeatures{
        wgpu::FeatureName::TextureCompressionBC,
    };
    const std::array enableToggles {
      /* clang-format off */
#if _WIN32
      "use_dxc",
#endif
#ifdef NDEBUG
      "skip_validation", "disable_robustness",
#else
      "use_user_defined_labels_in_backend",
#endif
      /* clang-format on */
    };
    wgpu::DawnTogglesDeviceDescriptor togglesDescriptor{};
    togglesDescriptor.forceEnabledTogglesCount = enableToggles.size();
    togglesDescriptor.forceEnabledToggles = enableToggles.data();
    const auto deviceDescriptor = wgpu::DeviceDescriptor{
        .nextInChain = &togglesDescriptor,
        .requiredFeaturesCount = requiredFeatures.size(),
        .requiredFeatures = requiredFeatures.data(),
        .requiredLimits = &requiredLimits,
    };
    g_device = wgpu::Device::Acquire(g_Adapter.CreateDevice(&deviceDescriptor));
    g_device.SetUncapturedErrorCallback(&error_callback, nullptr);
  }
  g_queue = g_device.GetQueue();

  g_BackendBinding =
      std::unique_ptr<utils::BackendBinding>(utils::CreateBinding(g_backendType, window, g_device.Get()));
  if (!g_BackendBinding) {
    Log.report(logvisor::Fatal, FMT_STRING("Unsupported backend {}"), backendName);
    unreachable();
  }

  auto swapChainFormat = static_cast<wgpu::TextureFormat>(g_BackendBinding->GetPreferredSwapChainTextureFormat());
  if (swapChainFormat == wgpu::TextureFormat::RGBA8UnormSrgb) {
    swapChainFormat = wgpu::TextureFormat::RGBA8Unorm;
  } else if (swapChainFormat == wgpu::TextureFormat::BGRA8UnormSrgb) {
    swapChainFormat = wgpu::TextureFormat::BGRA8Unorm;
  }
  Log.report(logvisor::Info, FMT_STRING("Using swapchain format {}"), magic_enum::enum_name(swapChainFormat));
  {
    const auto descriptor = wgpu::SwapChainDescriptor{
        .format = swapChainFormat,
        .implementation = g_BackendBinding->GetSwapChainImplementation(),
    };
    g_swapChain = g_device.CreateSwapChain(nullptr, &descriptor);
  }
  {
    const auto size = get_window_size();
    g_graphicsConfig = GraphicsConfig{
        .width = size.fb_width,
        .height = size.fb_height,
        .colorFormat = swapChainFormat,
        .depthFormat = wgpu::TextureFormat::Depth32Float,
        .msaaSamples = 1, // TODO 4
        .textureAnistropy = 16,
    };
    resize_swapchain(size.fb_width, size.fb_height);
    g_windowSize = size;
  }
}

void shutdown() {
  g_frameBuffer = {};
  g_frameBufferResolved = {};
  g_depthBuffer = {};
  wgpuSwapChainRelease(g_swapChain.Release());
  wgpuQueueRelease(g_queue.Release());
  g_BackendBinding.reset();
  wgpuDeviceRelease(g_device.Release());
  g_Instance.reset();
}

void resize_swapchain(uint32_t width, uint32_t height) {
  g_graphicsConfig.width = width;
  g_graphicsConfig.height = height;
  g_swapChain.Configure(g_graphicsConfig.colorFormat, wgpu::TextureUsage::RenderAttachment, width, height);
  g_frameBuffer = create_render_texture(true);
  g_frameBufferResolved = create_render_texture(false);
  g_depthBuffer = create_depth_texture();
}
} // namespace aurora::gpu
