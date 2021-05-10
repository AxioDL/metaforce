#pragma once

#include "../VISIRenderer.hpp"
#import <Metal/Metal.h>

#if !__has_feature(objc_arc)
#error ARC Required
#endif

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
  void Run(FPercent updatePercent) override;
  void RenderPVSOpaque(RGBA8* out, bool& needTransparent) override;
  void RenderPVSTransparent(const std::function<void(int)>& passFunc) override;
  void RenderPVSEntitiesAndLights(const std::function<void(int)>& passFunc,
                                  const std::function<void(int, EPVSVisSetState)>& lightPassFunc) override;
};
