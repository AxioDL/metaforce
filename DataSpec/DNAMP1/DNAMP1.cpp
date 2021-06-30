#include <cstdio>

#define NOD_ATHENA 1
#include "DNAMP1.hpp"
#include "STRG.hpp"
#include "SCAN.hpp"
#include "MLVL.hpp"
#include "DataSpec/DNACommon/TXTR.hpp"
#include "DataSpec/DNACommon/PART.hpp"
#include "DataSpec/DNACommon/ELSC.hpp"
#include "DataSpec/DNACommon/SWHC.hpp"
#include "DataSpec/DNACommon/CRSC.hpp"
#include "DataSpec/DNACommon/WPSC.hpp"
#include "DataSpec/DNACommon/DPSC.hpp"
#include "DataSpec/DNACommon/FONT.hpp"
#include "DataSpec/DNACommon/DGRP.hpp"
#include "DataSpec/DNACommon/ATBL.hpp"
#include "HINT.hpp"
#include "CMDL.hpp"
#include "AFSM.hpp"
#include "SAVW.hpp"
#include "ANCS.hpp"
#include "MREA.hpp"
#include "MAPA.hpp"
#include "MAPU.hpp"
#include "FRME.hpp"
#include "AGSC.hpp"
#include "CSNG.hpp"
#include "DCLN.hpp"
#include "PATH.hpp"

#include "DataSpec/DNACommon/Tweaks/TweakWriter.hpp"
#include "DataSpec/DNACommon/MetaforceVersionInfo.hpp"
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
#include "Tweaks/CTweakPlayerGun.hpp"
#include "MazeSeeds.hpp"
#include "SnowForces.hpp"

namespace DataSpec::DNAMP1 {
logvisor::Module Log("DataSpec::DNAMP1");

static bool GetNoShare(std::string_view name) {
  std::string lowerName(name);
  std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), tolower);
  if (!lowerName.compare(0, 7, "metroid"))
    return false;
  return true;
}

PAKBridge::PAKBridge(const nod::Node& node, bool doExtract)
: m_node(node), m_pak(false, GetNoShare(node.getName())), m_doExtract(doExtract) {
  nod::AthenaPartReadStream rs(node.beginReadStream());
  m_pak.read(rs);

  /* Append Level String */
  for (auto& [id, entry] : m_pak.m_entries) {
    if (entry.type == FOURCC('MLVL')) {
      m_levelId = entry.id;
      PAKEntryReadStream rs = entry.beginReadStream(m_node);
      MLVL mlvl;
      mlvl.read(rs);
      PAK::Entry* nameEnt = (PAK::Entry*)m_pak.lookupEntry(mlvl.worldNameId);
      if (nameEnt) {
        nameEnt->name = entry.name + "_name";
        PAKEntryReadStream rs = nameEnt->beginReadStream(m_node);
        STRG mlvlName;
        mlvlName.read(rs);
        if (m_levelString.size())
          m_levelString += ", ";
        m_levelString += mlvlName.getUTF8(FOURCC('ENGL'), 0);
      }
    }
  }
}

static std::string LayerName(std::string_view name) {
  std::string ret(name);
  for (auto& ch : ret)
    if (ch == '/' || ch == '\\')
      ch = '-';
  return ret;
}

