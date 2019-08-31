#pragma once

#include "CEntity.hpp"
#include "zeus/CVector3f.hpp"

namespace urde {

class CScriptGenerator : public CEntity {
  u32 x34_spawnCount;
  union {
    struct {
      bool x38_24_noReuseFollowers : 1;
      bool x38_25_noInheritTransform : 1;
    };
    u8 dummy1 = 0;
  };
  zeus::CVector3f x3c_offset;
  float x48_minScale;
  float x4c_maxScale;

public:
  CScriptGenerator(TUniqueId uid, std::string_view name, const CEntityInfo& info, u32 spawnCount, bool noReuseFollowers,
                   const zeus::CVector3f& vec1, bool noInheritXf, bool active, float minScale, float maxScale);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
};
} // namespace urde
