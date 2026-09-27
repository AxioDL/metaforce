#ifndef _CCAMERAFILTERPASS
#define _CCAMERAFILTERPASS

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"

class CTexture;

class CCameraFilterPass {
public:
  enum EFilterType {
    kFT_Passthru,
    kFT_Multiply,
    kFT_Invert,
    kFT_Add,
    kFT_Subtract,
    kFT_Blend,
    kFT_Widescreen,
    kFT_SceneAdd,
    kFT_NoColor,
    kFT_InvDstMultiply,
  };
  enum EFilterShape {
    kFS_Fullscreen,
    kFS_FullscreenHalvesLeftRight,
    kFS_FullscreenHalvesTopBottom,
    kFS_FullscreenQuarters,
    kFS_CinemaBars,
    kFS_ScanLinesEven,
    kFS_ScanLinesOdd,
    kFS_RandomStatic,
    kFS_CookieCutterDepthRandomStatic
  };

  CCameraFilterPass();

  void SetFilter(const EFilterType type, const EFilterShape shape, const float time,
                 const CColor& color, const CAssetId txtr);
  void DisableFilter(float time);
  void Update(float dt);
  void Draw() const;

  float GetT(bool invert) const;

  static void DrawWideScreen(const CColor& color, const CTexture* tex, float v);
  static void DrawFilter(EFilterType type, EFilterShape shape, const CColor& color,
                         const CTexture* tex, float lod);
  static void DrawFilterShape(EFilterShape shape, const CColor& color, const CTexture* tex,
                              float lod);
  static void DrawFullScreenColoredQuad(const CColor& color);
  static void DrawFullScreenTexturedQuad(const CColor& color, const CTexture* tex, float lod);
  static void DrawFullScreenTexturedQuadQuarters(const CColor& color, const CTexture* tex,
                                                 float lod);
  static void DrawScanLines(const CColor& color, bool even);
  static void DrawRandomStatic(const CColor& color, float alpha, bool cookieCutter);

private:
  EFilterType mCurType;
  EFilterType mNextType;
  EFilterShape mShape;
  float mDuration;
  float mRemTime;
  CColor mPrevColor;
  CColor mCurColor;
  CColor mNextColor;
  CAssetId mNextTxtr;
  rstl::auto_ptr< TLockedToken< CTexture > > mTexObj;
};
CHECK_SIZEOF(CCameraFilterPass, 0x2c)

#endif // _CCAMERAFILTERPASS
