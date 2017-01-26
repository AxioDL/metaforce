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
#include "../DNACommon/ATBL.hpp"
#include "HINT.hpp"
#include "CMDL.hpp"
#include "AFSM.hpp"
#include "SAVW.hpp"
#include "ANCS.hpp"
#include "MREA.hpp"
#include "MAPA.hpp"
#include "FRME.hpp"
#include "AGSC.hpp"
#include "CSNG.hpp"

#include "../DNACommon/Tweaks/TweakWriter.hpp"
#include "Tweaks/CTweakPlayerRes.hpp"
#include "Tweaks/CTweakGunRes.hpp"
#include "Tweaks/CTweakPlayer.hpp"
#include "Tweaks/CTweakCameraBob.hpp"
#include "Tweaks/CTweakSlideShow.hpp"
#include "Tweaks/CTweakGame.hpp"
#include "Tweaks/CTweakTargeting.hpp"
#include "Tweaks/CTweakAutoMapper.hpp"
#include "Tweaks/CTweakGui.hpp"
#include "Tweaks/CTweakPlayerControl.hpp"
#include "Tweaks/CTweakBall.hpp"
#include "Tweaks/CTweakParticle.hpp"
#include "Tweaks/CTweakGuiColors.hpp"

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
                    areaDeps.name = hecl::StringUtils::TrimWhitespace(areaDeps.name);
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
    for (PAK::Entry& entry : m_pak.m_entries)
    {
        entry.unique.checkEntry(*this, entry);
    }
}