void PAKBridge::build() {
  /* First pass: build per-area/per-layer dependency map */
  for (const auto& [id, entry] : m_pak.m_entries) {
    if (entry.type == FOURCC('MLVL')) {
      Level& level = m_levelDeps[entry.id];

      MLVL mlvl;
      {
        PAKEntryReadStream rs = entry.beginReadStream(m_node);
        mlvl.read(rs);
      }
      std::string catalogueName;
      std::string bestName = m_pak.bestEntryName(m_node, entry, catalogueName);
      level.name = bestName;
      level.areas.reserve(mlvl.areaCount);
      unsigned layerIdx = 0;

      /* Make MAPW available to lookup MAPAs */
      PAK::Entry* worldMapEnt = (PAK::Entry*)m_pak.lookupEntry(mlvl.worldMap);
      std::vector<UniqueID32> mapw;
      if (worldMapEnt) {
        worldMapEnt->name = entry.name + "_mapw";
        PAKEntryReadStream rs = worldMapEnt->beginReadStream(m_node);
        rs.seek(8, athena::SeekOrigin::Current);
        atUint32 areaCount = rs.readUint32Big();
        mapw.reserve(areaCount);
        for (atUint32 i = 0; i < areaCount; ++i)
          mapw.emplace_back(rs);
        level.resources.insert(mlvl.worldMap);
      }

      PAK::Entry* savwEnt = (PAK::Entry*)m_pak.lookupEntry(mlvl.saveWorldId);
      if (savwEnt) {
        savwEnt->name = entry.name + "_savw";
        level.resources.insert(mlvl.saveWorldId);
      }

      PAK::Entry* skyEnt = (PAK::Entry*)m_pak.lookupEntry(mlvl.worldSkyboxId);
      if (skyEnt) {
        skyEnt->name = entry.name + "_skybox";
        level.resources.insert(mlvl.worldSkyboxId);
      }

      /* Index areas */
      unsigned ai = 0;
      for (const MLVL::Area& area : mlvl.areas) {
        Level::Area& areaDeps = level.areas[area.areaMREAId];
        MLVL::LayerFlags& layerFlags = mlvl.layerFlags[ai];
        PAK::Entry* areaNameEnt = (PAK::Entry*)m_pak.lookupEntry(area.areaNameId);
        if (areaNameEnt) {
          STRG areaName;
          {
            PAKEntryReadStream rs = areaNameEnt->beginReadStream(m_node);
            areaName.read(rs);
          }
          areaDeps.name = areaName.getUTF8(FOURCC('ENGL'), 0);
          areaDeps.name = hecl::StringUtils::TrimWhitespace(areaDeps.name);
        }
        if (areaDeps.name.empty()) {
          std::string idStr = area.areaMREAId.toString();
          areaDeps.name = std::string("MREA_") + idStr;
        }
        std::string num = fmt::format(FMT_STRING("{:02d} "), ai);
        areaDeps.name = num + areaDeps.name;

        std::string lowerName(areaDeps.name);
        for (char& ch : lowerName) {
          ch = tolower(ch);
          if (ch == ' ')
            ch = '_';
        }
        if (areaNameEnt)
          areaNameEnt->name = lowerName + "_name";
        PAK::Entry* areaEnt = (PAK::Entry*)m_pak.lookupEntry(area.areaMREAId);
        if (areaEnt)
          areaEnt->name = lowerName;

        areaDeps.layers.reserve(area.depLayerCount - 1);
        unsigned r = 0;
        for (unsigned l = 1; l < area.depLayerCount; ++l) {
          areaDeps.layers.emplace_back();
          Level::Area::Layer& layer = areaDeps.layers.back();
          layer.name = LayerName(mlvl.layerNames[layerIdx++]);
          layer.active = layerFlags.flags >> (l - 1) & 0x1;
          layer.name = hecl::StringUtils::TrimWhitespace(layer.name);

          num = fmt::format(FMT_STRING("{:02d} "), l - 1);
          layer.name = num + layer.name;

          layer.resources.reserve(area.depLayers[l] - r);
          for (; r < area.depLayers[l]; ++r)
            layer.resources.emplace(area.deps[r].id);
        }
        areaDeps.resources.reserve(area.depCount - r + 2);
        for (; r < area.depCount; ++r)
          areaDeps.resources.emplace(area.deps[r].id);
        areaDeps.resources.emplace(area.areaMREAId);
        if (mapw.size() > ai)
          areaDeps.resources.emplace(mapw[ai]);
        ++ai;
      }
    }
  }

  /* Second pass: cross-compare uniqueness */
  for (auto& [id, entry] : m_pak.m_entries)
    entry.unique.checkEntry(*this, entry);
}

