#pragma once

#include "VISIRenderer.hpp"
#include "boo/graphicsdev/glew.h"

#include <zeus/CFrustum.hpp>

class VISIRendererOpenGL : public VISIRenderer {
  GLuint m_vtxShader, m_fragShader, m_program, m_uniLoc;
  GLuint m_uniformBufferGL;
  GLuint m_aabbIBO;

  std::array<zeus::CFrustum, 6> m_frustums;

  struct ModelBinding {
    GLuint vbo, ibo, vao;
  };
  struct EntityBinding {
    GLuint vbo, vao;
  };
  struct LightBinding {
    GLuint vbo, vao;
  };
  std::vector<ModelBinding> m_modelBindings;
  std::vector<EntityBinding> m_entityBindings;
  std::vector<LightBinding> m_lightBindings;

  size_t m_queryCount;
  std::unique_ptr<GLuint[]> m_queries;
  std::unique_ptr<bool[]> m_queryBools;

  bool SetupShaders() override;
  bool SetupVertexBuffersAndFormats() override;
  void SetupRenderPass(const zeus::CVector3f& pos) override;

public:
  VISIRendererOpenGL(int argc, const hecl::SystemChar** argv) : VISIRenderer(argc, argv) {}
  void RenderPVSOpaque(RGBA8* bufOut, bool& needTransparent) override;
  void RenderPVSTransparent(const std::function<void(int)>& passFunc) override;
  void RenderPVSEntitiesAndLights(const std::function<void(int)>& passFunc,
                                  const std::function<void(int, EPVSVisSetState)>& lightPassFunc) override;
};