#ifndef _CSCRIPTDOCK
#define _CSCRIPTDOCK

#include "MetroidPrime/CPhysicsActor.hpp"

class CScriptDock : public CPhysicsActor {
public:
  CScriptDock(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
              const CVector3f& position, const CVector3f& extent, int dock, TAreaId area,
              bool active, int dockReferenceCount, const bool loadConnected);
  ~CScriptDock();

  void SetDockReference(CStateManager& mgr, int);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  rstl::optional_object< CAABox > GetTouchBounds() const override;
  void Touch(CActor&, CStateManager&) override;
  CPlane GetPlane(const CStateManager&) const;
  TAreaId GetAreaId() const { return x260_area; }
  int GetDockId() const { return x25c_dock; }
  int GetDockReference(const CStateManager& mgr) const;
  TAreaId GetCurrentConnectedAreaId(const CStateManager&) const;
  void UpdateAreaActivateFlags(CStateManager&);
  bool HasPointCrossedDock(const CStateManager&, const CVector3f&) const;
  void AreaLoaded(CStateManager&);
  void AreaUnloaded(CStateManager&);
  void SetLoadConnected(CStateManager&, bool);

private:
  enum EDockState { kDS_InSourceRoom, kDS_PlayerTouched, kDS_EnterNextArea, kDS_InNextRoom };

  friend class CScriptDoor;
  int x258_dockReferenceCount;
  int x25c_dock;
  TAreaId x260_area;
  EDockState x264_dockState;
  bool x268_24_dockReferenced : 1;
  bool x268_25_loadConnected : 1;
  bool x268_26_areaPostConstructed : 1;
};
CHECK_SIZEOF(CScriptDock, 0x270)

#endif // _CSCRIPTDOCK
