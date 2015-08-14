#define NOD_ATHENA 1
#include "DNAMP2.hpp"
#include "STRG.hpp"
#include "MLVL.hpp"
#include "CMDL.hpp"
#include "ANCS.hpp"
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

    /* Append Level String */
    for (const DNAMP1::PAK::Entry& entry : m_pak.m_entries)
    {
        if (entry.type == SBIG('MLVL'))
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

UniqueResult PAKBridge::uniqueCheck(const DNAMP1::PAK::Entry& entry)
{
    UniqueResult::Type result = UniqueResult::UNIQUE_NOTFOUND;
    bool foundOneLayer = false;
    UniqueID32 areaId;
    unsigned layerIdx;
    for (const auto& pair : m_areaDeps)
    {
        unsigned l=0;
        for (const auto& layer : pair.second.layers)
        {
            if (layer.resources.find(entry.id) != layer.resources.end())
            {
                if (foundOneLayer)
                {
                    if (areaId == pair.first)
                        result = UniqueResult::UNIQUE_AREA;
                    else
                        return {UniqueResult::UNIQUE_LEVEL};
                    continue;
                }
                else
                    result = UniqueResult::UNIQUE_LAYER;
                areaId = pair.first;
                layerIdx = l;
                foundOneLayer = true;
            }
            ++l;
        }
        if (pair.second.resources.find(entry.id) != pair.second.resources.end())
        {
            if (foundOneLayer)
            {
                if (areaId == pair.first)
                    result = UniqueResult::UNIQUE_AREA;
                else
                    return {UniqueResult::UNIQUE_LEVEL};
                continue;
            }
            else
                result = UniqueResult::UNIQUE_AREA;
            areaId = pair.first;
            foundOneLayer = true;
        }
    }
    UniqueResult retval = {result};
    if (result == UniqueResult::UNIQUE_LAYER || result == UniqueResult::UNIQUE_AREA)
    {
        const PAKBridge::Area& area = m_areaDeps[areaId];
        retval.areaName = &area.name;
        if (result == UniqueResult::UNIQUE_LAYER)
        {
            const PAKBridge::Area::Layer& layer = area.layers[layerIdx];
            retval.layerName = &layer.name;
        }
    }
    return retval;
}

static HECL::SystemString LayerName(const std::string& name)
{
#if HECL_UCS2
    HECL::SystemString ret = HECL::UTF8ToWide(mlvl.layerNames[layerIdx++]);
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
        if (entry.type == SBIG('MLVL'))
        {
            PAKEntryReadStream rs = entry.beginReadStream(m_node);
            MLVL mlvl;
            mlvl.read(rs);
            m_areaDeps.reserve(mlvl.areaCount);
            unsigned layerIdx = 0;
            for (const MLVL::Area& area : mlvl.areas)
            {
                Area& areaDeps = m_areaDeps[area.areaMREAId];
                const DNAMP1::PAK::Entry* areaNameEnt = m_pak.lookupEntry(area.areaNameId);
                if (areaNameEnt)
                {
                    STRG areaName;
                    PAKEntryReadStream rs = areaNameEnt->beginReadStream(m_node);
                    areaName.read(rs);
                    areaDeps.name = areaName.getSystemString(FOURCC('ENGL'), 0);
                }
                if (areaDeps.name.empty())
                {
                    areaDeps.name = area.internalAreaName;
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

                areaDeps.layers.reserve(area.depLayerCount-1);
                unsigned r=0;
                for (unsigned l=1 ; l<area.depLayerCount ; ++l)
                {
                    areaDeps.layers.emplace_back();
                    Area::Layer& layer = areaDeps.layers.back();
                    layer.name = LayerName(mlvl.layerNames[layerIdx++]);
                    layer.resources.reserve(area.depLayers[l] - r);
                    for (; r<area.depLayers[l] ; ++r)
                        layer.resources.emplace(area.deps[r].id);
                }
                areaDeps.resources.reserve(area.depCount - r);
                for (; r<area.depCount ; ++r)
                    areaDeps.resources.emplace(area.deps[r].id);
                areaDeps.resources.emplace(area.areaMREAId);
            }
        }
    }

    /* Second pass: cross-compare uniqueness */
    for (DNAMP1::PAK::Entry& entry : m_pak.m_entries)
    {
        entry.unique = uniqueCheck(entry);
    }
}

ResExtractor<PAKBridge> PAKBridge::LookupExtractor(const DNAMP1::PAK::Entry& entry)
{
    switch (entry.type)
    {
    case SBIG('STRG'):
        return {STRG::Extract, nullptr, ".yaml"};
    case SBIG('TXTR'):
        return {TXTR::Extract, nullptr, ".png"};
    case SBIG('CMDL'):
        return {nullptr, CMDL::Extract, ".blend", 2};
    case SBIG('ANCS'):
        return {nullptr, ANCS::Extract, nullptr, 1};
    }
    return {};
}

}
}
