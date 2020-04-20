#if _WIN32
#define _CRT_RAND_S
#include <cstdlib>
#endif

#include "DataSpec/SpecBase.hpp"
#include "DataSpec/Blender/BlenderSupport.hpp"
#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DataSpec/DNACommon/TXTR.hpp"
#include "DataSpec/AssetNameMap.hpp"
#include "DataSpec/DNACommon/URDEVersionInfo.hpp"
#include "hecl/ClientProcess.hpp"
#include "nod/DiscBase.hpp"
#include "nod/nod.hpp"
#include "hecl/Blender/Connection.hpp"
#include "hecl/Blender/SDNARead.hpp"
#include "hecl/MultiProgressPrinter.hpp"

#include <png.h>

#define DUMP_CACHE_FILL 1

namespace DataSpec {

static logvisor::Module Log("urde::SpecBase");

static const hecl::SystemChar* MomErr[] = {_SYS_STR("Your metroid is in another castle"),
                                           _SYS_STR("HECL is experiencing a PTSD attack"),
                                           _SYS_STR("Unable to freeze metroids"),
                                           _SYS_STR("Ridley ate your homework"),
                                           _SYS_STR("Expected 0 maternal symbolisms, found 2147483647"),
                                           _SYS_STR("Contradictive narratives unsupported"),
                                           _SYS_STR("Wiimote profile \"NES + Zapper\" not recognized"),
                                           _SYS_STR("Unable to find Waldo"),
                                           _SYS_STR("Expected Ridley, found furby"),
                                           _SYS_STR("Adam has not authorized this, please do not bug the developers"),
                                           _SYS_STR("Lady returned objection"),
                                           _SYS_STR("Unterminated plot thread 'Deleter' detected")};

constexpr uint32_t MomErrCount = std::extent<decltype(MomErr)>::value;

static ERegion g_CurRegion = ERegion::Invalid;
static bool g_CurSpecIsWii = false;

ERegion getCurrentRegion() { return g_CurRegion; }
bool isCurrentSpecWii() { return g_CurSpecIsWii; }

SpecBase::SpecBase(const hecl::Database::DataSpecEntry* specEntry, hecl::Database::Project& project, bool pc)
: hecl::Database::IDataSpec(specEntry)
, m_project(project)
, m_pc(pc)
, m_masterShader(project.getProjectWorkingPath(), ".hecl/RetroMasterShader.blend")
, m_region(ERegion::Invalid)
, m_game(EGame::Invalid) {
  AssetNameMap::InitAssetNameMap();
  SpecBase::setThreadProject();
}

SpecBase::~SpecBase() { cancelBackgroundIndex(); }

static const hecl::SystemString regNONE = _SYS_STR("");
static const hecl::SystemString regE = _SYS_STR("NTSC");
static const hecl::SystemString regJ = _SYS_STR("NTSC-J");
static const hecl::SystemString regP = _SYS_STR("PAL");

void SpecBase::setThreadProject() { UniqueIDBridge::SetThreadProject(m_project); }

bool SpecBase::canExtract(const ExtractPassInfo& info, std::vector<ExtractReport>& reps) {
  m_disc = nod::OpenDiscFromImage(info.srcpath, m_isWii);
  if (!m_disc)
    return false;
  const char* gameID = m_disc->getHeader().m_gameID;

  if (!memcmp(gameID, "R3O", 3)) {
    std::srand(std::time(0));
    int r = std::rand() % MomErrCount;
    Log.report(logvisor::Fatal, FMT_STRING(_SYS_STR("{}")), MomErr[r]);
  }

  m_standalone = true;
  if (m_isWii && (!memcmp(gameID, "R3M", 3) || !memcmp(gameID, "R3I", 3) || !memcmp(gameID, "R32", 3)))
    m_standalone = false;

  if (m_standalone && !checkStandaloneID(gameID))
    return false;

  m_region = ERegion(m_disc->getHeader().m_gameID[3]);
  const hecl::SystemString* regstr = &regNONE;
  switch (m_region) {
  case ERegion::NTSC_U:
    regstr = &regE;
    break;
  case ERegion::NTSC_J:
    regstr = &regJ;
    break;
  case ERegion::PAL:
    regstr = &regP;
    break;
  default:
    break;
  }

  setCurRegion(m_region);
  setCurSpecIsWii(m_isWii);

  if (m_standalone)
    return checkFromStandaloneDisc(*m_disc, *regstr, info.extractArgs, reps);
  else
    return checkFromTrilogyDisc(*m_disc, *regstr, info.extractArgs, reps);
}

void SpecBase::doExtract(const ExtractPassInfo& info, const hecl::MultiProgressPrinter& progress) {
  setThreadProject();
  DataSpec::g_curSpec.reset(this);
  if (!Blender::BuildMasterShader(m_masterShader))
    Log.report(logvisor::Fatal, FMT_STRING("Unable to build master shader blend"));
  if (m_isWii) {
    /* Extract root files for repacking later */
    hecl::ProjectPath outDir(m_project.getProjectWorkingPath(), _SYS_STR("out"));
    outDir.makeDirChain(true);
    nod::ExtractionContext ctx = {info.force, nullptr};

    if (!m_standalone) {
      progress.print(_SYS_STR("Trilogy Files"), _SYS_STR(""), 0.0);
      nod::IPartition* data = m_disc->getDataPartition();
      const nod::Node& root = data->getFSTRoot();
      for (const nod::Node& child : root)
        if (child.getKind() == nod::Node::Kind::File)
          child.extractToDirectory(outDir.getAbsolutePath(), ctx);
      progress.print(_SYS_STR("Trilogy Files"), _SYS_STR(""), 1.0);
    }
  }
  extractFromDisc(*m_disc, info.force, progress);
}

bool IsPathAudioGroup(const hecl::ProjectPath& path) {
  return (path.getPathType() == hecl::ProjectPath::Type::Directory &&
          hecl::ProjectPath(path, _SYS_STR("!project.yaml")).isFile() &&
          hecl::ProjectPath(path, _SYS_STR("!pool.yaml")).isFile());
}

static bool IsPathSong(const hecl::ProjectPath& path) {
  if (path.getPathType() != hecl::ProjectPath::Type::Glob || !path.getWithExtension(_SYS_STR(".mid"), true).isFile() ||
      !path.getWithExtension(_SYS_STR(".yaml"), true).isFile()) {
    return path.isFile() && path.getLastComponentExt() == _SYS_STR("mid") &&
           path.getWithExtension(_SYS_STR(".yaml"), true).isFile();
  }
  return true;
}

bool SpecBase::canCook(const hecl::ProjectPath& path, hecl::blender::Token& btok) {
  if (!checkPathPrefix(path))
    return false;

  hecl::ProjectPath asBlend;
  if (path.getPathType() == hecl::ProjectPath::Type::Glob)
    asBlend = path.getWithExtension(_SYS_STR(".blend"), true);
  else
    asBlend = path;

  if (hecl::IsPathBlend(asBlend)) {
    hecl::blender::BlendType type = hecl::blender::GetBlendType(asBlend.getAbsolutePath());
    return type != hecl::blender::BlendType::None;
  }

  if (hecl::IsPathPNG(path)) {
    return true;
  } else if (hecl::IsPathYAML(path)) {
    athena::io::FileReader reader(path.getAbsolutePath());
    bool retval = validateYAMLDNAType(reader);
    return retval;
  } else if (IsPathAudioGroup(path)) {
    return true;
  } else if (IsPathSong(path)) {
    return true;
  }
  return false;
}

const hecl::Database::DataSpecEntry* SpecBase::overrideDataSpec(const hecl::ProjectPath& path,
                                                                const hecl::Database::DataSpecEntry* oldEntry) const {
  if (!checkPathPrefix(path))
    return nullptr;

  hecl::ProjectPath asBlend;
  if (path.getPathType() == hecl::ProjectPath::Type::Glob)
    asBlend = path.getWithExtension(_SYS_STR(".blend"), true);
  else
    asBlend = path;

  if (hecl::IsPathBlend(asBlend)) {
    if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _SYS_STR(".CSKR")) ||
        hecl::StringUtils::EndsWith(path.getAuxInfo(), _SYS_STR(".ANIM")))
      return oldEntry;

