#include "STRG.hpp"
#include "../Logging.hpp"

namespace Retro
{
namespace DNAMP3
{

void STRG::_read(Athena::io::IStreamReader& reader)
{
    atUint32 langCount = reader.readUint32();
    atUint32 strCount = reader.readUint32();

    atUint32 nameCount = reader.readUint32();
    atUint32 nameTableSz = reader.readUint32();
    std::unique_ptr<uint8_t[]> nameTableBuf(new uint8_t[nameTableSz]);
    reader.readUBytesToBuf(nameTableBuf.get(), nameTableSz);
    struct NameIdxEntry
    {
        atUint32 nameOff;
        atUint32 strIdx;
    }* nameIndex = (NameIdxEntry*)nameTableBuf.get();
    for (atUint32 n=0 ; n<nameCount ; ++n)
    {
        const char* name = (char*)(nameTableBuf.get() + HECL::SBig(nameIndex[n].nameOff));
        names[name] = HECL::SBig(nameIndex[n].strIdx);
    }

    std::vector<FourCC> readLangs;
    readLangs.reserve(langCount);
    for (atUint32 l=0 ; l<langCount ; ++l)
    {
        FourCC lang;
        lang.read(reader);
        readLangs.emplace_back(lang);
        reader.seek(strCount * 4 + 4);
    }

    langs.clear();
    langs.reserve(langCount);
    for (FourCC& lang : readLangs)
    {
        std::vector<std::string> strs;
        for (atUint32 s=0 ; s<strCount ; ++s)
        {
            atUint32 len = reader.readUint32();
            strs.emplace_back(reader.readString(len));
        }
        langs.emplace(lang, strs);
    }
}

void STRG::read(Athena::io::IStreamReader& reader)
{
    reader.setEndian(Athena::BigEndian);
    atUint32 magic = reader.readUint32();
    if (magic != 0x87654321)
        LogModule.report(LogVisor::Error, "invalid STRG magic");

    atUint32 version = reader.readUint32();
    if (version != 3)
        LogModule.report(LogVisor::Error, "invalid STRG version");

    _read(reader);
}

void STRG::write(Athena::io::IStreamWriter& writer) const
{
    writer.setEndian(Athena::BigEndian);
    writer.writeUint32(0x87654321);
    writer.writeUint32(3);
    writer.writeUint32(langs.size());
    atUint32 strCount = STRG::count();
    writer.writeUint32(strCount);

    atUint32 nameTableSz = names.size() * 8;
    for (const std::pair<std::string, int32_t>& name : names)
        nameTableSz += name.first.size() + 1;
    writer.writeUint32(names.size());
    writer.writeUint32(nameTableSz);
    atUint32 offset = names.size() * 8;
    for (const std::pair<std::string, int32_t>& name : names)
    {
        writer.writeUint32(offset);
        writer.writeInt32(name.second);
        offset += name.first.size() + 1;
    }
    for (const std::pair<std::string, int32_t>& name : names)
        writer.writeString(name.first);

    offset = 0;
    for (const std::pair<FourCC, std::vector<std::string>>& lang : langs)
    {
        lang.first.write(writer);

        atUint32 langSz = 0;
        for (const std::string& str : lang.second)
            langSz += str.size() + 4;
        writer.writeUint32(langSz);

        for (const std::string& str : lang.second)
        {
            writer.writeUint32(offset);
            offset += str.size() + 4;
        }
    }

    for (const std::pair<FourCC, std::vector<std::string>>& lang : langs)
    {
        for (const std::string& str : lang.second)
        {
            writer.writeUint32(str.size());
            writer.writeString(str, str.size());
        }
    }
}

}
}
