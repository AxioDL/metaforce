#ifndef _CCAMERABLURPASS
#define _CCAMERABLURPASS

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

#include "dolphin/gx/GXEnum.h"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/optional_object.hpp"

class CTexture;

class CCameraBlurPass {
public:
  enum EBlurType {
    kBT_NoBlur,
    kBT_LoBlur,
    kBT_HiBlur,
    kBT_XRay,
  };
  CCameraBlurPass();

  void Update(float dt);
  void SetBlur(EBlurType type, float amount, float duration, bool usePersistentFb);
  void DisableBlur(float duration);
  void Draw() const;

  static float GetXRayViewportScaleX() { return skXRayViewportScaleX; }
  static float GetXRayViewportScaleY() { return skXRayViewportScaleY; }

  static void DrawWideScreen(const CColor& color, const CTexture* tex, float v);

  EBlurType GetCurrType() const { return mCurType; }
  bool GetNoPersistentCopy() const { return mNoPersistentCopy; }

  void GetFbCopy(GXTexFmt fmt, uchar* buf) const;
  void FreePersistentFbTexture();
  void AllocatePersistentFbTexture();

private:
  static const float skXRayViewportScaleX;
  static const float skXRayViewportScaleY;

  rstl::optional_object< TCachedToken< CTexture > > mPaletteTex;
  EBlurType mCurType;
  EBlurType mEndType;
  float mEndValue;
  float mCurValue;
  float mStartValue;
  float mTotalTime;
  float mRemainingTime;
  bool mUsePersistent;
  mutable bool mNoPersistentCopy;
  void* mPersistentBuf;
};
CHECK_SIZEOF(CCameraBlurPass, 0x34)

#endif // _CCAMERABLURPASS