    hecl::blender::BlendType type = hecl::blender::GetBlendType(asBlend.getAbsolutePath());
    if (type == hecl::blender::BlendType::None) {
      Log.report(logvisor::Error, FMT_STRING(_SYS_STR("unable to cook '{}'")), path.getAbsolutePath());
      return nullptr;
    }
    if (type == hecl::blender::BlendType::Mesh || type == hecl::blender::BlendType::Area)
      return oldEntry;
  } else if (hecl::IsPathPNG(path)) {
    return oldEntry;
  }
  return &getOriginalSpec();
}

void SpecBase::doCook(const hecl::ProjectPath& path, const hecl::ProjectPath& cookedPath, bool fast,
                      hecl::blender::Token& btok, FCookProgress progress) {
  cookedPath.makeDirChain(false);
  DataSpec::g_curSpec.reset(this);

  hecl::ProjectPath asBlend;
  if (path.getPathType() == hecl::ProjectPath::Type::Glob)
    asBlend = path.getWithExtension(_SYS_STR(".blend"), true);
  else
    asBlend = path;

  if (hecl::IsPathBlend(asBlend)) {
    hecl::blender::Connection& conn = btok.getBlenderConnection();
    if (!conn.openBlend(asBlend))
      return;
    switch (conn.getBlendType()) {
    case hecl::blender::BlendType::Mesh: {
      hecl::blender::DataStream ds = conn.beginData();
      cookMesh(cookedPath, path, ds, fast, btok, progress);
      break;
    }
    case hecl::blender::BlendType::ColMesh: {
      hecl::blender::DataStream ds = conn.beginData();
      cookColMesh(cookedPath, path, ds, fast, btok, progress);
      break;
    }
    case hecl::blender::BlendType::Armature: {
      hecl::blender::DataStream ds = conn.beginData();
      cookArmature(cookedPath, path, ds, fast, btok, progress);
      break;
    }
    case hecl::blender::BlendType::PathMesh: {
      hecl::blender::DataStream ds = conn.beginData();
      cookPathMesh(cookedPath, path, ds, fast, btok, progress);
      break;
    }
    case hecl::blender::BlendType::Actor: {
      hecl::blender::DataStream ds = conn.beginData();
      cookActor(cookedPath, path, ds, fast, btok, progress);
      break;
    }
    case hecl::blender::BlendType::Area: {
      hecl::blender::DataStream ds = conn.beginData();
      cookArea(cookedPath, path, ds, fast, btok, progress);
      break;
    }
    case hecl::blender::BlendType::World: {
      hecl::blender::DataStream ds = conn.beginData();
      cookWorld(cookedPath, path, ds, fast, btok, progress);
      break;
    }
    case hecl::blender::BlendType::Frame: {
      hecl::blender::DataStream ds = conn.beginData();
      cookGuiFrame(cookedPath, path, ds, btok, progress);
      break;
    }
    case hecl::blender::BlendType::MapArea: {
      hecl::blender::DataStream ds = conn.beginData();
      cookMapArea(cookedPath, path, ds, btok, progress);
      break;
    }
    case hecl::blender::BlendType::MapUniverse: {
      hecl::blender::DataStream ds = conn.beginData();
      cookMapUniverse(cookedPath, path, ds, btok, progress);
      break;
    }
    default:
      break;
    }
  } else if (hecl::IsPathPNG(path)) {
    if (m_pc)
      TXTR::CookPC(path, cookedPath);
    else
      TXTR::Cook(path, cookedPath);
  } else if (hecl::IsPathYAML(path)) {
    athena::io::FileReader reader(path.getAbsolutePath());
    cookYAML(cookedPath, path, reader, btok, progress);
  } else if (IsPathAudioGroup(path)) {
    cookAudioGroup(cookedPath, path, progress);
  } else if (IsPathSong(path)) {
    cookSong(cookedPath, path, progress);
  }
}

