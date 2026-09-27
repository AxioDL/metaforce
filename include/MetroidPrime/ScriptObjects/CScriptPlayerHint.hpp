#ifndef _CSCRIPTPLAYERHINT
#define _CSCRIPTPLAYERHINT

#include "MetroidPrime/CActor.hpp"

class CScriptPlayerHint : public CActor {
public:
  CScriptPlayerHint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                    const CTransform4f& xf, bool active, int priority, int overrideFlags);
  ~CScriptPlayerHint();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  int GetPriority() const { return mPriority; }
  int GetOverrideFlags() const { return mOverrideFlags; }
  TUniqueId GetActorId() const { return mMpId; }
  void ClearObjectList();
  uint GetObjectCount() const { return mObjectList.size(); }
  bool GetDeactivated() const { return mDeactivated; }
  void SetDeactivated() { mDeactivated = true; }

private:
  rstl::reserved_vector< TUniqueId, 8 > mObjectList;
  bool mDeactivated;
  int mPriority;
  int mOverrideFlags;
  TUniqueId mMpId;

  void AddToObjectList(TUniqueId uid);
  void RemoveFromObjectList(TUniqueId uid, CStateManager& mgr);
};

#endif // _CSCRIPTPLAYERHINT