void PAKBridge::addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const {
  for (const auto& [id, entry] : m_pak.m_entries) {
    if (entry.type == FOURCC('ANCS')) {
      PAKEntryReadStream rs = entry.beginReadStream(m_node);
      ANCS ancs;
      ancs.read(rs);
      for (const ANCS::CharacterSet::CharacterInfo& ci : ancs.characterSet.characters) {
        charAssoc.m_cmdlRigs[ci.cmdl] = {ci.cskr, ci.cinf};
        charAssoc.m_cskrToCharacter[ci.cskr] =
            std::make_pair(entry.id, fmt::format(FMT_STRING("{}_{}.CSKR"), ci.name, ci.cskr));
        PAK::Entry* cmdlEnt = (PAK::Entry*)m_pak.lookupEntry(ci.cmdl);
        PAK::Entry* cskrEnt = (PAK::Entry*)m_pak.lookupEntry(ci.cskr);
        PAK::Entry* cinfEnt = (PAK::Entry*)m_pak.lookupEntry(ci.cinf);
        cmdlEnt->name = fmt::format(FMT_STRING("ANCS_{}_{}_model"), id, ci.name);
        cskrEnt->name = fmt::format(FMT_STRING("ANCS_{}_{}_skin"), id, ci.name);
        cinfEnt->name = fmt::format(FMT_STRING("ANCS_{}_{}_skel"), id, ci.name);
        if (ci.cmdlIce.isValid() && ci.cskrIce.isValid()) {
          charAssoc.m_cmdlRigs[ci.cmdlIce] = {ci.cskrIce, ci.cinf};
          charAssoc.m_cskrToCharacter[ci.cskrIce] =
              std::make_pair(entry.id, fmt::format(FMT_STRING("{}.ICE_{}.CSKR"), ci.name, ci.cskrIce));
          PAK::Entry* cmdlEnt = (PAK::Entry*)m_pak.lookupEntry(ci.cmdlIce);
          PAK::Entry* cskrEnt = (PAK::Entry*)m_pak.lookupEntry(ci.cskrIce);
          cmdlEnt->name = fmt::format(FMT_STRING("ANCS_{}_{}_icemodel"), id, ci.name);
          cskrEnt->name = fmt::format(FMT_STRING("ANCS_{}_{}_iceskin"), id, ci.name);
        }
      }
      std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>> animInfo;
      ancs.getAnimationResInfo(&pakRouter, animInfo);
      for (auto& [animIdx, animResInfo] : animInfo) {
        PAK::Entry* animEnt = (PAK::Entry*)m_pak.lookupEntry(animResInfo.animId);
        animEnt->name = fmt::format(FMT_STRING("ANCS_{}_{}"), id, animResInfo.name);
        charAssoc.m_cskrToCharacter[animResInfo.animId] =
            std::make_pair(entry.id, fmt::format(FMT_STRING("{}_{}.ANIM"), animResInfo.name, animResInfo.animId));
        if (animResInfo.evntId.isValid()) {
          PAK::Entry* evntEnt = (PAK::Entry*)m_pak.lookupEntry(animResInfo.evntId);
          evntEnt->name = fmt::format(FMT_STRING("ANCS_{}_{}_evnt"), id, animResInfo.name);
          charAssoc.m_cskrToCharacter[animResInfo.evntId] = std::make_pair(
              entry.id, fmt::format(FMT_STRING("{}_{}.evnt.yaml"), animResInfo.name, animResInfo.evntId));
        }
      }
    } else if (entry.type == FOURCC('MREA')) {
      PAKEntryReadStream rs = entry.beginReadStream(m_node);
      MREA::AddCMDLRigPairs(rs, pakRouter, charAssoc);
    }
  }
}

void PAKBridge::addPATHToMREA(PAKRouter<PAKBridge>& pakRouter,
                              std::unordered_map<UniqueID32, UniqueID32>& pathToMrea) const {
  for (const auto& [id, entry] : m_pak.m_entries) {
    if (entry.type == FOURCC('MREA')) {
      PAKEntryReadStream rs = entry.beginReadStream(m_node);
      UniqueID32 pathID = MREA::GetPATHId(rs);
      if (pathID.isValid())
        pathToMrea[pathID] = id;
    }
  }
}

static const atVec4f BottomRow = {{0.f, 0.f, 0.f, 1.f}};

