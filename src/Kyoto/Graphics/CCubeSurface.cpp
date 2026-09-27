#include "Kyoto/Graphics/CCubeSurface.hpp"


const CVector3f CCubeSurface::skDefaultNormal(1.f, 0.f, 0.f);


CAABox CCubeSurface::GetBounds() const {
  if (mData->mExtraSize != 0) {
    return mData->mBounds;
  }

  return CAABox(mData->mCenter, mData->mCenter);
}
