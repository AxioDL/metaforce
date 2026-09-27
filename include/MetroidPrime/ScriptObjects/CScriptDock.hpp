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
  TAreaId GetAreaId() const { return mArea; }
  int GetDockId() const { return mDock; }
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
  int mDockReferenceCount;
  int mDock;
  TAreaId mArea;
  EDockState mDockState;
  bool mDockReferenced : 1;
  bool mLoadConnected : 1;
  bool mAreaPostConstructed : 1;
};
CHECK_SIZEOF(CScriptDock, (VERSION >= VERSION_GM8E_02 ? 0x280 : 0x270))

#endif // _CSCRIPTDOCK
