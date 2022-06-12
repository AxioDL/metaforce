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

// EFB -> XFB copy pipeline
static wgpu::BindGroupLayout g_CopyBindGroupLayout;
wgpu::RenderPipeline g_CopyPipeline;
wgpu::BindGroup g_CopyBindGroup;

static std::unique_ptr<dawn::native::Instance> g_Instance;
static dawn::native::Adapter g_Adapter;
static wgpu::AdapterProperties g_AdapterProperties;
static std::unique_ptr<utils::BackendBinding> g_BackendBinding;

TextureWithSampler create_render_texture(bool multisampled) {
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
      .usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::CopySrc |
               wgpu::TextureUsage::CopyDst,
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

void create_copy_pipeline() {
  wgpu::ShaderModuleWGSLDescriptor sourceDescriptor{};
  sourceDescriptor.source = R"""(
@group(0) @binding(0)
var efb_sampler: sampler;
@group(0) @binding(1)
var efb_texture: texture_2d<f32>;

struct VertexOutput {
    @builtin(position) pos: vec4<f32>,
    @location(0) uv: vec2<f32>,
};

var<private> pos: array<vec2<f32>, 3> = array<vec2<f32>, 3>(
    vec2(-1.0, 1.0),
    vec2(-1.0, -3.0),
    vec2(3.0, 1.0),
);
var<private> uvs: array<vec2<f32>, 3> = array<vec2<f32>, 3>(
    vec2(0.0, 0.0),
    vec2(0.0, 2.0),
    vec2(2.0, 0.0),
);

@stage(vertex)
fn vs_main(@builtin(vertex_index) vtxIdx: u32) -> VertexOutput {
    var out: VertexOutput;
    out.pos = vec4<f32>(pos[vtxIdx], 0.0, 1.0);
    out.uv = uvs[vtxIdx];
    return out;
}

@stage(fragment)
fn fs_main(in: VertexOutput) -> @location(0) vec4<f32> {
    return textureSample(efb_texture, efb_sampler, in.uv);
}
)""";
  const wgpu::ShaderModuleDescriptor moduleDescriptor{
      .nextInChain = &sourceDescriptor,
      .label = "XFB Copy Module",
  };
  auto module = g_device.CreateShaderModule(&moduleDescriptor);
  const std::array colorTargets{
      wgpu::ColorTargetState{
          .format = g_graphicsConfig.colorFormat,
      },
  };
  const wgpu::FragmentState fragmentState{
      .module = module,
      .entryPoint = "fs_main",
      .targetCount = colorTargets.size(),
      .targets = colorTargets.data(),
  };
  const std::array bindGroupLayoutEntries{
      wgpu::BindGroupLayoutEntry{
          .binding = 0,
          .visibility = wgpu::ShaderStage::Fragment,
          .sampler =
              wgpu::SamplerBindingLayout{
                  .type = wgpu::SamplerBindingType::Filtering,
              },
      },
      wgpu::BindGroupLayoutEntry{
          .binding = 1,
          .visibility = wgpu::ShaderStage::Fragment,
          .texture =
              wgpu::TextureBindingLayout{
                  .sampleType = wgpu::TextureSampleType::Float,
                  .viewDimension = wgpu::TextureViewDimension::e2D,
              },
      },
  };
  const wgpu::BindGroupLayoutDescriptor bindGroupLayoutDescriptor{
      .entryCount = bindGroupLayoutEntries.size(),
      .entries = bindGroupLayoutEntries.data(),
  };
  g_CopyBindGroupLayout = g_device.CreateBindGroupLayout(&bindGroupLayoutDescriptor);
  const wgpu::PipelineLayoutDescriptor layoutDescriptor{
      .bindGroupLayoutCount = 1,
      .bindGroupLayouts = &g_CopyBindGroupLayout,
  };
  auto pipelineLayout = g_device.CreatePipelineLayout(&layoutDescriptor);
  const wgpu::RenderPipelineDescriptor pipelineDescriptor{
      .layout = pipelineLayout,
      .vertex =
          wgpu::VertexState{
              .module = module,
              .entryPoint = "vs_main",
          },
      .fragment = &fragmentState,
  };
  g_CopyPipeline = g_device.CreateRenderPipeline(&pipelineDescriptor);
}

void create_copy_bind_group() {
  const std::array bindGroupEntries{
      wgpu::BindGroupEntry{
          .binding = 0,
          .sampler = g_graphicsConfig.msaaSamples > 1 ? gpu::g_frameBufferResolved.sampler : gpu::g_frameBuffer.sampler,
      },
      wgpu::BindGroupEntry{
          .binding = 1,
          .textureView = g_graphicsConfig.msaaSamples > 1 ? gpu::g_frameBufferResolved.view : gpu::g_frameBuffer.view,
      },
  };
  const wgpu::BindGroupDescriptor bindGroupDescriptor{
      .layout = g_CopyBindGroupLayout,
      .entryCount = bindGroupEntries.size(),
      .entries = bindGroupEntries.data(),
  };
  g_CopyBindGroup = g_device.CreateBindGroup(&bindGroupDescriptor);
}

static void error_callback(WGPUErrorType type, char const* message, void* userdata) {
  Log.report(logvisor::Fatal, FMT_STRING("Dawn error {}: {}"),
             magic_enum::enum_name(static_cast<wgpu::ErrorType>(type)), message);
}

static void device_callback(WGPURequestDeviceStatus status, WGPUDevice device, char const* message, void* userdata) {
  if (status == WGPURequestDeviceStatus_Success) {
    g_device = wgpu::Device::Acquire(device);
  } else {
    Log.report(logvisor::Warning, FMT_STRING("Device request failed with message: {}"), message);
  }
  *static_cast<bool*>(userdata) = true;
}

bool initialize(SDL_Window* window, wgpu::BackendType backendType, uint32_t msaa, uint16_t aniso) {
  if (!g_Instance) {
    Log.report(logvisor::Info, FMT_STRING("Creating Dawn instance"));
    g_Instance = std::make_unique<dawn::native::Instance>();
  }
  Log.report(logvisor::Info, FMT_STRING("Attempting to initialize {}"), magic_enum::enum_name(backendType));
#if 0
  // D3D12's debug layer is very slow
  g_Instance->EnableBackendValidation(backendType != wgpu::BackendType::D3D12);
#endif
  if (!utils::DiscoverAdapter(g_Instance.get(), window, backendType)) {
    return false;
  }

  {
    std::vector<dawn::native::Adapter> adapters = g_Instance->GetAdapters();
    std::sort(adapters.begin(), adapters.end(), [&](const auto& a, const auto& b) {
      wgpu::AdapterProperties propertiesA;
      wgpu::AdapterProperties propertiesB;
      a.GetProperties(&propertiesA);
      b.GetProperties(&propertiesB);
      constexpr std::array PreferredTypeOrder{
          wgpu::AdapterType::DiscreteGPU,
          wgpu::AdapterType::IntegratedGPU,
          wgpu::AdapterType::CPU,
      };
      const auto typeItA = std::find(PreferredTypeOrder.begin(), PreferredTypeOrder.end(), propertiesA.adapterType);
      const auto typeItB = std::find(PreferredTypeOrder.begin(), PreferredTypeOrder.end(), propertiesB.adapterType);
      if (typeItA == PreferredTypeOrder.end() && typeItB != PreferredTypeOrder.end()) {
        return -1;
      }
      return static_cast<int>(typeItA - typeItB);
    });
    const auto adapterIt = std::find_if(adapters.begin(), adapters.end(), [=](const auto& adapter) -> bool {
      wgpu::AdapterProperties properties;
      adapter.GetProperties(&properties);
      return properties.backendType == backendType;
    });
    if (adapterIt == adapters.end()) {
      return false;
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
    WGPUSupportedLimits supportedLimits{};
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
    std::vector<wgpu::FeatureName> features;
    const auto supportedFeatures = g_Adapter.GetSupportedFeatures();
    for (const auto* const feature : supportedFeatures) {
      if (strcmp(feature, "texture-compression-bc") == 0) {
        features.push_back(wgpu::FeatureName::TextureCompressionBC);
      }
    }
    const std::array enableToggles {
      /* clang-format off */
#if _WIN32
      "use_dxc",
#endif
#ifdef NDEBUG
      "skip_validation",
      "disable_robustness",
#endif
      "use_user_defined_labels_in_backend",
      "disable_symbol_renaming",
      /* clang-format on */
    };
    wgpu::DawnTogglesDeviceDescriptor togglesDescriptor{};
    togglesDescriptor.forceEnabledTogglesCount = enableToggles.size();
    togglesDescriptor.forceEnabledToggles = enableToggles.data();
    const auto deviceDescriptor = wgpu::DeviceDescriptor{
        .nextInChain = &togglesDescriptor,
        .requiredFeaturesCount = static_cast<uint32_t>(features.size()),
        .requiredFeatures = features.data(),
        .requiredLimits = &requiredLimits,
    };
    bool deviceCallbackReceived = false;
    g_Adapter.RequestDevice(&deviceDescriptor, &device_callback, &deviceCallbackReceived);
    // while (!deviceCallbackReceived) {
    //   TODO wgpuInstanceProcessEvents
    // }
    if (!g_device) {
      return false;
    }
    g_device.SetUncapturedErrorCallback(&error_callback, nullptr);
  }
  g_device.SetDeviceLostCallback(nullptr, nullptr);
  g_queue = g_device.GetQueue();

  g_BackendBinding =
      std::unique_ptr<utils::BackendBinding>(utils::CreateBinding(g_backendType, window, g_device.Get()));
  if (!g_BackendBinding) {
    return false;
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
        .msaaSamples = msaa,
        .textureAnisotropy = aniso,
    };
    create_copy_pipeline();
    resize_swapchain(size.fb_width, size.fb_height);
    g_windowSize = size;
  }
  return true;
}

void shutdown() {
  g_CopyBindGroupLayout = {};
  g_CopyPipeline = {};
  g_CopyBindGroup = {};
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
  create_copy_bind_group();
}
} // namespace aurora::gpu
