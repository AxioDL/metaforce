#include "VISIRendererMetal.hh"
#include "ShaderTypes.h"

#include <zeus/CFrustum.hpp>

static zeus::CMatrix4f g_Proj;

constexpr zeus::CMatrix4f DepthCorrect(
    1.f, 0.f, 0.f, 0.f,
    0.f, 1.f, 0.f, 0.f,
    0.f, 0.f, 0.5f, 0.5f,
    0.f, 0.f, 0.f, 1.f);

static void CalculateProjMatrix() {
  float znear = 0.2f;
  float zfar = 1000.f;
  float tfov = std::tan(zeus::degToRad(90.f * 0.5f));
  float top = znear * tfov;
  float bottom = -top;
  float right = znear * tfov;
  float left = -right;

  float rml = right - left;
  float rpl = right + left;
  float tmb = top - bottom;
  float tpb = top + bottom;
  float fpn = zfar + znear;
  float fmn = zfar - znear;

  zeus::CMatrix4f mat2{
      2.f * znear / rml, 0.f, rpl / rml, 0.f,
      0.f, 2.f * znear / tmb, tpb / tmb, 0.f,
      0.f, 0.f, -fpn / fmn, -2.f * zfar * znear / fmn,
      0.f, 0.f, -1.f, 0.f};
  g_Proj = DepthCorrect * mat2;
}

static constexpr std::array<uint16_t, 20> AABBIdxs{0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 1, 7, 3, 5, 5, 0, 0, 2, 6, 4};

static const zeus::CMatrix4f LookMATs[] = {
    {// Forward
     1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f},
    {// Backward
     -1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f},
    {// Up
     1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 1.f},
    {// Down
     1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f},
    {// Left
     0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f},
    {// Right
     0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, -1.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 1.f},
};

using Vertex = VISIRenderer::Model::Vert;

@implementation MetalRenderer {
  MTLPixelFormat _pixelFormat;
  MTLPixelFormat _depthPixelFormat;

  id<MTLDevice> _device;
  id<MTLLibrary> _library;
  id<MTLCommandQueue> _commandQueue;
  id<MTLRenderPipelineState> _pipelineState;
  id<MTLDepthStencilState> _depthState;
  id<MTLDepthStencilState> _depthStateNoWrite;
  dispatch_semaphore_t _semaphore;
  id<MTLBuffer> _uniformBuffer;
  id<MTLBuffer> _vertexBuffer;
  id<MTLBuffer> _indexBuffer;
  id<MTLBuffer> _modelQueryBuffer;
  id<MTLBuffer> _entityLightQueryBuffer;
  id<MTLBuffer> _aabbIndexBuffer;
  id<MTLTexture> _renderTexture;
  id<MTLTexture> _depthTexture;

  std::array<zeus::CFrustum, 6> _frustums;
  NSUInteger _entityVertStart;
}

