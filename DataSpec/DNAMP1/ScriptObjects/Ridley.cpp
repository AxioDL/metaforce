#include "Ridley.hpp"

namespace DataSpec::DNAMP1 {

template <class Op>
void Ridley::Enumerate(typename Op::StreamT& s) {
  IScriptObject::Enumerate<Op>(s);
  Do<Op>({"name"}, name, s);
  Do<Op>({"location"}, location, s);
  Do<Op>({"orientation"}, orientation, s);
  Do<Op>({"scale"}, scale, s);
  Do<Op>({"patternedInfo"}, patternedInfo, s);
  Do<Op>({"actorParameters"}, actorParameters, s);
  Do<Op>({"model1"}, model1, s);
  Do<Op>({"model2"}, model2, s);
  if (propertyCount == 48) {
    Do<Op>({"model3"}, model3, s);
    Do<Op>({"model4"}, model4, s);
    Do<Op>({"model5"}, model5, s);
    Do<Op>({"model6"}, model6, s);
    Do<Op>({"model7"}, model7, s);
    Do<Op>({"model8"}, model8, s);
    Do<Op>({"model9"}, model9, s);
    Do<Op>({"model10"}, model10, s);
    Do<Op>({"model11"}, model11, s);
    Do<Op>({"model12"}, model12, s);
  }
  Do<Op>({"particle"}, particle, s);
  Do<Op>({"unknown1"}, unknown1, s);
  Do<Op>({"unknown2"}, unknown2, s);
  Do<Op>({"unknown3"}, unknown3, s);
  Do<Op>({"unknown4"}, unknown4, s);
  Do<Op>({"wpsc1"}, wpsc1, s);
  Do<Op>({"damageInfo1"}, damageInfo1, s);
  Do<Op>({"ridleyStruct1"}, ridleyStruct1, s);
  Do<Op>({"soundID1"}, soundID1, s);
  Do<Op>({"wpsc2"}, wpsc2, s);
  if (propertyCount == 40)
    Do<Op>({"wpsc3"}, wpsc3, s);
  Do<Op>({"damageInfo2"}, damageInfo2, s);
  Do<Op>({"ridleyStruct2_1"}, ridleyStruct2_1, s);
  Do<Op>({"wpsc4"}, wpsc4, s);
  Do<Op>({"damageInfo3"}, damageInfo3, s);
  Do<Op>({"ridleyStruct2_2"}, ridleyStruct2_2, s);
  Do<Op>({"soundID2"}, soundID2, s);
  Do<Op>({"damageInfo4"}, damageInfo4, s);
  Do<Op>({"ridleyStruct2_3"}, ridleyStruct2_3, s);
  Do<Op>({"unknown5"}, unknown5, s);
  Do<Op>({"unknown6"}, unknown6, s);
  Do<Op>({"damageInfo5"}, damageInfo5, s);
  Do<Op>({"unknown7"}, unknown7, s);
  Do<Op>({"damageInfo6"}, damageInfo6, s);
  Do<Op>({"unknown8"}, unknown8, s);
  Do<Op>({"damageInfo7"}, damageInfo7, s);
  Do<Op>({"unknown9"}, unknown9, s);
  Do<Op>({"elsc"}, elsc, s);
  Do<Op>({"unknown10"}, unknown10, s);
  Do<Op>({"soundID3"}, soundID3, s);
  Do<Op>({"damageInfo8"}, damageInfo8, s);
  if (propertyCount == 40)
    Do<Op>({"damageInfo9"}, damageInfo9, s);
}

const char* Ridley::DNAType() { return "urde::DNAMP1::Ridley"; }

AT_SPECIALIZE_DNA_YAML(Ridley)

} // namespace DataSpec::DNAMP1
