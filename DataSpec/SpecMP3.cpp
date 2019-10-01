#include <utility>
#include <set>

#include "SpecBase.hpp"
#include "DNAMP3/DNAMP3.hpp"

#include "DNAMP3/MLVL.hpp"
#include "DNAMP3/STRG.hpp"
#include "DNAMP3/MAPA.hpp"
#include "DNAMP2/STRG.hpp"

#include "hecl/ClientProcess.hpp"
#include "hecl/Blender/Connection.hpp"
#include "hecl/MultiProgressPrinter.hpp"

#include "Runtime/RetroTypes.hpp"
#include "nod/nod.hpp"

namespace DataSpec {

using namespace std::literals;

static logvisor::Module Log("urde::SpecMP3");
extern hecl::Database::DataSpecEntry SpecEntMP3;
extern hecl::Database::DataSpecEntry SpecEntMP3ORIG;

struct SpecMP3 : SpecBase {
  bool checkStandaloneID(const char* id) const override {
    if (!memcmp(id, "RM3", 3))
      return true;
    return false;
  }

  bool doMP3 = false;
  std::vector<const nod::Node*> m_nonPaks;
  std::vector<DNAMP3::PAKBridge> m_paks;
  std::map<std::string, DNAMP3::PAKBridge*, hecl::CaseInsensitiveCompare> m_orderedPaks;

  hecl::ProjectPath m_workPath;
  hecl::ProjectPath m_cookPath;
  PAKRouter<DNAMP3::PAKBridge> m_pakRouter;

  /* These are populated when extracting MPT's frontend (uses MP3's DataSpec) */
  bool doMPTFE = false;
  std::vector<const nod::Node*> m_feNonPaks;
  std::vector<DNAMP3::PAKBridge> m_fePaks;
  std::map<std::string, DNAMP3::PAKBridge*, hecl::CaseInsensitiveCompare> m_feOrderedPaks;

  hecl::ProjectPath m_feWorkPath;
  hecl::ProjectPath m_feCookPath;
  PAKRouter<DNAMP3::PAKBridge> m_fePakRouter;

  SpecMP3(const hecl::Database::DataSpecEntry* specEntry, hecl::Database::Project& project, bool pc)
  : SpecBase(specEntry, project, pc)
  , m_workPath(project.getProjectWorkingPath(), _SYS_STR("MP3"))
  , m_cookPath(project.getProjectCookedPath(SpecEntMP3), _SYS_STR("MP3"))
  , m_pakRouter(*this, m_workPath, m_cookPath)
  , m_feWorkPath(project.getProjectWorkingPath(), _SYS_STR("fe"))
  , m_feCookPath(project.getProjectCookedPath(SpecEntMP3), _SYS_STR("fe"))
  , m_fePakRouter(*this, m_feWorkPath, m_feCookPath) {
    setThreadProject();
  }

  void buildPaks(nod::Node& root, const std::vector<hecl::SystemString>& args, ExtractReport& rep, bool fe) {
    if (fe) {
      m_feNonPaks.clear();
      m_fePaks.clear();
    } else {
      m_nonPaks.clear();
      m_paks.clear();
    }
    for (const nod::Node& child : root) {
      bool isPak = false;
      auto name = child.getName();
      std::string lowerName(name);
      std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), tolower);
      if (name.size() > 4) {
        std::string::iterator extit = lowerName.end() - 4;
        if (std::string(extit, lowerName.end()) == ".pak") {
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

          if (fe)
            m_fePaks.emplace_back(child, good);
          else
            m_paks.emplace_back(child, good);
        }
      }

      if (!isPak) {
        if (fe)
          m_feNonPaks.push_back(&child);
        else
          m_nonPaks.push_back(&child);
      }
    }

    /* Sort PAKs alphabetically */
    if (fe) {
      m_feOrderedPaks.clear();
      for (DNAMP3::PAKBridge& dpak : m_fePaks)
        m_feOrderedPaks[std::string(dpak.getName())] = &dpak;
    } else {
      m_orderedPaks.clear();
      for (DNAMP3::PAKBridge& dpak : m_paks)
        m_orderedPaks[std::string(dpak.getName())] = &dpak;
    }