- (bool)setup {
  CalculateProjMatrix();

  // Create device.
  _device = MTLCreateSystemDefaultDevice();

  // Set view settings.
  _pixelFormat = MTLPixelFormatRGBA8Unorm;
  _depthPixelFormat = MTLPixelFormatDepth32Float_Stencil8;

  // Load shaders.
  NSError *error = nil;
  _library = [_device newLibraryWithFile:@"Shader.metallib" error:&error];
  if (_library == nullptr) {
    NSLog(@"Failed to load library. error %@", error);
    return false;
  }
  id<MTLFunction> vertFunc = [_library newFunctionWithName:@"vertexShader"];
  id<MTLFunction> fragFunc = [_library newFunctionWithName:@"fragmentShader"];

  // Create render texture.
  MTLTextureDescriptor *renderTexDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:_pixelFormat
                                                                                           width:768
                                                                                          height:512
                                                                                       mipmapped:NO];
  renderTexDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
  _renderTexture = [_device newTextureWithDescriptor:renderTexDesc];

  // Create depth texture.
  MTLTextureDescriptor *depthTexDesc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:_depthPixelFormat
                                                                                          width:768
                                                                                         height:512
                                                                                      mipmapped:NO];
  depthTexDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
  _depthTexture = [_device newTextureWithDescriptor:depthTexDesc];

  // Create depth state.
  MTLDepthStencilDescriptor *depthDesc = [[MTLDepthStencilDescriptor alloc] init];
  depthDesc.depthCompareFunction = MTLCompareFunctionLessEqual;
  depthDesc.depthWriteEnabled = YES;
  _depthState = [_device newDepthStencilStateWithDescriptor:depthDesc];

  // Create depth state (no write).
  MTLDepthStencilDescriptor *depthDescNoWrite = [[MTLDepthStencilDescriptor alloc] init];
  depthDescNoWrite.depthCompareFunction = MTLCompareFunctionLessEqual;
  depthDescNoWrite.depthWriteEnabled = NO;
  _depthStateNoWrite = [_device newDepthStencilStateWithDescriptor:depthDescNoWrite];

  // Create vertex descriptor.
  MTLVertexDescriptor *vertDesc = [[MTLVertexDescriptor alloc] init];
  vertDesc.attributes[VertexAttributePosition].format = MTLVertexFormatFloat3;
  vertDesc.attributes[VertexAttributePosition].offset = offsetof(Vertex, pos);
  vertDesc.attributes[VertexAttributePosition].bufferIndex = BufferIndexVertex;
  vertDesc.attributes[VertexAttributeColor].format = MTLVertexFormatFloat4;
  vertDesc.attributes[VertexAttributeColor].offset = offsetof(Vertex, color);
  vertDesc.attributes[VertexAttributeColor].bufferIndex = BufferIndexVertex;
  vertDesc.layouts[BufferIndexVertex].stride = sizeof(Vertex);
  vertDesc.layouts[BufferIndexVertex].stepRate = 1;
  vertDesc.layouts[BufferIndexVertex].stepFunction = MTLVertexStepFunctionPerVertex;

  // Create pipeline state.
  MTLRenderPipelineDescriptor *pipelineDesc = [[MTLRenderPipelineDescriptor alloc] init];
  pipelineDesc.rasterSampleCount = 1;
  pipelineDesc.vertexFunction = vertFunc;
  pipelineDesc.fragmentFunction = fragFunc;
  pipelineDesc.vertexDescriptor = vertDesc;
  pipelineDesc.colorAttachments[0].pixelFormat = _pixelFormat;
  pipelineDesc.depthAttachmentPixelFormat = _depthPixelFormat;
  _pipelineState = [_device newRenderPipelineStateWithDescriptor:pipelineDesc error:&error];
  if (_pipelineState == nullptr) {
    NSLog(@"Failed to create pipeline state, error %@", error);
    return false;
  }
  return true;
}

- (bool)setupModels:(std::vector<VISIRenderer::Model> &)models
           entities:(std::vector<VISIRenderer::Entity> &)entities
             lights:(std::vector<VISIRenderer::Light> &)lights {
  NSUInteger vertCount = 0;
  NSUInteger indexCount = 0;
  for (const auto &model : models) {
    vertCount += model.verts.size();
    indexCount += model.idxs.size();
  }
  _entityVertStart = vertCount;
  vertCount += 8 * entities.size();
  vertCount += lights.size();
  _vertexBuffer = [_device newBufferWithLength:vertCount * sizeof(Vertex) options:MTLResourceStorageModeManaged];
  _indexBuffer = [_device newBufferWithLength:indexCount * sizeof(uint32_t) options:MTLResourceStorageModeManaged];
  _modelQueryBuffer = [_device newBufferWithLength:models.size() * 6 * sizeof(uint64_t)
                                           options:MTLResourceStorageModeManaged];
  _entityLightQueryBuffer = [_device newBufferWithLength:(entities.size() + lights.size()) * 6 * sizeof(uint64_t)
                                                 options:MTLResourceStorageModeManaged];

  auto *buffer = static_cast<Vertex *>([_vertexBuffer contents]);
  auto *indexBuffer = static_cast<uint32_t *>([_indexBuffer contents]);
  for (const auto &model : models) {
    memcpy(buffer, model.verts.data(), model.verts.size() * sizeof(Vertex));
    memcpy(indexBuffer, model.idxs.data(), model.idxs.size() * sizeof(uint32_t));
    buffer += model.verts.size();
    indexBuffer += model.idxs.size();
  }
  auto idx = static_cast<uint32_t>(models.size());
  for (const auto &ent : entities) {
    auto verts = VISIRenderer::AABBToVerts(ent.aabb, VISIRenderer::ColorForIndex(idx++));
    memcpy(buffer, verts.data(), verts.size() * sizeof(Vertex));
    buffer += verts.size();
  }
  for (const auto &light : lights) {
    auto *vert = buffer++;
    vert->pos = light.point;
    vert->color = VISIRenderer::ColorForIndex(idx++);
  }
  [_vertexBuffer didModifyRange:NSMakeRange(0, [_vertexBuffer length])];
  [_indexBuffer didModifyRange:NSMakeRange(0, [_indexBuffer length])];

  _uniformBuffer = [_device newBufferWithLength:sizeof(Uniforms) * 6 options:MTLResourceStorageModeManaged];
  _aabbIndexBuffer = [_device newBufferWithBytes:AABBIdxs.data()
                                          length:AABBIdxs.size() * sizeof(uint16_t)
                                         options:MTLResourceStorageModeManaged];

  _semaphore = dispatch_semaphore_create(0);
  _commandQueue = [_device newCommandQueue];
  return true;
}

