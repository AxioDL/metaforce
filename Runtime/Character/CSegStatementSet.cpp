#include "CSegStatementSet.hpp"
#include "CSegIdList.hpp"
#include "CCharLayoutInfo.hpp"

namespace urde {

void CSegStatementSet::Add(const CSegIdList& list, const CCharLayoutInfo& layout, const CSegStatementSet& other,
                           float weight) {
  for (const CSegId& id : list.GetList()) {
    x4_segData[id].x0_rotation *=
        zeus::CQuaternion::slerp(zeus::CQuaternion(), other.x4_segData[id].x0_rotation, weight);
    if (other.x4_segData[id].x1c_hasOffset && x4_segData[id].x1c_hasOffset) {
      zeus::CVector3f off = other.x4_segData[id].x10_offset - layout.GetFromParentUnrotated(id);
      x4_segData[id].x10_offset += off * weight;
    }
  }
}

} // namespace urde