void SpecBase::flattenDependenciesBlend(const hecl::ProjectPath& in, std::vector<hecl::ProjectPath>& pathsOut,
                                        hecl::blender::Token& btok, int charIdx) {
  hecl::blender::Connection& conn = btok.getBlenderConnection();
  if (!conn.openBlend(in))
    return;
  switch (conn.getBlendType()) {
  case hecl::blender::BlendType::Mesh:
  case hecl::blender::BlendType::Area: {
    hecl::blender::DataStream ds = conn.beginData();
    std::vector<hecl::ProjectPath> texs = ds.getTextures();
    pathsOut.insert(pathsOut.end(), std::make_move_iterator(texs.begin()), std::make_move_iterator(texs.end()));
    break;
  }
  case hecl::blender::BlendType::Actor: {
    hecl::ProjectPath asGlob = in.getWithExtension(_SYS_STR(".*"), true);
    hecl::ProjectPath parentPath = asGlob.getParentPath();
    hecl::DirectoryEnumerator dEnum(parentPath.getAbsolutePath());
    hecl::blender::DataStream ds = conn.beginData();
    hecl::blender::Actor actor = ds.compileActorCharacterOnly();
    auto actNames = ds.getActionNames();
    ds.close();

    auto doSubtype = [&](Actor::Subtype& sub) {
      if (sub.armature >= 0) {
        if (hecl::IsPathBlend(sub.mesh)) {
          flattenDependenciesBlend(sub.mesh, pathsOut, btok);
          pathsOut.push_back(sub.mesh);
        }

        hecl::SystemStringConv chSysName(sub.name);
        if (!sub.cskrId.empty()) {
          hecl::SystemStringConv cskrSysName(sub.cskrId);
          pathsOut.push_back(
              asGlob.ensureAuxInfo(fmt::format(FMT_STRING(_SYS_STR("{}_{}.CSKR")), chSysName, cskrSysName)));
        } else {
          pathsOut.push_back(asGlob.ensureAuxInfo(fmt::format(FMT_STRING(_SYS_STR("{}.CSKR")), chSysName)));
        }

        const auto& arm = actor.armatures[sub.armature];
        if (hecl::IsPathBlend(arm.path))
          pathsOut.push_back(arm.path);

        for (const auto& overlay : sub.overlayMeshes) {
          hecl::SystemStringConv overlaySys(overlay.name);
          hecl::SystemStringConv overlayCskrId(overlay.cskrId);
          if (hecl::IsPathBlend(overlay.mesh)) {
            flattenDependenciesBlend(overlay.mesh, pathsOut, btok);
            pathsOut.push_back(overlay.mesh);
          }
          pathsOut.push_back(asGlob.ensureAuxInfo(
              fmt::format(FMT_STRING(_SYS_STR("{}.{}_{}.CSKR")), chSysName, overlaySys, overlayCskrId)));
        }
      }
    };
    if (charIdx < 0)
      for (auto& sub : actor.subtypes)
        doSubtype(sub);
    else if (charIdx < actor.subtypes.size())
      doSubtype(actor.subtypes[charIdx]);

    for (const Actor::Attachment& att : actor.attachments) {
      if (hecl::IsPathBlend(att.mesh)) {
        flattenDependenciesBlend(att.mesh, pathsOut, btok);
        pathsOut.push_back(att.mesh);
      }

      hecl::SystemStringConv chSysName(att.name);
      hecl::SystemStringConv sysCskrId(att.cskrId);
      pathsOut.push_back(
          asGlob.ensureAuxInfo(fmt::format(FMT_STRING(_SYS_STR("ATTACH.{}_{}.CSKR")), chSysName, sysCskrId)));

      if (att.armature >= 0) {
        const auto& arm = actor.armatures[att.armature];
        if (hecl::IsPathBlend(arm.path))
          pathsOut.push_back(arm.path);
      }
    }

    for (const auto& act : actNames) {
      hecl::SystemStringConv actSysName(act.first);
      hecl::SystemStringConv actAnimId(act.second);
      pathsOut.push_back(asGlob.ensureAuxInfo(fmt::format(FMT_STRING(_SYS_STR("{}_{}.ANIM")), actSysName, actAnimId)));
      hecl::SystemString searchPrefix(
          asGlob.getWithExtension(fmt::format(FMT_STRING(_SYS_STR(".{}_")), actSysName).c_str(), true)
              .getLastComponent());
      hecl::ProjectPath evntPath;
      for (const auto& ent : dEnum) {
        if (hecl::StringUtils::BeginsWith(ent.m_name, searchPrefix.c_str()) &&
            hecl::StringUtils::EndsWith(ent.m_name, _SYS_STR(".evnt.yaml"))) {
          evntPath = hecl::ProjectPath(parentPath, ent.m_name);
          break;
        }
      }
      if (evntPath.isFile())
        pathsOut.push_back(evntPath);
    }

    hecl::ProjectPath yamlPath = asGlob.getWithExtension(_SYS_STR(".yaml"), true);
    if (yamlPath.isFile()) {
      athena::io::FileReader reader(yamlPath.getAbsolutePath());
      flattenDependenciesANCSYAML(reader, pathsOut, charIdx);
    }

    pathsOut.push_back(asGlob);
    return;
  }
  default:
    break;
  }
}

void SpecBase::flattenDependencies(const hecl::ProjectPath& path, std::vector<hecl::ProjectPath>& pathsOut,
                                   hecl::blender::Token& btok, int charIdx) {
  DataSpec::g_curSpec.reset(this);
  g_ThreadBlenderToken.reset(&btok);

  hecl::ProjectPath asBlend;
  if (path.getPathType() == hecl::ProjectPath::Type::Glob)
    asBlend = path.getWithExtension(_SYS_STR(".blend"), true);
  else
    asBlend = path;

  if (hecl::IsPathBlend(asBlend)) {
    flattenDependenciesBlend(asBlend, pathsOut, btok, charIdx);
  } else if (hecl::IsPathYAML(path)) {
    athena::io::FileReader reader(path.getAbsolutePath());
    flattenDependenciesYAML(reader, pathsOut);
  }

  pathsOut.push_back(path);
}

void SpecBase::flattenDependencies(const UniqueID32& id, std::vector<hecl::ProjectPath>& pathsOut, int charIdx) {
  hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(id);
  if (path)
    flattenDependencies(path, pathsOut, *g_ThreadBlenderToken.get(), charIdx);
}

void SpecBase::flattenDependencies(const UniqueID64& id, std::vector<hecl::ProjectPath>& pathsOut, int charIdx) {
  hecl::ProjectPath path = UniqueIDBridge::TranslatePakIdToPath(id);
  if (path)
    flattenDependencies(path, pathsOut, *g_ThreadBlenderToken.get(), charIdx);
}

bool SpecBase::canPackage(const hecl::ProjectPath& path) {
  auto components = path.getPathComponents();
  if (components.size() <= 1)
    return false;
  return path.isFile() || path.isDirectory();
}

void SpecBase::recursiveBuildResourceList(std::vector<urde::SObjectTag>& listOut,
                                          std::unordered_set<urde::SObjectTag>& addedTags,
                                          const hecl::ProjectPath& path, hecl::blender::Token& btok) {
  hecl::DirectoryEnumerator dEnum(path.getAbsolutePath(), hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted, false,
                                  false, true);
  for (const auto& ent : dEnum) {
    hecl::ProjectPath childPath(path, ent.m_name);
    if (ent.m_isDir) {
      if (hecl::ProjectPath(childPath, _SYS_STR("!project.yaml")).isFile() &&
          hecl::ProjectPath(childPath, _SYS_STR("!pool.yaml")).isFile()) {
        /* Handle AudioGroup case */
        if (urde::SObjectTag tag = tagFromPath(childPath)) {
          if (addedTags.find(tag) != addedTags.end())
            continue;
          addedTags.insert(tag);
          listOut.push_back(tag);
        }
        continue;
      }

      recursiveBuildResourceList(listOut, addedTags, childPath, btok);
    } else {
      std::vector<hecl::ProjectPath> subPaths;
      flattenDependencies(childPath, subPaths, btok);
      for (const auto& subPath : subPaths) {
        if (urde::SObjectTag tag = tagFromPath(subPath)) {
          if (addedTags.find(tag) != addedTags.end())
            continue;
          addedTags.insert(tag);
          listOut.push_back(tag);
        }
      }
    }
  }
}