void PAKBridge::addMAPATransforms(PAKRouter<PAKBridge>& pakRouter,
                                  std::unordered_map<UniqueID32, zeus::CMatrix4f>& addTo,
                                  std::unordered_map<UniqueID32, hecl::ProjectPath>& pathOverrides) const {
  for (const auto& [id, entry] : m_pak.m_entries) {
    if (entry.type == FOURCC('MLVL')) {
      MLVL mlvl;
      {
        PAKEntryReadStream rs = entry.beginReadStream(m_node);
        mlvl.read(rs);
      }
      hecl::ProjectPath mlvlDirPath = pakRouter.getWorking(&entry).getParentPath();

      if (mlvl.worldNameId.isValid())
        pathOverrides[mlvl.worldNameId] =
            hecl::ProjectPath(mlvlDirPath, fmt::format(FMT_STRING("!name_{}.yaml"), mlvl.worldNameId));

      for (const MLVL::Area& area : mlvl.areas) {
        {
          /* Get PATH transform */
          const nod::Node* areaNode;
          const PAK::Entry* areaEntry = pakRouter.lookupEntry(area.areaMREAId, &areaNode);
          PAKEntryReadStream rs = areaEntry->beginReadStream(*areaNode);
          UniqueID32 pathId = MREA::GetPATHId(rs);
          if (pathId.isValid())
            addTo[pathId] = zeus::CMatrix4f(area.transformMtx[0], area.transformMtx[1], area.transformMtx[2], BottomRow)
                                .transposed();
        }

        hecl::ProjectPath areaDirPath = pakRouter.getWorking(area.areaMREAId).getParentPath();
        if (area.areaNameId.isValid())
          pathOverrides[area.areaNameId] =
              hecl::ProjectPath(areaDirPath, fmt::format(FMT_STRING("!name_{}.yaml"), area.areaNameId));
      }

      if (mlvl.worldMap.isValid()) {
        const nod::Node* mapNode;
        const PAK::Entry* mapEntry = pakRouter.lookupEntry(mlvl.worldMap, &mapNode);
        if (mapEntry) {
          PAKEntryReadStream rs = mapEntry->beginReadStream(*mapNode);
          u32 magic = rs.readUint32Big();
          if (magic == 0xDEADF00D) {
            rs.readUint32Big();
            u32 count = rs.readUint32Big();
            for (u32 i = 0; i < count && i < mlvl.areas.size(); ++i) {
              MLVL::Area& areaData = mlvl.areas[i];
              UniqueID32 mapaId;
              mapaId.read(rs);
              addTo[mapaId] = zeus::CMatrix4f(areaData.transformMtx[0], areaData.transformMtx[1],
                                              areaData.transformMtx[2], BottomRow)
                                  .transposed();
            }
          }
        }
      }
    }
  }
}

