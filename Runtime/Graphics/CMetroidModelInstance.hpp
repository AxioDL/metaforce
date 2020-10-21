#pragma once

#include <memory>
#include <unordered_map>
#include <vector>

#include "Runtime/RetroTypes.hpp"

#include "Shaders/CModelShaders.hpp"

#include <hecl/HMDLMeta.hpp>

#include <zeus/CAABox.hpp>
#include <zeus/CTransform.hpp>

namespace urde {
class CBooModel;
struct CBooSurface;

class CMetroidModelInstance {
  friend class CBooRenderer;
  friend class CGameArea;

public:
  int x0_visorFlags;
  zeus::CTransform x4_xf;
  zeus::CAABox x34_aabb;
  std::vector<CBooSurface> m_surfaces;
  std::unique_ptr<CBooModel> m_instance;
  hecl::HMDLMeta m_hmdlMeta;
  hsh::owner<hsh::vertex_buffer_typeless> m_staticVbo;
  hsh::owner<hsh::index_buffer<u32>> m_staticIbo;

public:
  CMetroidModelInstance() = default;
  CMetroidModelInstance(CMetroidModelInstance&&) = default;
  void Clear() {
    x0_visorFlags = 0;
    x4_xf = {};
    x34_aabb = {};
    m_surfaces.clear();
    m_instance.reset();
    m_hmdlMeta = {};
  }
};

} // namespace urde
