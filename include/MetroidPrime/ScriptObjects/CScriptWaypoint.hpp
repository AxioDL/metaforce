#ifndef _CSCRIPTWAYPOINT
#define _CSCRIPTWAYPOINT

#include "types.h"

#include "MetroidPrime/CActor.hpp"

class CScriptWaypoint : public CActor {
public:
  CScriptWaypoint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                  const CTransform4f& xf, const bool active, float speed, float pause,
                  int patternTranslate, int patternOrient, int patternFit, int behaviour,
                  int behaviourOrient, int behaviourModifiers, uint animation);

  // CEntity
  ~CScriptWaypoint() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;

  // CScriptWaypoint
  TUniqueId NextWaypoint(const CStateManager& mgr) const;
  TUniqueId FollowWaypoint(CStateManager& mgr) const;

  float GetSpeed() const { return xe8_speed; }
  uint GetAnimation() const { return xec_animation; }
  float GetPause() const { return xf0_pause; }
  uchar GetPatternTranslate() const { return xf4_patternTranslate; }
  uchar GetPatternOrient() const { return xf5_patternOrient; }
  uchar GetPatternFit() const { return xf6_patternFit; }
  uchar GetBehaviour() const { return xf7_behaviour; }
  uchar GetBehaviourOrient() const { return xf8_behaviourOrient; }
  ushort GetBehaviourModifiers() const { return xfa_behaviourModifiers; }

private:
  float xe8_speed;
  uint xec_animation;
  float xf0_pause;
  uchar xf4_patternTranslate;
  uchar xf5_patternOrient;
  uchar xf6_patternFit;
  uchar xf7_behaviour;
  uchar xf8_behaviourOrient;
  ushort xfa_behaviourModifiers;
};

#endif // _CSCRIPTWAYPOINT
