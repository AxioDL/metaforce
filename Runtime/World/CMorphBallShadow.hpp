#pragma once

#include <list>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/Graphics/CTexture.hpp"

#include <zeus/CAABox.hpp>

namespace metaforce {
class CActor;
class CGameArea;
class CPlayer;
class CStateManager;

class CMorphBallShadow {
  std::list<CActor*> x0_actors;
  std::list<TAreaId> x18_areas;
  std::vector<u32> x30_worldModelBits;
  // CTexture x40_;
  // TToken<CTexture> xa8_ballFade;
  // int xb0_idW;
  // int xb4_idH;
  zeus::CAABox xb8_shadowVolume;
  bool xd0_hasIds = false;
  void GatherAreas(const CStateManager& mgr);
  bool AreasValid(const CStateManager& mgr) const;

public:
  void RenderIdBuffer(const zeus::CAABox& aabb, const CStateManager& mgr, CPlayer& player);
  void Render(const CStateManager& mgr, float alpha);
};
} // namespace metaforce
