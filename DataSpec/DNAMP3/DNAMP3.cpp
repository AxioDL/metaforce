#include <set>

#define NOD_ATHENA 1
#include "DNAMP3.hpp"
#include "STRG.hpp"
#include "MLVL.hpp"
#include "CMDL.hpp"
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

    /* Append Level String */
    std::set<HECL::SystemString, CaseInsensitiveCompare> uniq;
    for (const PAK::Entry& entry : m_pak.m_entries)
    {
        if (entry.type == FOURCC('MLVL'))
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
                uniq.insert(mlvlName.getSystemString(FOURCC('ENGL'), 0));
            }
        }
    }
    bool comma = false;
    for (const HECL::SystemString& str : uniq)
    {
        if (comma)
            m_levelString += _S(", ");
        comma = true;
        m_levelString += str;
    }
}

void PAKBridge::build()
{
}

void PAKBridge::addCMDLRigPairs(std::unordered_map<UniqueID64, std::pair<UniqueID64, UniqueID64>>& addTo) const
{
    for (const std::pair<UniqueID64, PAK::Entry*>& entry : m_pak.m_idMap)
    {
        if (entry.second->type == FOURCC('CHAR'))
        {
        }
    }
}

ResExtractor<PAKBridge> PAKBridge::LookupExtractor(const PAK::Entry& entry)
{
    switch (entry.type)
    {
    case SBIG('STRG'):
        return {STRG::Extract, nullptr, {_S(".yaml")}};
    case SBIG('TXTR'):
        return {TXTR::Extract, nullptr, {_S(".png")}};
    case SBIG('CMDL'):
        return {nullptr, CMDL::Extract, {_S(".blend")}, 1};
    }
    return {};
}

}
}
