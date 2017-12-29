#include "EVNT.hpp"

namespace DataSpec::DNAMP1
{

void EVNT::read(athena::io::IStreamReader& reader)
{
    version = reader.readUint32Big();

    atUint32 loopCount = reader.readUint32Big();
    reader.enumerate(boolPOINodes, loopCount);

    int32POINodes.clear();
    atUint32 uevtCount = reader.readUint32Big();
    reader.enumerate(int32POINodes, uevtCount);

    atUint32 effectCount = reader.readUint32Big();
    reader.enumerate(particlePOINodes, effectCount);

    if (version == 2)
    {
        atUint32 sfxCount = reader.readUint32Big();
        reader.enumerate(soundPOINodes, sfxCount);
    }
}

void EVNT::write(athena::io::IStreamWriter& writer) const
{
    writer.writeUint32Big(version);

    writer.writeUint32Big(boolPOINodes.size());
    writer.enumerate(boolPOINodes);

    writer.writeUint32Big(int32POINodes.size());
    writer.enumerate(int32POINodes);

    writer.writeUint32Big(particlePOINodes.size());
    writer.enumerate(particlePOINodes);

    if (version == 2)
    {
        writer.writeUint32Big(soundPOINodes.size());
        writer.enumerate(soundPOINodes);
    }
}

void EVNT::read(athena::io::YAMLDocReader& reader)
{
    version = reader.readUint32("version");

    reader.enumerate("boolPOINodes", boolPOINodes);

    int32POINodes.clear();
    reader.enumerate("int32POINodes", int32POINodes);

    reader.enumerate("particlePOINodes", particlePOINodes);

    if (version == 2)
    {
        reader.enumerate("soundPOINodes", soundPOINodes);
    }
}

void EVNT::write(athena::io::YAMLDocWriter& writer) const
{
    writer.writeUint32("version", version);

    writer.enumerate("boolPOINodes", boolPOINodes);

    writer.enumerate("int32POINodes", int32POINodes);

    writer.enumerate("particlePOINodes", particlePOINodes);

    if (version == 2)
    {
        writer.enumerate("soundPOINodes", soundPOINodes);
    }
}

const char* EVNT::DNAType()
{
    return "urde::DNAMP1::EVNT";
}

size_t EVNT::binarySize(size_t __isz) const
{
    __isz = __EnumerateSize(__isz, boolPOINodes);
    __isz = __EnumerateSize(__isz, int32POINodes);
    __isz = __EnumerateSize(__isz, particlePOINodes);
    if (version == 2)
        __isz = __EnumerateSize(__isz, soundPOINodes);

    return __isz + (version == 2 ? 20 : 16);
}

}
