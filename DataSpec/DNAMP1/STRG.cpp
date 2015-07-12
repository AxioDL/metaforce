#include "STRG.hpp"
#include "../Logging.hpp"

namespace Retro
{
namespace DNAMP1
{

const HECL::FourCC ENGLfcc("ENGL");
const HECL::FourCC FRENfcc("FREN");
const HECL::FourCC GERMfcc("GERM");
const HECL::FourCC SPANfcc("SPAN");
const HECL::FourCC ITALfcc("ITAL");
const HECL::FourCC JAPNfcc("JAPN");

void STRG::read(Athena::io::IStreamReader& reader)
{
    reader.setEndian(Athena::BigEndian);
    uint32_t magic = reader.readUint32();
    if (magic != 0x87654321)
        LogModule.report(LogVisor::FatalError, "invalid STRG magic");

    version = reader.readUint32();
    langCount = reader.readUint32();
    strCount = reader.readUint32();

    langs.clear();
    langs.reserve(langCount);
    for (uint32_t l=0 ; l<langCount ; ++l)
    {
        langs.emplace_back();
        Language& lang = langs.back();
        lang.lang.read(reader);
        reader.readUint32();
    }

    for (uint32_t l=0 ; l<langCount ; ++l)
    {
        Language& lang = langs[l];
        reader.readUint32();
        for (uint32_t s=0 ; s<strCount ; ++s)
            reader.readUint32();
        for (uint32_t s=0 ; s<strCount ; ++s)
            lang.strings.push_back(reader.readWString());
    }
}

void STRG::write(Athena::io::IStreamWriter& writer) const
{
    writer.setEndian(Athena::BigEndian);
    writer.writeUint32(0x87654321);
    writer.writeUint32(version);
    writer.writeUint32(langs.size());
    writer.writeUint32(strCount);

    uint32_t offset = 0;
    for (const Language& lang : langs)
    {
        lang.lang.write(writer);
        writer.writeUint32(offset);
        offset += strCount * 4 + 4;
        for (uint32_t s=0 ; s<strCount ; ++s)
        {
            if (s < lang.strings.size())
                offset += lang.strings[s].size() * 2 + 1;
            else
                offset += 1;
        }
    }

    for (const Language& lang : langs)
    {
        offset = strCount * 4;
        for (uint32_t s=0 ; s<strCount ; ++s)
        {
            writer.writeUint32(offset);
            if (s < lang.strings.size())
                offset += lang.strings[s].size() * 2 + 1;
            else
                offset += 1;
        }

        for (uint32_t s=0 ; s<strCount ; ++s)
        {
            if (s < lang.strings.size())
                writer.writeWString(lang.strings[s]);
            else
                writer.writeUByte(0);
        }
    }
}

}
}