ResExtractor<PAKBridge> PAKBridge::LookupExtractor(const nod::Node& pakNode, const PAK& pak, const PAK::Entry& entry) {
  switch (entry.type.toUint32()) {
  case SBIG('STRG'):
    return {STRG::Extract, {".yaml"}};
  case SBIG('SCAN'):
    return {SCAN::Extract, {".yaml"}, 0, SCAN::Name};
  case SBIG('HINT'):
    return {HINT::Extract, {".yaml"}};
  case SBIG('TXTR'):
    return {TXTR::Extract, {".png"}};
  case SBIG('AFSM'):
    return {AFSM::Extract, {".yaml"}};
  case SBIG('FRME'):
    return {FRME::Extract, {".blend"}, 2};
  case SBIG('CINF'):
    return {CINF::Extract, {".blend"}, 1};
  case SBIG('CMDL'):
    return {CMDL::Extract, {".blend"}, 1, CMDL::Name};
  case SBIG('DCLN'):
    return {DCLN::Extract, {".blend"}};
  case SBIG('ANCS'):
    return {ANCS::Extract, {".yaml", ".blend"}, 2};
  case SBIG('MLVL'):
    return {MLVL::Extract, {".yaml", ".blend"}, 3};
  case SBIG('SAVW'):
    return {MLVL::ExtractSAVW, {".yaml"}, 3};
  case SBIG('MREA'):
    return {MREA::Extract, {".blend"}, 4, MREA::Name};
  case SBIG('MAPA'):
    return {MAPA::Extract, {".blend"}, 4};
  case SBIG('MAPW'):
    return {MLVL::ExtractMAPW, {".yaml"}, 4};
  case SBIG('MAPU'):
    return {MAPU::Extract, {".blend"}, 5};
  case SBIG('PATH'):
    return {PATH::Extract, {".blend"}, 5};
  case SBIG('PART'):
    return {DNAParticle::ExtractGPSM<UniqueID32>, {".gpsm.yaml"}};
  case SBIG('ELSC'):
    return {DNAParticle::ExtractELSM<UniqueID32>, {".elsm.yaml"}};
  case SBIG('SWHC'):
    return {DNAParticle::ExtractSWSH<UniqueID32>, {".swsh.yaml"}};
  case SBIG('CRSC'):
    return {DNAParticle::ExtractCRSM<UniqueID32>, {".crsm.yaml"}};
  case SBIG('WPSC'):
    return {DNAParticle::ExtractWPSM<UniqueID32>, {".wpsm.yaml"}};
  case SBIG('DPSC'):
    return {DNAParticle::ExtractDPSM<UniqueID32>, {".dpsm.yaml"}};
  case SBIG('FONT'):
    return {DNAFont::ExtractFONT<UniqueID32>, {".yaml"}};
  case SBIG('DGRP'):
    return {DNADGRP::ExtractDGRP<UniqueID32>, {".yaml"}};
  case SBIG('AGSC'):
    return {AGSC::Extract, {}};
  case SBIG('CSNG'):
    return {CSNG::Extract, {".mid", ".yaml"}};
  case SBIG('ATBL'):
    return {DNAAudio::ATBL::Extract, {".yaml"}};
  case SBIG('CTWK'):
  case SBIG('DUMB'): {
    std::string catalogueName;
    std::string name = pak.bestEntryName(pakNode, entry, catalogueName);
    if (!catalogueName.empty()) {
      if (catalogueName == "PlayerRes"sv) {
        if (isCurrentSpecWii() || getCurrentRegion() == ERegion::PAL || getCurrentRegion() == ERegion::NTSC_J) {
          /* We need to use the new rep for these tweaks */
          return {ExtractTweak<CTweakPlayerRes<true>>, {".yaml"}};
        }
        /* We need to use the old rep for these tweaks */
        return {ExtractTweak<CTweakPlayerRes<false>>, {".yaml"}};
      }
      if (catalogueName == "GunRes"sv)
        return {ExtractTweak<CTweakGunRes>, {".yaml"}};
      if (catalogueName == "Player"sv)
        return {ExtractTweak<CTweakPlayer>, {".yaml"}};
      if (catalogueName == "CameraBob"sv)
        return {ExtractTweak<CTweakCameraBob>, {".yaml"}};
      if (catalogueName == "SlideShow"sv)
        return {ExtractTweak<CTweakSlideShow>, {".yaml"}};
      if (catalogueName == "Game"sv)
        return {ExtractTweak<CTweakGame>, {".yaml"}};
      if (catalogueName == "Targeting"sv) {
        if (isCurrentSpecWii() || getCurrentRegion() == ERegion::PAL || getCurrentRegion() == ERegion::NTSC_J) {
          /* We need to use the new rep for these tweaks */
          return {ExtractTweak<CTweakTargeting<true>>, {".yaml"}};
        }
        /* We need to use the old rep for these tweaks */
        return {ExtractTweak<CTweakTargeting<false>>, {".yaml"}};
      }
      if (catalogueName == "Gui"sv)
        return {ExtractTweak<CTweakGui>, {".yaml"}};
      if (catalogueName == "AutoMapper"sv)
        return {ExtractTweak<CTweakAutoMapper>, {".yaml"}};
      if (catalogueName == "PlayerControls"sv || catalogueName == "PlayerControls2"sv)
        return {ExtractTweak<CTweakPlayerControl>, {".yaml"}};
      if (catalogueName == "Ball"sv)
        return {ExtractTweak<CTweakBall>, {".yaml"}};
      if (catalogueName == "Particle"sv)
        return {ExtractTweak<CTweakParticle>, {".yaml"}};
      if (catalogueName == "GuiColors"sv)
        return {ExtractTweak<CTweakGuiColors>, {".yaml"}};
      if (catalogueName == "PlayerGun"sv)
        return {ExtractTweak<CTweakPlayerGun>, {".yaml"}};
      if (catalogueName == "DUMB_MazeSeeds"sv)
        return {ExtractTweak<MazeSeeds>, {".yaml"}};
      if (catalogueName == "DUMB_SnowForces"sv)
        return {ExtractTweak<SnowForces>, {".yaml"}};
    }
    break;
  }
  }
  return {};
}

} // namespace DataSpec::DNAMP1