void SpecBase::copyBuildListData(std::vector<std::tuple<size_t, size_t, bool>>& fileIndex,
                                 const std::vector<urde::SObjectTag>& buildList,
                                 const hecl::Database::DataSpecEntry* entry, bool fast,
                                 const hecl::MultiProgressPrinter& progress, athena::io::FileWriter& pakOut,
                                 const std::unordered_map<urde::CAssetId, std::vector<uint8_t>>& mlvlData) {
  fileIndex.reserve(buildList.size());
  int loadIdx = 0;
  for (const auto& tag : buildList) {
    hecl::SystemString str = fmt::format(FMT_STRING(_SYS_STR("Copying {}")), tag);
    progress.print(str.c_str(), nullptr, ++loadIdx / float(buildList.size()));

    auto& [positionOut, sizeOut, compressedOut] = fileIndex.emplace_back();

    if (tag.type == FOURCC('MLVL')) {
      auto search = mlvlData.find(tag.id);
      if (search == mlvlData.end())
        Log.report(logvisor::Fatal, FMT_STRING(_SYS_STR("Unable to find MLVL {}")), tag.id);

      positionOut = pakOut.position();
      sizeOut = ROUND_UP_32(search->second.size());
      compressedOut = false;
      pakOut.writeUBytes(search->second.data(), search->second.size());
      for (atUint64 i = search->second.size(); i < sizeOut; ++i)
        pakOut.writeUByte(0xff);

      continue;
    }

    hecl::ProjectPath path = pathFromTag(tag);
    hecl::ProjectPath cooked = getCookedPath(path, true);
    athena::io::FileReader r(cooked.getAbsolutePath());
    if (r.hasError())
      Log.report(logvisor::Fatal, FMT_STRING(_SYS_STR("Unable to open resource {}")), cooked.getRelativePath());
    atUint64 size = r.length();
    auto data = r.readUBytes(size);
    auto compData = compressPakData(tag, data.get(), size);
    if (compData.first) {
      positionOut = pakOut.position();
      sizeOut = ROUND_UP_32(compData.second + 4);
      compressedOut = true;
      pakOut.writeUint32Big(atUint32(size));
      pakOut.writeUBytes(compData.first.get(), compData.second);
      for (atUint64 i = compData.second + 4; i < sizeOut; ++i)
        pakOut.writeUByte(0xff);
    } else {
      positionOut = pakOut.position();
      sizeOut = ROUND_UP_32(size);
      compressedOut = false;
      pakOut.writeUBytes(data.get(), size);
      for (atUint64 i = size; i < sizeOut; ++i)
        pakOut.writeUByte(0xff);
    }
  }
  progress.startNewLine();
}

static bool IsWorldBlend(const hecl::ProjectPath& path) {
  if (path.isFile()) {
    auto lastComp = path.getLastComponent();
    return hecl::StringUtils::BeginsWith(lastComp, _SYS_STR("!world_")) &&
           hecl::StringUtils::EndsWith(lastComp, _SYS_STR(".blend"));
  }
  return false;
}

void SpecBase::doPackage(const hecl::ProjectPath& path, const hecl::Database::DataSpecEntry* entry, bool fast,
                         hecl::blender::Token& btok, const hecl::MultiProgressPrinter& progress,
                         hecl::ClientProcess* cp) {
  /* Prepare complete resource index */
  if (!m_backgroundRunning && m_tagToPath.empty())
    beginBackgroundIndex();
  waitForIndexComplete();

  /* Name pak based on root-relative components */
  auto components = path.getWithExtension(_SYS_STR(""), true).getPathComponents();
  if (components.size() <= 1)
    return;
  hecl::ProjectPath outPath;
  if (hecl::ProjectPath(m_project.getProjectWorkingPath(), _SYS_STR("out/files/") + components[0]).isDirectory())
    outPath.assign(m_project.getProjectWorkingPath(),
                   _SYS_STR("out/files/") + components[0] + _SYS_STR("/") + components[1] + entry->m_pakExt.data());
  else
    outPath.assign(m_project.getProjectWorkingPath(), _SYS_STR("out/files/") + components[1] + entry->m_pakExt.data());
  outPath.makeDirChain(false);

  /* Output file */
  athena::io::FileWriter pakOut(outPath.getAbsolutePath());
  std::vector<urde::SObjectTag> buildList;
  atUint64 resTableOffset = 0;
  std::unordered_map<urde::CAssetId, std::vector<uint8_t>> mlvlData;

  if (IsWorldBlend(path)) /* World PAK */
  {
    /* Force-cook MLVL and write resource list structure */
    m_project.cookPath(path, progress, false, true, fast, entry, cp);
    if (cp)
      cp->waitUntilComplete();
    progress.startNewLine();
    hecl::ProjectPath cooked = getCookedPath(path, true);
    buildWorldPakList(path, cooked, btok, pakOut, buildList, resTableOffset, mlvlData);
    if (int64_t rem = pakOut.position() % 32)
      for (int64_t i = 0; i < 32 - rem; ++i)
        pakOut.writeUByte(0xff);
  } else if (path.getPathType() == hecl::ProjectPath::Type::Directory) /* General PAK */
  {
    /* Build resource list */
    std::unordered_set<urde::SObjectTag> addedTags;
    recursiveBuildResourceList(buildList, addedTags, path, btok);
    std::vector<std::pair<urde::SObjectTag, std::string>> nameList;

    /* Build name list */
    for (const auto& item : buildList) {
      auto search = m_catalogTagToNames.find(item);
      if (search != m_catalogTagToNames.end())
        for (const auto& name : search->second)
          nameList.emplace_back(item, name);
    }

    /* Write resource list structure */
    buildPakList(btok, pakOut, buildList, nameList, resTableOffset);
    if (int64_t rem = pakOut.position() % 32)
      for (int64_t i = 0; i < 32 - rem; ++i)
        pakOut.writeUByte(0xff);
  } else if (path.getPathType() == hecl::ProjectPath::Type::File) /* One-file General PAK */
  {
    /* Build resource list */
    std::vector<hecl::ProjectPath> subPaths;
    flattenDependencies(path, subPaths, btok);
    std::unordered_set<urde::SObjectTag> addedTags;
    std::vector<std::pair<urde::SObjectTag, std::string>> nameList;
    for (const auto& subPath : subPaths) {
      if (urde::SObjectTag tag = tagFromPath(subPath)) {
        if (addedTags.find(tag) != addedTags.end())
          continue;
        addedTags.insert(tag);
        buildList.push_back(tag);
      }
    }

    /* Build name list */
    for (const auto& item : buildList) {
      auto search = m_catalogTagToNames.find(item);
      if (search != m_catalogTagToNames.end())
        for (const auto& name : search->second)
          nameList.emplace_back(item, name);
    }

    /* Write resource list structure */
    buildPakList(btok, pakOut, buildList, nameList, resTableOffset);
    if (int64_t rem = pakOut.position() % 32)
      for (int64_t i = 0; i < 32 - rem; ++i)
        pakOut.writeUByte(0xff);
  }

  /* Async cook resource list if using ClientProcess */
  if (cp) {
    Log.report(logvisor::Info, FMT_STRING(_SYS_STR("Validating resources")));
    progress.setMainIndeterminate(true);
    std::vector<urde::SObjectTag> cookTags;
    cookTags.reserve(buildList.size());

    /* Ensure CMDLs are enqueued first to minimize synchronous dependency cooking */
    for (int i = 0; i < 2; ++i) {
      std::unordered_set<urde::SObjectTag> addedTags;
      addedTags.reserve(buildList.size());
      for (auto& tag : buildList) {
        if ((i == 0 && tag.type == FOURCC('CMDL')) || (i == 1 && tag.type != FOURCC('CMDL'))) {
          if (addedTags.find(tag) != addedTags.end())
            continue;
          addedTags.insert(tag);
          cookTags.push_back(tag);
        }
      }
    }

    /* Cook ordered tags */
    for (auto& tag : cookTags) {
      hecl::ProjectPath depPath = pathFromTag(tag);
      if (!depPath)
        Log.report(logvisor::Fatal, FMT_STRING(_SYS_STR("Unable to resolve {}")), tag);
      m_project.cookPath(depPath, progress, false, false, fast, entry, cp);
    }
    progress.setMainIndeterminate(false);
    cp->waitUntilComplete();
    progress.startNewLine();
  }

  /* Write resource data and build file index */
  std::vector<std::tuple<size_t, size_t, bool>> fileIndex;
  Log.report(logvisor::Info, FMT_STRING(_SYS_STR("Copying data into {}")), outPath.getRelativePath());
  copyBuildListData(fileIndex, buildList, entry, fast, progress, pakOut, mlvlData);

  /* Write file index */
  writePakFileIndex(pakOut, buildList, fileIndex, resTableOffset);
  pakOut.close();
}

