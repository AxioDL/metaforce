#ifndef _CPROJECTEDSHADOW
#define _CPROJECTEDSHADOW

#include "GameVersions.h"

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

#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  void RenderShadowBuffer(CStateManager&, int, const CModelData* const*, const CTransform4f* const*,
                          int, const CVector3f&, float, float);

  void SetNextShadow(CProjectedShadow* shadow) { x9c_nextShadow = shadow; }
  CProjectedShadow* GetNextShadow() const { return x9c_nextShadow; }
#endif

  void SetOpacity(float opacity) { x98_opacity = opacity; }

  void Disable();

  static void ModelDrawCallback(const float*, const float*, const SShadowDrawContext*);

private:
  void ExpandBoundsForTexture();

  CTexture x0_texture;
  CAABox x68_bounds;
  bool x80_enabled;
  uchar x81_persistent;
  float x84_scale;
  CVector3f x88_translation;
  float x94_zDistanceAdjust;
  float x98_opacity;
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  CProjectedShadow* x9c_nextShadow;
#endif
};
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
CHECK_SIZEOF(CProjectedShadow, 0xa0)
#else
CHECK_SIZEOF(CProjectedShadow, 0x9c)
#endif

#endif // _CPROJECTEDSHADOW
