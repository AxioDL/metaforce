#ifndef _CPROJECTEDSHADOW
#define _CPROJECTEDSHADOW

#include "GameVersions.h"
#include "Kyoto/Graphics/ModelTypes.hpp"

#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAABox.hpp"

class CStateManager;
class CModelData;
struct SShadowDrawContext;

class CProjectedShadow {
public:
  CProjectedShadow(int, int, uchar);
  ~CProjectedShadow();

  void Render(const CStateManager&) const;
  void RenderShadowBuffer(CStateManager&, const CModelData&, const CTransform4f&, int,
                          const CVector3f&, float, float);

#if VERSION >= VERSION_GM8P_00
  void RenderShadowBuffer(CStateManager&, int, const CModelData* const*, const CTransform4f* const*,
                          int, const CVector3f&, float, float);

  void SetNextShadow(CProjectedShadow* shadow) { mNextShadow = shadow; }
  CProjectedShadow* GetNextShadow() const { return mNextShadow; }
#endif

  void SetOpacity(float opacity) { mOpacity = opacity; }

  void Disable();

  static void ModelDrawCallback(TModelPositions, TModelNormals, const SShadowDrawContext*);

private:
  void ExpandBoundsForTexture();

  CTexture mTexture;
  CAABox mBounds;
  bool mEnabled;
  uchar mPersistent;
  float mScale;
  CVector3f mTranslation;
  float mZDistanceAdjust;
  float mOpacity;
#if VERSION >= VERSION_GM8P_00
  CProjectedShadow* mNextShadow;
#endif
};
#if VERSION >= VERSION_GM8P_00
CHECK_SIZEOF(CProjectedShadow, 0xa0)
#else
CHECK_SIZEOF(CProjectedShadow, 0x9c)
#endif

#endif // _CPROJECTEDSHADOW
