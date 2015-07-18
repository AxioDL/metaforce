#include "STRG.hpp"
#include "DNAMP2.hpp"

namespace Retro
{
namespace DNAMP2
{

void STRG::_read(Athena::io::IStreamReader& reader)
{
    atUint32 langCount = reader.readUint32();
    atUint32 strCount = reader.readUint32();

    std::vector<FourCC> readLangs;
    readLangs.reserve(langCount);
    for (atUint32 l=0 ; l<langCount ; ++l)
    {
        FourCC lang;
        lang.read(reader);
        readLangs.emplace_back(lang);
        reader.seek(8);
    }

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

    langs.clear();
    langs.reserve(langCount);
    for (FourCC& lang : readLangs)
    {
        std::vector<std::wstring> strs;
        reader.seek(strCount * 4);
        for (atUint32 s=0 ; s<strCount ; ++s)
            strs.emplace_back(reader.readWString());
        langs.emplace_back(lang, strs);
    }

    langMap.clear();
    langMap.reserve(langCount);
    for (std::pair<FourCC, std::vector<std::wstring>>& item : langs)
        langMap.emplace(item.first, &item.second);
}

void STRG::read(Athena::io::IStreamReader& reader)
{
    reader.setEndian(Athena::BigEndian);
    atUint32 magic = reader.readUint32();
    if (magic != 0x87654321)
        Log.report(LogVisor::Error, "invalid STRG magic");

    atUint32 version = reader.readUint32();
    if (version != 1)
        Log.report(LogVisor::Error, "invalid STRG version");

    _read(reader);
}

void STRG::write(Athena::io::IStreamWriter& writer) const
{
    writer.setEndian(Athena::BigEndian);
    writer.writeUint32(0x87654321);
    writer.writeUint32(1);
    writer.writeUint32(langs.size());
    atUint32 strCount = STRG::count();
    writer.writeUint32(strCount);

    atUint32 offset = 0;
    for (const std::pair<FourCC, std::vector<std::wstring>>& lang : langs)
    {
        lang.first.write(writer);
        writer.writeUint32(offset);
        offset += strCount * 4 + 4;
        atUint32 langStrCount = lang.second.size();
        atUint32 tableSz = strCount * 4;
        for (atUint32 s=0 ; s<strCount ; ++s)
        {
            atUint32 chCount = lang.second[s].size();
            if (s < langStrCount)
            {
                offset += chCount * 2 + 1;
                tableSz += chCount * 2 + 1;
            }
            else
            {
                offset += 1;
                tableSz += 1;
            }
        }
        writer.writeUint32(tableSz);
    }

    atUint32 nameTableSz = names.size() * 8;
    for (const std::pair<std::string, int32_t>& name : names)
        nameTableSz += name.first.size() + 1;
    writer.writeUint32(names.size());
    writer.writeUint32(nameTableSz);
    offset = names.size() * 8;
    for (const std::pair<std::string, int32_t>& name : names)
    {
        writer.writeUint32(offset);
        writer.writeInt32(name.second);
        offset += name.first.size() + 1;
    }
    for (const std::pair<std::string, int32_t>& name : names)
        writer.writeString(name.first);

    for (const std::pair<FourCC, std::vector<std::wstring>>& lang : langs)
    {
        offset = strCount * 4;
        atUint32 langStrCount = lang.second.size();
        for (atUint32 s=0 ; s<strCount ; ++s)
        {
            writer.writeUint32(offset);
            if (s < langStrCount)
                offset += lang.second[s].size() * 2 + 1;
            else
                offset += 1;
        }

        for (atUint32 s=0 ; s<strCount ; ++s)
        {
            if (s < langStrCount)
                writer.writeWString(lang.second[s]);
            else
                writer.writeUByte(0);
        }
    }
}

bool STRG::readAngelScript(const AngelScript::asIScriptModule& in)
{
}

void STRG::writeAngelScript(std::ofstream& out) const
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> wconv;
    for (const std::pair<FourCC, std::vector<std::wstring>>& lang : langs)
    {
        out << "STRG::Language " << lang.first.toString() << "({";
        bool comma = false;
        unsigned idx = 0;
        for (const std::wstring& str : lang.second)
        {
            if (comma)
                out << ",";
            out << "\n/* " << idx++ << " */ \"";
            out << wconv.to_bytes(str);
            out << "\"";
            comma = true;
        }
        out << "\n});\n";
    }

    out << "STRG::Names NAMES({";
    bool comma = false;
    for (const std::pair<std::string, int32_t>& name : names)
    {
        if (comma)
            out << ",";
        out << "\n    ";
        comma = true;
        out << "{\"" << name.first << "\", " << name.second << "}";
    }
    out << "\n});\n";
}

}
}
