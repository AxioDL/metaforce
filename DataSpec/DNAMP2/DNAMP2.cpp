#define NOD_ATHENA 1
#include "DNAMP2.hpp"
#include "STRG.hpp"
#include "MLVL.hpp"
#include "CMDL.hpp"
#include "ANCS.hpp"
#include "CINF.hpp"
#include "MREA.hpp"
#include "MAPA.hpp"
#include "MAPU.hpp"
#include "PATH.hpp"
#include "AFSM.hpp"
#include "SAVW.hpp"
#include "AGSC.hpp"
#include "../DNAMP1/HINT.hpp"
#include "../DNAMP1/CSNG.hpp"
#include "DataSpec/DNACommon/FSM2.hpp"
#include "DataSpec/DNACommon/TXTR.hpp"
#include "DataSpec/DNACommon/FONT.hpp"
#include "DataSpec/DNACommon/DGRP.hpp"
#include "DataSpec/DNACommon/ATBL.hpp"
#include "Runtime/GCNTypes.hpp"

namespace DataSpec::DNAMP2 {
logvisor::Module Log("DataSpec::DNAMP2");

static bool GetNoShare(std::string_view name) {
  std::string lowerName(name);
  std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), tolower);
  if (lowerName.compare(0, 7, "metroid") == 0)
    return false;
  if (lowerName.compare(0, 8, "frontend") == 0)
    return false;
  return true;
}

