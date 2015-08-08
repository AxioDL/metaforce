#include "STRG.hpp"
#include "DNAMP3.hpp"

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
    if (nameTableSz)
    {
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
    }

    std::vector<FourCC> readLangs;
    readLangs.reserve(langCount);
    for (atUint32 l=0 ; l<langCount ; ++l)
    {
        FourCC lang;
        lang.read(reader);
        readLangs.emplace_back(lang);
    }
    std::unique_ptr<atUint32[]> strOffs(new atUint32[langCount * strCount]);
    for (atUint32 l=0 ; l<langCount ; ++l)
    {
        reader.readUint32();
        for (atUint32 s=0 ; s<strCount ; ++s)
            strOffs[l*strCount+s] = reader.readUint32();
    }

    atUint64 strBase = reader.position();
    langs.clear();
    langs.reserve(langCount);
    for (atUint32 l=0 ; l<langCount ; ++l)
    {
        std::vector<std::string> strs;
        for (atUint32 s=0 ; s<strCount ; ++s)
        {
            reader.seek(strBase + strOffs[l*strCount+s], Athena::Begin);
            atUint32 len = reader.readUint32();
            strs.emplace_back(reader.readString(len));
        }
        langs.emplace_back(readLangs[l], strs);
    }

    langMap.clear();
    langMap.reserve(langCount);
    for (std::pair<FourCC, std::vector<std::string>>& item : langs)
        langMap.emplace(item.first, &item.second);
}

void STRG::read(Athena::io::IStreamReader& reader)
{
    reader.setEndian(Athena::BigEndian);
    atUint32 magic = reader.readUint32();
    if (magic != 0x87654321)
    {
        Log.report(LogVisor::Error, "invalid STRG magic");
        return;
    }

    atUint32 version = reader.readUint32();
    if (version != 3)
    {
        Log.report(LogVisor::Error, "invalid STRG version");
        return;
    }

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
    for (const auto& name : names)
        nameTableSz += name.first.size() + 1;
    writer.writeUint32(names.size());
    writer.writeUint32(nameTableSz);
    atUint32 offset = names.size() * 8;
    for (const auto& name : names)
    {
        writer.writeUint32(offset);
        writer.writeInt32(name.second);
        offset += name.first.size() + 1;
    }
    for (const auto& name : names)
        writer.writeString(name.first);

    for (const auto& lang : langs)
        lang.first.write(writer);

    offset = 0;
    for (const auto& lang : langs)
    {
        atUint32 langSz = 0;
        for (const std::string& str : lang.second)
            langSz += str.size() + 5;
        writer.writeUint32(langSz);

        for (const std::string& str : lang.second)
        {
            writer.writeUint32(offset);
            offset += str.size() + 5;
        }
    }

    for (atUint32 s=0 ; s<strCount ; ++s)
    {
        for (const auto& lang : langs)
        {
            if (s >= lang.second.size())
            {
                writer.writeUint32(1);
                writer.writeUByte(0);
            }
            else
            {
                const std::string& str = lang.second[s];
                writer.writeUint32(str.size() + 1);
                writer.writeString(str);
            }
        }
    }
}

}
}