    /* Assemble extract report */
    for (const std::pair<std::string, DNAMP3::PAKBridge*>& item : fe ? m_feOrderedPaks : m_orderedPaks) {
      if (!item.second->m_doExtract)
        continue;
      rep.childOpts.emplace_back();
      ExtractReport& childRep = rep.childOpts.back();
      hecl::SystemStringConv nameView(item.first);
      childRep.name = hecl::SystemString(nameView.sys_str());
      if (item.first == "Worlds.pak")
        continue;
      else if (item.first == "Metroid6.pak") {
        /* Phaaze doesn't have a world name D: */
        childRep.desc = _SYS_STR("Phaaze");
        continue;
      } else if (item.first == "Metroid8.pak") {
        /* Space world is misnamed */
        childRep.desc = _SYS_STR("Space");
        continue;
      }
      childRep.desc = item.second->getLevelString();
    }
  }

  bool checkFromStandaloneDisc(nod::DiscBase& disc, const hecl::SystemString& regstr,
                               const std::vector<hecl::SystemString>& args, std::vector<ExtractReport>& reps) override {
    doMP3 = true;
    nod::IPartition* partition = disc.getDataPartition();
    std::unique_ptr<uint8_t[]> dolBuf = partition->getDOLBuf();
    const char* buildInfo = (char*)memmem(dolBuf.get(), partition->getDOLSize(), "MetroidBuildInfo", 16) + 19;
    if (!buildInfo)
      return false;

    /* We don't want no stinking demo dammit */
    if (!strcmp(buildInfo, "Build v3.068 3/2/2006 14:55:13"))
      return false;

    /* Root Report */
    reps.emplace_back();
    ExtractReport& rep = reps.back();
    rep.name = _SYS_STR("MP3");
    rep.desc = _SYS_STR("Metroid Prime 3 ") + regstr;
    std::string buildStr(buildInfo);
    hecl::SystemStringConv buildView(buildStr);
    rep.desc += _SYS_STR(" (") + buildView + _SYS_STR(")");

    /* Iterate PAKs and build level options */
    nod::Node& root = partition->getFSTRoot();
    buildPaks(root, args, rep, false);

    return true;
  }

  bool checkFromTrilogyDisc(nod::DiscBase& disc, const hecl::SystemString& regstr,
                            const std::vector<hecl::SystemString>& args, std::vector<ExtractReport>& reps) override {
    std::vector<hecl::SystemString> mp3args;
    std::vector<hecl::SystemString> feargs;
    if (args.size()) {
      /* Needs filter */
      for (const hecl::SystemString& arg : args) {
        hecl::SystemString lowerArg = arg;
        hecl::ToLower(lowerArg);
        if (!lowerArg.compare(0, 3, _SYS_STR("mp3"))) {
          doMP3 = true;
          mp3args.reserve(args.size());
          size_t slashPos = arg.find(_SYS_STR('/'));
          if (slashPos == hecl::SystemString::npos)
            slashPos = arg.find(_SYS_STR('\\'));
          if (slashPos != hecl::SystemString::npos)
            mp3args.emplace_back(hecl::SystemString(arg.begin() + slashPos + 1, arg.end()));
        }
      }

      for (const hecl::SystemString& arg : args) {
        hecl::SystemString lowerArg = arg;
        hecl::ToLower(lowerArg);
        if (!lowerArg.compare(0, 2, _SYS_STR("fe"))) {
          doMPTFE = true;
          feargs.reserve(args.size());
          size_t slashPos = arg.find(_SYS_STR('/'));
          if (slashPos == hecl::SystemString::npos)
            slashPos = arg.find(_SYS_STR('\\'));
          if (slashPos != hecl::SystemString::npos)
            feargs.emplace_back(hecl::SystemString(arg.begin() + slashPos + 1, arg.end()));
        }
      }
    } else {
      doMP3 = true;
      doMPTFE = true;
    }

    if (!doMP3 && !doMPTFE)
      return false;

    nod::IPartition* partition = disc.getDataPartition();
    nod::Node& root = partition->getFSTRoot();

    /* MP3 extract */
    while (doMP3) {
      nod::Node::DirectoryIterator dolIt = root.find("rs5mp3_p.dol");
      if (dolIt == root.end()) {
        doMP3 = false;
        break;
      }

      std::unique_ptr<uint8_t[]> dolBuf = dolIt->getBuf();
      const char* buildInfo = (char*)memmem(dolBuf.get(), dolIt->size(), "MetroidBuildInfo", 16) + 19;

      if (!buildInfo) {
        doMP3 = false;
        break;
      }

      /* We don't want no stinking demo dammit */
      if (!strcmp(buildInfo, "Build v3.068 3/2/2006 14:55:13")) {
        doMP3 = false;
        break;
      }

      /* Root Report */
      reps.emplace_back();
      ExtractReport& rep = reps.back();
      rep.name = _SYS_STR("MP3");
      rep.desc = _SYS_STR("Metroid Prime 3 ") + regstr;

      std::string buildStr(buildInfo);
      hecl::SystemStringConv buildView(buildStr);
      rep.desc += _SYS_STR(" (") + buildView + _SYS_STR(")");

      /* Iterate PAKs and build level options */
      nod::Node::DirectoryIterator mp3It = root.find("MP3");
      if (mp3It == root.end()) {
        doMP3 = false;
        break;
      }
      buildPaks(*mp3It, mp3args, rep, false);
      break;
    }

    /* MPT Frontend extract */
    while (doMPTFE) {
      nod::Node::DirectoryIterator dolIt = root.find("rs5fe_p.dol");
      if (dolIt == root.end()) {
        doMPTFE = false;
        break;
      }

      std::unique_ptr<uint8_t[]> dolBuf = dolIt->getBuf();
      const char* buildInfo = (char*)memmem(dolBuf.get(), dolIt->size(), "MetroidBuildInfo", 16) + 19;

      /* Root Report */
      reps.emplace_back();
      ExtractReport& rep = reps.back();
      rep.name = _SYS_STR("fe");
      rep.desc = _SYS_STR("Metroid Prime Trilogy Frontend ") + regstr;
      if (buildInfo) {
        std::string buildStr(buildInfo);
        hecl::SystemStringConv buildView(buildStr);
        rep.desc += _SYS_STR(" (") + buildView + _SYS_STR(")");
      }

      /* Iterate PAKs and build level options */
      nod::Node::DirectoryIterator feIt = root.find("fe");
      if (feIt == root.end()) {
        doMPTFE = false;
        break;
      }
      buildPaks(*feIt, feargs, rep, true);
      break;
    }

    return doMP3 || doMPTFE;
  }

  bool extractFromDisc(nod::DiscBase& disc, bool force, const hecl::MultiProgressPrinter& progress) override {
    hecl::SystemString currentTarget = _SYS_STR("");
    size_t nodeCount = 0;
    int prog = 0;
    nod::ExtractionContext ctx = {force, [&](std::string_view name, float) {
                                    hecl::SystemStringConv nameView(name);
                                    progress.print(currentTarget.c_str(), nameView.c_str(), prog / (float)nodeCount);
                                  }};
    if (doMP3) {
      m_workPath.makeDir();

      progress.startNewLine();
      progress.print(_SYS_STR("Indexing PAKs"), _SYS_STR(""), 0.0);
      m_pakRouter.build(m_paks,
                        [&progress](float factor) { progress.print(_SYS_STR("Indexing PAKs"), _SYS_STR(""), factor); });
      progress.print(_SYS_STR("Indexing PAKs"), _SYS_STR(""), 1.0);
      progress.startNewLine();

      hecl::ProjectPath outPath(m_project.getProjectWorkingPath(), _SYS_STR("out"));
      outPath.makeDir();
      disc.getDataPartition()->extractSysFiles(outPath.getAbsolutePath(), ctx);
      hecl::ProjectPath mp3OutPath(outPath, m_standalone ? _SYS_STR("files") : _SYS_STR("files/MP3"));
      mp3OutPath.makeDirChain(true);

      currentTarget = _SYS_STR("MP3 Root");
      progress.print(currentTarget.c_str(), _SYS_STR(""), 0.0);
      prog = 0;

      nodeCount = m_nonPaks.size();
      // TODO: Make this more granular
      for (const nod::Node* node : m_nonPaks) {
        node->extractToDirectory(mp3OutPath.getAbsolutePath(), ctx);
        prog++;
      }
      ctx.progressCB = nullptr;

      progress.print(currentTarget.c_str(), _SYS_STR(""), 1.0);
      progress.startNewLine();

      hecl::ClientProcess process;
      for (std::pair<const std::string, DNAMP3::PAKBridge*>& pair : m_orderedPaks) {
        DNAMP3::PAKBridge& pak = *pair.second;
        if (!pak.m_doExtract)
          continue;

        auto name = pak.getName();
        hecl::SystemStringConv sysName(name);

        auto pakName = hecl::SystemString(sysName.sys_str());
        process.addLambdaTransaction([this, &progress, &pak, pakName, force](hecl::blender::Token& btok) {
          m_pakRouter.extractResources(pak, force, btok,
                                       [&progress, &pakName](const hecl::SystemChar* substr, float factor) {
                                         progress.print(pakName.c_str(), substr, factor);
                                       });
        });
      }

      process.waitUntilComplete();
    }

    if (doMPTFE) {
      m_feWorkPath.makeDir();

      progress.startNewLine();
      progress.print(_SYS_STR("Indexing PAKs"), _SYS_STR(""), 0.0);
      m_fePakRouter.build(
          m_fePaks, [&progress](float factor) { progress.print(_SYS_STR("Indexing PAKs"), _SYS_STR(""), factor); });
      progress.print(_SYS_STR("Indexing PAKs"), _SYS_STR(""), 1.0);
      progress.startNewLine();

      hecl::ProjectPath outPath(m_project.getProjectWorkingPath(), _SYS_STR("out"));
      outPath.makeDir();
      disc.getDataPartition()->extractSysFiles(outPath.getAbsolutePath(), ctx);
      hecl::ProjectPath feOutPath(outPath, m_standalone ? _SYS_STR("files") : _SYS_STR("files/fe"));
      feOutPath.makeDirChain(true);

      currentTarget = _SYS_STR("fe Root");
      progress.print(currentTarget.c_str(), _SYS_STR(""), 0.0);
      prog = 0;
      nodeCount = m_feNonPaks.size();

      // TODO: Make this more granular
      for (const nod::Node* node : m_feNonPaks) {
        node->extractToDirectory(feOutPath.getAbsolutePath(), ctx);
        prog++;
      }
      progress.print(currentTarget.c_str(), _SYS_STR(""), 1.0);
      progress.startNewLine();

      hecl::ClientProcess process;
      for (std::pair<std::string, DNAMP3::PAKBridge*> pair : m_feOrderedPaks) {
        DNAMP3::PAKBridge& pak = *pair.second;
        if (!pak.m_doExtract)
          continue;

        auto name = pak.getName();
        hecl::SystemStringConv sysName(name);

        hecl::SystemString pakName(sysName.sys_str());
        process.addLambdaTransaction([this, &progress, &pak, pakName, force](hecl::blender::Token& btok) {
          m_fePakRouter.extractResources(pak, force, btok,
                                         [&progress, &pakName](const hecl::SystemChar* substr, float factor) {
                                           progress.print(pakName.c_str(), substr, factor);
                                         });
        });
      }

      process.waitUntilComplete();
    }
    return true;
  }

  const hecl::Database::DataSpecEntry& getOriginalSpec() const override { return SpecEntMP3; }

  const hecl::Database::DataSpecEntry& getUnmodifiedSpec() const override { return SpecEntMP3ORIG; }

  hecl::ProjectPath getWorking(class UniqueID64& id) override { return m_pakRouter.getWorking(id); }

  bool checkPathPrefix(const hecl::ProjectPath& path) const override {
    return path.getRelativePath().compare(0, 4, _SYS_STR("MP3/")) == 0;
  }

  bool validateYAMLDNAType(athena::io::IStreamReader& fp) const override {
    if (athena::io::ValidateFromYAMLStream<DNAMP3::MLVL>(fp))
      return true;
    if (athena::io::ValidateFromYAMLStream<DNAMP3::STRG>(fp))
      return true;
    if (athena::io::ValidateFromYAMLStream<DNAMP2::STRG>(fp))
      return true;
    return false;
  }

  urde::SObjectTag buildTagFromPath(const hecl::ProjectPath& path) const override { return {}; }

  void cookMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                hecl::blender::Token& btok, FCookProgress progress) override {}

  void cookColMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                   hecl::blender::Token& btok, FCookProgress progress) override {}

  void cookArmature(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                    hecl::blender::Token& btok, FCookProgress progress) override {}

  void cookPathMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                    hecl::blender::Token& btok, FCookProgress progress) override {}

  void cookActor(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                 hecl::blender::Token& btok, FCookProgress progress) override {}

  void cookArea(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                hecl::blender::Token& btok, FCookProgress progress) override {}

  void cookWorld(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                 hecl::blender::Token& btok, FCookProgress progress) override {}

  void cookGuiFrame(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds,
                    hecl::blender::Token& btok, FCookProgress progress) override {}

  void cookYAML(const hecl::ProjectPath& out, const hecl::ProjectPath& in, athena::io::IStreamReader& fin,
                hecl::blender::Token& btok, FCookProgress progress) override {}

  void flattenDependenciesYAML(athena::io::IStreamReader& fin, std::vector<hecl::ProjectPath>& pathsOut) override {}

  void flattenDependenciesANCSYAML(athena::io::IStreamReader& fin, std::vector<hecl::ProjectPath>& pathsOut,
                                   int charIdx) override {}

  void cookAudioGroup(const hecl::ProjectPath& out, const hecl::ProjectPath& in, FCookProgress progress) override {}

  void cookSong(const hecl::ProjectPath& out, const hecl::ProjectPath& in, FCookProgress progress) override {}

  void cookMapArea(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds,
                   hecl::blender::Token& btok, FCookProgress progress) override {
    hecl::blender::MapArea mapa = ds.compileMapArea();
    ds.close();
    DNAMP3::MAPA::Cook(mapa, out);
    progress(_SYS_STR("Done"));
  }

  void cookMapUniverse(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds,
                       hecl::blender::Token& btok, FCookProgress progress) override {}
};

