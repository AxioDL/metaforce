#include "CSKR.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec::DNAMP3 {

void CSKR::weightVertex(hecl::blender::PyOutStream& os, const CINF& cinf, atInt16 skinIdx) const {
  if (skinIdx < 0)
    return;
  const DNAMP2::CSKR::SkinningRule& rule = data.skinningRules[skinIdx];
  for (const DNAMP2::CSKR::SkinningRule::Weight& weight : rule.weights)
    os.format("vert[dvert_lay][%u] = %f\n", cinf.getBoneIdxFromId(weight.boneId), weight.weight);
}

} // namespace DataSpec::DNAMP3
