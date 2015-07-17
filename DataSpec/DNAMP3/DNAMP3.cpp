#include <set>

#define NOD_ATHENA 1
#include "DNAMP3.hpp"
#include "STRG.hpp"
#include "MLVL.hpp"

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

std::string PAKBridge::getLevelString() const
{
    std::string retval;
    std::set<HECL::SystemString> worldNames;
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
                worldNames.emplace(mlvlName.getSystemString(ENGL, 0));
            }
        }
    }

    for (const std::string& name : worldNames)
    {
        if (retval.size())
            retval += _S(", ");
        retval += name;
    }
    return retval;
}

ResExtractor PAKBridge::LookupExtractor(const PAK::Entry& entry)
{
    if (entry.type == Retro::STRG)
        return {STRG::Extract<STRG>, ".strg"};
    return {};
}

bool PAKBridge::extractResources(const HECL::ProjectPath& dirOut)
{
    for (const std::pair<UniqueID64, PAK::Entry*>& item : m_pak.m_idMap)
    {
        ResExtractor extractor = LookupExtractor(*item.second);
        if (extractor.func)
        {
            PAKEntryReadStream strgIn = item.second->beginReadStream(m_node);
            HECL::ProjectPath resPath(dirOut, m_pak.bestEntryName(*item.second) + extractor.fileExt);
            extractor.func(strgIn, resPath);
        }
    }
    return true;
}

}
}
