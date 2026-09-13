#ifndef _CENTITY
#define _CENTITY

#include "types.h"

#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "rstl/string.hpp"
#include "rstl/vector.hpp"

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
  const TUniqueId GetUniqueId() const { return x8_uid; }
  const TEditorId GetEditorId() const { return xc_editorId; }
  const rstl::string& GetDebugName() const { return x10_name; }
  const TAreaId GetAreaId() const;
  const TAreaId GetCurrentAreaId() const { return x4_areaId; }
  const bool GetActive() const { return x30_24_active; }
  bool IsInGraveyard() const { return x30_25_inGraveyard; }
  void SetIsInGraveyard() { x30_25_inGraveyard = true; }
  bool IsScriptingBlocked() const { return x30_26_scriptingBlocked; }

  // might be fake?
  rstl::vector< SConnection >& ConnectionList() { return x20_conns; }
  const rstl::vector< SConnection >& GetConnectionList() const { return x20_conns; }

  static rstl::vector< SConnection > NullConnectionList;

private:
  friend class CStateManager;

  void __SetCurrentAreaId(TAreaId areaId) { x4_areaId = areaId; }

  TAreaId x4_areaId;
  TUniqueId x8_uid;
  TEditorId xc_editorId;
  rstl::string x10_name;
  rstl::vector< SConnection > x20_conns;
  bool x30_24_active : 1;
  bool x30_25_inGraveyard : 1;
  bool x30_26_scriptingBlocked : 1;
  bool x30_27_notInArea : 1;
};

CHECK_SIZEOF(CEntity, 0x34)

#endif // _CENTITY