- (void)setupRenderPass:(const zeus::CVector3f &)pos {
  auto posMat = zeus::CTransform::Translate(-pos).toMatrix4f();
  auto *buffer = static_cast<Uniforms *>([_uniformBuffer contents]);
  for (uint16_t j = 0; j < 6; ++j) {
    static_assert(sizeof(zeus::CMatrix4f) == sizeof(matrix_float4x4));
    zeus::CMatrix4f modelView = LookMATs[j] * posMat;
    _frustums[j].updatePlanes(modelView, g_Proj);
    memcpy(&buffer->projectionMatrix, &g_Proj, sizeof(matrix_float4x4));
    memcpy(&buffer->modelViewMatrix, &modelView, sizeof(matrix_float4x4));
    buffer++;
  }
  [_uniformBuffer didModifyRange:NSMakeRange(0, [_uniformBuffer length])];
}

- (void)renderPVSOpaque:(std::vector<VISIRenderer::Model> &)models
                    out:(VISIRenderer::RGBA8 *)out
        needTransparent:(bool &)needTransparent {
  id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];

  __block dispatch_semaphore_t semaphore = _semaphore;
  [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
    dispatch_semaphore_signal(semaphore);
  }];

  MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
  passDescriptor.colorAttachments[0].texture = _renderTexture;
  passDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
  passDescriptor.depthAttachment.texture = _depthTexture;
  passDescriptor.depthAttachment.storeAction = MTLStoreActionStore; // stored for following render passes
  id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];

  [encoder setDepthStencilState:_depthState];
  [encoder setRenderPipelineState:_pipelineState];
  [encoder setCullMode:MTLCullModeBack];
  [encoder setVertexBuffer:_vertexBuffer offset:0 atIndex:BufferIndexVertex];
  [encoder setVertexBuffer:_uniformBuffer offset:0 atIndex:BufferIndexUniforms];

  for (int j = 0; j < 6; ++j) {
    NSUInteger x = (j % 3) * 256;
    NSUInteger y = (j / 3) * 256;
    [encoder setViewport:{x, y, 256, 256, 0, 1}];
    if (j > 0) {
      [encoder setVertexBufferOffset:j * sizeof(Uniforms) atIndex:BufferIndexUniforms];
    }
    NSUInteger vertexBufferOffset = 0;
    NSUInteger indexBufferOffset = 0;
    for (const auto &model : models) {
      if (_frustums[j].aabbFrustumTest(model.aabb)) {
        [encoder setVertexBufferOffset:vertexBufferOffset atIndex:BufferIndexVertex];
        for (const auto &surf : model.surfaces) {
          // Non-transparents first
          if (surf.transparent) {
            needTransparent = true;
          } else {
            MTLPrimitiveType type = model.topology == hecl::HMDLTopology::TriStrips ? MTLPrimitiveTypeTriangleStrip
                                                                                    : MTLPrimitiveTypeTriangle;
            [encoder drawIndexedPrimitives:type
                                indexCount:surf.count
                                 indexType:MTLIndexTypeUInt32
                               indexBuffer:_indexBuffer
                         indexBufferOffset:indexBufferOffset + surf.first * sizeof(uint32_t)];
          }
        }
      }
      vertexBufferOffset += model.verts.size() * sizeof(Vertex);
      indexBufferOffset += model.idxs.size() * sizeof(uint32_t);
    }
  }

  [encoder endEncoding];
  [commandBuffer commit];

  dispatch_semaphore_wait(_semaphore, DISPATCH_TIME_FOREVER);

  [_renderTexture getBytes:out
               bytesPerRow:sizeof(VISIRenderer::RGBA8) * _renderTexture.width
                fromRegion:MTLRegionMake2D(0, 0, _renderTexture.width, _renderTexture.height)
               mipmapLevel:0];
}

