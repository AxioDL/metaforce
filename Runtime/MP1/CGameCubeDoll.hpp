#pragma once

#include <memory>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CActorLights.hpp"

namespace urde {
class CModel;
namespace MP1 {

class CGameCubeDoll {
  TLockedToken<CModel> x0_model;
  std::vector<CLight> x8_lights;
  std::unique_ptr<CActorLights> x18_actorLights;
  float x1c_fader = 0.f;
  bool x20_24_loaded : 1 = false;
  void UpdateActorLights();

public:
  CGameCubeDoll();
  void Update(float dt);
  void Draw(float alpha);
  void Touch();
  bool CheckLoadComplete();
  bool IsLoaded() const { return x20_24_loaded; }
};

} // namespace MP1
} // namespace urde
