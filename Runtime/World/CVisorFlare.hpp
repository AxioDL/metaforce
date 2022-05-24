#pragma once

#include <optional>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"

#include <zeus/CColor.hpp>

namespace metaforce {
class CActor;
class CStateManager;
class CTexture;

class CVisorFlare {
public:
  enum class EBlendMode {
    Additive = 0,
    Blend = 1,
  };
  class CFlareDef {
    mutable TToken<CTexture> x0_tex;
    float x8_pos;
    float xc_scale;
    zeus::CColor x10_color;

  public:
    CFlareDef() = default;
    CFlareDef(const CFlareDef&) = default;
    CFlareDef(const TToken<CTexture>& tex, float pos, float scale, const zeus::CColor& color)
    : x0_tex(tex), x8_pos(pos), xc_scale(scale), x10_color(color) {
      x0_tex.Lock();
    }

    TToken<CTexture>& GetTexture() const { return x0_tex; }
    zeus::CColor GetColor() const { return x10_color; }
    float GetPosition() const { return x8_pos; }
    float GetScale() const { return xc_scale; }
  };

private:
  EBlendMode x0_blendMode;
  std::vector<CFlareDef> x4_flareDefs;
  bool x14_b1;
  float x18_f1;
  float x1c_f2;
  float x20_f3;
  float x24_ = 0.f;
  float x28_ = 0.f;
  u32 x2c_w1;
  u32 x30_w2;

  void SetupRenderState(const CStateManager& mgr) const;
  void ResetTevSwapMode(const CStateManager& mgr) const;
  void DrawDirect(const zeus::CColor& color, float f1, float f2) const;
  void DrawStreamed(const zeus::CColor& color, float f1, float f2) const;

public:
  CVisorFlare(EBlendMode blendMode, bool, float, float, float, u32, u32, std::vector<CFlareDef> flares);
  void Update(float dt, const zeus::CVector3f& pos, const CActor* act, CStateManager& mgr);
  void Render(const zeus::CVector3f& pos, const CStateManager& mgr) const;
  static std::optional<CFlareDef> LoadFlareDef(CInputStream& in);
};

} // namespace metaforce
