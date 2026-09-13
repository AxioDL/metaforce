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
  CVector3f x34_rotation;
  float x40_maxTime;
  float x44_currentTime; // = 0.f;
  // rstl::map< TUniqueId, CTransform4f > x48_actors;
  rstl::vector< rstl::pair< TUniqueId, CTransform4f > > x48_actors;

  bool x58_24_updateRotation : 1;            // = false;
  bool x58_25_updateSpiderBallWaypoints : 1; // = false;
  bool x58_26_updateActors : 1;
  bool x58_27_updateOnCreation : 1;

  void UpdateActors(bool, CStateManager&);
  void RebuildSpiderBallWaypoints(CStateManager&);
  void UpdatePlatformRiders(CScriptPlatform&, const CTransform4f&, CStateManager&);
  void UpdatePlatformRiders(rstl::vector< SRiders >&, CScriptPlatform&, const CTransform4f&,
                            CStateManager&);
};

#endif // _CSCRIPTACTORROTATE
