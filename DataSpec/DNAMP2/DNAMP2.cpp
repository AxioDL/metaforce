#define NOD_ATHENA 1
#include "DNAMP2.hpp"
#include "STRG.hpp"
#include "MLVL.hpp"
#include "CMDL.hpp"
#include "ANCS.hpp"
#include "MREA.hpp"
#include "../DNACommon/TXTR.hpp"

namespace Retro
{
namespace DNAMP2
{
LogVisor::LogModule Log("Retro::DNAMP2");

static bool GetNoShare(const std::string& name)
{
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), tolower);
    if (!lowerName.compare(0, 7, "metroid"))
        return false;
    return true;
}

PAKBridge::PAKBridge(HECL::Database::Project& project,
                     const NOD::Node& node,
                     bool doExtract)
: m_project(project), m_node(node), m_pak(true, GetNoShare(node.getName())), m_doExtract(doExtract)
{
    NOD::AthenaPartReadStream rs(node.beginReadStream());
    m_pak.read(rs);

    /* Append Level String */
    for (DNAMP1::PAK::Entry& entry : m_pak.m_entries)
    {
        if (entry.type == FOURCC('MLVL'))
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
                if (m_levelString.size())
                    m_levelString += _S(", ");
                m_levelString += mlvlName.getSystemString(FOURCC('ENGL'), 0);
            }
        }
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
    for (const DNAMP1::PAK::Entry& entry : m_pak.m_entries)
    {
        if (entry.type == FOURCC('MLVL'))
        {
            Level& level = m_levelDeps[entry.id];

            MLVL mlvl;
            {
                PAKEntryReadStream rs = entry.beginReadStream(m_node);
                mlvl.read(rs);
            }
#if HECL_UCS2
            level.name = HECL::UTF8ToWide(m_pak.bestEntryName(entry));
#else
            level.name = m_pak.bestEntryName(entry);
#endif
            level.areas.reserve(mlvl.areaCount);
            unsigned layerIdx = 0;

            /* Make MAPW available to lookup MAPAs */
            const DNAMP1::PAK::Entry* worldMapEnt = m_pak.lookupEntry(mlvl.worldMap);
            std::vector<UniqueID32> mapw;
            if (worldMapEnt)
            {
                PAKEntryReadStream rs = worldMapEnt->beginReadStream(m_node);
                rs.seek(8, Athena::Current);
                atUint32 areaCount = rs.readUint32Big();
                mapw.reserve(areaCount);
                for (atUint32 i=0 ; i<areaCount ; ++i)
                    mapw.emplace_back(rs);
            }

            /* Index areas */
            unsigned ai = 0;
            for (const MLVL::Area& area : mlvl.areas)
            {
                Level::Area& areaDeps = level.areas[area.areaMREAId];
                MLVL::LayerFlags& layerFlags = mlvl.layerFlags[ai];
                const DNAMP1::PAK::Entry* areaNameEnt = m_pak.lookupEntry(area.areaNameId);
                if (areaNameEnt)
                {
                    STRG areaName;
                    {
                        PAKEntryReadStream rs = areaNameEnt->beginReadStream(m_node);
                        areaName.read(rs);
                    }
                    areaDeps.name = areaName.getSystemString(FOURCC('ENGL'), 0);

                    /* Trim possible trailing whitespace */
#if HECL_UCS2
                    while (areaDeps.name.size() && iswspace(areaDeps.name.back()))
                        areaDeps.name.pop_back();
#else
                    while (areaDeps.name.size() && isspace(areaDeps.name.back()))
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
                HECL::SystemChar num[16];
                HECL::SNPrintf(num, 16, _S("%02u "), ai);
                areaDeps.name = num + areaDeps.name;

                areaDeps.layers.reserve(area.depLayerCount-1);
                unsigned r=0;
                for (unsigned l=1 ; l<area.depLayerCount ; ++l)
                {
                    areaDeps.layers.emplace_back();
                    Level::Area::Layer& layer = areaDeps.layers.back();
                    layer.name = LayerName(mlvl.layerNames[layerIdx++]);
                    layer.active = layerFlags.flags >> (l-1) & 0x1;
                    /* Trim possible trailing whitespace */
#if HECL_UCS2
                    while (layer.name.size() && iswspace(layer.name.back()))
                        layer.name.pop_back();
#else
                    while (layer.name.size() && isspace(layer.name.back()))
                        layer.name.pop_back();
#endif
                    HECL::SNPrintf(num, 16, layer.active ? _S("%02ua ") : _S("%02u "), l-1);
                    layer.name = num + layer.name;

                    layer.resources.reserve(area.depLayers[l] - r);
                    for (; r<area.depLayers[l] ; ++r)
                        layer.resources.emplace(area.deps[r].id);
                }
                areaDeps.resources.reserve(area.depCount - r + 2);
                for (; r<area.depCount ; ++r)
                    areaDeps.resources.emplace(area.deps[r].id);
                areaDeps.resources.emplace(area.areaMREAId);
                if (mapw.size() > ai)
                    areaDeps.resources.emplace(mapw[ai]);
                ++ai;
            }
        }
    }

    /* Second pass: cross-compare uniqueness */
    for (DNAMP1::PAK::Entry& entry : m_pak.m_entries)
    {
        entry.unique.checkEntry(*this, entry);
    }
}

void PAKBridge::addCMDLRigPairs(std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
{
    for (const std::pair<UniqueID32, DNAMP1::PAK::Entry*>& entry : m_pak.m_idMap)
    {
        if (entry.second->type == FOURCC('ANCS'))
        {
            PAKEntryReadStream rs = entry.second->beginReadStream(m_node);
            ANCS ancs;
            ancs.read(rs);
            for (const ANCS::CharacterSet::CharacterInfo& ci : ancs.characterSet.characters)
            {
                addTo[ci.cmdl] = std::make_pair(ci.cskr, ci.cinf);
                if (ci.cmdlOverlay)
                    addTo[ci.cmdlOverlay] = std::make_pair(ci.cskrOverlay, ci.cinf);
            }
        }
    }
}

ResExtractor<PAKBridge> PAKBridge::LookupExtractor(const DNAMP1::PAK::Entry& entry)
{
    switch (entry.type)
    {
    case SBIG('STRG'):
        return {STRG::Extract, nullptr, {_S(".yaml")}};
    case SBIG('TXTR'):
        return {TXTR::Extract, nullptr, {_S(".png")}};
    case SBIG('CMDL'):
        return {nullptr, CMDL::Extract, {_S(".blend")}, 1};
    case SBIG('ANCS'):
        return {nullptr, ANCS::Extract, {_S(".yaml"), _S(".blend")}, 2};
    case SBIG('MREA'):
        return {nullptr, MREA::Extract, {_S(".blend")}, 3};
    case SBIG('MLVL'):
        return {MLVL::Extract, nullptr, {_S(".yaml")}};
    }
    return {};
}

}
}
