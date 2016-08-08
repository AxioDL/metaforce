#include "CScriptDistanceFog.hpp"

namespace urde
{
CScriptDistanceFog::CScriptDistanceFog(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                       const ERglFogMode& mode, const zeus::CColor& col,
                                       const zeus::CVector2f& v1, float f1, const zeus::CVector2f& v2,
                                       bool b1, bool active, float f2, float f3, float f4, float f5)
    : CEntity(uid, info, active, name),
      x34_mode(mode),
      x38_col(col),
      x3c_(v1),
      x44_(f1),
      x48_(v2),
      x50_(f2),
      x54_(f3),
      x58_(f4),
      x5c_(f5)

{
}
}