void SpecBase::interruptCook() { cancelBackgroundIndex(); }

std::optional<hecl::blender::World> SpecBase::compileWorldFromDir(const hecl::ProjectPath& dir,
                                                                  hecl::blender::Token& btok) const {
  hecl::ProjectPath asBlend;
  for (const auto& ent : hecl::DirectoryEnumerator(dir.getAbsolutePath())) {
    if (hecl::StringUtils::BeginsWith(ent.m_name, _SYS_STR("!world_"))) {
      asBlend = hecl::ProjectPath(dir, ent.m_name).getWithExtension(_SYS_STR(".blend"), true);
      break;
    }
  }
  if (hecl::IsPathBlend(asBlend)) {
    hecl::blender::Connection& conn = btok.getBlenderConnection();
    if (!conn.openBlend(asBlend))
      return {};
    hecl::blender::DataStream ds = conn.beginData();
    return {ds.compileWorld()};
  }
  return {};
}

hecl::ProjectPath SpecBase::getCookedPath(const hecl::ProjectPath& working, bool pcTarget) const {
  const hecl::Database::DataSpecEntry* spec = &getOriginalSpec();
  if (pcTarget)
    spec = overrideDataSpec(working, getDataSpecEntry());
  if (!spec)
    return {};
  return working.getCookedPath(*spec);
}

static void PNGErr(png_structp png, png_const_charp msg) { Log.report(logvisor::Error, FMT_STRING("{}"), msg); }

static void PNGWarn(png_structp png, png_const_charp msg) { Log.report(logvisor::Warning, FMT_STRING("{}"), msg); }

constexpr uint8_t Convert4To8(uint8_t v) {
  /* Swizzle bits: 00001234 -> 12341234 */
  return (v << 4) | v;
}

void SpecBase::extractRandomStaticEntropy(const uint8_t* buf, const hecl::ProjectPath& pakPath) {
  hecl::ProjectPath entropyPath(pakPath, _SYS_STR("RandomStaticEntropy.png"));
  hecl::ProjectPath catalogPath(pakPath, _SYS_STR("!catalog.yaml"));
  entropyPath.makeDirChain(false);

  if (const auto fp = hecl::FopenUnique(catalogPath.getAbsolutePath().data(), _SYS_STR("a"))) {
    fmt::print(fp.get(), FMT_STRING("RandomStaticEntropy: {}\n"), entropyPath.getRelativePathUTF8());
  }

  auto fp = hecl::FopenUnique(entropyPath.getAbsolutePath().data(), _SYS_STR("wb"));
  if (fp == nullptr) {
    Log.report(logvisor::Error, FMT_STRING(_SYS_STR("Unable to open '{}' for writing")), entropyPath.getAbsolutePath());
    return;
  }
  png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, PNGErr, PNGWarn);
  png_init_io(png, fp.get());
  png_infop info = png_create_info_struct(png);

  png_text textStruct = {};
  textStruct.key = png_charp("urde_nomip");
  png_set_text(png, info, &textStruct, 1);

  png_set_IHDR(png, info, 1024, 512, 8, PNG_COLOR_TYPE_GRAY_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png, info);

  std::unique_ptr<uint8_t[]> rowbuf(new uint8_t[1024 * 2]);
  for (int y = 0; y < 512; ++y) {
    for (int x = 0; x < 1024; ++x) {
      uint8_t texel = buf[y * 1024 + x];
      rowbuf[x * 2] = Convert4To8(texel >> 4 & 0xf);
      rowbuf[x * 2 + 1] = Convert4To8(texel & 0xf);
    }
    png_write_row(png, rowbuf.get());
  }

  png_write_end(png, info);
  png_write_flush(png);
  png_destroy_write_struct(&png, &info);
}

void SpecBase::clearTagCache() {
  m_tagToPath.clear();
  m_pathToTag.clear();
  m_catalogNameToTag.clear();
  m_catalogTagToNames.clear();
}

hecl::ProjectPath SpecBase::pathFromTag(const urde::SObjectTag& tag) const {
  std::unique_lock lk(m_backgroundIndexMutex);
  auto search = m_tagToPath.find(tag);
  if (search != m_tagToPath.cend())
    return search->second;
  return {};
}

urde::SObjectTag SpecBase::tagFromPath(const hecl::ProjectPath& path) const {
  auto search = m_pathToTag.find(path.hash());
  if (search != m_pathToTag.cend())
    return search->second;
  return buildTagFromPath(path);
}

