#ifndef _CSCRIPTSPAWNPOINT
#define _CSCRIPTSPAWNPOINT

#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/Player/CPlayerState.hpp"

#include "Kyoto/Math/CTransform4f.hpp"

class CScriptSpawnPoint : public CEntity {
  CTransform4f mXf;
  rstl::reserved_vector< int, int(CPlayerState::kIT_Max) > mItemCounts;
  bool mFirstSpawn : 1;
  bool mMorphed : 1;

public:
  CScriptSpawnPoint(TUniqueId, const rstl::string& name, const CEntityInfo& info,
                    const CTransform4f& xf,
                    const rstl::reserved_vector< int, int(CPlayerState::kIT_Max) >& itemCounts,
                    bool, bool, const bool);

  ~CScriptSpawnPoint() override;
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;

  bool FirstSpawn() const { return mFirstSpawn; }
  const CTransform4f& GetTransform() const;
  int GetPowerup(const CPlayerState::EItemType&) const;
};

#endif // _CSCRIPTSPAWNPOINT
