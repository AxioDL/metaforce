#include "BackendBinding.hpp"

#include <SDL_metal.h>
#include <dawn/native/MetalBackend.h>

#import <QuartzCore/CAMetalLayer.h>

template <typename T> DawnSwapChainImplementation CreateSwapChainImplementation(T *swapChain) {
  DawnSwapChainImplementation impl = {};
  impl.userData = swapChain;
  impl.Init = [](void *userData, void *wsiContext) {
    auto *ctx = static_cast<typename T::WSIContext *>(wsiContext);
    reinterpret_cast<T *>(userData)->Init(ctx);
  };
  impl.Destroy = [](void *userData) { delete reinterpret_cast<T *>(userData); };
  impl.Configure = [](void *userData, WGPUTextureFormat format, WGPUTextureUsage allowedUsage, uint32_t width,
                      uint32_t height) {
    return static_cast<T *>(userData)->Configure(format, allowedUsage, width, height);
  };
  impl.GetNextTexture = [](void *userData, DawnSwapChainNextTexture *nextTexture) {
    return static_cast<T *>(userData)->GetNextTexture(nextTexture);
  };
  impl.Present = [](void *userData) { return static_cast<T *>(userData)->Present(); };
  return impl;
}

namespace aurora::gpu::utils {
class SwapChainImplMTL {
public:
  using WSIContext = DawnWSIContextMetal;

  explicit SwapChainImplMTL(SDL_Window *window) : m_view(SDL_Metal_CreateView(window)) {}

  ~SwapChainImplMTL() { SDL_Metal_DestroyView(m_view); }

  void Init(DawnWSIContextMetal *ctx) {
    mMtlDevice = ctx->device;
    mCommandQueue = ctx->queue;
  }

  DawnSwapChainError Configure(WGPUTextureFormat format, WGPUTextureUsage usage, uint32_t width, uint32_t height) {
    if (format != WGPUTextureFormat_BGRA8Unorm) {
      return "unsupported format";
    }
    assert(width > 0);
    assert(height > 0);

    CGSize size = {};
    size.width = width;
    size.height = height;

    mLayer = (__bridge CAMetalLayer *)(SDL_Metal_GetLayer(m_view));
    [mLayer setDevice:mMtlDevice];
    [mLayer setPixelFormat:MTLPixelFormatBGRA8Unorm];
    [mLayer setDrawableSize:size];

    constexpr uint32_t kFramebufferOnlyTextureUsages = WGPUTextureUsage_RenderAttachment | WGPUTextureUsage_Present;
    bool hasOnlyFramebufferUsages = (usage & (~kFramebufferOnlyTextureUsages)) == 0u;
    if (hasOnlyFramebufferUsages) {
      [mLayer setFramebufferOnly:YES];
    }

    return DAWN_SWAP_CHAIN_NO_ERROR;
  }

  DawnSwapChainError GetNextTexture(DawnSwapChainNextTexture *nextTexture) {
    mCurrentDrawable = [mLayer nextDrawable];
    mCurrentTexture = mCurrentDrawable.texture;
    nextTexture->texture.ptr = (__bridge void *)(mCurrentTexture);
    return DAWN_SWAP_CHAIN_NO_ERROR;
  }

  DawnSwapChainError Present() {
    id<MTLCommandBuffer> commandBuffer = [mCommandQueue commandBuffer];
    [commandBuffer presentDrawable:mCurrentDrawable];
    [commandBuffer commit];
    return DAWN_SWAP_CHAIN_NO_ERROR;
  }

private:
  SDL_MetalView m_view = nil;
  id<MTLDevice> mMtlDevice = nil;
  id<MTLCommandQueue> mCommandQueue = nil;

  CAMetalLayer *mLayer = nullptr;
  id<CAMetalDrawable> mCurrentDrawable = nil;
  id<MTLTexture> mCurrentTexture = nil;
};

class MetalBinding : public BackendBinding {
public:
  MetalBinding(SDL_Window *window, WGPUDevice device) : BackendBinding(window, device) {}

  uint64_t GetSwapChainImplementation() override {
    if (m_swapChainImpl.userData == nullptr) {
      m_swapChainImpl = CreateSwapChainImplementation(new SwapChainImplMTL(m_window));
    }
    return reinterpret_cast<uint64_t>(&m_swapChainImpl);
  }

  WGPUTextureFormat GetPreferredSwapChainTextureFormat() override { return WGPUTextureFormat_BGRA8Unorm; }

private:
  DawnSwapChainImplementation m_swapChainImpl{};
};

BackendBinding *CreateMetalBinding(SDL_Window *window, WGPUDevice device) { return new MetalBinding(window, device); }
} // namespace aurora::gpu::utils
