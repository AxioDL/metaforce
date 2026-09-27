#ifndef _CSCRIPTPICKUPGENERATOR
#define _CSCRIPTPICKUPGENERATOR

#include "MetroidPrime/CEntity.hpp"

class CScriptPickupGenerator : public CEntity {
public:
  CScriptPickupGenerator(TUniqueId, const rstl::string&, const CEntityInfo&, const CVector3f&,
                         float, bool);
  ~CScriptPickupGenerator();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender,
                       CStateManager& stateMgr) override;

  // float GetNothingPossibility() const;

private:
  CVector3f mPosition;
  float mFrequency;
  float mDelayTimer;

  void ResetSpawnNothingCounter();
  void GetTargets(CStateManager& mgr, TUniqueId sender, rstl::vector< TUniqueId >& idsOut) const;
  float GetSpawnablePickups(CStateManager& mgr,
                            rstl::vector< rstl::pair< float, TEditorId > >& idsOut) const;
  void SpawnPickup(CStateManager& mgr, TEditorId templateId, TUniqueId generatorId) const;
};

#endif // _CSCRIPTPICKUPGENERATOR
