#include "CSKR.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAMP1 {

void CSKR::weightVertex(hecl::blender::PyOutStream& os, const CINF& cinf, atUint32 idx) const {
  atUint32 accum = 0;
  for (const SkinningRule& rule : skinningRules) {
    if (idx >= accum && idx < accum + rule.vertCount)
      for (const SkinningRule::Weight& weight : rule.weights)
        os.format(fmt("vert[dvert_lay][{}] = {}\n"), cinf.getBoneIdxFromId(weight.boneId), weight.weight);
    accum += rule.vertCount;
  }
}

} // namespace DataSpec::DNAMP1