- (void)renderPVSTransparent:(std::vector<VISIRenderer::Model> &)models
                    passFunc:(const std::function<void(int)> &)passFunc {
  // Zero out query buffer
  memset([_modelQueryBuffer contents], 0, [_modelQueryBuffer length]);
  [_modelQueryBuffer didModifyRange:NSMakeRange(0, [_modelQueryBuffer length])];

  id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];

  __block dispatch_semaphore_t semaphore = _semaphore;
  [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
    dispatch_semaphore_signal(semaphore);
  }];

  MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
  passDescriptor.colorAttachments[0].texture = _renderTexture;
  passDescriptor.colorAttachments[0].storeAction = MTLStoreActionDontCare; // no longer care about the render texture
  passDescriptor.depthAttachment.texture = _depthTexture;
  passDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;
  passDescriptor.depthAttachment.storeAction = MTLStoreActionStore; // still stored for following render passes
  passDescriptor.visibilityResultBuffer = _modelQueryBuffer;
  id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];

  [encoder setDepthStencilState:_depthStateNoWrite];
  [encoder setRenderPipelineState:_pipelineState];
  [encoder setCullMode:MTLCullModeBack];
  [encoder setVertexBuffer:_vertexBuffer offset:0 atIndex:BufferIndexVertex];
  [encoder setVertexBuffer:_uniformBuffer offset:0 atIndex:BufferIndexUniforms];

  NSUInteger queryCount = 0;
  for (int j = 0; j < 6; ++j) {
    GLint x = (j % 3) * 256;
    GLint y = (j / 3) * 256;
    [encoder setViewport:{x, y, 256, 256, 0, 1}];
    if (j > 0) {
      [encoder setVertexBufferOffset:j * sizeof(Uniforms) atIndex:BufferIndexUniforms];
    }
    NSUInteger vertexBufferOffset = 0;
    NSUInteger indexBufferOffset = 0;
    for (const auto &model : models) {
      if (_frustums[j].aabbFrustumTest(model.aabb)) {
        [encoder setVertexBufferOffset:vertexBufferOffset atIndex:BufferIndexVertex];
        [encoder setVisibilityResultMode:MTLVisibilityResultModeBoolean offset:queryCount * sizeof(uint64_t)];
        for (const auto &surf : model.surfaces) {
          // Only transparent surfaces
          if (surf.transparent) {
            MTLPrimitiveType type = model.topology == hecl::HMDLTopology::TriStrips ? MTLPrimitiveTypeTriangleStrip
                                                                                    : MTLPrimitiveTypeTriangle;
            [encoder drawIndexedPrimitives:type
                                indexCount:surf.count
                                 indexType:MTLIndexTypeUInt32
                               indexBuffer:_indexBuffer
                         indexBufferOffset:indexBufferOffset + surf.first * sizeof(uint32_t)];
          }
        }
      }
      vertexBufferOffset += model.verts.size() * sizeof(Vertex);
      indexBufferOffset += model.idxs.size() * sizeof(uint32_t);
      ++queryCount;
    }
  }

  [encoder endEncoding];
  [commandBuffer commit];

  dispatch_semaphore_wait(_semaphore, DISPATCH_TIME_FOREVER);

  auto *queries = static_cast<uint64_t *>([_modelQueryBuffer contents]);
  for (int i = 0; i < models.size(); ++i) {
    for (int j = 0; j < 6; ++j) {
      if (queries[i + j * models.size()] != 0u) {
        passFunc(i);
        break;
      }
    }
  }
}

