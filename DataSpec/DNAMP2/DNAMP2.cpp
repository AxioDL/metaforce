#define NOD_ATHENA 1
#include "DNAMP2.hpp"
#include "STRG.hpp"
#include "MLVL.hpp"
#include "CMDL.hpp"
#include "ANCS.hpp"
#include "MREA.hpp"
#include "MAPA.hpp"
#include "MAPU.hpp"
#include "AFSM.hpp"
#include "SAVW.hpp"
#include "AGSC.hpp"
#include "../DNAMP1/HINT.hpp"
#include "../DNAMP1/CSNG.hpp"
#include "../DNACommon/FSM2.hpp"
#include "../DNACommon/TXTR.hpp"
#include "../DNACommon/FONT.hpp"
#include "../DNACommon/DGRP.hpp"
#include "../DNACommon/ATBL.hpp"
#include "Runtime/GCNTypes.hpp"

namespace DataSpec
{
namespace DNAMP2
{
logvisor::Module Log("urde::DNAMP2");

static bool GetNoShare(const std::string& name)
{
    std::string lowerName = name;
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), tolower);
    if (!lowerName.compare(0, 7, "metroid"))
        return false;
    return true;
}

PAKBridge::PAKBridge(hecl::Database::Project& project,
                     const nod::Node& node,
                     bool doExtract)
