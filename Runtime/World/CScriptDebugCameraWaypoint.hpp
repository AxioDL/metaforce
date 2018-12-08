#pragma once

#include "CActor.hpp"

namespace urde {

class CScriptDebugCameraWaypoint : public CActor {
  u32 xe8_w1;

public:
  CScriptDebugCameraWaypoint(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                             u32 w1);

  void Accept(IVisitor&);
};

} // namespace urde
