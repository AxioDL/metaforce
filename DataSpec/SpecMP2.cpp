#include <utility>

#include "SpecBase.hpp"
#include "DNAMP2/DNAMP2.hpp"

#include "DNAMP2/MLVL.hpp"
#include "DNAMP2/STRG.hpp"
#include "DNAMP2/AGSC.hpp"
#include "DNAMP2/MAPA.hpp"
#include "DNAMP1/CSNG.hpp"
#include "DNACommon/MAPU.hpp"

#include "hecl/ClientProcess.hpp"
#include "hecl/Blender/Connection.hpp"
#include "hecl/MultiProgressPrinter.hpp"

#include "Runtime/RetroTypes.hpp"
#include "nod/nod.hpp"

namespace DataSpec {

using namespace std::literals;

static logvisor::Module Log("urde::SpecMP2");
extern hecl::Database::DataSpecEntry SpecEntMP2;
extern hecl::Database::DataSpecEntry SpecEntMP2ORIG;

static const std::unordered_set<uint32_t> IndividualOrigIDs = {
    0xB7BBD0B4, 0x1F9DA858, 0x2A13C23E, 0xF13452F8, 0xA91A7703, 0xC042EC91, 0x12A12131, 0x5F556002, 0xA9798329,
    0xB306E26F, 0xCD7B1ACA, 0x8ADA8184, 0x1A29C0E6, 0x5D9F9796, 0x951546A8, 0x7946C4C5, 0x409AA72E,
};

struct OriginalIDs {
  static void Generate(PAKRouter<DNAMP2::PAKBridge>& pakRouter, hecl::Database::Project& project) {
    std::unordered_set<UniqueID32> addedIDs;
    std::vector<UniqueID32> originalIDs;

    pakRouter.enumerateResources([&](const DNAMP2::PAK::Entry* ent) {
      if (ent->type == FOURCC('MLVL') || ent->type == FOURCC('SCAN') || ent->type == FOURCC('MREA') ||
          IndividualOrigIDs.find(ent->id.toUint32()) != IndividualOrigIDs.end()) {
        if (addedIDs.find(ent->id) == addedIDs.cend()) {
          addedIDs.insert(ent->id);
          originalIDs.push_back(ent->id);
        }
      }
      return true;
    });
    std::sort(originalIDs.begin(), originalIDs.end());

    athena::io::YAMLDocWriter yamlW("MP2OriginalIDs");
    for (const UniqueID32& id : originalIDs) {
      hecl::ProjectPath path = pakRouter.getWorking(id);
      yamlW.writeString(id.toString().c_str(), path.getRelativePathUTF8());
    }
    hecl::ProjectPath path(project.getProjectWorkingPath(), "MP2/!original_ids.yaml");
    path.makeDirChain(false);
    athena::io::FileWriter fileW(path.getAbsolutePath());
    yamlW.finish(&fileW);
  }

  static void Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath) {
    hecl::Database::Project& project = inPath.getProject();
    athena::io::YAMLDocReader r;
    athena::io::FileReader fr(inPath.getAbsolutePath());
    if (!fr.isOpen() || !r.parse(&fr))
      return;

    std::vector<std::pair<UniqueID32, UniqueID32>> originalIDs;
    originalIDs.reserve(r.getRootNode()->m_mapChildren.size());
    for (const auto& node : r.getRootNode()->m_mapChildren) {
      char* end = const_cast<char*>(node.first.c_str());
      u32 id = strtoul(end, &end, 16);
      if (end != node.first.c_str() + 8)
        continue;

      hecl::ProjectPath path(project.getProjectWorkingPath(), node.second->m_scalarString.c_str());
      originalIDs.push_back(std::make_pair(id, path.hash().val32()));
    }
    std::sort(originalIDs.begin(), originalIDs.end(),
              [](const std::pair<UniqueID32, UniqueID32>& a, const std::pair<UniqueID32, UniqueID32>& b) {
                return a.first < b.first;
              });

    athena::io::FileWriter w(outPath.getAbsolutePath());
    w.writeUint32Big(originalIDs.size());
    for (const auto& idPair : originalIDs) {
      idPair.first.write(w);
      idPair.second.write(w);
    }

    std::sort(originalIDs.begin(), originalIDs.end(),
              [](const std::pair<UniqueID32, UniqueID32>& a, const std::pair<UniqueID32, UniqueID32>& b) {
                return a.second < b.second;
              });
    for (const auto& idPair : originalIDs) {
      idPair.second.write(w);
      idPair.first.write(w);
    }
  }
};

