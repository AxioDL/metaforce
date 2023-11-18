#pragma once

#include <memory>
#include <optional>

#include "Runtime/CToken.hpp"
#include "Runtime/Graphics/CTexture.hpp"
#include "Runtime/Graphics/Shaders/CCameraBlurFilter.hpp"
#include "Runtime/Graphics/Shaders/CXRayBlurFilter.hpp"
#include "Runtime/RetroTypes.hpp"

#include <zeus/CColor.hpp>

namespace metaforce {
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

class CCameraFilterPass {
private:
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

  [[nodiscard]] float GetT(bool invert) const;

  static void DrawFilterShape(EFilterShape shape, const zeus::CColor& color, CTexture* tex, float lod);
  static void DrawFullScreenColoredQuad(const zeus::CColor& color);
  static void DrawFullScreenTexturedQuad(const zeus::CColor& color, CTexture* tex, float lod);
  static void DrawFullScreenTexturedQuadQuarters(const zeus::CColor& color, CTexture* tex, float lod);
  static void DrawRandomStatic(const zeus::CColor& color, float alpha, bool cookieCutterDepth);
  static void DrawScanLines(const zeus::CColor& color, bool even);
  static void DrawWideScreen(const zeus::CColor& color, CTexture* tex, float lod);

public:
  void Update(float dt);
  void SetFilter(EFilterType type, EFilterShape shape, float time, const zeus::CColor& color, CAssetId txtr);
  void DisableFilter(float time);
  void Draw();

  static void DrawFilter(EFilterType type, EFilterShape shape, const zeus::CColor& color, CTexture* tex, float lod);
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
  bool x2c_usePersistent = false;
  bool x2d_noPersistentCopy = false;
  u32 x30_persistentBuf = 0;

public:
  void Draw(bool clearDepth = false);
  void Update(float dt);
  void SetBlur(EBlurType type, float amount, float duration, bool usePersistentFb);
  void DisableBlur(float duration);
  EBlurType GetCurrType() const { return x10_curType; }
};

} // namespace metaforce
