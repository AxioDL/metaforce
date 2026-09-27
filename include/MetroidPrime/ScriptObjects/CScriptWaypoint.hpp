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

  float GetSpeed() const { return mSpeed; }
  uint GetAnimation() const { return mAnimation; }
  float GetPause() const { return mPause; }
  uchar GetPatternTranslate() const { return mPatternTranslate; }
  uchar GetPatternOrient() const { return mPatternOrient; }
  uchar GetPatternFit() const { return mPatternFit; }
  uchar GetBehaviour() const { return mBehaviour; }
  uchar GetBehaviourOrient() const { return mBehaviourOrient; }
  ushort GetBehaviourModifiers() const { return mBehaviourModifiers; }

private:
  float mSpeed;
  uint mAnimation;
  float mPause;
  uchar mPatternTranslate;
  uchar mPatternOrient;
  uchar mPatternFit;
  uchar mBehaviour;
  uchar mBehaviourOrient;
  ushort mBehaviourModifiers;
};

#endif // _CSCRIPTWAYPOINT
