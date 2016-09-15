#include <stdio.h>

#define NOD_ATHENA 1
#include "DNAMP1.hpp"
#include "STRG.hpp"
#include "SCAN.hpp"
#include "MLVL.hpp"
#include "../DNACommon/TXTR.hpp"
#include "../DNACommon/PART.hpp"
#include "../DNACommon/ELSC.hpp"
#include "../DNACommon/SWHC.hpp"
#include "../DNACommon/CRSC.hpp"
#include "../DNACommon/WPSC.hpp"
#include "../DNACommon/DPSC.hpp"
#include "../DNACommon/FONT.hpp"
#include "../DNACommon/DGRP.hpp"
#include "HINT.hpp"
#include "CMDL.hpp"
#include "AFSM.hpp"
#include "SAVW.hpp"
#include "ANCS.hpp"
#include "MREA.hpp"
#include "MAPA.hpp"
#include "FRME.hpp"

#include "../DNACommon/Tweaks/TweakWriter.hpp"
#include "Tweaks/CTweakPlayerRes.hpp"
#include "Tweaks/CTweakGunRes.hpp"
#include "Tweaks/CTweakPlayer.hpp"
#include "Tweaks/CTweakCameraBob.hpp"

namespace DataSpec
{
namespace DNAMP1
{
logvisor::Module Log("urde::DNAMP1");

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
: m_project(project), m_node(node), m_pak(false, GetNoShare(node.getName())), m_doExtract(doExtract)
{
    nod::AthenaPartReadStream rs(node.beginReadStream());
    m_pak.read(rs);

    /* Append Level String */
    for (PAK::Entry& entry : m_pak.m_entries)
    {
        if (entry.type == FOURCC('MLVL'))
        {
            PAKEntryReadStream rs = entry.beginReadStream(m_node);
            MLVL mlvl;
            mlvl.read(rs);
            PAK::Entry* nameEnt = (PAK::Entry*)m_pak.lookupEntry(mlvl.worldNameId);
            if (nameEnt)
            {
                nameEnt->name = entry.name + "_name";
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
            bool named;
            std::string bestName = m_pak.bestEntryName(entry, named);
            level.name = hecl::SystemStringView(bestName).sys_str();
            level.areas.reserve(mlvl.areaCount);
            unsigned layerIdx = 0;

            /* Make MAPW available to lookup MAPAs */
            PAK::Entry* worldMapEnt = (PAK::Entry*)m_pak.lookupEntry(mlvl.worldMap);
            std::vector<UniqueID32> mapw;
            if (worldMapEnt)
            {
                worldMapEnt->name = entry.name + "_mapw";
                PAKEntryReadStream rs = worldMapEnt->beginReadStream(m_node);
                rs.seek(8, athena::Current);
                atUint32 areaCount = rs.readUint32Big();
                mapw.reserve(areaCount);
                for (atUint32 i=0 ; i<areaCount ; ++i)
                    mapw.emplace_back(rs);
            }

            PAK::Entry* savwEnt = (PAK::Entry*)m_pak.lookupEntry(mlvl.saveWorldId);
            if (savwEnt)
                savwEnt->name = entry.name + "_savw";

            PAK::Entry* skyEnt = (PAK::Entry*)m_pak.lookupEntry(mlvl.worldSkyboxId);
            if (skyEnt)
                skyEnt->name = entry.name + "_skybox";

            /* Index areas */
            unsigned ai = 0;
            for (const MLVL::Area& area : mlvl.areas)
            {
                Level::Area& areaDeps = level.areas[area.areaMREAId];
                MLVL::LayerFlags& layerFlags = mlvl.layerFlags[ai];
                PAK::Entry* areaNameEnt = (PAK::Entry*)m_pak.lookupEntry(area.areaNameId);
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
                    std::string idStr = area.areaMREAId.toString();
                    areaDeps.name = _S("MREA_") + hecl::SystemStringView(idStr).sys_str();
                }
                hecl::SystemChar num[16];
                hecl::SNPrintf(num, 16, _S("%02u "), ai);
                areaDeps.name = num + areaDeps.name;

                std::string lowerName = hecl::SystemUTF8View(areaDeps.name).str();
                for (char& ch : lowerName)
                {
                    ch = tolower(ch);
                    if (ch == ' ')
                        ch = '_';
                }
                if (areaNameEnt)
                    areaNameEnt->name = lowerName + "_name";
                PAK::Entry* areaEnt = (PAK::Entry*)m_pak.lookupEntry(area.areaMREAId);
                if (areaEnt)
                    areaEnt->name = lowerName;

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
    for (PAK::Entry& entry : m_pak.m_entries)
    {
        entry.unique.checkEntry(*this, entry);
    }
}

void PAKBridge::addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
        std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
{
    for (const std::pair<UniqueID32, PAK::Entry*>& entry : m_pak.m_idMap)
    {
        if (entry.second->type == FOURCC('ANCS'))
        {
            PAKEntryReadStream rs = entry.second->beginReadStream(m_node);
            ANCS ancs;
            ancs.read(rs);
            for (const ANCS::CharacterSet::CharacterInfo& ci : ancs.characterSet.characters)
            {
                addTo[ci.cmdl] = std::make_pair(ci.cskr, ci.cinf);
                PAK::Entry* cmdlEnt = (PAK::Entry*)m_pak.lookupEntry(ci.cmdl);
                PAK::Entry* cskrEnt = (PAK::Entry*)m_pak.lookupEntry(ci.cskr);
                PAK::Entry* cinfEnt = (PAK::Entry*)m_pak.lookupEntry(ci.cinf);
                cmdlEnt->name = hecl::Format("ANCS_%08X_%s_model", entry.first.toUint32(), ci.name.c_str());
                cskrEnt->name = hecl::Format("ANCS_%08X_%s_skin", entry.first.toUint32(), ci.name.c_str());
                cinfEnt->name = hecl::Format("ANCS_%08X_%s_skel", entry.first.toUint32(), ci.name.c_str());
                if (ci.cmdlOverlay && ci.cskrOverlay)
                {
                    addTo[ci.cmdlOverlay] = std::make_pair(ci.cskrOverlay, ci.cinf);
                    PAK::Entry* cmdlEnt = (PAK::Entry*)m_pak.lookupEntry(ci.cmdlOverlay);
                    PAK::Entry* cskrEnt = (PAK::Entry*)m_pak.lookupEntry(ci.cskrOverlay);
                    cmdlEnt->name = hecl::Format("ANCS_%08X_%s_overmodel", entry.first.toUint32(), ci.name.c_str());
                    cskrEnt->name = hecl::Format("ANCS_%08X_%s_overskin", entry.first.toUint32(), ci.name.c_str());
                }
            }
            std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>> animInfo;
            ancs.getAnimationResInfo(animInfo);
            for (auto& ae : animInfo)
            {
                PAK::Entry* animEnt = (PAK::Entry*)m_pak.lookupEntry(ae.second.animId);
                animEnt->name = hecl::Format("ANCS_%08X_%s", entry.first.toUint32(), ae.second.name.c_str());
                if (ae.second.evntId)
                {
                    PAK::Entry* evntEnt = (PAK::Entry*)m_pak.lookupEntry(ae.second.evntId);
                    evntEnt->name = hecl::Format("ANCS_%08X_%s_evnt", entry.first.toUint32(), ae.second.name.c_str());
                }
            }
        }
        else if (entry.second->type == FOURCC('MREA'))
        {
            PAKEntryReadStream rs = entry.second->beginReadStream(m_node);
            MREA::AddCMDLRigPairs(rs, pakRouter, addTo);
        }
    }
}

ResExtractor<PAKBridge> PAKBridge::LookupExtractor(const PAK& pak, const PAK::Entry& entry)
{
    switch (entry.type)
    {
    case SBIG('STRG'):
        return {STRG::Extract, nullptr, {_S(".yaml")}};
    case SBIG('SCAN'):
        return {SCAN::Extract, nullptr, {_S(".yaml")}, 0, SCAN::Name};
    case SBIG('HINT'):
        return {HINT::Extract, nullptr, {_S(".yaml")}};
    case SBIG('SAVW'):
        return {SAVWCommon::ExtractSAVW<SAVW>, nullptr, {_S(".yaml")}};
    case SBIG('TXTR'):
        return {TXTR::Extract, nullptr, {_S(".png")}};
    case SBIG('AFSM'):
        return {AFSM::Extract, nullptr, {_S(".yaml")}};
    case SBIG('FRME'):
        return {nullptr, FRME::Extract, {_S(".blend")}, 2};
    case SBIG('CMDL'):
        return {nullptr, CMDL::Extract, {_S(".blend")}, 1, CMDL::Name};
    case SBIG('ANCS'):
        return {nullptr, ANCS::Extract, {_S(".yaml"), _S(".blend")}, 2};
    case SBIG('MLVL'):
        return {nullptr, MLVL::Extract, {_S(".blend")}, 3};
    case SBIG('MREA'):
        return {nullptr, MREA::Extract, {_S(".blend")}, 4, MREA::Name};
    case SBIG('MAPA'):
        return {nullptr, MAPA::Extract, {_S(".blend")}, 4};
    case SBIG('PART'):
        return {DNAParticle::ExtractGPSM<UniqueID32>, nullptr, {_S(".gpsm.yaml")}};
    case SBIG('ELSC'):
        return {DNAParticle::ExtractELSM<UniqueID32>, nullptr, {_S(".elsm.yaml")}};
    case SBIG('SWHC'):
        return {DNAParticle::ExtractSWSH<UniqueID32>, nullptr, {_S(".swsh.yaml")}};
    case SBIG('CRSC'):
        return {DNAParticle::ExtractCRSM<UniqueID32>, nullptr, {_S(".crsm.yaml")}};
    case SBIG('WPSC'):
        return {DNAParticle::ExtractWPSM<UniqueID32>, nullptr, {_S(".wpsm.yaml")}};
    case SBIG('DPSC'):
        return {DNAParticle::ExtractDPSM<UniqueID32>, nullptr, {_S(".dpsm.yaml")}};
    case SBIG('FONT'):
        return {DNAFont::ExtractFONT<UniqueID32>, nullptr, {_S(".yaml")}};
    case SBIG('DGRP'):
        return {DNADGRP::ExtractDGRP<UniqueID32>, nullptr, {_S(".yaml")}};
    case SBIG('CTWK'):
    {
        bool named;
        std::string name = pak.bestEntryName(entry, named);
        if (named)
        {
            if (!name.compare("PlayerRes"))
                return {ExtractTweak<CTweakPlayerRes>, nullptr, {_S(".yaml")}};
            if (!name.compare("GunRes"))
                return {ExtractTweak<CTweakGunRes>, nullptr, {_S(".yaml")}};
            if (!name.compare("Player"))
                return {ExtractTweak<CTweakPlayer>, nullptr, {_S(".yaml")}};
            if (!name.compare("CameraBob"))
                return {ExtractTweak<CTweakCameraBob>, nullptr, {_S(".yaml")}};
        }
        break;
    }
    }
    return {};
}

}
}
