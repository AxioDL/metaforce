#include "EVNT.hpp"

namespace DataSpec
{
namespace DNAMP1
{

void EVNT::read(Athena::io::IStreamReader& reader)
{
    version = reader.readUint32Big();

    atUint32 loopCount = reader.readUint32Big();
    reader.enumerate(loopEvents, loopCount);

    uevtEvents.clear();
    atUint32 uevtCount = reader.readUint32Big();
    reader.enumerate(uevtEvents, uevtCount);

    atUint32 effectCount = reader.readUint32Big();
    reader.enumerate(effectEvents, effectCount);

    if (version == 2)
    {
        atUint32 sfxCount = reader.readUint32Big();
        reader.enumerate(sfxEvents, sfxCount);
    }
}

void EVNT::write(Athena::io::IStreamWriter& writer) const
{
    writer.writeUint32Big(version);

    writer.writeUint32Big(loopEvents.size());
    writer.enumerate(loopEvents);

    writer.writeUint32Big(uevtEvents.size());
    writer.enumerate(uevtEvents);

    writer.writeUint32Big(effectEvents.size());
    writer.enumerate(effectEvents);

    if (version == 2)
    {
        writer.writeUint32Big(sfxEvents.size());
        writer.enumerate(sfxEvents);
    }
}

void EVNT::read(Athena::io::YAMLDocReader& reader)
{
    version = reader.readUint32("version");

    atUint32 loopCount = reader.readUint32("loopCount");
    reader.enumerate("loopEvents", loopEvents, loopCount);

    uevtEvents.clear();
    atUint32 uevtCount = reader.readUint32("uevtCount");
    reader.enumerate("uevtEvents", uevtEvents, uevtCount);

    atUint32 effectCount = reader.readUint32("effectCount");
    reader.enumerate("effectEvents", effectEvents, effectCount);

    if (version == 2)
    {
        atUint32 sfxCount = reader.readUint32("sfxCount");
        reader.enumerate("sfxEvents", sfxEvents, sfxCount);
    }
}

void EVNT::write(Athena::io::YAMLDocWriter& writer) const
{
    writer.writeUint32("version", version);

    writer.writeUint32("loopCount", loopEvents.size());
    writer.enumerate("loopEvents", loopEvents);

    writer.writeUint32("uevtCount", uevtEvents.size());
    writer.enumerate("uevtEvents", uevtEvents);

    writer.writeUint32("effectCount", effectEvents.size());
    writer.enumerate("effectEvents", effectEvents);

    if (version == 2)
    {
        writer.writeUint32("sfxCount", sfxEvents.size());
        writer.enumerate("sfxEvents", sfxEvents);
    }
}

const char* EVNT::DNAType()
{
    return "Retro::DNAMP1::EVNT";
}

size_t EVNT::binarySize(size_t __isz) const
{
    __isz = __EnumerateSize(__isz, loopEvents);
    __isz = __EnumerateSize(__isz, uevtEvents);
    __isz = __EnumerateSize(__isz, effectEvents);
    if (version == 2)
        __isz = __EnumerateSize(__isz, sfxEvents);

    return __isz + (version == 2 ? 20 : 16);
}

}
}