struct SpecMP2 : SpecBase {
  bool checkStandaloneID(const char* id) const {
    if (!memcmp(id, "G2M", 3))
      return true;
    return false;
  }

  std::vector<const nod::Node*> m_nonPaks;
  std::vector<DNAMP2::PAKBridge> m_paks;
  std::map<std::string, DNAMP2::PAKBridge*, hecl::CaseInsensitiveCompare> m_orderedPaks;

  hecl::ProjectPath m_workPath;
  hecl::ProjectPath m_cookPath;
  PAKRouter<DNAMP2::PAKBridge> m_pakRouter;
  IDRestorer<UniqueID32> m_idRestorer;

  void setThreadProject() {
    SpecBase::setThreadProject();
    UniqueIDBridge::SetIDRestorer(&m_idRestorer);
  }

  SpecMP2(const hecl::Database::DataSpecEntry* specEntry, hecl::Database::Project& project, bool pc)
  : SpecBase(specEntry, project, pc)
  , m_workPath(project.getProjectWorkingPath(), _SYS_STR("MP2"))
  , m_cookPath(project.getProjectCookedPath(SpecEntMP2), _SYS_STR("MP2"))
  , m_pakRouter(*this, m_workPath, m_cookPath)
  , m_idRestorer({project.getProjectWorkingPath(), "MP2/!original_ids.yaml"}, project) {
    setThreadProject();
  }

  void buildPaks(nod::Node& root, const std::vector<hecl::SystemString>& args, ExtractReport& rep) {
    m_nonPaks.clear();
    m_paks.clear();
    for (const nod::Node& child : root) {
      bool isPak = false;
      auto name = child.getName();
      std::string lowerName(name);
      std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), tolower);
      if (name.size() > 4) {
        std::string::iterator extit = lowerName.end() - 4;
        if (!std::string(extit, lowerName.end()).compare(".pak")) {
          /* This is a pak */
          isPak = true;
          std::string lowerBase(lowerName.begin(), extit);

          /* Needs filter */
          bool good = true;
          if (args.size()) {
            good = false;
            if (!lowerName.compare(0, 7, "metroid")) {
              hecl::SystemChar idxChar = lowerName[7];
              for (const hecl::SystemString& arg : args) {
                if (arg.size() == 1 && iswdigit(arg[0]))
                  if (arg[0] == idxChar)
                    good = true;
              }
            } else
              good = true;

            if (!good) {
              for (const hecl::SystemString& arg : args) {
                std::string lowerArg(hecl::SystemUTF8Conv(arg).str());
                std::transform(lowerArg.begin(), lowerArg.end(), lowerArg.begin(), tolower);
                if (!lowerArg.compare(0, lowerBase.size(), lowerBase))
                  good = true;
              }
            }
          }

          m_paks.emplace_back(child, good);
        }
      }