bool SpecBase::waitForTagReady(const urde::SObjectTag& tag, const hecl::ProjectPath*& pathOut) {
  std::unique_lock lk(m_backgroundIndexMutex);
  auto search = m_tagToPath.find(tag);
  if (search == m_tagToPath.end()) {
    if (m_backgroundRunning) {
      while (m_backgroundRunning) {
        lk.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        lk.lock();
        search = m_tagToPath.find(tag);
        if (search != m_tagToPath.end())
          break;
      }
      if (search == m_tagToPath.end())
        return false;
    } else
      return false;
  }
  lk.unlock();
  pathOut = &search->second;
  return true;
}

const urde::SObjectTag* SpecBase::getResourceIdByName(std::string_view name) const {
  std::string lower(name);
  std::transform(lower.cbegin(), lower.cend(), lower.begin(), tolower);

  std::unique_lock lk(m_backgroundIndexMutex);
  auto search = m_catalogNameToTag.find(lower);
  if (search == m_catalogNameToTag.end()) {
    if (m_backgroundRunning) {
      while (m_backgroundRunning) {
        lk.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        lk.lock();
        search = m_catalogNameToTag.find(lower);
        if (search != m_catalogNameToTag.end())
          break;
      }
      if (search == m_catalogNameToTag.end())
        return nullptr;
    } else
      return nullptr;
  }
  return &search->second;
}

FourCC SpecBase::getResourceTypeById(urde::CAssetId id) const {
  if (!id.IsValid())
    return {};

  std::unique_lock lk(m_backgroundIndexMutex);
  urde::SObjectTag searchTag = {FourCC(), id};
  auto search = m_tagToPath.find(searchTag);
  if (search == m_tagToPath.end()) {
    if (m_backgroundRunning) {
      while (m_backgroundRunning) {
        lk.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        lk.lock();
        search = m_tagToPath.find(searchTag);
        if (search != m_tagToPath.end())
          break;
      }
      if (search == m_tagToPath.end())
        return {};
    } else
      return {};
  }

  return search->first.type;
}

void SpecBase::enumerateResources(const std::function<bool(const urde::SObjectTag&)>& lambda) const {
  waitForIndexComplete();
  for (const auto& pair : m_tagToPath) {
    if (!lambda(pair.first))
      break;
  }
}

void SpecBase::enumerateNamedResources(
    const std::function<bool(std::string_view, const urde::SObjectTag&)>& lambda) const {
  waitForIndexComplete();
  for (const auto& pair : m_catalogNameToTag) {
    if (!lambda(pair.first, pair.second))
      break;
  }
}

static void WriteTag(athena::io::YAMLDocWriter& cacheWriter, const urde::SObjectTag& pathTag,
                     const hecl::ProjectPath& path) {
  auto key = fmt::format(FMT_STRING("{}"), pathTag.id);
  if (auto* existing = cacheWriter.getCurNode()->findMapChild(key)) {
    existing->m_seqChildren.emplace_back(athena::io::ValToNode(path.getEncodableStringUTF8()));
  } else if (auto v = cacheWriter.enterSubVector(key)) {
    cacheWriter.writeString(pathTag.type.toString());
    cacheWriter.writeString(path.getEncodableStringUTF8());
  }
}

static void WriteNameTag(athena::io::YAMLDocWriter& nameWriter, const urde::SObjectTag& pathTag,
                         std::string_view name) {
  nameWriter.writeString(name.data(), fmt::format(FMT_STRING("{}"), pathTag.id));
}

void SpecBase::readCatalog(const hecl::ProjectPath& catalogPath, athena::io::YAMLDocWriter& nameWriter) {
  athena::io::FileReader freader(catalogPath.getAbsolutePath());
  if (!freader.isOpen())
    return;

  athena::io::YAMLDocReader reader;
  bool res = reader.parse(&freader);
  if (!res)
    return;

  const athena::io::YAMLNode* root = reader.getRootNode();
  for (const auto& p : root->m_mapChildren) {
    /* Hash as lowercase since lookup is case-insensitive */
    std::string pLower = p.first;
    std::transform(pLower.cbegin(), pLower.cend(), pLower.begin(), tolower);

    /* Avoid redundant filesystem access for re-caches */
    if (m_catalogNameToTag.find(pLower) != m_catalogNameToTag.cend())
      continue;

    athena::io::YAMLNode& node = *p.second;
    hecl::ProjectPath path;
    if (node.m_type == YAML_SCALAR_NODE) {
      path = hecl::ProjectPath(m_project.getProjectWorkingPath(), node.m_scalarString);
    } else if (node.m_type == YAML_SEQUENCE_NODE) {
      if (node.m_seqChildren.size() >= 2)
        path = hecl::ProjectPath(m_project.getProjectWorkingPath(), node.m_seqChildren[0]->m_scalarString)
                   .ensureAuxInfo(node.m_seqChildren[1]->m_scalarString);
      else if (node.m_seqChildren.size() == 1)
        path = hecl::ProjectPath(m_project.getProjectWorkingPath(), node.m_seqChildren[0]->m_scalarString);
    }
    if (path.isNone())
      continue;
    urde::SObjectTag pathTag = tagFromPath(path);
    if (pathTag) {
      std::unique_lock lk(m_backgroundIndexMutex);
      m_catalogNameToTag[pLower] = pathTag;
      m_catalogTagToNames[pathTag].insert(p.first);

      WriteNameTag(nameWriter, pathTag, p.first);
#if 0
      fmt::print(stderr, FMT_STRING("{} {} {:08X}\n"), p.first, pathTag.type.toString(), pathTag.id.Value());
#endif
    }
  }
}

void SpecBase::backgroundIndexRecursiveCatalogs(const hecl::ProjectPath& dir, athena::io::YAMLDocWriter& nameWriter,
                                                int level) {
  hecl::DirectoryEnumerator dEnum(dir.getAbsolutePath(), hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted, false,
                                  false, true);

  /* Enumerate all items */
  for (const hecl::DirectoryEnumerator::Entry& ent : dEnum) {
    hecl::ProjectPath path(dir, ent.m_name);
    if (ent.m_isDir && level < 1)
      backgroundIndexRecursiveCatalogs(path, nameWriter, level + 1);
    else {
      if (!path.isFile())
        continue;

      /* Read catalog.yaml for .pak directory if exists */
      if (level == 1 && ent.m_name == _SYS_STR("!catalog.yaml")) {
        readCatalog(path, nameWriter);
        continue;
      }
    }

    /* bail if cancelled by client */
    if (!m_backgroundRunning)
      break;
  }
}