hecl::Database::DataSpecEntry SpecEntMP3(
    _SYS_STR("MP3"sv), _SYS_STR("Data specification for original Metroid Prime 3 engine"sv), _SYS_STR(".pak"sv),
    [](hecl::Database::Project& project, hecl::Database::DataSpecTool) -> std::unique_ptr<hecl::Database::IDataSpec> {
      return std::make_unique<SpecMP3>(&SpecEntMP3, project, false);
    });

hecl::Database::DataSpecEntry SpecEntMP3PC = {
    _SYS_STR("MP3-PC"sv), _SYS_STR("Data specification for PC-optimized Metroid Prime 3 engine"sv), _SYS_STR(".upak"sv),
    [](hecl::Database::Project& project,
       hecl::Database::DataSpecTool tool) -> std::unique_ptr<hecl::Database::IDataSpec> {
      if (tool != hecl::Database::DataSpecTool::Extract)
        return std::make_unique<SpecMP3>(&SpecEntMP3PC, project, true);
      return nullptr;
    }};

hecl::Database::DataSpecEntry SpecEntMP3ORIG = {
    _SYS_STR("MP3-ORIG"sv), _SYS_STR("Data specification for unmodified Metroid Prime 3 resources"sv), {}, {}};

} // namespace DataSpec
