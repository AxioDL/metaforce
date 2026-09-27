#ifndef _CSIMPLESHADOW
#define _CSIMPLESHADOW

#include "types.h"

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/TToken.hpp"

class CAABox;
class CStateManager;
class CTexture;

class CSimpleShadow {
public:
  CSimpleShadow(float scale, float userAlpha, float maxObjHeight, float displacement);

  void Calculate(const CAABox& bounds, const CTransform4f& xf, const CStateManager& mgr);
  void Render(const CTexture* tex) const;
  const CTransform4f& GetTransform() const;
  void SetUserAlpha(float);
  float GetMaxObjectHeight() const;

  void SetAlwaysCalculateRadius(bool);
  CAABox GetBounds() const;
  CAABox GetMaxShadowBox(const CAABox& bounds) const;
  bool Valid() const;

private:
  CTransform4f mXf;
  float mScale;
  float mRadius;
  float mUserAlpha;
  float mHeightAlpha;
  float mMaxObjHeight;
  float mDisplacement;
  bool mCollision : 1;
  bool mAlwaysCalculateRadius : 1;
  bool mRadiusCalculated : 1;
};
CHECK_SIZEOF(CSimpleShadow, 0x4c)

#endif // _CSIMPLESHADOW