void SpecBase::insertPathTag(athena::io::YAMLDocWriter& cacheWriter, const urde::SObjectTag& tag,
                             const hecl::ProjectPath& path, bool dump) {
#if 0
  auto search = m_tagToPath.find(tag);
  /* ANCS subresources are allowed to be weak-linked */
  if (search != m_tagToPath.end() && search->second != path &&
      tag.type != FOURCC('CINF') && tag.type != FOURCC('CSKR') &&
      tag.type != FOURCC('ANIM') && tag.type != FOURCC('EVNT')) {
    Log.report(logvisor::Fatal, FMT_STRING(_SYS_STR("'{}|{}' already exists for tag {} as '{}|{}'")),
               path.getRelativePath(), path.getAuxInfo(), tag,
               search->second.getRelativePath(), search->second.getAuxInfo());
  }
#endif
  m_tagToPath.insert(std::make_pair(tag, path));
  m_pathToTag[path.hash()] = tag;
  WriteTag(cacheWriter, tag, path);
#if DUMP_CACHE_FILL
  if (dump)
    fmt::print(stderr, FMT_STRING("{} {}\n"), tag, path.getRelativePathUTF8());
#endif
}

bool SpecBase::addFileToIndex(const hecl::ProjectPath& path, athena::io::YAMLDocWriter& cacheWriter) {
  /* Avoid redundant filesystem access for re-caches */
  if (m_pathToTag.find(path.hash()) != m_pathToTag.cend())
    return true;

  /* Try as glob */
  hecl::ProjectPath asGlob = path.getWithExtension(_SYS_STR(".*"), true);
  if (m_pathToTag.find(asGlob.hash()) != m_pathToTag.cend())
    return true;

  /* Classify intermediate into tag */
  urde::SObjectTag pathTag = buildTagFromPath(path);
  if (pathTag) {
    std::unique_lock lk{m_backgroundIndexMutex};
    bool useGlob = false;

    /* Special multi-resource intermediates */
    if (pathTag.type == SBIG('ANCS')) {
      hecl::blender::Connection& conn = m_backgroundBlender.getBlenderConnection();
      if (!conn.openBlend(path) || conn.getBlendType() != hecl::blender::BlendType::Actor)
        return false;

      /* Transform tag to glob */
      pathTag = {SBIG('ANCS'), asGlob.parsedHash32()};
      useGlob = true;

      hecl::blender::DataStream ds = conn.beginData();
      std::vector<std::pair<std::string, std::string>> subtypeNames = ds.getSubtypeNames();
      std::vector<std::pair<std::string, std::string>> actionNames = ds.getActionNames();

      for (const auto& sub : subtypeNames) {
        hecl::SystemStringConv subName(sub.first);
        hecl::ProjectPath subPath;
        if (!sub.second.empty()) {
          hecl::SystemStringConv cskrId(sub.second);
          subPath = asGlob.ensureAuxInfo(fmt::format(FMT_STRING(_SYS_STR("{}_{}.CSKR")), subName, cskrId));
        } else {
          subPath = asGlob.ensureAuxInfo(fmt::format(FMT_STRING(_SYS_STR("{}.CSKR")), subName));
        }
        insertPathTag(cacheWriter, buildTagFromPath(subPath), subPath);

        std::vector<std::pair<std::string, std::string>> overlayNames = ds.getSubtypeOverlayNames(sub.first);
        for (const auto& overlay : overlayNames) {
          hecl::SystemStringConv overlaySys(overlay.first);
          hecl::SystemStringConv overlayCskrId(overlay.second);
          if (!overlay.second.empty()) {
            subPath = asGlob.ensureAuxInfo(
                fmt::format(FMT_STRING(_SYS_STR("{}.{}_{}.CSKR")), subName, overlaySys, overlayCskrId));
          } else {
            subPath = asGlob.ensureAuxInfo(fmt::format(FMT_STRING(_SYS_STR("{}.{}.CSKR")), subName, overlaySys));
          }
          insertPathTag(cacheWriter, buildTagFromPath(subPath), subPath);
        }
      }

      std::vector<std::pair<std::string, std::string>> attachmentNames = ds.getAttachmentNames();
      for (const auto& attachment : attachmentNames) {
        hecl::SystemStringConv attachmentSys(attachment.first);
        hecl::SystemStringConv attachmentCskrId(attachment.second);
        hecl::ProjectPath subPath;
        if (!attachment.second.empty()) {
          subPath = asGlob.ensureAuxInfo(
              fmt::format(FMT_STRING(_SYS_STR("ATTACH.{}_{}.CSKR")), attachmentSys, attachmentCskrId));
        } else {
          subPath = asGlob.ensureAuxInfo(fmt::format(FMT_STRING(_SYS_STR("ATTACH.{}.CSKR")), attachmentSys));
        }
        insertPathTag(cacheWriter, buildTagFromPath(subPath), subPath);
      }

      for (const auto& act : actionNames) {
        hecl::SystemStringConv sysStr(act.first);
        hecl::SystemStringConv animId(act.second);
        hecl::ProjectPath subPath;
        if (!act.second.empty()) {
          subPath = asGlob.ensureAuxInfo(fmt::format(FMT_STRING(_SYS_STR("{}_{}.ANIM")), sysStr, animId));
        } else {
          subPath = asGlob.ensureAuxInfo(fmt::format(FMT_STRING(_SYS_STR("{}.ANIM")), sysStr));
        }
        insertPathTag(cacheWriter, buildTagFromPath(subPath), subPath);
      }
    }

    /* Cache in-memory */
    const hecl::ProjectPath& usePath = useGlob ? asGlob : path;
    insertPathTag(cacheWriter, pathTag, usePath);
  }

  return true;
}

void SpecBase::backgroundIndexRecursiveProc(const hecl::ProjectPath& dir, athena::io::YAMLDocWriter& cacheWriter,
                                            athena::io::YAMLDocWriter& nameWriter, int level) {
  hecl::DirectoryEnumerator dEnum(dir.getAbsolutePath(), hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted, false,
                                  false, true);

  /* Enumerate all items */
  for (const hecl::DirectoryEnumerator::Entry& ent : dEnum) {
    /* bail if cancelled by client */
    if (!m_backgroundRunning)
      break;

    hecl::ProjectPath path(dir, ent.m_name);
    if (ent.m_isDir) {
      /* Index AGSC here */
      if (hecl::ProjectPath(path, "!project.yaml").isFile() && hecl::ProjectPath(path, "!pool.yaml").isFile()) {
        /* Avoid redundant filesystem access for re-caches */
        if (m_pathToTag.find(path.hash()) == m_pathToTag.cend()) {
          urde::SObjectTag pathTag(SBIG('AGSC'), path.parsedHash32());
          insertPathTag(cacheWriter, pathTag, path);
        }
      } else {
        backgroundIndexRecursiveProc(path, cacheWriter, nameWriter, level + 1);
      }
    } else {
      if (!path.isFile())
        continue;

      /* Read catalog.yaml for .pak directory if exists */
      if (level == 1 && ent.m_name == _SYS_STR("!catalog.yaml")) {
        readCatalog(path, nameWriter);
        continue;
      }

      /* Index the regular file */
      addFileToIndex(path, cacheWriter);
    }
  }
}

