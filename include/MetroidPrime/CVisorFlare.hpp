#ifndef _CVISORFLARE
#define _CVISORFLARE

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/vector.hpp"

class CTexture;
class CStateManager;
class CVector3f;
class CActor;

class CVisorFlare {
public:
  enum EBlendMode {
    kBM_Additive = 0,
    kBM_Blend = 1,
  };
  class CFlareDef {
    mutable TToken< CTexture > mTex;
    float mPos;
    float mScale;
    CColor mColor;

  public:
    CFlareDef(const TToken< CTexture >& tex, float pos, float scale, uint color);

    TToken< CTexture >& GetTexture() const { return mTex; }
    CColor GetColor() const { return mColor; }
    float GetPosition() const { return mPos; }
    float GetScale() const { return mScale; }
  };

  CVisorFlare(EBlendMode blendMode, bool, float, float, float, uint, uint,
              const rstl::vector< CFlareDef >& flares);
  ~CVisorFlare() {}

  void Update(float dt, const CVector3f& pos, const CActor* act, CStateManager& mgr);
  void Render(const CVector3f& pos, const CStateManager& mgr) const;
  static rstl::optional_object< CFlareDef > LoadFlareDef(CInputStream& in);

private:
  EBlendMode mBlendMode;
  rstl::vector< CFlareDef > mFlareDefs;
  bool mDistanceScaled;
  float mFadeTime;
  float mAngularFalloff;
  float mRotationScale;
  float mIntensity;
  float mOcclusionTime;
  int mThermalVisorMode;
  int mCombatVisorMode;

  void SetupRenderState(const CStateManager& mgr) const;
  void ResetTevSwapMode(const CStateManager& mgr) const;
  void DrawDirect(const CColor& color, float f1, float f2) const;
  void DrawStreamed(const CColor& color, float f1, float f2) const;
};

CHECK_SIZEOF(CVisorFlare, 0x34)
NESTED_CHECK_SIZEOF(CVisorFlare, CFlareDef, 0x14)

#endif // _CVISORFLARE
