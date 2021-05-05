#pragma once

#include "VISIRenderer.hpp"
#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

@interface MetalRenderer : NSObject
@end

class VISIRendererMetal : public VISIRenderer {
  MetalRenderer* view;

  bool SetupShaders() override;
  bool SetupVertexBuffersAndFormats() override;
  void SetupRenderPass(const zeus::CVector3f& pos) override;

public:
  VISIRendererMetal(int argc, const hecl::SystemChar** argv) : VISIRenderer(argc, argv) {
    view = [[MetalRenderer alloc] init];
  }
  void RenderPVSOpaque(RGBA8* out, bool& needTransparent) override;
  void RenderPVSTransparent(const std::function<void(int)>& passFunc) override;
  void RenderPVSEntitiesAndLights(const std::function<void(int)>& passFunc,
                                  const std::function<void(int, EPVSVisSetState)>& lightPassFunc) override;
};