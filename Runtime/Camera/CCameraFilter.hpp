#pragma once

#include <memory>
#include <optional>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/Shaders/CCameraBlurFilter.hpp"
#include "Runtime/Graphics/Shaders/CXRayBlurFilter.hpp"

#include <zeus/CColor.hpp>

namespace urde {
enum class EFilterType {
  Passthru,
  Multiply,
  Invert,
  Add,
  Subtract,
  Blend,
  Widescreen,
  SceneAdd,
  NoColor,
  InvDstMultiply
};

enum class EFilterShape {
  Fullscreen,
  FullscreenHalvesLeftRight,
  FullscreenHalvesTopBottom,
  FullscreenQuarters,
  CinemaBars,
  ScanLinesEven,
  ScanLinesOdd,
  RandomStatic,
  CookieCutterDepthRandomStatic
};

class CCameraFilterPassBase {
protected:
  EFilterType x0_curType = EFilterType::Passthru;
  EFilterType x4_nextType = EFilterType::Passthru;
  EFilterShape x8_shape = EFilterShape::Fullscreen;
  float xc_duration = 0.f;
  float x10_remTime = 0.f;
  zeus::CColor x14_prevColor;
  zeus::CColor x18_curColor;
  zeus::CColor x1c_nextColor;
  CAssetId x20_nextTxtr;
  TLockedToken<CTexture> x24_texObj; // Used to be auto_ptr
  float GetT(bool invert) const;

public:
  virtual ~CCameraFilterPassBase() = default;
  virtual void Update(float dt) = 0;
  virtual void SetFilter(EFilterType type, EFilterShape shape, float time, const zeus::CColor& color,
                         CAssetId txtr) = 0;
  virtual void DisableFilter(float time) = 0;
  virtual void Draw() = 0;
};

template <class S>
class CCameraFilterPass final : public CCameraFilterPassBase {
  std::optional<S> m_shader;

public:
  void Update(float dt) override;
  void SetFilter(EFilterType type, EFilterShape shape, float time, const zeus::CColor& color, CAssetId txtr) override;
  void DisableFilter(float time) override;
  void Draw() override;
};

class CCameraFilterPassPoly {
  EFilterShape m_shape{};
  std::unique_ptr<CCameraFilterPassBase> m_filter;

public:
  void Update(float dt) {
    if (m_filter)
      m_filter->Update(dt);
  }
  void SetFilter(EFilterType type, EFilterShape shape, float time, const zeus::CColor& color, CAssetId txtr);
  void DisableFilter(float time) {
    if (m_filter)
      m_filter->DisableFilter(time);
  }
  void Draw() const {
    if (m_filter)
      m_filter->Draw();
  }
};

enum class EBlurType { NoBlur, LoBlur, HiBlur, Xray };

class CCameraBlurPass {
  TLockedToken<CTexture> x0_paletteTex;
  EBlurType x10_curType = EBlurType::NoBlur;
  EBlurType x14_endType = EBlurType::NoBlur;
  float x18_endValue = 0.f;
  float x1c_curValue = 0.f;
  float x20_startValue = 0.f;
  float x24_totalTime = 0.f;
  float x28_remainingTime = 0.f;
  // bool x2c_usePersistent = false;
  // bool x2d_noPersistentCopy = false;
  // u32 x30_persistentBuf = 0;

  std::optional<CCameraBlurFilter> m_shader;
  std::optional<CXRayBlurFilter> m_xrayShader;

public:
  void Draw(bool clearDepth = false);
  void Update(float dt);
  void SetBlur(EBlurType type, float amount, float duration);
  void DisableBlur(float duration);
  EBlurType GetCurrType() const { return x10_curType; }
};

} // namespace urde
