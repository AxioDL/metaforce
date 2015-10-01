#include "STRG.hpp"
#include "DNAMP2.hpp"

namespace Retro
{
namespace DNAMP2
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
        reader.seek(8);
    }

    atUint32 nameCount = reader.readUint32Big();
    atUint32 nameTableSz = reader.readUint32Big();
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
    if (version != 1)
        Log.report(LogVisor::Error, "invalid STRG version");

    _read(reader);
}

void STRG::write(Athena::io::IStreamWriter& writer) const
{
    writer.writeUint32Big(0x87654321);
    writer.writeUint32Big(1);
    writer.writeUint32Big(langs.size());
    atUint32 strCount = STRG::count();
    writer.writeUint32Big(strCount);

    atUint32 offset = 0;
    for (const std::pair<DNAFourCC, std::vector<std::wstring>>& lang : langs)
    {
        lang.first.write(writer);
        writer.writeUint32Big(offset);
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
        writer.writeUint32Big(tableSz);
    }

    atUint32 nameTableSz = names.size() * 8;
    for (const std::pair<std::string, int32_t>& name : names)
        nameTableSz += name.first.size() + 1;
    writer.writeUint32Big(names.size());
    writer.writeUint32Big(nameTableSz);
    offset = names.size() * 8;
    for (const std::pair<std::string, int32_t>& name : names)
    {
        writer.writeUint32Big(offset);
        writer.writeInt32Big(name.second);
        offset += name.first.size() + 1;
    }
    for (const std::pair<std::string, int32_t>& name : names)
        writer.writeString(name.first);

    for (const std::pair<DNAFourCC, std::vector<std::wstring>>& lang : langs)
    {
        offset = strCount * 4;
        atUint32 langStrCount = lang.second.size();
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

void STRG::fromYAML(Athena::io::YAMLDocReader& reader)
{
    const Athena::io::YAMLNode* root = reader.getRootNode();

    /* Validate Pass */
    if (root->m_type == YAML_MAPPING_NODE)
    {
        for (const auto& lang : root->m_mapChildren)
        {
            if (!lang.first.compare("names"))
                continue;
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

    names.clear();
    const Athena::io::YAMLNode* namesNode = root->findMapChild("names");
    if (namesNode)
        for (const auto& item : namesNode->m_mapChildren)
            names[item.first] = Athena::io::NodeToVal<atInt32>(item.second.get());

    langMap.clear();
    langMap.reserve(langs.size());
    for (std::pair<FourCC, std::vector<std::wstring>>& item : langs)
        langMap.emplace(item.first, &item.second);
}

void STRG::toYAML(Athena::io::YAMLDocWriter& writer) const
{
    for (const auto& lang : langs)
    {
        writer.enterSubVector(lang.first.toString().c_str());
        for (const std::wstring& str : lang.second)
            writer.writeWString(nullptr, str);
        writer.leaveSubVector();
    }
    if (names.size())
    {
        writer.enterSubRecord("names");
        for (const auto& name : names)
        {
            writer.enterSubRecord(name.first.c_str());
            writer.writeInt32(nullptr, name.second);
            writer.leaveSubRecord();
        }
        writer.leaveSubRecord();
    }
}

const char* STRG::DNAType()
{
    return "Retro::DNAMP2::STRG";
}

}
}
