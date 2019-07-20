#include "WorldTeleporter.hpp"

namespace DataSpec::DNAMP1 {

template <class Op>
void WorldTeleporter::Enumerate(typename Op::StreamT& s) {
  IScriptObject::Enumerate<Op>(s);
  Do<Op>({"name"}, name, s);
  Do<Op>({"active"}, active, s);
  Do<Op>({"mlvl"}, mlvl, s);
  Do<Op>({"mrea"}, mrea, s);
  Do<Op>({"animationParameters"}, animationParameters, s);
  Do<Op>({"playerScale"}, playerScale, s);
  Do<Op>({"platformModel"}, platformModel, s);
  Do<Op>({"platformScale"}, platformScale, s);
  Do<Op>({"blackgroundModel"}, backgroundModel, s);
  Do<Op>({"backgroundScale"}, backgroundScale, s);
  Do<Op>({"upElevator"}, upElevator, s);
  Do<Op>({"elevatorSound"}, elevatorSound, s);
  Do<Op>({"volume"}, volume, s);
  Do<Op>({"panning"}, panning, s);
  Do<Op>({"showText"}, showText, s);
  Do<Op>({"font"}, font, s);
  Do<Op>({"strg"}, strg, s);
  Do<Op>({"fadeWhite"}, fadeWhite, s);
  Do<Op>({"charFadeInTime"}, charFadeInTime, s);
  Do<Op>({"charsPerSecond"}, charsPerSecond, s);
  Do<Op>({"showDelay"}, showDelay, s);

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
