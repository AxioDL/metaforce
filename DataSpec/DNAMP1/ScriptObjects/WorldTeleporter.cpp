#include "WorldTeleporter.hpp"

namespace DataSpec::DNAMP1 {

template <class Op>
void WorldTeleporter::Enumerate(typename Op::StreamT& s) {
  IScriptObject::Enumerate<Op>(s);
  Do<Op>(athena::io::PropId{"name"}, name, s);
  Do<Op>(athena::io::PropId{"active"}, active, s);
  Do<Op>(athena::io::PropId{"mlvl"}, mlvl, s);
  Do<Op>(athena::io::PropId{"mrea"}, mrea, s);
  Do<Op>(athena::io::PropId{"animationParameters"}, animationParameters, s);
  Do<Op>(athena::io::PropId{"playerScale"}, playerScale, s);
  Do<Op>(athena::io::PropId{"platformModel"}, platformModel, s);
  Do<Op>(athena::io::PropId{"platformScale"}, platformScale, s);
  Do<Op>(athena::io::PropId{"blackgroundModel"}, backgroundModel, s);
  Do<Op>(athena::io::PropId{"backgroundScale"}, backgroundScale, s);
  Do<Op>(athena::io::PropId{"upElevator"}, upElevator, s);
  Do<Op>(athena::io::PropId{"elevatorSound"}, elevatorSound, s);
  Do<Op>(athena::io::PropId{"volume"}, volume, s);
  Do<Op>(athena::io::PropId{"panning"}, panning, s);
  Do<Op>(athena::io::PropId{"showText"}, showText, s);
  Do<Op>(athena::io::PropId{"font"}, font, s);
  Do<Op>(athena::io::PropId{"strg"}, strg, s);
  Do<Op>(athena::io::PropId{"fadeWhite"}, fadeWhite, s);
  Do<Op>(athena::io::PropId{"charFadeInTime"}, charFadeInTime, s);
  Do<Op>(athena::io::PropId{"charsPerSecond"}, charsPerSecond, s);
  Do<Op>(athena::io::PropId{"showDelay"}, showDelay, s);

  if (propertyCount == 26) {
    Do<Op>(athena::io::PropId{"audioStream"}, audioStream, s);
    Do<Op>(athena::io::PropId{"unknown13"}, unknown13, s);
    Do<Op>(athena::io::PropId{"unknown14"}, unknown14, s);
    Do<Op>(athena::io::PropId{"unknown15"}, unknown15, s);
    Do<Op>(athena::io::PropId{"unknown16"}, unknown16, s);
  } else {
    unknown13 = false;
    unknown14 = 0.0;
    unknown15 = 0.0;
    unknown16 = 0.0;
  }
}

std::string_view WorldTeleporter::DNAType() { return "urde::DNAMP1::WorldTeleporter"sv; }

AT_SPECIALIZE_DNA_YAML(WorldTeleporter)

} // namespace DataSpec::DNAMP1
