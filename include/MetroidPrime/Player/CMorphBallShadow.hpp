#ifndef _CMORPHBALLSHADOW
#define _CMORPHBALLSHADOW

#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/TToken.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/list.hpp"
#include "rstl/vector.hpp"

class CActor;
class CPlayer;
class CStateManager;

class CMorphBallShadow {
public:
  CMorphBallShadow(int width, int height, const TToken< CTexture >& ballFade);
  ~CMorphBallShadow();

  void Render(CStateManager& mgr, float alpha);
  void RenderIdBuffer(const CAABox& aabb, CStateManager& mgr, CPlayer& player);

private:
  void GatherAreas(CStateManager& mgr);
  bool AreasValid(const CStateManager& mgr) const;

  rstl::list< CActor* > mActors;
  rstl::list< TAreaId > mAreas;
  rstl::vector< uint > mWorldModelBits;
  CTexture mTexture;
  TToken< CTexture > mBallFade;
  int mWidth;
  int mHeight;
  CAABox mShadowVolume;
  bool mHasIds;
};
CHECK_SIZEOF(CMorphBallShadow, 0xd4)

#endif // _CMORPHBALLSHADOW
