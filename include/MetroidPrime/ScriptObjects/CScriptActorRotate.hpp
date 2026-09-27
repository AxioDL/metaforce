#ifndef _CSCRIPTACTORROTATE
#define _CSCRIPTACTORROTATE

#include "MetroidPrime/CEntity.hpp"

#include "rstl/map.hpp"

struct SRiders;
class CScriptPlatform;

class CScriptActorRotate : public CEntity {
public:
  CScriptActorRotate(TUniqueId, const rstl::string&, const CEntityInfo&, const CVector3f&,
                     float, bool, bool, bool);
  ~CScriptActorRotate();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;

private:
  CVector3f mRotation;
  float mMaxTime;
  float mCurrentTime; // = 0.f;
  // rstl::map< TUniqueId, CTransform4f > x48_actors;
  rstl::vector< rstl::pair< TUniqueId, CTransform4f > > mActors;

  bool mUpdateRotation : 1;            // = false;
  bool mUpdateSpiderBallWaypoints : 1; // = false;
  bool mUpdateActors : 1;
  bool mUpdateOnCreation : 1;

  void UpdateActors(bool, CStateManager&);
  void RebuildSpiderBallWaypoints(CStateManager&);
  void UpdatePlatformRiders(CScriptPlatform&, const CTransform4f&, CStateManager&);
  void UpdatePlatformRiders(rstl::vector< SRiders >&, CScriptPlatform&, const CTransform4f&,
                            CStateManager&);
};

#endif // _CSCRIPTACTORROTATE
