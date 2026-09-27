#ifndef _CSCRIPTDEBRIS
#define _CSCRIPTDEBRIS

#include "types.h"

#include "MetroidPrime/CPhysicsActor.hpp"

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TReservedAverage.hpp"

#include "rstl/single_ptr.hpp"

class CActorParameters;
class CElementGen;
class CEntityInfo;
class CModelData;

class CScriptDebris : public CPhysicsActor {
public:
  enum EOrientationType {
    kOT_NotOriented,
    kOT_AlongVelocity,
    kOT_ToObject,
    kOT_AlongCollisionNormal,
  };

  enum EScaleType {
    kST_NoScale,
    kST_EndsToZero,
  };

  // Extended constructor (LoadDebrisExtended)
  CScriptDebris(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& mData, const CActorParameters& aParams,
                float linConeAngle, float linMinMag, float linMaxMag, float angMinMag,
                float angMaxMag, float minDuration, float maxDuration, float colorInT,
                float colorOutT, const CColor& color, const CColor& endsColor, float scaleOutStartT,
                const CVector3f& scale, const CVector3f& endScale, float restitution,
                float downwardSpeed, const CVector3f& localOffset, uint particle0,
                const CVector3f& particle0Scale, bool particle1GlobalTranslation,
                bool deferDeleteTillParticle1Done, EOrientationType particleOr0, uint particle1,
                const CVector3f& particle1Scale, bool particle2GlobalTranslation,
                bool deferDeleteTillParticle2Done, EOrientationType particleOr1, uint particle2,
                const CVector3f& particle2Scale, EOrientationType particleOr2,
                bool solid, bool dieOnProjectile, bool noBounce, bool active);

  // Simple constructor (LoadDebris)
  CScriptDebris(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                const CTransform4f& xf, const CModelData& mData, const CActorParameters& aParams,
                uint particleId, const CVector3f& particleScale, float zImpulse,
                const CVector3f& velocity, const CColor& endsColor, float mass,
                float restitution, float duration, EScaleType scaleType,
                bool unused, bool randomAngImpulse, bool active);

  // CEntity
  ~CScriptDebris() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) override;
  void AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const override;
  void Render(const CStateManager& mgr) const override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor& other, CStateManager& mgr) override;

  // CPhysicsActor
  void CollidedWith(const TUniqueId& id, const CCollisionInfoList& list,
                    CStateManager& mgr) override;

private:
  CVector3f mVelocity;
  CColor mColor;
  CColor mEndsColor;
  float mZImpulse;
  float mCurTime;
  float mDuration;
  float mOoDuration;
  float mRestitution;
  uchar mScaleType;
  bool mRandomAngImpulse : 1;
  bool mParticle1GlobalTranslation : 1;
  bool mDeferDeleteTillParticle1Done : 1;
  bool mParticle2GlobalTranslation : 1;
  bool mDeferDeleteTillParticle2Done : 1;
  bool mParticle3Active : 1;
  bool mDebrisExtended : 1;
  bool mDieOnProjectile : 1;
  bool mNoBounce : 1;
  char mParticleOr0;
  char mParticleOr1;
  char mParticleOr2;
  float mLinConeAngle;
  float mLinMinMag;
  float mLinMaxMag;
  float mAngMinMag;
  float mAngMaxMag;
  float mMinDuration;
  float mMaxDuration;
  float mColorInT;
  float mColorOutT;
  float mScaleOutStartT;
  CVector3f mScale;
  CVector3f mEndScale;
  CVector3f mCollisionNormal;
  rstl::single_ptr< CElementGen > mParticleGen0;
  rstl::single_ptr< CElementGen > mParticleGen1;
  rstl::single_ptr< CElementGen > mParticleGen2;
  TReservedAverage< float, 8 > mSpeedAvg;
};
CHECK_SIZEOF(CScriptDebris, (VERSION >= VERSION_GM8E_02 ? 0x318 : 0x308))

#endif // _CSCRIPTDEBRIS
