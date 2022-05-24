#pragma once

#include <optional>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Graphics/CTexture.hpp"

namespace metaforce {
class CStateManager;

namespace MP1 {

class CFaceplateDecoration {
  CAssetId x0_id;
  TToken<CTexture> x4_tex;
  bool xc_ready = false;

public:
  explicit CFaceplateDecoration(CStateManager& stateMgr);
  void Update(float dt, CStateManager& stateMgr);
  void Draw(CStateManager& stateMgr);
};

} // namespace MP1
} // namespace metaforce
