#include "EVNT.hpp"

namespace Retro
{
namespace DNAMP1
{

void EVNT::read(Athena::io::IStreamReader& reader)
{
    version = reader.readUint32Big();

    atUint32 loopCount = reader.readUint32Big();
    reader.enumerate(loopEvents, loopCount);

    uevtEvents.clear();
    if (version == 2)
    {
        atUint32 uevtCount = reader.readUint32Big();
        reader.enumerate(uevtEvents, uevtCount);
    }

    atUint32 effectCount = reader.readUint32Big();
    reader.enumerate(effectEvents, effectCount);

    atUint32 sfxCount = reader.readUint32Big();
    reader.enumerate(sfxEvents, sfxCount);
}

void EVNT::write(Athena::io::IStreamWriter& writer) const
{
    writer.writeUint32Big(version);

    writer.writeUint32Big(loopEvents.size());
    writer.enumerate(loopEvents);

    if (version == 2)
    {
        writer.writeUint32Big(uevtEvents.size());
        writer.enumerate(uevtEvents);
    }

    writer.writeUint32Big(effectEvents.size());
    writer.enumerate(effectEvents);

    writer.writeUint32Big(sfxEvents.size());
    writer.enumerate(sfxEvents);
}

void EVNT::fromYAML(Athena::io::YAMLDocReader& reader)
{
    version = reader.readUint32("version");

    atUint32 loopCount = reader.readUint32("loopCount");
    reader.enumerate("loopEvents", loopEvents, loopCount);

    uevtEvents.clear();
    if (version == 2)
    {
        atUint32 uevtCount = reader.readUint32("uevtCount");
        reader.enumerate("uevtEvents", uevtEvents, uevtCount);
    }

    atUint32 effectCount = reader.readUint32("effectCount");
    reader.enumerate("effectEvents", effectEvents, effectCount);

    atUint32 sfxCount = reader.readUint32("sfxCount");
    reader.enumerate("sfxEvents", sfxEvents, sfxCount);
}

void EVNT::toYAML(Athena::io::YAMLDocWriter& writer) const
{
    writer.writeUint32("version", version);

    writer.writeUint32("loopCount", loopEvents.size());
    writer.enumerate("loopEvents", loopEvents);

    if (version == 2)
    {
        writer.writeUint32("uevtCount", uevtEvents.size());
        writer.enumerate("uevtEvents", uevtEvents);
    }

    writer.writeUint32("effectCount", effectEvents.size());
    writer.enumerate("effectEvents", effectEvents);

    writer.writeUint32("sfxCount", sfxEvents.size());
    writer.enumerate("sfxEvents", sfxEvents);
}

}
}
