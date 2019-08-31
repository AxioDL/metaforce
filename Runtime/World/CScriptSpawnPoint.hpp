#pragma once

#include "CEntity.hpp"
#include "zeus/CTransform.hpp"
#include "CPlayerState.hpp"

namespace urde {

class CScriptSpawnPoint : public CEntity {
  zeus::CTransform x34_xf;
  rstl::reserved_vector<u32, int(CPlayerState::EItemType::Max)> x64_itemCounts;
  union {
    struct {
      bool x10c_24_firstSpawn : 1;
      bool x10c_25_morphed : 1;
    };
    u8 _dummy = 0;
  };

public:
  CScriptSpawnPoint(TUniqueId, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                    const rstl::reserved_vector<u32, int(CPlayerState::EItemType::Max)>& itemCounts, bool, bool, bool);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  bool FirstSpawn() const { return x10c_24_firstSpawn; }
  const zeus::CTransform& GetTransform() const { return x34_xf; }
  u32 GetPowerup(CPlayerState::EItemType item) const;
};

} // namespace urde
