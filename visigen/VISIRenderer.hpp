#pragma once

#include "hecl/SystemChar.hpp"
#include "hecl/HMDLMeta.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CAABox.hpp"

typedef void (*FPercent)(float);

enum class EPVSVisSetState { EndOfTree, NodeFound, OutOfBounds };

class VISIRenderer {
  friend struct VISIBuilder;

public:
  struct Model {
    hecl::HMDLTopology topology;
    zeus::CAABox aabb;

    struct Vert {
      zeus::CVector3f pos;
      zeus::CColor color;
    };
    std::vector<Vert> verts;

    std::vector<uint32_t> idxs;

    struct Surface {
      uint32_t first;
      uint32_t count;
      bool transparent;
    };
    std::vector<Surface> surfaces;
  };

  struct Entity {
    uint32_t entityId;
    zeus::CAABox aabb;
  };

  struct Light {
    zeus::CVector3f point;
  };

protected:
  int m_argc;
  const hecl::SystemChar** m_argv;
  int m_return = 0;

  zeus::CAABox m_totalAABB;

  virtual bool SetupShaders() = 0;

  std::vector<Model> m_models;
  std::vector<Entity> m_entities;
  std::vector<Light> m_lights;
  virtual bool SetupVertexBuffersAndFormats() = 0;
  virtual void SetupRenderPass(const zeus::CVector3f& pos) = 0;

  FPercent m_updatePercent;

public:
  bool m_terminate = false;
  struct RGBA8 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
  };

  VISIRenderer(int argc, const hecl::SystemChar** argv) : m_argc(argc), m_argv(argv) {}
  void Run(FPercent updatePercent);
  void Terminate();
  virtual void RenderPVSOpaque(RGBA8* bufOut, bool& needTransparent) = 0;
  virtual void RenderPVSTransparent(const std::function<void(int)>& passFunc) = 0;
  virtual void RenderPVSEntitiesAndLights(const std::function<void(int)>& passFunc,
                                          const std::function<void(int, EPVSVisSetState)>& lightPassFunc) = 0;
  int ReturnVal() const { return m_return; }

  static std::vector<Model::Vert> AABBToVerts(const zeus::CAABox& aabb, const zeus::CColor& color);
  static zeus::CColor ColorForIndex(uint32_t i);
  static void* makePNGBuffer(unsigned char* rgba, int width, int height, size_t* outsize);
};
