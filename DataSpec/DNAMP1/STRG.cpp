#include "STRG.hpp"
#include "DNAMP1.hpp"

namespace Retro
{
namespace DNAMP1
{

void STRG::_read(Athena::io::IStreamReader& reader)
{
    atUint32 langCount = reader.readUint32Big();
    atUint32 strCount = reader.readUint32Big();

    std::vector<FourCC> readLangs;
    readLangs.reserve(langCount);
    for (atUint32 l=0 ; l<langCount ; ++l)
    {
        DNAFourCC lang;
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
            strs.emplace_back(reader.readWStringBig());
        langs.emplace_back(lang, strs);
    }

    langMap.clear();
    langMap.reserve(langCount);
    for (std::pair<FourCC, std::vector<std::wstring>>& item : langs)
        langMap.emplace(item.first, &item.second);
}

void STRG::read(Athena::io::IStreamReader& reader)
{
    atUint32 magic = reader.readUint32Big();
    if (magic != 0x87654321)
        Log.report(LogVisor::Error, "invalid STRG magic");

    atUint32 version = reader.readUint32Big();
    if (version != 0)
        Log.report(LogVisor::Error, "invalid STRG version");

    _read(reader);
}

void STRG::write(Athena::io::IStreamWriter& writer) const
{
    writer.writeUint32Big(0x87654321);
    writer.writeUint32Big(0);
    writer.writeUint32Big(langs.size());
    atUint32 strCount = STRG::count();
    writer.writeUint32Big(strCount);

    atUint32 offset = 0;
    for (const std::pair<FourCC, std::vector<std::wstring>>& lang : langs)
    {
        DNAFourCC(lang.first).write(writer);
        writer.writeUint32Big(offset);
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
        writer.writeUint32Big(tableSz);

        offset = strCount * 4;
        for (atUint32 s=0 ; s<strCount ; ++s)
        {
            writer.writeUint32Big(offset);
            if (s < langStrCount)
                offset += lang.second[s].size() * 2 + 1;
            else
                offset += 1;
        }

        for (atUint32 s=0 ; s<strCount ; ++s)
        {
            if (s < langStrCount)
                writer.writeWStringBig(lang.second[s]);
            else
                writer.writeUByte(0);
        }
    }
}

size_t STRG::binarySize(size_t __isz) const
{
    __isz += 16;
    __isz += langs.size() * 12;

    size_t strCount = STRG::count();
    __isz += langs.size() * strCount * 4;
    for (const std::pair<FourCC, std::vector<std::wstring>>& lang : langs)
    {
        atUint32 langStrCount = lang.second.size();
        for (atUint32 s=0 ; s<strCount ; ++s)
        {
            if (s < langStrCount)
                __isz += (lang.second[s].size() + 1) * 2;
            else
                __isz += 1;
        }
    }

    return __isz;
}

void STRG::read(Athena::io::YAMLDocReader& reader)
{
    const Athena::io::YAMLNode* root = reader.getRootNode();

    /* Validate Pass */
    if (root->m_type == YAML_MAPPING_NODE)
    {
        for (const auto& lang : root->m_mapChildren)
        {
            if (lang.first.size() != 4)
            {
                Log.report(LogVisor::Warning, "STRG language string '%s' must be exactly 4 characters; skipping", lang.first.c_str());
                return;
            }
            if (lang.second->m_type != YAML_SEQUENCE_NODE)
            {
                Log.report(LogVisor::Warning, "STRG language string '%s' must contain a sequence; skipping", lang.first.c_str());
                return;
            }
            for (const auto& str : lang.second->m_seqChildren)
            {
                if (str->m_type != YAML_SCALAR_NODE)
                {
                    Log.report(LogVisor::Warning, "STRG language '%s' must contain all scalars; skipping", lang.first.c_str());
                    return;
                }
            }
        }
    }
    else
    {
        Log.report(LogVisor::Warning, "STRG must have a mapping root node; skipping");
        return;
    }

    /* Read Pass */
    langs.clear();
    for (const auto& lang : root->m_mapChildren)
    {
        std::vector<std::wstring> strs;
        for (const auto& str : lang.second->m_seqChildren)
            strs.emplace_back(HECL::UTF8ToWide(str->m_scalarString));
        langs.emplace_back(FourCC(lang.first.c_str()), strs);
    }

    langMap.clear();
    langMap.reserve(langs.size());
    for (auto& item : langs)
        langMap.emplace(item.first, &item.second);
}

void STRG::write(Athena::io::YAMLDocWriter& writer) const
{
    for (const auto& lang : langs)
    {
        writer.enterSubVector(lang.first.toString().c_str());
        for (const std::wstring& str : lang.second)
            writer.writeWString(nullptr, str);
        writer.leaveSubVector();
    }
}

const char* STRG::DNAType()
{
    return "Retro::DNAMP1::STRG";
}

}
}
