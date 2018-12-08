#pragma once

#include <vector>
#include "RetroTypes.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CAABox.hpp"
#include "hecl/HMDLMeta.hpp"
#include "Shaders/CModelShaders.hpp"

namespace urde {
class CBooModel;
struct CBooSurface;

class CMetroidModelInstance {
  friend class CBooRenderer;
  friend class CGameArea;

  int x0_visorFlags;
  zeus::CTransform x4_xf;
  zeus::CAABox x34_aabb;
  std::vector<CBooSurface> m_surfaces;
  std::unique_ptr<CBooModel> m_instance;
  hecl::HMDLMeta m_hmdlMeta;
  std::unordered_map<int, CModelShaders::ShaderPipelines> m_shaders;

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
    m_shaders.clear();
  }
};

} // namespace urde
