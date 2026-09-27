#ifndef _CWORLDSHADOW
#define _CWORLDSHADOW

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/single_ptr.hpp"

class CTexture;
class CStateManager;
class CAABox;

class CWorldShadow {
  rstl::single_ptr< CTexture > mTexture;
  CTransform4f mView;
  CTransform4f mModel;
  float mObjHalfExtent;
  CVector3f mObjPos;
  CVector3f mLightPos;
  TAreaId mAid;
  int mLightIdx;
  bool mBlurReset;

public:
  CWorldShadow(uint w, uint h, bool rgba8);
  ~CWorldShadow();

  void EnableModelProjectedShadow(const CTransform4f& pos, uint lightIdx, float f1) const;
  void DisableModelProjectedShadow() const;
  void BuildLightShadowTexture(const CStateManager& mgr, TAreaId aid, uint lightIdx,
                               const CAABox& aabb, bool motionBlur, bool lighten);
  void ResetBlur();
};
CHECK_SIZEOF(CWorldShadow, 0x8c)

#endif // _CWORLDSHADOW
