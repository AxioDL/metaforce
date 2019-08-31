#include "EVNT.hpp"

namespace DataSpec::DNAMP1 {

template <class Op>
void EVNT::Enumerate(typename Op::StreamT& s) {
  Do<Op>(athena::io::PropId{"version"}, version, s);

  DoSize<Op>(athena::io::PropId{"boolPOICount"}, boolPOICount, s);
  Do<Op>(athena::io::PropId{"boolPOINodes"}, boolPOINodes, boolPOICount, s);

  DoSize<Op>(athena::io::PropId{"int32POICount"}, int32POICount, s);
  Do<Op>(athena::io::PropId{"int32POINodes"}, int32POINodes, int32POICount, s);

  DoSize<Op>(athena::io::PropId{"particlePOICount"}, particlePOICount, s);
  Do<Op>(athena::io::PropId{"particlePOINodes"}, particlePOINodes, particlePOICount, s);

  if (version == 2) {
    DoSize<Op>(athena::io::PropId{"soundPOICount"}, soundPOICount, s);
    Do<Op>(athena::io::PropId{"soundPOINodes"}, soundPOINodes, soundPOICount, s);
  }
}

AT_SPECIALIZE_DNA_YAML(EVNT)

const char* EVNT::DNAType() { return "urde::DNAMP1::EVNT"; }

} // namespace DataSpec::DNAMP1
