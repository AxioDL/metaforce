#ifndef _CBSLOCOMOTION
#define _CBSLOCOMOTION

#include "MetroidPrime/BodyState/CBodyState.hpp"

#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"

class CActor;

class CBSLocomotion : public CBodyState {
public:
  CBSLocomotion();
  ~CBSLocomotion() override {}

  bool IsMoving() const override = 0;
  bool CanShoot() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  void Shutdown(CBodyController& bc) override;
  virtual bool IsPitchable() const;
  virtual float GetLocomotionSpeed(pas::ELocomotionType type, pas::ELocomotionAnim anim) const = 0;
  virtual float ApplyLocomotionPhysics(float dt, CBodyController& bc);
  virtual float UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc, bool init) = 0;
  virtual pas::EAnimationState GetBodyStateTransition(float dt, CBodyController& bc);

protected:
  pas::ELocomotionType mLocomotionType;

  float GetStartVelocityMagnitude(CBodyController& bc) const;
  void ReStartBodyState(CBodyController& bc, bool maintainVel);
  float ComputeWeightPercentage(const rstl::pair< int, float >& a, const rstl::pair< int, float >& b,
                                float f) const;
};
CHECK_SIZEOF(CBSLocomotion, 0x8)

class CBSBiPedLocomotion : public CBSLocomotion {
public:
  explicit CBSBiPedLocomotion(CActor& actor);
  ~CBSBiPedLocomotion() override;

  bool IsMoving() const override;
  void Start(CBodyController& bc, CStateManager& mgr) override;
  pas::EAnimationState UpdateBody(float dt, CBodyController& bc, CStateManager& mgr) override;
  float GetLocomotionSpeed(pas::ELocomotionType type, pas::ELocomotionAnim anim) const override;
  float UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc, bool init) override;
  virtual bool IsStrafing(CBodyController& bc) const;

protected:
  static const float skMinWalkPercent;

  rstl::reserved_vector< rstl::reserved_vector< rstl::pair< int, float >, 8 >, 14 > mAnims;
  pas::ELocomotionAnim mAnim;
  float mPrimeTime;

  float UpdateRun(float vel, CBodyController& bc, pas::ELocomotionAnim anim);
  float UpdateWalk(float vel, CBodyController& bc, pas::ELocomotionAnim anim);
  float UpdateStrafe(float vel, CBodyController& bc, pas::ELocomotionAnim anim);
  const rstl::pair< int, float >& GetLocoAnimation(pas::ELocomotionType type, pas::ELocomotionAnim anim) const;
};
CHECK_SIZEOF(CBSBiPedLocomotion, 0x3CC)

class CBSWallWalkerLocomotion : public CBSBiPedLocomotion {
public:
  explicit CBSWallWalkerLocomotion(CActor& actor);
  ~CBSWallWalkerLocomotion() override;

  float ApplyLocomotionPhysics(float dt, CBodyController& bc) override;
};
CHECK_SIZEOF(CBSWallWalkerLocomotion, 0x3CC)

class CBSAiMovedFlyerLocomotion : public CBSBiPedLocomotion {
public:
  explicit CBSAiMovedFlyerLocomotion(CActor& actor);
  ~CBSAiMovedFlyerLocomotion() override;

  float ApplyLocomotionPhysics(float dt, CBodyController& bc) override;
  float UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc, bool init) override;
};
CHECK_SIZEOF(CBSAiMovedFlyerLocomotion, 0x3CC)

class CBSFlyerLocomotion : public CBSBiPedLocomotion {
public:
  explicit CBSFlyerLocomotion(CActor& actor, bool pitchable);
  ~CBSFlyerLocomotion() override;

  bool IsPitchable() const override;
  float ApplyLocomotionPhysics(float dt, CBodyController& bc) override;
  virtual bool IsBackPedal(CBodyController& bc) const;

private:
  bool mPitchable;
};
CHECK_SIZEOF(CBSFlyerLocomotion, 0x3D0)

class CBSRestrictedLocomotion : public CBSLocomotion {
public:
  explicit CBSRestrictedLocomotion(CActor& actor);
  ~CBSRestrictedLocomotion() override {}

  bool IsMoving() const override;
  float GetLocomotionSpeed(pas::ELocomotionType type, pas::ELocomotionAnim anim) const override;
  float UpdateLocomotionAnimation(float dt, float velMag, CBodyController& bc, bool init) override;

private:
  rstl::reserved_vector< int, 14 > mAnims;
  pas::ELocomotionAnim mAnim;
};
CHECK_SIZEOF(CBSRestrictedLocomotion, 0x48)

class CBSFloaterLocomotion : public CBSRestrictedLocomotion {
public:
  explicit CBSFloaterLocomotion(CActor& actor);
  ~CBSFloaterLocomotion() override;

  float ApplyLocomotionPhysics(float dt, CBodyController& bc) override;
};
CHECK_SIZEOF(CBSFloaterLocomotion, 0x48)

#endif // _CBSLOCOMOTION
