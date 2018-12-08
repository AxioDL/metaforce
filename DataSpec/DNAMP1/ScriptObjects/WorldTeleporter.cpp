#include "WorldTeleporter.hpp"

namespace DataSpec::DNAMP1 {

template <class Op>
void WorldTeleporter::Enumerate(typename Op::StreamT& s) {
  IScriptObject::Enumerate<Op>(s);
  Do<Op>({"name"}, name, s);
  Do<Op>({"unknown1"}, unknown1, s);
  Do<Op>({"mlvl"}, mlvl, s);
  Do<Op>({"mrea"}, mrea, s);
  Do<Op>({"animationParameters"}, animationParameters, s);
  Do<Op>({"unknown2"}, unknown2, s);
  Do<Op>({"model1"}, model1, s);
  Do<Op>({"unknown3"}, unknown3, s);
  Do<Op>({"model2"}, model2, s);
  Do<Op>({"unknown4"}, unknown4, s);
  Do<Op>({"unknown5"}, unknown5, s);
  Do<Op>({"soundID"}, soundID, s);
  Do<Op>({"unknown6"}, unknown6, s);
  Do<Op>({"unknown7"}, unknown7, s);
  Do<Op>({"unknown8"}, unknown8, s);
  Do<Op>({"font"}, font, s);
  Do<Op>({"strg"}, strg, s);
  Do<Op>({"unknown9"}, unknown9, s);
  Do<Op>({"unknown10"}, unknown10, s);
  Do<Op>({"unknown11"}, unknown11, s);
  Do<Op>({"unknown12"}, unknown12, s);

  if (propertyCount == 26) {
    Do<Op>({"audioStream"}, audioStream, s);
    Do<Op>({"unknown13"}, unknown13, s);
    Do<Op>({"unknown14"}, unknown14, s);
    Do<Op>({"unknown15"}, unknown15, s);
    Do<Op>({"unknown16"}, unknown16, s);
  } else {
    unknown13 = false;
    unknown14 = 0.0;
    unknown15 = 0.0;
    unknown16 = 0.0;
  }
}

const char* WorldTeleporter::DNAType() { return "urde::DNAMP1::WorldTeleporter"; }

AT_SPECIALIZE_DNA_YAML(WorldTeleporter)

} // namespace DataSpec::DNAMP1
