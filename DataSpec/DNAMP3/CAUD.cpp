#include "CAUD.hpp"

namespace DataSpec::DNAMP3 {

template <class Op>
void CAUD::CSMPInfo::UnknownStruct2::UnknownQuad::Enumerate(typename Op::StreamT& s) {
  Do<Op>({"unknown1"}, unknown1, s);
  Do<Op>({"unknown2"}, unknown2, s);
  Do<Op>({"unknown3"}, unknown3, s);
  Do<Op>({"unknown4"}, unknown4, s);
  if (unknown4 == 5)
    Do<Op>({"unknown5"}, unknown5, s);
}

AT_SPECIALIZE_DNA_YAML(CAUD::CSMPInfo::UnknownStruct2::UnknownQuad)

const char* CAUD::CSMPInfo::UnknownStruct2::UnknownQuad::DNAType() {
  return "DataSpec::DNAMP3::CAUD::CSMPInfo::UnknownStruct2::UnknownQuad";
}

} // namespace DataSpec::DNAMP3