- (void)renderPVSEntities:(std::vector<VISIRenderer::Entity> &)entities
           entityPassFunc:(const std::function<void(int)> &)entityPassFunc
                   lights:(std::vector<VISIRenderer::Light> &)lights
            lightPassFunc:(const std::function<void(int, EPVSVisSetState)> &)lightPassFunc
                totalAABB:(const zeus::CAABox &)totalAABB {
  // Zero out query buffer
  memset([_entityLightQueryBuffer contents], 0, [_entityLightQueryBuffer length]);
  [_entityLightQueryBuffer didModifyRange:NSMakeRange(0, [_entityLightQueryBuffer length])];

  id<MTLCommandBuffer> commandBuffer = [_commandQueue commandBuffer];

  __block dispatch_semaphore_t semaphore = _semaphore;
  [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
    dispatch_semaphore_signal(semaphore);
  }];

  MTLRenderPassDescriptor *passDescriptor = [MTLRenderPassDescriptor renderPassDescriptor];
  passDescriptor.colorAttachments[0].texture = _renderTexture;
  passDescriptor.colorAttachments[0].storeAction = MTLStoreActionDontCare; // no longer care about the render texture
  passDescriptor.depthAttachment.texture = _depthTexture;
  passDescriptor.depthAttachment.loadAction = MTLLoadActionLoad;
  passDescriptor.visibilityResultBuffer = _entityLightQueryBuffer;
  id<MTLRenderCommandEncoder> encoder = [commandBuffer renderCommandEncoderWithDescriptor:passDescriptor];

  [encoder setDepthStencilState:_depthStateNoWrite];
  [encoder setRenderPipelineState:_pipelineState];
  [encoder setCullMode:MTLCullModeBack];
  [encoder setVertexBuffer:_vertexBuffer offset:0 atIndex:BufferIndexVertex];
  [encoder setVertexBuffer:_uniformBuffer offset:0 atIndex:BufferIndexUniforms];

  NSUInteger queryCount = 0;
  for (int j = 0; j < 6; ++j) {
    GLint x = (j % 3) * 256;
    GLint y = (j / 3) * 256;
    [encoder setViewport:{x, y, 256, 256, 0, 1}];
    if (j > 0) {
      [encoder setVertexBufferOffset:j * sizeof(Uniforms) atIndex:BufferIndexUniforms];
    }

    NSUInteger vertexBufferOffset = _entityVertStart * sizeof(Vertex);

    for (const auto &ent : entities) {
      if (_frustums[j].aabbFrustumTest(ent.aabb)) {
        [encoder setVertexBufferOffset:vertexBufferOffset atIndex:BufferIndexVertex];
        [encoder setVisibilityResultMode:MTLVisibilityResultModeBoolean offset:queryCount * sizeof(uint64_t)];
        [encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangleStrip
                            indexCount:20
                             indexType:MTLIndexTypeUInt16
                           indexBuffer:_aabbIndexBuffer
                     indexBufferOffset:0];
      }
      vertexBufferOffset += 20 * sizeof(Vertex);
      ++queryCount;
    }

    for (const auto &light : lights) {
      if (_frustums[j].pointFrustumTest(light.point)) {
        [encoder setVertexBufferOffset:vertexBufferOffset atIndex:BufferIndexVertex];
        [encoder setVisibilityResultMode:MTLVisibilityResultModeBoolean offset:queryCount * sizeof(uint64_t)];
        [encoder drawPrimitives:MTLPrimitiveTypePoint vertexStart:0 vertexCount:1];
      }
      vertexBufferOffset += sizeof(Vertex);
      ++queryCount;
    }
  }

  [encoder endEncoding];
  [commandBuffer commit];

  dispatch_semaphore_wait(_semaphore, DISPATCH_TIME_FOREVER);

  NSUInteger total = entities.size() + lights.size();
  auto *queries = static_cast<uint64_t *>([_entityLightQueryBuffer contents]);
  for (int i = 0; i < entities.size(); ++i) {
    for (int j = 0; j < 6; ++j) {
      if (queries[i + j * total] != 0u) {
        entityPassFunc(i);
        break;
      }
    }
  }
  for (int i = 0; i < lights.size(); ++i) {
    bool pointInside = totalAABB.pointInside(lights[i].point);
    EPVSVisSetState state = pointInside ? EPVSVisSetState::EndOfTree : EPVSVisSetState::OutOfBounds;
    if (pointInside) {
      for (int j = 0; j < 6; ++j) {
        if (queries[entities.size() + i + j * total] != 0u) {
          state = EPVSVisSetState::NodeFound;
          break;
        }
      }
    }
    lightPassFunc(i, state);
  }
}
@end

void VISIRendererMetal::Run(FPercent updatePercent) {
  @autoreleasepool {
    VISIRenderer::Run(updatePercent);
  }
}

bool VISIRendererMetal::SetupShaders() { return [view setup]; }

bool VISIRendererMetal::SetupVertexBuffersAndFormats() {
  return [view setupModels:m_models entities:m_entities lights:m_lights];
}

void VISIRendererMetal::SetupRenderPass(const zeus::CVector3f &pos) { [view setupRenderPass:pos]; }

void VISIRendererMetal::RenderPVSOpaque(RGBA8 *bufOut, bool &needTransparent) {
  [view renderPVSOpaque:m_models out:bufOut needTransparent:needTransparent];
}

void VISIRendererMetal::RenderPVSTransparent(const std::function<void(int)> &passFunc) {
  [view renderPVSTransparent:m_models passFunc:passFunc];
}

void VISIRendererMetal::RenderPVSEntitiesAndLights(const std::function<void(int)> &passFunc,
                                                   const std::function<void(int, EPVSVisSetState)> &lightPassFunc) {
  [view renderPVSEntities:m_entities
           entityPassFunc:passFunc
                   lights:m_lights
            lightPassFunc:lightPassFunc
                totalAABB:m_totalAABB];
}
