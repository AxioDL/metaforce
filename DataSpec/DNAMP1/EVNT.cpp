#include "EVNT.hpp"

namespace DataSpec::DNAMP1 {

template <class Op>
void EVNT::Enumerate(typename Op::StreamT& s) {
  Do<Op>({"version"}, version, s);

  DoSize<Op>({"boolPOICount"}, boolPOICount, s);
  Do<Op>({"boolPOINodes"}, boolPOINodes, boolPOICount, s);

  DoSize<Op>({"int32POICount"}, int32POICount, s);
  Do<Op>({"int32POINodes"}, int32POINodes, int32POICount, s);

  DoSize<Op>({"particlePOICount"}, particlePOICount, s);
  Do<Op>({"particlePOINodes"}, particlePOINodes, particlePOICount, s);

  if (version == 2) {
    DoSize<Op>({"soundPOICount"}, soundPOICount, s);
    Do<Op>({"soundPOINodes"}, soundPOINodes, soundPOICount, s);
  }
}

AT_SPECIALIZE_DNA_YAML(EVNT)

const char* EVNT::DNAType() { return "urde::DNAMP1::EVNT"; }

} // namespace DataSpec::DNAMP1
