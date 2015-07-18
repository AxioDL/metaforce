#include "STRG.hpp"
#include "DNAMP1.hpp"

namespace Retro
{
namespace DNAMP1
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
        reader.seek(4);
    }

    langs.clear();
    langs.reserve(langCount);
    for (FourCC& lang : readLangs)
    {
        std::vector<std::wstring> strs;
        reader.seek(strCount * 4 + 4);
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
    if (version != 0)
        Log.report(LogVisor::Error, "invalid STRG version");

    _read(reader);
}

void STRG::write(Athena::io::IStreamWriter& writer) const
{
    writer.setEndian(Athena::BigEndian);
    writer.writeUint32(0x87654321);
    writer.writeUint32(0);
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
        for (atUint32 s=0 ; s<strCount ; ++s)
        {
            atUint32 chCount = lang.second[s].size();
            if (s < langStrCount)
                offset += chCount * 2 + 1;
            else
                offset += 1;
        }
    }

    for (const std::pair<FourCC, std::vector<std::wstring>>& lang : langs)
    {
        atUint32 langStrCount = lang.second.size();
        atUint32 tableSz = strCount * 4;
        for (atUint32 s=0 ; s<strCount ; ++s)
        {
            if (s < langStrCount)
                tableSz += lang.second[s].size() * 2 + 1;
            else
                tableSz += 1;
        }
        writer.writeUint32(tableSz);

        offset = strCount * 4;
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
    std::wstring_convert<std::codecvt_utf8<wchar_t>> wconv;

    /* Validate pass */
    for (AngelScript::asUINT i=0 ; i<in.GetGlobalVarCount() ; ++i)
    {
        const char* name;
        int typeId;
        if (in.GetGlobalVar(i, &name, 0, &typeId) < 0)
            continue;
        if (typeId == ASTYPE_STRGLanguage.getTypeID())
        {
            if (strlen(name) != 4)
            {
                Log.report(LogVisor::Error, "STRG language string '%s' from %s must be exactly 4 characters", name, in.GetName());
                return false;
            }
        }
    }

    /* Read pass */
    langs.clear();
    for (AngelScript::asUINT i=0 ; i<in.GetGlobalVarCount() ; ++i)
    {
        const char* name;
        int typeId;
        if (in.GetGlobalVar(i, &name, 0, &typeId) < 0)
            continue;
        if (typeId == ASTYPE_STRGLanguage.getTypeID())
        {
            const std::vector<std::string*>& strsin = ASTYPE_STRGLanguage.vectorCast(in.GetAddressOfGlobalVar(i));
            std::vector<std::wstring> strs;
            for (const std::string* str : strsin)
                strs.emplace_back(wconv.from_bytes(*str));
            langs.emplace_back(FourCC(name), strs);
        }
    }

    langMap.clear();
    langMap.reserve(langs.size());
    for (std::pair<FourCC, std::vector<std::wstring>>& item : langs)
        langMap.emplace(item.first, &item.second);

    return true;
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
}

}
}
