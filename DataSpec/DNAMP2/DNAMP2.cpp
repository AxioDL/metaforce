#define NOD_ATHENA 1
#include "DNAMP2.hpp"
#include "STRG.hpp"
#include "MLVL.hpp"
#include "../DNACommon/TXTR.hpp"

namespace Retro
{
namespace DNAMP2
{

LogVisor::LogModule Log("Retro::DNAMP2");

PAKBridge::PAKBridge(HECL::Database::Project& project, const NOD::DiscBase::IPartition::Node& node)
: m_project(project), m_node(node), m_pak(true)
{
    NOD::AthenaPartReadStream rs(node.beginReadStream());
    m_pak.read(rs);
}

HECL::SystemString PAKBridge::getLevelString() const
{
    HECL::SystemString retval;
    for (const DNAMP1::PAK::Entry& entry : m_pak.m_entries)
    {
        if (entry.type == Retro::MLVL)
        {
            PAKEntryReadStream rs = entry.beginReadStream(m_node);
            MLVL mlvl;
            mlvl.read(rs);
            const DNAMP1::PAK::Entry* nameEnt = m_pak.lookupEntry(mlvl.worldNameId);
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

ResExtractor PAKBridge::LookupExtractor(const DNAMP1::PAK::Entry& entry)
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

bool PAKBridge::extractResources(const PAKRouter<PAKBridge>& router,
                                 bool force,
                                 std::function<void(float)> progress)
{
    size_t count = 0;
    for (const std::pair<UniqueID32, DNAMP1::PAK::Entry*>& item : m_pak.m_idMap)
    {
        PAKEntryReadStream s;

        auto cooked = router.getCooked(item.first);
        if (force || cooked.first.getPathType() == HECL::ProjectPath::PT_NONE)
        {
            if (!s)
                s = item.second->beginReadStream(m_node);
            FILE* fout = HECL::Fopen(cooked.first.getAbsolutePath().c_str(), _S("wb"));
            fwrite(s.data(), 1, s.length(), fout);
            fclose(fout);
        }

        auto working = router.getWorking(item.first);
        if (working.second.func)
        {
            if (force || working.first.getPathType() == HECL::ProjectPath::PT_NONE)
            {
                s = item.second->beginReadStream(m_node);
                working.second.func(s, working.first);
            }
        }

        ++count;
        progress(count / (float)m_pak.m_idMap.size());
    }
    return true;
}

}
}
