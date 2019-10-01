#pragma once

#include <string_view>

#include "Runtime/Camera/CCameraShakeData.hpp"
#include "Runtime/World/CEntity.hpp"

namespace urde {

class CScriptCameraShaker : public CEntity {
  CCameraShakeData x34_shakeData;

public:
  CScriptCameraShaker(TUniqueId uid, std::string_view name, const CEntityInfo& info, bool active,
                      const CCameraShakeData& shakeData);
  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
};

} // namespace urde
