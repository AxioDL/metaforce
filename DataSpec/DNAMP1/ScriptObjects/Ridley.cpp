#include "Ridley.hpp"

namespace DataSpec::DNAMP1 {

template <class Op>
void Ridley::Enumerate(typename Op::StreamT& s) {
  IScriptObject::Enumerate<Op>(s);
  Do<Op>(athena::io::PropId{"name"}, name, s);
  Do<Op>(athena::io::PropId{"location"}, location, s);
  Do<Op>(athena::io::PropId{"orientation"}, orientation, s);
  Do<Op>(athena::io::PropId{"scale"}, scale, s);
  Do<Op>(athena::io::PropId{"patternedInfo"}, patternedInfo, s);
  Do<Op>(athena::io::PropId{"actorParameters"}, actorParameters, s);
  Do<Op>(athena::io::PropId{"model1"}, model1, s);
  Do<Op>(athena::io::PropId{"model2"}, model2, s);
  if (propertyCount == 48) {
    Do<Op>(athena::io::PropId{"model3"}, model3, s);
    Do<Op>(athena::io::PropId{"model4"}, model4, s);
    Do<Op>(athena::io::PropId{"model5"}, model5, s);
    Do<Op>(athena::io::PropId{"model6"}, model6, s);
    Do<Op>(athena::io::PropId{"model7"}, model7, s);
    Do<Op>(athena::io::PropId{"model8"}, model8, s);
    Do<Op>(athena::io::PropId{"model9"}, model9, s);
    Do<Op>(athena::io::PropId{"model10"}, model10, s);
    Do<Op>(athena::io::PropId{"model11"}, model11, s);
    Do<Op>(athena::io::PropId{"model12"}, model12, s);
  }
  Do<Op>(athena::io::PropId{"particle"}, particle, s);
  Do<Op>(athena::io::PropId{"unknown1"}, unknown1, s);
  Do<Op>(athena::io::PropId{"unknown2"}, unknown2, s);
  Do<Op>(athena::io::PropId{"unknown3"}, unknown3, s);
  Do<Op>(athena::io::PropId{"unknown4"}, unknown4, s);
  Do<Op>(athena::io::PropId{"wpsc1"}, wpsc1, s);
  Do<Op>(athena::io::PropId{"damageInfo1"}, damageInfo1, s);
  Do<Op>(athena::io::PropId{"ridleyStruct1"}, ridleyStruct1, s);
  Do<Op>(athena::io::PropId{"soundID1"}, soundID1, s);
  Do<Op>(athena::io::PropId{"wpsc2"}, wpsc2, s);
  if (propertyCount == 40)
    Do<Op>(athena::io::PropId{"wpsc3"}, wpsc3, s);
  Do<Op>(athena::io::PropId{"damageInfo2"}, damageInfo2, s);
  Do<Op>(athena::io::PropId{"ridleyStruct2_1"}, ridleyStruct2_1, s);
  Do<Op>(athena::io::PropId{"wpsc4"}, wpsc4, s);
  Do<Op>(athena::io::PropId{"damageInfo3"}, damageInfo3, s);
  Do<Op>(athena::io::PropId{"ridleyStruct2_2"}, ridleyStruct2_2, s);
  Do<Op>(athena::io::PropId{"soundID2"}, soundID2, s);
  Do<Op>(athena::io::PropId{"damageInfo4"}, damageInfo4, s);
  Do<Op>(athena::io::PropId{"ridleyStruct2_3"}, ridleyStruct2_3, s);
  Do<Op>(athena::io::PropId{"unknown5"}, unknown5, s);
  Do<Op>(athena::io::PropId{"unknown6"}, unknown6, s);
  Do<Op>(athena::io::PropId{"damageInfo5"}, damageInfo5, s);
  Do<Op>(athena::io::PropId{"unknown7"}, unknown7, s);
  Do<Op>(athena::io::PropId{"damageInfo6"}, damageInfo6, s);
  Do<Op>(athena::io::PropId{"unknown8"}, unknown8, s);
  Do<Op>(athena::io::PropId{"damageInfo7"}, damageInfo7, s);
  Do<Op>(athena::io::PropId{"unknown9"}, unknown9, s);
  Do<Op>(athena::io::PropId{"elsc"}, elsc, s);
  Do<Op>(athena::io::PropId{"unknown10"}, unknown10, s);
  Do<Op>(athena::io::PropId{"soundID3"}, soundID3, s);
  Do<Op>(athena::io::PropId{"damageInfo8"}, damageInfo8, s);
  if (propertyCount == 40)
    Do<Op>(athena::io::PropId{"damageInfo9"}, damageInfo9, s);
}

const char* Ridley::DNAType() { return "urde::DNAMP1::Ridley"; }

AT_SPECIALIZE_DNA_YAML(Ridley)

} // namespace DataSpec::DNAMP1
