#include <set>

#define NOD_ATHENA 1
#include "DNAMP3.hpp"
#include "STRG.hpp"
#include "MLVL.hpp"
#include "CMDL.hpp"
#include "CHAR.hpp"
#include "MREA.hpp"
#include "../DNACommon/TXTR.hpp"

namespace Retro
{
namespace DNAMP3
{
LogVisor::LogModule Log("Retro::DNAMP3");

static bool GetNoShare(const std::string& name)
{
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), tolower);
    if (!lowerName.compare(0, 7, "metroid"))
        return false;
    return true;
}

PAKBridge::PAKBridge(HECL::Database::Project& project,
                     const NOD::DiscBase::IPartition::Node& node,
                     bool doExtract)
: m_project(project), m_node(node), m_pak(GetNoShare(node.getName())), m_doExtract(doExtract)
{
    NOD::AthenaPartReadStream rs(node.beginReadStream());
    m_pak.read(rs);

    /* Append Level String */
    std::set<HECL::SystemString, CaseInsensitiveCompare> uniq;
    for (PAK::Entry& entry : m_pak.m_entries)
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

static HECL::SystemString LayerName(const std::string& name)
{
#if HECL_UCS2
    HECL::SystemString ret = HECL::UTF8ToWide(name);
#else
    HECL::SystemString ret = name;
#endif
    for (auto& ch : ret)
        if (ch == _S('/') || ch == _S('\\'))
            ch = _S('-');
    return ret;
}

void PAKBridge::build()
{
    /* First pass: build per-area/per-layer dependency map */
    for (const PAK::Entry& entry : m_pak.m_entries)
    {
        if (entry.type == FOURCC('MLVL'))
        {
            Level& level = m_levelDeps[entry.id];

            PAKEntryReadStream rs = entry.beginReadStream(m_node);
            MLVL mlvl;
            mlvl.read(rs);
#if HECL_UCS2
            level.name = HECL::UTF8ToWide(m_pak.bestEntryName(entry));
#else
            level.name = m_pak.bestEntryName(entry);
#endif
            level.areas.reserve(mlvl.areaCount);
            unsigned layerIdx = 0;

            /* Pre-pass: find duplicate area names */
            std::unordered_map<HECL::SystemString, std::pair<atUint32, atUint32>> dupeTracker;
            dupeTracker.reserve(mlvl.areas.size());
            for (const MLVL::Area& area : mlvl.areas)
            {
                const PAK::Entry* areaNameEnt = m_pak.lookupEntry(area.areaNameId);
                if (areaNameEnt)
                {
                    STRG areaName;
                    PAKEntryReadStream rs = areaNameEnt->beginReadStream(m_node);
                    areaName.read(rs);
                    HECL::SystemString name = areaName.getSystemString(FOURCC('ENGL'), 0);
                    auto search = dupeTracker.find(name);
                    if (search != dupeTracker.end())
                        ++search->second.first;
                    else
                        dupeTracker[name] = std::make_pair(1, 1);
                }
            }

            /* Main-pass: index areas */
            auto layerFlagsIt = mlvl.layerFlags.begin();
            for (const MLVL::Area& area : mlvl.areas)
            {
                Level::Area& areaDeps = level.areas[area.areaMREAId];
                const PAK::Entry* areaNameEnt = m_pak.lookupEntry(area.areaNameId);
                if (areaNameEnt)
                {
                    STRG areaName;
                    PAKEntryReadStream rs = areaNameEnt->beginReadStream(m_node);
                    areaName.read(rs);
                    areaDeps.name = areaName.getSystemString(FOURCC('ENGL'), 0);
                    auto search = dupeTracker.find(areaDeps.name);
                    if (search != dupeTracker.end() && search->second.first > 1)
                    {
                        HECL::SystemChar num[16];
                        HECL::SNPrintf(num, 16, _S(" (%d)"), search->second.second++);
                        areaDeps.name += num;
                    }

                    /* Trim possible trailing whitespace */
#if HECL_UCS2
                    while (areaDeps.name.size() && iswblank(areaDeps.name.back()))
                        areaDeps.name.pop_back();
#else
                    while (areaDeps.name.size() && isblank(areaDeps.name.back()))
                        areaDeps.name.pop_back();
#endif
                }
                if (areaDeps.name.empty())
                {
#if HECL_UCS2
                    areaDeps.name = HECL::UTF8ToWide(area.internalAreaName);
#else
                    areaDeps.name = area.internalAreaName;
#endif
                    if (areaDeps.name.empty())
                    {
#if HECL_UCS2
                        areaDeps.name = _S("MREA_") + HECL::UTF8ToWide(area.areaMREAId.toString());
#else
                        areaDeps.name = "MREA_" + area.areaMREAId.toString();
#endif
                    }
                }

                const MLVL::LayerFlags& areaLayers = *layerFlagsIt++;
                if (areaLayers.layerCount)
                {
                    areaDeps.layers.reserve(areaLayers.layerCount);
                    for (unsigned l=0 ; l<areaLayers.layerCount ; ++l)
                    {
                        areaDeps.layers.emplace_back();
                        Level::Area::Layer& layer = areaDeps.layers.back();
                        layer.name = LayerName(mlvl.layerNames[layerIdx++]);
                        /* Trim possible trailing whitespace */
#if HECL_UCS2
                        while (layer.name.size() && iswblank(layer.name.back()))
                            layer.name.pop_back();
#else
                        while (layer.name.size() && isblank(layer.name.back()))
                            layer.name.pop_back();
#endif
                    }
                }

                /* Load area DEPS */
                const PAK::Entry* areaEntry = m_pak.lookupEntry(area.areaMREAId);
                if (areaEntry)
                {
                    PAKEntryReadStream ars = areaEntry->beginReadStream(m_node);
                    MREA::ExtractLayerDeps(ars, areaDeps);
                    areaDeps.resources.emplace(area.areaMREAId);
                }
            }
        }
    }

    /* Second pass: cross-compare uniqueness */
    for (PAK::Entry& entry : m_pak.m_entries)
    {
        entry.unique.checkEntry(*this, entry);
    }
}

void PAKBridge::addCMDLRigPairs(std::unordered_map<UniqueID64, std::pair<UniqueID64, UniqueID64>>& addTo) const
{
    for (const std::pair<UniqueID64, PAK::Entry*>& entry : m_pak.m_idMap)
    {
        if (entry.second->type == FOURCC('CHAR'))
        {
            PAKEntryReadStream rs = entry.second->beginReadStream(m_node);
            CHAR aChar;
            aChar.read(rs);
            const CHAR::CharacterInfo& ci = aChar.characterInfo;
            addTo[ci.cmdl] = std::make_pair(ci.cskr, ci.cinf);
            for (const CHAR::CharacterInfo::Overlay& overlay : ci.overlays)
                addTo[overlay.cmdl] = std::make_pair(overlay.cskr, ci.cinf);
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
    case SBIG('CHAR'):
        return {nullptr, CHAR::Extract, {_S(".yaml"), _S(".blend")}, 2};
    case SBIG('MREA'):
        return {nullptr, MREA::Extract, {_S(".blend")}, 3};
    case SBIG('MLVL'):
        return {MLVL::Extract, nullptr, {_S(".yaml")}};
    }
    return {};
}

}
}