void PAKBridge::addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
        std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo,
        std::unordered_map<UniqueID32, std::pair<UniqueID32, std::string>>& cskrCinfToAncs) const
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
                cskrCinfToAncs[ci.cskr] = std::make_pair(entry.second->id, hecl::Format("%s.CSKR", ci.name.c_str()));
                cskrCinfToAncs[ci.cinf] = std::make_pair(entry.second->id, hecl::Format("CINF_%08X.CINF", ci.cinf.toUint32()));
                PAK::Entry* cmdlEnt = (PAK::Entry*)m_pak.lookupEntry(ci.cmdl);
                PAK::Entry* cskrEnt = (PAK::Entry*)m_pak.lookupEntry(ci.cskr);
                PAK::Entry* cinfEnt = (PAK::Entry*)m_pak.lookupEntry(ci.cinf);
                cmdlEnt->name = hecl::Format("ANCS_%08X_%s_model", entry.first.toUint32(), ci.name.c_str());
                cskrEnt->name = hecl::Format("ANCS_%08X_%s_skin", entry.first.toUint32(), ci.name.c_str());
                cinfEnt->name = hecl::Format("ANCS_%08X_%s_skel", entry.first.toUint32(), ci.name.c_str());
                if (ci.cmdlOverlay && ci.cskrOverlay)
                {
                    addTo[ci.cmdlOverlay] = std::make_pair(ci.cskrOverlay, ci.cinf);
                    cskrCinfToAncs[ci.cskrOverlay] = std::make_pair(entry.second->id, hecl::Format("%s.over.CSKR", ci.name.c_str()));
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
                cskrCinfToAncs[ae.second.animId] = std::make_pair(entry.second->id, hecl::Format("%s.ANIM", ae.second.name.c_str()));
                if (ae.second.evntId)
                {
                    PAK::Entry* evntEnt = (PAK::Entry*)m_pak.lookupEntry(ae.second.evntId);
                    evntEnt->name = hecl::Format("ANCS_%08X_%s_evnt", entry.first.toUint32(), ae.second.name.c_str());
                    cskrCinfToAncs[ae.second.evntId] = std::make_pair(entry.second->id, hecl::Format("%s.evnt.yaml", ae.second.name.c_str()));
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

static const atVec4f BottomRow = {0.f, 0.f, 0.f, 1.f};

void PAKBridge::addMAPATransforms(PAKRouter<PAKBridge>& pakRouter,
        std::unordered_map<UniqueID32, zeus::CMatrix4f>& addTo,
        std::unordered_map<UniqueID32, hecl::ProjectPath>& pathOverrides) const
{
    for (const std::pair<UniqueID32, PAK::Entry*>& entry : m_pak.m_idMap)
    {
        if (entry.second->type == FOURCC('MLVL'))
        {
            MLVL mlvl;
            {
                PAKEntryReadStream rs = entry.second->beginReadStream(m_node);
                mlvl.read(rs);
            }
            hecl::ProjectPath mlvlDirPath = pakRouter.getWorking(entry.second).getParentPath();

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

ResExtractor<PAKBridge> PAKBridge::LookupExtractor(const PAK& pak, const PAK::Entry& entry)
{
    switch (entry.type)
    {
    case SBIG('STRG'):
        return {STRG::Extract, {_S(".yaml")}};
    case SBIG('SCAN'):
        return {SCAN::Extract, {_S(".yaml")}, 0, SCAN::Name};
    case SBIG('HINT'):
        return {HINT::Extract, {_S(".yaml")}};
    case SBIG('SAVW'):
        return {SAVWCommon::ExtractSAVW<SAVW>, {_S(".yaml")}};
    case SBIG('TXTR'):
        return {TXTR::Extract, {_S(".png")}};
    case SBIG('AFSM'):
        return {AFSM::Extract, {_S(".yaml")}};
    case SBIG('FRME'):
        return {FRME::Extract, {_S(".blend")}, 2};
    case SBIG('CMDL'):
        return {CMDL::Extract, {_S(".blend")}, 1, CMDL::Name};
    case SBIG('ANCS'):
        return {ANCS::Extract, {_S(".yaml"), _S(".blend")}, 2};
    case SBIG('MLVL'):
        return {MLVL::Extract, {_S(".yaml"), _S(".blend")}, 3};
    case SBIG('MREA'):
        return {MREA::Extract, {_S(".blend")}, 4, MREA::Name};
    case SBIG('MAPA'):
        return {MAPA::Extract, {_S(".blend")}, 4};
    case SBIG('PART'):
        return {DNAParticle::ExtractGPSM<UniqueID32>, {_S(".gpsm.yaml")}};
    case SBIG('ELSC'):
        return {DNAParticle::ExtractELSM<UniqueID32>, {_S(".elsm.yaml")}};
    case SBIG('SWHC'):
        return {DNAParticle::ExtractSWSH<UniqueID32>, {_S(".swsh.yaml")}};
    case SBIG('CRSC'):
        return {DNAParticle::ExtractCRSM<UniqueID32>, {_S(".crsm.yaml")}};
    case SBIG('WPSC'):
        return {DNAParticle::ExtractWPSM<UniqueID32>, {_S(".wpsm.yaml")}};
    case SBIG('DPSC'):
        return {DNAParticle::ExtractDPSM<UniqueID32>, {_S(".dpsm.yaml")}};
    case SBIG('FONT'):
        return {DNAFont::ExtractFONT<UniqueID32>, {_S(".yaml")}};
    case SBIG('DGRP'):
        return {DNADGRP::ExtractDGRP<UniqueID32>, {_S(".yaml")}};
    case SBIG('AGSC'):
        return {AGSC::Extract, {_S(".pool"), _S(".proj"), _S(".samp"), _S(".sdir")}};
    case SBIG('CSNG'):
        return {CSNG::Extract, {_S(".mid"), _S(".yaml")}};
    case SBIG('ATBL'):
        return {DNAAudio::ATBL::Extract, {_S(".yaml")}};
    case SBIG('CTWK'):
    {
        bool named;
        std::string name = pak.bestEntryName(entry, named);
        if (named)
        {
            if (!name.compare("PlayerRes"))
                return {ExtractTweak<CTweakPlayerRes>, {_S(".yaml")}};
            if (!name.compare("GunRes"))
                return {ExtractTweak<CTweakGunRes>, {_S(".yaml")}};
            if (!name.compare("Player"))
                return {ExtractTweak<CTweakPlayer>, {_S(".yaml")}};
            if (!name.compare("CameraBob"))
                return {ExtractTweak<CTweakCameraBob>, {_S(".yaml")}};
            if (!name.compare("SlideShow"))
                return {ExtractTweak<CTweakSlideShow>, {_S(".yaml")}};
            if (!name.compare("Game"))
                return {ExtractTweak<CTweakGame>, {_S(".yaml")}};
            if (!name.compare("Targeting"))
                return {ExtractTweak<CTweakTargeting>, {_S(".yaml")}};
            if (!name.compare("Gui"))
                return {ExtractTweak<CTweakGui>, {_S(".yaml")}};
            if (!name.compare("AutoMapper"))
                return {ExtractTweak<CTweakAutoMapper>, {_S(".yaml")}};
            if (!name.compare("PlayerControls") || !name.compare("PlayerControls2"))
                return {ExtractTweak<CTweakPlayerControl>, {_S(".yaml")}};
            if (!name.compare("Ball"))
                return {ExtractTweak<CTweakBall>, {_S(".yaml")}};
            if (!name.compare("Particle"))
                return {ExtractTweak<CTweakParticle>, {_S(".yaml")}};
            if (!name.compare("GuiColors"))
                return {ExtractTweak<CTweakGuiColors>, {_S(".yaml")}};
        }
        break;
    }
    }
    return {};
}

}
}
