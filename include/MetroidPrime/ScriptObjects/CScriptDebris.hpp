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
  CVector3f x258_velocity;
  CColor x264_color;
  CColor x268_endsColor;
  float x26c_zImpulse;
  float x270_curTime;
  float x274_duration;
  float x278_ooDuration;
  float x27c_restitution;
  uchar x280_scaleType;
  bool x281_24_randomAngImpulse : 1;
  bool x281_25_particle1GlobalTranslation : 1;
  bool x281_26_deferDeleteTillParticle1Done : 1;
  bool x281_27_particle2GlobalTranslation : 1;
  bool x281_28_deferDeleteTillParticle2Done : 1;
  bool x281_29_particle3Active : 1;
  bool x281_30_debrisExtended : 1;
  bool x281_31_dieOnProjectile : 1;
  bool x282_24_noBounce : 1;
  char x283_particleOr0;
  char x284_particleOr1;
  char x285_particleOr2;
  float x288_linConeAngle;
  float x28c_linMinMag;
  float x290_linMaxMag;
  float x294_angMinMag;
  float x298_angMaxMag;
  float x29c_minDuration;
  float x2a0_maxDuration;
  float x2a4_colorInT;
  float x2a8_colorOutT;
  float x2ac_scaleOutStartT;
  CVector3f x2b0_scale;
  CVector3f x2bc_endScale;
  CVector3f x2c8_collisionNormal;
  rstl::single_ptr< CElementGen > x2d4_particleGen0;
  rstl::single_ptr< CElementGen > x2d8_particleGen1;
  rstl::single_ptr< CElementGen > x2dc_particleGen2;
  TReservedAverage< float, 8 > x2e0_speedAvg;
};
CHECK_SIZEOF(CScriptDebris, (VERSION >= VERSION_GM8P_00 ? 0x318 : 0x308))

#endif // _CSCRIPTDEBRIS
