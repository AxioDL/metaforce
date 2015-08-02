#include <stdio.h>

#define NOD_ATHENA 1
#include "DNAMP1.hpp"
#include "STRG.hpp"
#include "MLVL.hpp"
#include "../DNACommon/TXTR.hpp"
#include "CMDL.hpp"

namespace Retro
{
namespace DNAMP1
{
LogVisor::LogModule Log("Retro::DNAMP1");

PAKBridge::PAKBridge(HECL::Database::Project& project, const NOD::DiscBase::IPartition::Node& node)
: m_project(project), m_node(node), m_pak(false)
{
    NOD::AthenaPartReadStream rs(node.beginReadStream());
    m_pak.read(rs);
}

HECL::SystemString PAKBridge::getLevelString() const
{
    HECL::SystemString retval;
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
                if (retval.size())
                    retval += _S(", ");
                retval += mlvlName.getSystemString(ENGL, 0);
            }
        }
    }
    return retval;
}

ResExtractor PAKBridge::LookupExtractor(const PAK::Entry& entry)
{
    switch (entry.type.toUint32())
    {
    case SBIG('STRG'):
        return {STRG::Extract, ".yaml"};
    case SBIG('TXTR'):
        return {TXTR::Extract, ".png"};
    case SBIG('CMDL'):
        return {CMDL::Extract, ".blend", 1};
    case SBIG('MLVL'):
        return {MLVL::Extract, ".yaml"};
    }
    return {};
}

}
}
