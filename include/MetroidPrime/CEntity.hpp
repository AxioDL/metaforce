#ifndef _CENTITY
#define _CENTITY

#include "types.h"

#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/string.hpp"
#include "rstl/vector.hpp"

class CValidEntityPredicate;

class CEntity {
public:
  virtual ~CEntity();
#ifndef HAS_TYPES_MATCH
  virtual void Accept(IVisitor& visitor) = 0;
#else
  virtual CEntity* TypesMatch(int type);
#endif
  virtual void PreThink(float dt, CStateManager& mgr);
  virtual void Think(float dt, CStateManager& mgr);
  virtual void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr);
  virtual void SetActive(const bool active);

  CEntity(TUniqueId id, const CEntityInfo& info, const bool active, const rstl::string& name);

  void SendScriptMsgs(const EScriptObjectState state, CStateManager& mgr,
                      const EScriptObjectMessage msg);
  const TUniqueId GetUniqueId() const { return mUid; }
  const TEditorId GetEditorId() const { return mEditorId; }
  const rstl::string& GetDebugName() const { return mName; }
  const TAreaId GetAreaId() const;
  TUniqueId CheckConnectedObject_if(const CStateManager& mgr, EScriptObjectState state,
                                   EScriptObjectMessage msg,
                                   const CValidEntityPredicate& predicate) const;
  const TAreaId GetCurrentAreaId() const { return mAreaId; }
  const bool GetActive() const { return mActive; }
  bool IsInGraveyard() const { return mInGraveyard; }
  void SetIsInGraveyard() { mInGraveyard = true; }
  bool IsScriptingBlocked() const { return mScriptingBlocked; }

  // might be fake?
  rstl::vector< SConnection >& ConnectionList() { return mConns; }
  const rstl::vector< SConnection >& GetConnectionList() const { return mConns; }

  static rstl::vector< SConnection > NullConnectionList;

private:
  friend class CStateManager;

  void __SetCurrentAreaId(TAreaId areaId) { mAreaId = areaId; }

  TAreaId mAreaId;
  TUniqueId mUid;
  TEditorId mEditorId;
  rstl::string mName;
  rstl::vector< SConnection > mConns;
  bool mActive : 1;
  bool mInGraveyard : 1;
  bool mScriptingBlocked : 1;
  bool mNotInArea : 1;
};

CHECK_SIZEOF(CEntity, (VERSION >= VERSION_R3IJ_00 ? 0x2c : 0x34))

#endif // _CENTITY
