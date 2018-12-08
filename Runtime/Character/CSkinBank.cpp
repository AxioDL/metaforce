#include "CSkinBank.hpp"
#include "CPoseAsTransforms.hpp"

namespace urde {

CSkinBank::CSkinBank(CInputStream& in) {
  u32 boneCount = in.readUint32Big();
  x0_segments.reserve(boneCount);
  for (u32 i = 0; i < boneCount; ++i)
    x0_segments.emplace_back(in);
}

void CSkinBank::GetBankTransforms(std::vector<const zeus::CTransform*>& out, const CPoseAsTransforms& pose) const {
  for (CSegId id : x0_segments) {
    const zeus::CTransform& xf = pose.GetRestToAccumTransform(id);
    out.push_back(&xf);
  }
}

} // namespace urde
