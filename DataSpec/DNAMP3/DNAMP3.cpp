#include <set>

#define NOD_ATHENA 1
#include "DNAMP3.hpp"
#include "STRG.hpp"
#include "MLVL.hpp"
#include "../DNACommon/TXTR.hpp"

namespace Retro
{
namespace DNAMP3
{

LogVisor::LogModule Log("Retro::DNAMP3");

PAKBridge::PAKBridge(HECL::Database::Project& project, const NOD::DiscBase::IPartition::Node& node)
: m_project(project), m_node(node)
{
    NOD::AthenaPartReadStream rs(node.beginReadStream());
    m_pak.read(rs);
}

HECL::SystemString PAKBridge::getLevelString() const
{
    std::set<HECL::SystemString, CaseInsensitiveCompare> uniq;
    for (const PAK::Entry& entry : m_pak.m_entries)
    {
        if (entry.type == Retro::MLVL)
        {
            PAKEntryReadStream rs = entry.beginReadStream(m_node);
            MLVL mlvl;
            mlvl.read(rs);
            const PAK::Entry* nameEnt = m_pak.lookupEntry(mlvl.worldNameId);
            if (nameEnt)
            {
                PAKEntryReadStream rs = nameEnt->beginReadStream(m_node);
                STRG mlvlName;
                mlvlName.read(rs);
                uniq.insert(mlvlName.getSystemString(ENGL, 0));
            }
        }
    }
    HECL::SystemString retval;
    bool comma = false;
    for (const HECL::SystemString& str : uniq)
    {
        if (comma)
            retval += _S(", ");
        comma = true;
        retval += str;
    }
    return retval;
}

ResExtractor PAKBridge::LookupExtractor(const PAK::Entry& entry)
{
    switch (entry.type.toUint32())
    {
    case SBIG('STRG'):
        return {STRG::Extract, ".as"};
    case SBIG('TXTR'):
        return {TXTR::Extract, ".png"};
    }
    return {};
}

bool PAKBridge::extractResources(const HECL::ProjectPath& workingOut,
                                 const HECL::ProjectPath& cookedOut,
                                 bool force)
{
    for (const std::pair<UniqueID64, PAK::Entry*>& item : m_pak.m_idMap)
    {
        PAKEntryReadStream s;
        ResExtractor extractor = LookupExtractor(*item.second);
        if (extractor.func)
        {
            HECL::ProjectPath workPath(workingOut, m_pak.bestEntryName(*item.second) + extractor.fileExt);
            if (force || workPath.getPathType() == HECL::ProjectPath::PT_NONE)
            {
                s = item.second->beginReadStream(m_node);
                extractor.func(s, workPath);
            }
        }
        HECL::ProjectPath cookPath(cookedOut, m_pak.bestEntryName(*item.second));
        if (force || cookPath.getPathType() == HECL::ProjectPath::PT_NONE)
        {
            if (!s)
                s = item.second->beginReadStream(m_node);
            FILE* fout = HECL::Fopen(cookPath.getAbsolutePath().c_str(), _S("wb"));
            fwrite(s.data(), 1, s.length(), fout);
            fclose(fout);
        }
    }
    return true;
}

}
}