: m_project(project), m_node(node), m_pak(true, GetNoShare(node.getName())), m_doExtract(doExtract)
{
    nod::AthenaPartReadStream rs(node.beginReadStream());
    m_pak.read(rs);

    /* Append Level String */
    for (const auto& entry : m_pak.m_entries)
    {
        const DNAMP1::PAK::Entry& e = entry.second;
        if (e.type == FOURCC('MLVL'))
        {
            PAKEntryReadStream rs = e.beginReadStream(m_node);
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

static hecl::SystemString LayerName(const std::string& name)
{
    hecl::SystemString ret = hecl::SystemStringView(name).sys_str();
    for (auto& ch : ret)
        if (ch == _S('/') || ch == _S('\\'))
            ch = _S('-');
    return ret;
}

void PAKBridge::build()
{
    /* First pass: build per-area/per-layer dependency map */
    for (const auto& entry : m_pak.m_entries)
    {
        const DNAMP1::PAK::Entry& e = entry.second;
        if (e.type == FOURCC('MLVL'))
        {
            Level& level = m_levelDeps[e.id];

            MLVL mlvl;
            {
                PAKEntryReadStream rs = e.beginReadStream(m_node);
                mlvl.read(rs);
            }
            bool named;
            std::string bestName = m_pak.bestEntryName(e, named);
            level.name = hecl::SystemStringView(bestName).sys_str();
            level.areas.reserve(mlvl.areaCount);
            unsigned layerIdx = 0;

            /* Make MAPW available to lookup MAPAs */
            const DNAMP1::PAK::Entry* worldMapEnt = m_pak.lookupEntry(mlvl.worldMap);
            std::vector<UniqueID32> mapw;
            if (worldMapEnt)
            {
                PAKEntryReadStream rs = worldMapEnt->beginReadStream(m_node);
                rs.seek(8, athena::Current);
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
                    areaDeps.name = hecl::StringUtils::TrimWhitespace(areaDeps.name);
                }
                if (areaDeps.name.empty())
                {
                    areaDeps.name = hecl::SystemStringView(area.internalAreaName).sys_str();
                    if (areaDeps.name.empty())
                    {
                        std::string idStr = area.areaMREAId.toString();
                        areaDeps.name = _S("MREA_") + hecl::SystemStringView(idStr).sys_str();
                    }
                }
                hecl::SystemChar num[16];
                hecl::SNPrintf(num, 16, _S("%02u "), ai);
                areaDeps.name = num + areaDeps.name;

                areaDeps.layers.reserve(area.depLayerCount-1);
                unsigned r=0;
                for (unsigned l=1 ; l<area.depLayerCount ; ++l)
                {
                    areaDeps.layers.emplace_back();
                    Level::Area::Layer& layer = areaDeps.layers.back();
                    layer.name = LayerName(mlvl.layerNames[layerIdx++]);
                    layer.active = layerFlags.flags >> (l-1) & 0x1;
                    layer.name = hecl::StringUtils::TrimWhitespace(layer.name);
                    hecl::SNPrintf(num, 16, _S("%02u "), l-1);
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
    for (auto& entry : m_pak.m_entries)
    {
        entry.second.unique.checkEntry(*this, entry.second);
    }
}

void PAKBridge::addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
        std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo,
        std::unordered_map<UniqueID32, std::pair<UniqueID32, std::string>>& cskrCinfToAncs) const
{
    for (const std::pair<UniqueID32, DNAMP1::PAK::Entry>& entry : m_pak.m_entries)
    {
        if (entry.second.type == FOURCC('ANCS'))
        {
            PAKEntryReadStream rs = entry.second.beginReadStream(m_node);
            ANCS ancs;
            ancs.read(rs);
            for (const ANCS::CharacterSet::CharacterInfo& ci : ancs.characterSet.characters)
            {
                addTo[ci.cmdl] = std::make_pair(ci.cskr, ci.cinf);
                cskrCinfToAncs[ci.cskr] = std::make_pair(entry.second.id, hecl::Format("%s.CSKR", ci.name.c_str()));
                cskrCinfToAncs[ci.cinf] = std::make_pair(entry.second.id, hecl::Format("CINF_%08X.CINF", ci.cinf.toUint32()));
                if (ci.cmdlOverlay)
                {
                    addTo[ci.cmdlOverlay] = std::make_pair(ci.cskrOverlay, ci.cinf);
                    cskrCinfToAncs[ci.cskrOverlay] = std::make_pair(entry.second.id, hecl::Format("%s.over.CSKR", ci.name.c_str()));
                }
            }
        }
    }
}

static const atVec4f BottomRow = {0.f, 0.f, 0.f, 1.f};

void PAKBridge::addMAPATransforms(PAKRouter<PAKBridge>& pakRouter,
        std::unordered_map<UniqueID32, zeus::CMatrix4f>& addTo,
        std::unordered_map<UniqueID32, hecl::ProjectPath>& pathOverrides) const
{
    for (const std::pair<UniqueID32, DNAMP1::PAK::Entry>& entry : m_pak.m_entries)
    {
        if (entry.second.type == FOURCC('MLVL'))
        {
            MLVL mlvl;
            {
                PAKEntryReadStream rs = entry.second.beginReadStream(m_node);
                mlvl.read(rs);
            }
            hecl::ProjectPath mlvlDirPath = pakRouter.getWorking(&entry.second).getParentPath();

            if (mlvl.worldNameId)
                pathOverrides[mlvl.worldNameId] = hecl::ProjectPath(mlvlDirPath, _S("!name.yaml"));

            for (const MLVL::Area& area : mlvl.areas)
            {
                hecl::ProjectPath areaDirPath = pakRouter.getWorking(area.areaMREAId).getParentPath();
                if (area.areaNameId)
                    pathOverrides[area.areaNameId] = hecl::ProjectPath(areaDirPath, _S("!name.yaml"));
            }

            if (mlvl.worldMap)
            {
                const nod::Node* mapNode;
                const DNAMP1::PAK::Entry* mapEntry = pakRouter.lookupEntry(mlvl.worldMap, &mapNode);
                if (mapEntry)
                {
                    PAKEntryReadStream rs = mapEntry->beginReadStream(*mapNode);
                    u32 magic = rs.readUint32Big();
                    if (magic == 0xDEADF00D)
                    {
                        rs.readUint32Big();
                        u32 count = rs.readUint32Big();
                        for (u32 i=0 ; i<count && i<mlvl.areas.size() ; ++i)
                        {
                            MLVL::Area& areaData = mlvl.areas[i];
                            UniqueID32 mapaId;
                            mapaId.read(rs);
                            addTo[mapaId] = zeus::CMatrix4f(
                                    areaData.transformMtx[0],
                                    areaData.transformMtx[1],
                                    areaData.transformMtx[2],
                                    BottomRow).transposed();
                        }
                    }
                }
            }
        }
    }
}

ResExtractor<PAKBridge> PAKBridge::LookupExtractor(const DNAMP1::PAK& pak, const DNAMP1::PAK::Entry& entry)
{
    switch (entry.type)
    {
    case SBIG('HINT'):
        return {DNAMP1::HINT::Extract, {_S(".yaml")}};
    case SBIG('STRG'):
        return {STRG::Extract, {_S(".yaml")}};
    case SBIG('TXTR'):
        return {TXTR::Extract, {_S(".png")}};
    case SBIG('AFSM'):
        return {AFSM::Extract, {_S(".yaml")}};
    case SBIG('SAVW'):
        return {SAVWCommon::ExtractSAVW<SAVW>, {_S(".yaml")}};
    case SBIG('CMDL'):
        return {CMDL::Extract, {_S(".blend")}, 1};
    case SBIG('ANCS'):
        return {ANCS::Extract, {_S(".yaml"), _S(".blend")}, 2};
    case SBIG('MLVL'):
        return {MLVL::Extract, {_S(".yaml"), _S(".blend")}, 3};
    case SBIG('MREA'):
        return {MREA::Extract, {_S(".blend")}, 4};
    case SBIG('MAPA'):
        return {MAPA::Extract, {_S(".blend")}, 4};
    case SBIG('MAPU'):
        return {MAPU::Extract, {_S(".blend")}, 5};
    case SBIG('FSM2'):
        return {DNAFSM2::ExtractFSM2<UniqueID32>, {_S(".yaml")}};
    case SBIG('FONT'):
        return {DNAFont::ExtractFONT<UniqueID32>, {_S(".yaml")}};
    case SBIG('DGRP'):
        return {DNADGRP::ExtractDGRP<UniqueID32>, {_S(".yaml")}};
    case SBIG('AGSC'):
        return {AGSC::Extract, {_S(".pool"), _S(".proj"), _S(".samp"), _S(".sdir")}};
    case SBIG('CSNG'):
        return {DNAMP1::CSNG::Extract, {_S(".mid"), _S(".yaml")}};
    case SBIG('ATBL'):
        return {DNAAudio::ATBL::Extract, {_S(".yaml")}};
    }
    return {};
}

}
}