      if (!isPak)
        m_nonPaks.push_back(&child);
    }

    /* Sort PAKs alphabetically */
    m_orderedPaks.clear();
    for (DNAMP2::PAKBridge& dpak : m_paks)
      m_orderedPaks[std::string(dpak.getName())] = &dpak;

    /* Assemble extract report */
    for (const std::pair<std::string, DNAMP2::PAKBridge*>& item : m_orderedPaks) {
      if (!item.second->m_doExtract)
        continue;
      rep.childOpts.emplace_back();
      ExtractReport& childRep = rep.childOpts.back();
      hecl::SystemStringConv nameView(item.first);
      childRep.name = hecl::SystemString(nameView.sys_str());
      childRep.desc = item.second->getLevelString();
    }
  }

  bool checkFromStandaloneDisc(nod::DiscBase& disc, const hecl::SystemString& regstr,
                               const std::vector<hecl::SystemString>& args, std::vector<ExtractReport>& reps) {
    nod::IPartition* partition = disc.getDataPartition();
    std::unique_ptr<uint8_t[]> dolBuf = partition->getDOLBuf();
    const char* buildInfo = (char*)memmem(dolBuf.get(), partition->getDOLSize(), "MetroidBuildInfo", 16) + 19;
    if (!buildInfo)
      return false;

    /* Root Report */
    reps.emplace_back();
    ExtractReport& rep = reps.back();
    rep.name = _SYS_STR("MP2");
    rep.desc = _SYS_STR("Metroid Prime 2 ") + regstr;
    std::string buildStr(buildInfo);
    hecl::SystemStringConv buildView(buildStr);
    rep.desc += _SYS_STR(" (") + buildView + _SYS_STR(")");

    /* Iterate PAKs and build level options */
    nod::Node& root = partition->getFSTRoot();
    buildPaks(root, args, rep);

    return true;
  }

  bool checkFromTrilogyDisc(nod::DiscBase& disc, const hecl::SystemString& regstr,
                            const std::vector<hecl::SystemString>& args, std::vector<ExtractReport>& reps) {
    std::vector<hecl::SystemString> mp2args;
    bool doExtract = false;
    if (args.size()) {
      /* Needs filter */
      for (const hecl::SystemString& arg : args) {
        hecl::SystemString lowerArg = arg;
        hecl::ToLower(lowerArg);
        if (!lowerArg.compare(0, 3, _SYS_STR("mp2"))) {
          doExtract = true;
          mp2args.reserve(args.size());
          size_t slashPos = arg.find(_SYS_STR('/'));
          if (slashPos == hecl::SystemString::npos)
            slashPos = arg.find(_SYS_STR('\\'));
          if (slashPos != hecl::SystemString::npos)
            mp2args.emplace_back(hecl::SystemString(arg.begin() + slashPos + 1, arg.end()));
        }
      }
    } else
      doExtract = true;

    if (!doExtract)
      return false;

    nod::IPartition* partition = disc.getDataPartition();
    nod::Node& root = partition->getFSTRoot();
    nod::Node::DirectoryIterator dolIt = root.find("rs5mp2_p.dol");
    if (dolIt == root.end()) {
      dolIt = root.find("rs5mp2jpn_p.dol");
      if (dolIt == root.end())
        return false;
    }

    std::unique_ptr<uint8_t[]> dolBuf = dolIt->getBuf();
    const char* buildInfo = (char*)memmem(dolBuf.get(), dolIt->size(), "MetroidBuildInfo", 16) + 19;

    /* Root Report */
    reps.emplace_back();
    ExtractReport& rep = reps.back();
    rep.name = _SYS_STR("MP2");
    rep.desc = _SYS_STR("Metroid Prime 2 ") + regstr;
    if (buildInfo) {
      std::string buildStr(buildInfo);
      hecl::SystemStringConv buildView(buildStr);
      rep.desc += _SYS_STR(" (") + buildView + _SYS_STR(")");
    }

    /* Iterate PAKs and build level options */
    nod::Node::DirectoryIterator mp2It = root.find("MP2");
    if (mp2It == root.end()) {
      mp2It = root.find("MP2JPN");
      if (mp2It == root.end())
        return false;
    }
    buildPaks(*mp2It, mp2args, rep);

    return true;
  }

  bool extractFromDisc(nod::DiscBase& disc, bool force, const hecl::MultiProgressPrinter& progress) {
    nod::ExtractionContext ctx = {force, nullptr};

    m_workPath.makeDir();

    progress.startNewLine();
    progress.print(_SYS_STR("Indexing PAKs"), _SYS_STR(""), 0.0);
    m_pakRouter.build(m_paks,
                      [&progress](float factor) { progress.print(_SYS_STR("Indexing PAKs"), _SYS_STR(""), factor); });
    progress.print(_SYS_STR("Indexing PAKs"), _SYS_STR(""), 1.0);

    hecl::ProjectPath outPath(m_project.getProjectWorkingPath(), _SYS_STR("out"));
    outPath.makeDir();
    disc.getDataPartition()->extractSysFiles(outPath.getAbsolutePath(), ctx);
    hecl::ProjectPath mp2OutPath(outPath, m_standalone ? _SYS_STR("files") : _SYS_STR("files/MP2"));
    mp2OutPath.makeDirChain(true);

    progress.startNewLine();
    progress.print(_SYS_STR("MP2 Root"), _SYS_STR(""), 0.0);
    int prog = 0;
    ctx.progressCB = [&prog, &progress](std::string_view name, float) {
      hecl::SystemStringConv nameView(name);
      progress.print(_SYS_STR("MP2 Root"), nameView.c_str(), prog);
    };
    for (const nod::Node* node : m_nonPaks) {
      node->extractToDirectory(mp2OutPath.getAbsolutePath(), ctx);
      prog++;
    }
    progress.print(_SYS_STR("MP2 Root"), _SYS_STR(""), 1.0);

    hecl::ClientProcess process;
    progress.startNewLine();
    for (std::pair<const std::string, DNAMP2::PAKBridge*>& pair : m_orderedPaks) {
      DNAMP2::PAKBridge& pak = *pair.second;
      if (!pak.m_doExtract)
        continue;

      auto name = pak.getName();
      hecl::SystemStringConv sysName(name);

      auto pakName = hecl::SystemString(sysName.sys_str());
      process.addLambdaTransaction([this, &progress, &pak, pakName, force](hecl::blender::Token& btok) {
        int threadIdx = hecl::ClientProcess::GetThreadWorkerIdx();
        m_pakRouter.extractResources(pak, force, btok,
                                     [&progress, &pakName, threadIdx](const hecl::SystemChar* substr, float factor) {
                                       progress.print(pakName.c_str(), substr, factor, threadIdx);
                                     });
      });
    }

    process.waitUntilComplete();

    /* Generate original ID mapping for MLVL and SCAN entries - marks complete project */
    OriginalIDs::Generate(m_pakRouter, m_project);

    return true;
  }

  const hecl::Database::DataSpecEntry& getOriginalSpec() const { return SpecEntMP2; }

  const hecl::Database::DataSpecEntry& getUnmodifiedSpec() const { return SpecEntMP2ORIG; }

  hecl::ProjectPath getWorking(class UniqueID32& id) { return m_pakRouter.getWorking(id); }

  bool checkPathPrefix(const hecl::ProjectPath& path) const {
    return path.getRelativePath().compare(0, 4, _SYS_STR("MP2/")) == 0;
  }

  bool validateYAMLDNAType(athena::io::IStreamReader& fp) const {
    athena::io::YAMLDocReader reader;
    yaml_parser_set_input(reader.getParser(), (yaml_read_handler_t*)athena::io::YAMLAthenaReader, &fp);
    return reader.ClassTypeOperation([](const char* classType) {
      if (!strcmp(classType, DNAMP2::MLVL::DNAType()))
        return true;
      else if (!strcmp(classType, DNAMP2::STRG::DNAType()))
        return true;
      else if (!strcmp(classType, "ATBL"))
        return true;
      return false;
    });
  }

  urde::SObjectTag buildTagFromPath(const hecl::ProjectPath& path) const { return {}; }

  void cookMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                hecl::blender::Token& btok, FCookProgress progress) {}

  void cookColMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                   hecl::blender::Token& btok, FCookProgress progress) {}

  void cookPathMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                    hecl::blender::Token& btok, FCookProgress progress) {}

  void cookActor(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                 hecl::blender::Token& btok, FCookProgress progress) {}

  void cookArea(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                hecl::blender::Token& btok, FCookProgress progress) {}

  void cookWorld(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                 hecl::blender::Token& btok, FCookProgress progress) {}

  void cookGuiFrame(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds,
                    hecl::blender::Token& btok, FCookProgress progress) {}

  void cookYAML(const hecl::ProjectPath& out, const hecl::ProjectPath& in, athena::io::IStreamReader& fin,
                FCookProgress progress) {}

  void flattenDependenciesYAML(athena::io::IStreamReader& fin, std::vector<hecl::ProjectPath>& pathsOut) {}

  void flattenDependenciesANCSYAML(athena::io::IStreamReader& fin, std::vector<hecl::ProjectPath>& pathsOut,
                                   int charIdx) {}

  void cookAudioGroup(const hecl::ProjectPath& out, const hecl::ProjectPath& in, FCookProgress progress) {
    DNAMP2::AGSC::Cook(in, out);
    progress(_SYS_STR("Done"));
  }

  void cookSong(const hecl::ProjectPath& out, const hecl::ProjectPath& in, FCookProgress progress) {
    DNAMP1::CSNG::Cook(in, out);
    progress(_SYS_STR("Done"));
  }

  void cookMapArea(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds,
                   hecl::blender::Token& btok, FCookProgress progress) {
    hecl::blender::MapArea mapa = ds.compileMapArea();
    ds.close();
    DNAMP2::MAPA::Cook(mapa, out);
    progress(_SYS_STR("Done"));
  }

  void cookMapUniverse(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds,
                       hecl::blender::Token& btok, FCookProgress progress) {
    hecl::blender::MapUniverse mapu = ds.compileMapUniverse();
    ds.close();
    DNAMAPU::MAPU::Cook(mapu, out);
    progress(_SYS_STR("Done"));
  }

  UniqueID32 newToOriginal(urde::CAssetId id) const {
    UniqueID32 origId = m_idRestorer.newToOriginal({uint32_t(id.Value()), true});
    if (origId.isValid())
      return {origId.toUint32(), true};
    return {uint32_t(id.Value()), true};
  }

  urde::CAssetId originalToNew(UniqueID32 id) const {
    UniqueID32 newId = m_idRestorer.originalToNew(id);
    if (newId.isValid())
      return newId.toUint32();
    return id.toUint32();
  }
};

hecl::Database::DataSpecEntry SpecEntMP2(
    _SYS_STR("MP2"sv), _SYS_STR("Data specification for original Metroid Prime 2 engine"sv), _SYS_STR(".pak"sv),
    [](hecl::Database::Project& project, hecl::Database::DataSpecTool) -> std::unique_ptr<hecl::Database::IDataSpec> {
      return std::make_unique<SpecMP2>(&SpecEntMP2, project, false);
    });

hecl::Database::DataSpecEntry SpecEntMP2PC = {
    _SYS_STR("MP2-PC"sv), _SYS_STR("Data specification for PC-optimized Metroid Prime 2 engine"sv), _SYS_STR(".upak"sv),
    [](hecl::Database::Project& project,
       hecl::Database::DataSpecTool tool) -> std::unique_ptr<hecl::Database::IDataSpec> {
      if (tool != hecl::Database::DataSpecTool::Extract)
        return std::make_unique<SpecMP2>(&SpecEntMP2PC, project, true);
      return {};
    }};

hecl::Database::DataSpecEntry SpecEntMP2ORIG = {
    _SYS_STR("MP2-ORIG"sv), _SYS_STR("Data specification for unmodified Metroid Prime 2 resources"sv), {}, {}};

} // namespace DataSpec
