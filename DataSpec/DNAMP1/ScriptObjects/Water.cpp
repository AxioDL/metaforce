#include "Water.hpp"

namespace DataSpec::DNAMP1
{

template <>
void Water::UnusedBitset::Enumerate<BigDNA::Read>(typename Read::StreamT& in)
{
    if (in.readBool())
    {
        atUint32 bitVal0 = in.readUint16Big();
        atUint32 bitVal1 = in.readUint16Big();
        atUint32 len = ((bitVal0 * bitVal1) + 31) / 31;
        in.seek(len * 4);
    }
}
template <>
void Water::UnusedBitset::Enumerate<BigDNA::Write>(typename Write::StreamT& out) { out.writeBool(false); }
template <>
void Water::UnusedBitset::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {}
template <>
void Water::UnusedBitset::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {}
template <>
void Water::UnusedBitset::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) { s += 1; }

}