void SpecBase::backgroundIndexProc() {
  logvisor::RegisterThreadName("Resource Index");

  hecl::ProjectPath tagCachePath(m_project.getProjectCookedPath(getOriginalSpec()), _SYS_STR("tag_cache.yaml"));
  hecl::ProjectPath nameCachePath(m_project.getProjectCookedPath(getOriginalSpec()), _SYS_STR("name_cache.yaml"));
  hecl::ProjectPath specRoot(m_project.getProjectWorkingPath(), getOriginalSpec().m_name);

  /* Cache will be overwritten with validated entries afterwards */
  athena::io::YAMLDocWriter cacheWriter;
  athena::io::YAMLDocWriter nameWriter;

  /* Read in tag cache */
  if (tagCachePath.isFile()) {
    athena::io::FileReader reader(tagCachePath.getAbsolutePath());
    if (reader.isOpen()) {
      Log.report(logvisor::Info, FMT_STRING(_SYS_STR("Cache index of '{}' loading")), getOriginalSpec().m_name);
      athena::io::YAMLDocReader cacheReader;
      if (cacheReader.parse(&reader)) {
        std::unique_lock lk(m_backgroundIndexMutex);
        size_t tagCount = cacheReader.getRootNode()->m_mapChildren.size();
        m_tagToPath.reserve(tagCount);
        m_pathToTag.reserve(tagCount);
        size_t loadIdx = 0;
        for (const auto& child : cacheReader.getRootNode()->m_mapChildren) {
          if (!m_backgroundRunning)
            return;

          const athena::io::YAMLNode& node = *child.second;
          if (node.m_seqChildren.size() >= 2) {
            unsigned long id = strtoul(child.first.c_str(), nullptr, 16);
            hecl::FourCC type(node.m_seqChildren[0]->m_scalarString.c_str());
            urde::SObjectTag pathTag(type, id);
            for (auto I = node.m_seqChildren.begin() + 1, E = node.m_seqChildren.end(); I != E; ++I) {
              hecl::ProjectPath path(m_project.getProjectWorkingPath(), (*I)->m_scalarString);
              if (!path.isNone())
                insertPathTag(cacheWriter, pathTag, path, false);
            }
          }

          ++loadIdx;
          if (!(loadIdx % 100))
            fmt::print(stderr, FMT_STRING("\r {} / {}"), loadIdx, tagCount);
        }
        fmt::print(stderr, FMT_STRING("\r {} / {}\n"), loadIdx, tagCount);
      }
      Log.report(logvisor::Info, FMT_STRING(_SYS_STR("Cache index of '{}' loaded; {} tags")), getOriginalSpec().m_name,
                 m_tagToPath.size());

      if (nameCachePath.isFile()) {
        /* Read in name cache */
        Log.report(logvisor::Info, FMT_STRING(_SYS_STR("Name index of '{}' loading")), getOriginalSpec().m_name);
        athena::io::FileReader nreader(nameCachePath.getAbsolutePath());
        athena::io::YAMLDocReader nameReader;
        if (nameReader.parse(&nreader)) {
          std::unique_lock lk(m_backgroundIndexMutex);
          m_catalogNameToTag.reserve(nameReader.getRootNode()->m_mapChildren.size());
          m_catalogTagToNames.reserve(nameReader.getRootNode()->m_mapChildren.size());
          for (const auto& child : nameReader.getRootNode()->m_mapChildren) {
            unsigned long id = strtoul(child.second->m_scalarString.c_str(), nullptr, 16);
            auto search = m_tagToPath.find(urde::SObjectTag(FourCC(), uint32_t(id)));
            if (search != m_tagToPath.cend()) {
              std::string chLower = child.first;
              std::transform(chLower.cbegin(), chLower.cend(), chLower.begin(), tolower);
              m_catalogNameToTag[chLower] = search->first;
              m_catalogTagToNames[search->first].insert(child.first);
              WriteNameTag(nameWriter, search->first, child.first);
            }
          }
        }
        Log.report(logvisor::Info, FMT_STRING(_SYS_STR("Name index of '{}' loaded; {} names")),
                   getOriginalSpec().m_name, m_catalogNameToTag.size());
      }
    }
  }

  Log.report(logvisor::Info, FMT_STRING(_SYS_STR("Background index of '{}' started")), getOriginalSpec().m_name);
  backgroundIndexRecursiveProc(specRoot, cacheWriter, nameWriter, 0);

  tagCachePath.makeDirChain(false);
  athena::io::FileWriter twriter(tagCachePath.getAbsolutePath());
  cacheWriter.finish(&twriter);

  athena::io::FileWriter nwriter(nameCachePath.getAbsolutePath());
  nameWriter.finish(&nwriter);

  m_backgroundBlender.shutdown();
  Log.report(logvisor::Info, FMT_STRING(_SYS_STR("Background index of '{}' complete; {} tags, {} names")),
             getOriginalSpec().m_name, m_tagToPath.size(), m_catalogNameToTag.size());
  m_backgroundRunning = false;
}

void SpecBase::cancelBackgroundIndex() {
  m_backgroundRunning = false;
  if (m_backgroundIndexTh.joinable())
    m_backgroundIndexTh.join();
}

void SpecBase::beginBackgroundIndex() {
  cancelBackgroundIndex();
  clearTagCache();
  m_backgroundRunning = true;
  m_backgroundIndexTh = std::thread(&SpecBase::backgroundIndexProc, this);
}

void SpecBase::waitForIndexComplete() const {
  std::unique_lock lk(m_backgroundIndexMutex);
  while (m_backgroundRunning) {
    lk.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    lk.lock();
  }
}

void SpecBase::WriteVersionInfo(hecl::Database::Project& project, const hecl::ProjectPath& pakPath) {
  hecl::ProjectPath versionPath(pakPath, _SYS_STR("version.yaml"));
  versionPath.makeDirChain(false);

  URDEVersionInfo info;
  info.version = m_version;
  info.region = m_region;
  info.game = m_game;
  info.isTrilogy = !m_standalone;
  athena::io::FileWriter writer(versionPath.getAbsolutePath());
  athena::io::ToYAMLStream(info, writer);
}
void SpecBase::setCurRegion(ERegion region) {
  g_CurRegion = region;
}
void SpecBase::setCurSpecIsWii(bool isWii) {
  g_CurSpecIsWii = isWii;
}

} // namespace DataSpec
