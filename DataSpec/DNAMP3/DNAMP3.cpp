#include <set>

#define NOD_ATHENA 1
#include "DNAMP3.hpp"
#include "STRG.hpp"
#include "MLVL.hpp"
#include "CMDL.hpp"
#include "CHAR.hpp"
#include "MREA.hpp"
#include "MAPA.hpp"
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
                     const NOD::Node& node,
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
            const PAK::Entry* worldMapEnt = m_pak.lookupEntry(mlvl.worldMap);
            std::vector<UniqueID64> mapw;
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
            auto layerFlagsIt = mlvl.layerFlags.begin();
            for (const MLVL::Area& area : mlvl.areas)
            {
                Level::Area& areaDeps = level.areas[area.areaMREAId];
                const PAK::Entry* areaNameEnt = m_pak.lookupEntry(area.areaNameId);
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

                const MLVL::LayerFlags& layerFlags = *layerFlagsIt++;
                if (layerFlags.layerCount)
                {
                    areaDeps.layers.reserve(layerFlags.layerCount);
                    for (unsigned l=1 ; l<layerFlags.layerCount ; ++l)
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
                    }
                }

                /* Load area DEPS */
                const PAK::Entry* areaEntry = m_pak.lookupEntry(area.areaMREAId);
                if (areaEntry)
                {
                    PAKEntryReadStream ars = areaEntry->beginReadStream(m_node);
                    MREA::ExtractLayerDeps(ars, areaDeps);
                }
                areaDeps.resources.emplace(area.areaMREAId);
                if (mapw.size() > ai)
                    areaDeps.resources.emplace(mapw[ai]);
                ++ai;
            }
        }
    }

    /* Second pass: cross-compare uniqueness */
    for (PAK::Entry& entry : m_pak.m_entries)
    {
        entry.unique.checkEntry(*this, entry);
    }
}

void PAKBridge::addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
        std::unordered_map<UniqueID64, std::pair<UniqueID64, UniqueID64>>& addTo) const
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
    case SBIG('MLVL'):
        return {nullptr, MLVL::Extract, {_S(".blend")}, 3};
    case SBIG('MREA'):
        return {nullptr, MREA::Extract, {_S(".blend")}, 4};
    case SBIG('MAPA'):
        return {nullptr, MAPA::Extract, {_S(".blend")}, 4};
    }
    return {};
}

}
}
