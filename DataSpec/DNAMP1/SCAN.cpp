#include "SCAN.hpp"

namespace DataSpec::DNAMP1
{

static const std::vector<std::string> PaneNames =
{
    "imagepane_pane0",  "imagepane_pane1",   "imagepane_pane2",   "imagepane_pane3",   "imagepane_pane01",
    "imagepane_pane12", "imagepane_pane23",  "imagepane_pane012", "imagepane_pane123", "imagepane_pane0123",
    "imagepane_pane4",  "imagepane_pane5",   "imagepane_pane6",   "imagepane_pane7",   "imagepane_pane45",
    "imagepane_pane56", "imagepane_pane67",  "imagepane_pane456", "imagepane_pane567", "imagepane_pane4567"
};

template <>
void SCAN::Texture::Enumerate<BigDNA::Read>(typename Read::StreamT& r)
{
    texture.read(r);
    appearanceRange = r.readFloatBig();
    position = Position(r.readUint32Big());
    width = r.readUint32Big();
    height = r.readUint32Big();
    interval = r.readFloatBig();
    fadeDuration = r.readFloatBig();
}

template <>
void SCAN::Texture::Enumerate<BigDNA::Write>(typename Write::StreamT& w)
{
    texture.write(w);
    w.writeFloatBig(appearanceRange);
    w.writeUint32Big(atUint32(position));
    w.writeUint32Big(width);
    w.writeUint32Big(height);
    w.writeFloatBig(interval);
    w.writeFloatBig(fadeDuration);
}

template <>
void SCAN::Texture::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r)
{
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
void SCAN::Texture::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w)
{
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

const char* SCAN::Texture::DNAType() { return "urde::DNAMP1::SCAN::Texture"; }

template <>
void SCAN::Texture::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s)
{
    texture.binarySize(s);
    s += 24;
}

}
