#ifndef _CSAMUSFACEREFLECTION
#define _CSAMUSFACEREFLECTION

#include "MetroidPrime/CModelData.hpp"

#include "Kyoto/Math/CQuaternion.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/single_ptr.hpp"

class CStateManager;
class CRandom16;
class CActorLights;

class CSamusFaceReflection {

public:
  explicit CSamusFaceReflection(const CStateManager& mgr);
  void PreDraw(const CStateManager& mgr);
  void Draw(const CStateManager& mgr) const;
  void Update(float dt, const CStateManager& mgr, CRandom16& rand);

private:
  CModelData mModelData;
  rstl::single_ptr< CActorLights > mLights;
  CQuaternion mLookRot;
  CVector3f mLookDir;
  int x6c_;
  bool mHidden;
};

#endif // _CSAMUSFACEREFLECTION
