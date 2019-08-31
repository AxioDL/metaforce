#include "Oculus.hpp"

namespace DataSpec::DNAMP1 {

template <class Op>
void Oculus::Enumerate(typename Op::StreamT& s) {
  IScriptObject::Enumerate<Op>(s);
  Do<Op>(athena::io::PropId{"name"}, name, s);
  Do<Op>(athena::io::PropId{"location"}, location, s);
  Do<Op>(athena::io::PropId{"orientation"}, orientation, s);
  Do<Op>(athena::io::PropId{"scale"}, scale, s);
  Do<Op>(athena::io::PropId{"patternedInfo"}, patternedInfo, s);
  Do<Op>(athena::io::PropId{"actorParameters"}, actorParameters, s);
  Do<Op>(athena::io::PropId{"unknown1"}, unknown1, s);
  Do<Op>(athena::io::PropId{"unknown2"}, unknown2, s);
  Do<Op>(athena::io::PropId{"unknown3"}, unknown3, s);
  Do<Op>(athena::io::PropId{"unknown4"}, unknown4, s);
  Do<Op>(athena::io::PropId{"unknown5"}, unknown5, s);
  Do<Op>(athena::io::PropId{"unknown6"}, unknown6, s);
  Do<Op>(athena::io::PropId{"damageVulnerabilty"}, damageVulnerabilty, s);
  Do<Op>(athena::io::PropId{"unknown7"}, unknown7, s);
  Do<Op>(athena::io::PropId{"damageInfo"}, damageInfo, s);
  if (propertyCount == 16)
    Do<Op>(athena::io::PropId{"unknown8"}, unknown8, s);
  else
    unknown8 = 0.f;
}

const char* Oculus::DNAType() { return "urde::DNAMP1::Oculus"; }

AT_SPECIALIZE_DNA_YAML(Oculus)

} // namespace DataSpec::DNAMP1