PAKBridge::PAKBridge(const nod::Node& node, bool doExtract)
: m_node(node), m_pak(true, GetNoShare(node.getName())), m_doExtract(doExtract) {
  nod::AthenaPartReadStream rs(node.beginReadStream());
  m_pak.read(rs);

  /* Append Level String */
  for (const auto& entry : m_pak.m_entries) {
    const DNAMP2::PAK::Entry& e = entry.second;
    if (e.type == FOURCC('MLVL')) {
      PAKEntryReadStream rs = e.beginReadStream(m_node);
      MLVL mlvl;
      mlvl.read(rs);
      const DNAMP2::PAK::Entry* nameEnt = m_pak.lookupEntry(mlvl.worldNameId);
      if (nameEnt) {
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
  for (const auto& entry : m_pak.m_entries) {
    const DNAMP2::PAK::Entry& e = entry.second;
    if (e.type == FOURCC('MLVL')) {
      Level& level = m_levelDeps[e.id];

      MLVL mlvl;
      {
        PAKEntryReadStream rs = e.beginReadStream(m_node);
        mlvl.read(rs);
      }
      std::string catalogueName;
      level.name = m_pak.bestEntryName(m_node, e, catalogueName);
      level.areas.reserve(mlvl.areaCount);
      unsigned layerIdx = 0;

      /* Make MAPW available to lookup MAPAs */
      const DNAMP2::PAK::Entry* worldMapEnt = m_pak.lookupEntry(mlvl.worldMap);
      std::vector<UniqueID32> mapw;
      if (worldMapEnt) {
        PAKEntryReadStream rs = worldMapEnt->beginReadStream(m_node);
        rs.seek(8, athena::SeekOrigin::Current);
        atUint32 areaCount = rs.readUint32Big();
        mapw.reserve(areaCount);
        for (atUint32 i = 0; i < areaCount; ++i)
          mapw.emplace_back(rs);
      }

      /* Index areas */
      unsigned ai = 0;
      for (const MLVL::Area& area : mlvl.areas) {
        Level::Area& areaDeps = level.areas[area.areaMREAId];
        MLVL::LayerFlags& layerFlags = mlvl.layerFlags[ai];
        const DNAMP2::PAK::Entry* areaNameEnt = m_pak.lookupEntry(area.areaNameId);
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
          areaDeps.name = area.internalAreaName;
          if (areaDeps.name.empty()) {
            areaDeps.name = "MREA_" + area.areaMREAId.toString();
          }
        }
        std::string num = fmt::format(FMT_STRING("{:02d} "), ai);
        areaDeps.name = num + areaDeps.name;

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
  for (auto& entry : m_pak.m_entries) {
    entry.second.unique.checkEntry(*this, entry.second);
  }
}

void PAKBridge::addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const {
  for (const auto& entry : m_pak.m_entries) {
    if (entry.second.type == FOURCC('ANCS')) {
      PAKEntryReadStream rs = entry.second.beginReadStream(m_node);
      ANCS ancs;
      ancs.read(rs);
      for (const ANCS::CharacterSet::CharacterInfo& ci : ancs.characterSet.characters) {
        charAssoc.m_cmdlRigs[ci.cmdl] = {ci.cskr, ci.cinf};
        charAssoc.m_cskrToCharacter[ci.cskr] =
            std::make_pair(entry.second.id, fmt::format(FMT_STRING("{}_{}.CSKR"), ci.name, ci.cskr));
        if (ci.cmdlIce.isValid()) {
          charAssoc.m_cmdlRigs[ci.cmdlIce] = {ci.cskrIce, ci.cinf};
          charAssoc.m_cskrToCharacter[ci.cskrIce] =
              std::make_pair(entry.second.id, fmt::format(FMT_STRING("{}.ICE_{}.CSKR"), ci.name, ci.cskrIce));
        }
      }
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
  for (const auto& entry : m_pak.m_entries) {
    if (entry.second.type == FOURCC('MLVL')) {
      MLVL mlvl;
      {
        PAKEntryReadStream rs = entry.second.beginReadStream(m_node);
        mlvl.read(rs);
      }
      hecl::ProjectPath mlvlDirPath = pakRouter.getWorking(&entry.second).getParentPath();

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
        const DNAMP2::PAK::Entry* mapEntry = pakRouter.lookupEntry(mlvl.worldMap, &mapNode);
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

ResExtractor<PAKBridge> PAKBridge::LookupExtractor(const nod::Node& pakNode, const DNAMP2::PAK& pak,
                                                   const DNAMP2::PAK::Entry& entry) {
  switch (entry.type.toUint32()) {
  case SBIG('HINT'):
    return {DNAMP1::HINT::Extract, {".yaml"}};
  case SBIG('STRG'):
    return {STRG::Extract, {".yaml"}};
  case SBIG('TXTR'):
    return {TXTR::Extract, {".png"}};
  case SBIG('AFSM'):
    return {AFSM::Extract, {".yaml"}};
  case SBIG('SAVW'):
    return {SAVWCommon::ExtractSAVW<SAVW>, {".yaml"}};
  case SBIG('CMDL'):
    return {CMDL::Extract, {".blend"}, 1};
  case SBIG('CINF'):
    return {CINF::Extract<PAKBridge>, {".blend"}, 1};
  case SBIG('ANCS'):
    return {ANCS::Extract, {".yaml", ".blend"}, 2};
  case SBIG('MLVL'):
    return {MLVL::Extract, {".yaml", ".blend"}, 3};
  case SBIG('MREA'):
    return {MREA::Extract, {".blend"}, 4};
  case SBIG('MAPA'):
    return {MAPA::Extract, {".blend"}, 4};
  case SBIG('MAPU'):
    return {MAPU::Extract, {".blend"}, 5};
  case SBIG('PATH'):
    return {PATH::Extract, {".blend"}, 5};
  case SBIG('FSM2'):
    return {DNAFSM2::ExtractFSM2<UniqueID32>, {".yaml"}};
  case SBIG('FONT'):
    return {DNAFont::ExtractFONT<UniqueID32>, {".yaml"}};
  case SBIG('DGRP'):
    return {DNADGRP::ExtractDGRP<UniqueID32>, {".yaml"}};
  case SBIG('AGSC'):
    return {AGSC::Extract, {}};
  case SBIG('CSNG'):
    return {DNAMP1::CSNG::Extract, {".mid", ".yaml"}};
  case SBIG('ATBL'):
    return {DNAAudio::ATBL::Extract, {".yaml"}};
  }
  return {};
}

} // namespace DataSpec::DNAMP2
