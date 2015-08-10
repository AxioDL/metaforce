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

ResExtractor<PAKBridge> PAKBridge::LookupExtractor(const PAK::Entry& entry)
{
    switch (entry.type.toUint32())
    {
    case SBIG('STRG'):
        return {STRG::Extract, nullptr, ".yaml"};
    case SBIG('TXTR'):
        return {TXTR::Extract, nullptr, ".png"};
    case SBIG('CMDL'):
        return {nullptr, CMDL::Extract, ".blend", 1};
    }
    return {};
}

}
}
