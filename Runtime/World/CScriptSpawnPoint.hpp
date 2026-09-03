#pragma once

#include <string_view>

#include "Runtime/CPlayerState.hpp"
#include "Runtime/GCNTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/World/CEntity.hpp"

#include <zeus/CTransform4f.hpp>

namespace metaforce {

class CScriptSpawnPoint : public CEntity {
  zeus::CTransform4f x34_xf;
  rstl::reserved_vector<u32, int(CPlayerState::EItemType::Max)> x64_itemCounts;
  bool x10c_24_firstSpawn : 1;
  bool x10c_25_morphed : 1;

public:
  DEFINE_ENTITY
  CScriptSpawnPoint(TUniqueId, std::string_view name, const CEntityInfo& info, const zeus::CTransform4f& xf,
                    const rstl::reserved_vector<u32, int(CPlayerState::EItemType::Max)>& itemCounts, bool, bool, bool);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  bool FirstSpawn() const { return x10c_24_firstSpawn; }
  const zeus::CTransform4f& GetTransform() const { return x34_xf; }
  u32 GetPowerup(CPlayerState::EItemType item) const;
};

} // namespace metaforce
