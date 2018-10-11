#include <set>

#define NOD_ATHENA 1
#include "DNAMP3.hpp"
#include "STRG.hpp"
#include "MLVL.hpp"
#include "CAUD.hpp"
#include "CMDL.hpp"
#include "CHAR.hpp"
#include "MREA.hpp"
#include "MAPA.hpp"
#include "SAVW.hpp"
#include "HINT.hpp"
#include "DataSpec/DNACommon/TXTR.hpp"
#include "DataSpec/DNACommon/FONT.hpp"
#include "DataSpec/DNACommon/FSM2.hpp"
#include "DataSpec/DNACommon/DGRP.hpp"
#include "Runtime/GCNTypes.hpp"

namespace DataSpec::DNAMP3
{
logvisor::Module Log("urde::DNAMP3");

static bool GetNoShare(std::string_view name)
{
    std::string lowerName(name);
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), tolower);
    if (!lowerName.compare(0, 7, "metroid"))
        return false;
    return true;
}

PAKBridge::PAKBridge(const nod::Node& node,
                     bool doExtract)
: m_node(node), m_pak(GetNoShare(node.getName())), m_doExtract(doExtract)
{
    nod::AthenaPartReadStream rs(node.beginReadStream());
    m_pak.read(rs);

    /* Append Level String */
    std::set<hecl::SystemString, hecl::CaseInsensitiveCompare> uniq;
    for (auto& ent : m_pak.m_entries)
    {
        PAK::Entry& entry = ent.second;
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
    for (const hecl::SystemString& str : uniq)
    {
        if (comma)
            m_levelString += _S(", ");
        comma = true;
        m_levelString += str;
    }
}

static hecl::SystemString LayerName(std::string_view name)
{
    hecl::SystemString ret(hecl::SystemStringConv(name).sys_str());
    for (auto& ch : ret)
        if (ch == _S('/') || ch == _S('\\'))
            ch = _S('-');
    return ret;
}

void PAKBridge::build()
{
    /* First pass: build per-area/per-layer dependency map */
    for (const auto& ent : m_pak.m_entries)
    {
        const PAK::Entry& entry = ent.second;
        if (entry.type == FOURCC('MLVL'))
        {
            Level& level = m_levelDeps[entry.id];

            MLVL mlvl;
            {
                PAKEntryReadStream rs = entry.beginReadStream(m_node);
                mlvl.read(rs);
            }
            bool named;
            std::string bestName = m_pak.bestEntryName(m_node, entry, named);
            level.name = hecl::SystemStringConv(bestName).sys_str();
            level.areas.reserve(mlvl.areaCount);
            unsigned layerIdx = 0;

            /* Make MAPW available to lookup MAPAs */
            const PAK::Entry* worldMapEnt = m_pak.lookupEntry(mlvl.worldMap);
            std::vector<UniqueID64> mapw;
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
                    areaDeps.name = hecl::StringUtils::TrimWhitespace(areaDeps.name);
                }
                if (areaDeps.name.empty())
                {
                    areaDeps.name = hecl::SystemStringConv(area.internalAreaName).sys_str();
                    if (areaDeps.name.empty())
                    {
                        std::string idStr = area.areaMREAId.toString();
                        areaDeps.name = hecl::SystemString(_S("MREA_")) + hecl::SystemStringConv(idStr).c_str();
                    }
                }
                hecl::SystemChar num[16];
                hecl::SNPrintf(num, 16, _S("%02u "), ai);
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
                        layer.name = hecl::StringUtils::TrimWhitespace(layer.name);
                        hecl::SNPrintf(num, 16, _S("%02u "), l-1);
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
    for (auto& entry : m_pak.m_entries)
    {
        entry.second.unique.checkEntry(*this, entry.second);
    }
}

void PAKBridge::addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID64>& charAssoc) const
{
    for (const std::pair<UniqueID64, PAK::Entry>& entry : m_pak.m_entries)
    {
        if (entry.second.type == FOURCC('CHAR'))
        {
            PAKEntryReadStream rs = entry.second.beginReadStream(m_node);
            CHAR aChar;
            aChar.read(rs);
            const CHAR::CharacterInfo& ci = aChar.characterInfo;
            charAssoc.m_cmdlRigs[ci.cmdl] = std::make_pair(ci.cskr, ci.cinf);
            charAssoc.m_cskrCinfToCharacter[ci.cskr] =
                std::make_pair(entry.second.id, hecl::Format("%s.CSKR", ci.name.c_str()));
            charAssoc.m_cskrCinfToCharacter[ci.cinf] =
                std::make_pair(entry.second.id, hecl::Format("CINF_%" PRIX64 ".CINF", ci.cinf.toUint64()));
            for (const CHAR::CharacterInfo::Overlay& overlay : ci.overlays)
            {
                charAssoc.m_cmdlRigs[overlay.cmdl] = std::make_pair(overlay.cskr, ci.cinf);
                charAssoc.m_cskrCinfToCharacter[overlay.cskr] = std::make_pair(entry.second.id,
                    hecl::Format("%s.%.4s.CSKR", ci.name.c_str(), overlay.type.getChars()));
            }
        }
    }
}

static const atVec4f BottomRow = {0.f, 0.f, 0.f, 1.f};

void PAKBridge::addMAPATransforms(PAKRouter<PAKBridge>& pakRouter,
        std::unordered_map<UniqueID64, zeus::CMatrix4f>& addTo,
        std::unordered_map<UniqueID64, hecl::ProjectPath>& pathOverrides) const
{
    for (const std::pair<UniqueID64, PAK::Entry>& entry : m_pak.m_entries)
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
                const PAK::Entry* mapEntry = pakRouter.lookupEntry(mlvl.worldMap, &mapNode);
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
                            UniqueID64 mapaId;
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

ResExtractor<PAKBridge> PAKBridge::LookupExtractor(const nod::Node& pakNode, const PAK& pak, const PAK::Entry& entry)
{
    switch (entry.type)
    {
//    case SBIG('CAUD'):
//        return {CAUD::Extract, {_S(".yaml")}};
    case SBIG('STRG'):
        return {STRG::Extract, {_S(".yaml")}};
    case SBIG('TXTR'):
        return {TXTR::Extract, {_S(".png")}};
    case SBIG('SAVW'):
        return {SAVWCommon::ExtractSAVW<SAVW>, {_S(".yaml")}};
    case SBIG('HINT'):
        return {HINT::Extract, {_S(".yaml")}};
    case SBIG('CMDL'):
        return {CMDL::Extract, {_S(".blend")}, 1};
    case SBIG('CHAR'):
        return {CHAR::Extract, {_S(".yaml"), _S(".blend")}, 2};
    case SBIG('MLVL'):
        return {MLVL::Extract, {_S(".yaml"), _S(".blend")}, 3};
    case SBIG('MREA'):
        return {MREA::Extract, {_S(".blend")}, 4};
    case SBIG('MAPA'):
        return {MAPA::Extract, {_S(".blend")}, 4};
    case SBIG('FSM2'):
        return {DNAFSM2::ExtractFSM2<UniqueID64>, {_S(".yaml")}};
    case SBIG('FONT'):
        return {DNAFont::ExtractFONT<UniqueID64>, {_S(".yaml")}};
    case SBIG('DGRP'):
        return {DNADGRP::ExtractDGRP<UniqueID64>, {_S(".yaml")}};
    }
    return {};
}

}
