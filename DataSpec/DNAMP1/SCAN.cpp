#include "SCAN.hpp"

#include <algorithm>
#include <array>

namespace DataSpec::DNAMP1 {

constexpr std::array PaneNames{
    "imagepane_pane0"sv,  "imagepane_pane1"sv,  "imagepane_pane2"sv,   "imagepane_pane3"sv,   "imagepane_pane01"sv,
    "imagepane_pane12"sv, "imagepane_pane23"sv, "imagepane_pane012"sv, "imagepane_pane123"sv, "imagepane_pane0123"sv,
    "imagepane_pane4"sv,  "imagepane_pane5"sv,  "imagepane_pane6"sv,   "imagepane_pane7"sv,   "imagepane_pane45"sv,
    "imagepane_pane56"sv, "imagepane_pane67"sv, "imagepane_pane456"sv, "imagepane_pane567"sv, "imagepane_pane4567"sv,
};

template <>
void SCAN::Texture::Enumerate<BigDNA::Read>(typename Read::StreamT& r) {
  texture.read(r);
  appearanceRange = r.readFloatBig();
  position = Position(r.readUint32Big());
  width = r.readUint32Big();
  height = r.readUint32Big();
  interval = r.readFloatBig();
  fadeDuration = r.readFloatBig();
}

template <>
void SCAN::Texture::Enumerate<BigDNA::Write>(typename Write::StreamT& w) {
  texture.write(w);
  w.writeFloatBig(appearanceRange);
  w.writeUint32Big(atUint32(position));
  w.writeUint32Big(width);
  w.writeUint32Big(height);
  w.writeFloatBig(interval);
  w.writeFloatBig(fadeDuration);
}

template <>
void SCAN::Texture::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  r.enumerate("texture", texture);
  appearanceRange = r.readFloat("appearanceRange");
  std::string tmp = r.readString("position");

  auto idx = std::find(PaneNames.begin(), PaneNames.end(), tmp);
  if (idx != PaneNames.end())
    position = Position(idx - PaneNames.begin());
  else
    position = Position::Invalid;

  width = r.readUint32("width");
  height = r.readUint32("height");
  interval = r.readFloat("interval");
  fadeDuration = r.readFloat("fadeDuration");
}

template <>
void SCAN::Texture::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  w.enumerate("texture", texture);
  w.writeFloat("appearanceRange", appearanceRange);
  if (position != Position::Invalid)
    w.writeString("position", PaneNames.at(atUint32(position)));
  else
    w.writeString("position", "undefined");
  w.writeUint32("width", width);
  w.writeUint32("height", height);
  w.writeFloat("interval", interval);
  w.writeFloat("fadeDuration", fadeDuration);
}

std::string_view SCAN::Texture::DNAType() { return "urde::DNAMP1::SCAN::Texture"sv; }

template <>
void SCAN::Texture::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  texture.binarySize(s);
  s += 24;
}

} // namespace DataSpec::DNAMP1
