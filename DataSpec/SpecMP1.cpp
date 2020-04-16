#include <utility>
#include <cstdio>
#include <cstring>

#include "SpecBase.hpp"
#include "DNAMP1/DNAMP1.hpp"

#include "DNAMP1/HINT.hpp"
#include "DNAMP1/MLVL.hpp"
#include "DNAMP1/STRG.hpp"
#include "DNAMP1/SCAN.hpp"
#include "DNAMP1/CMDL.hpp"
#include "DNAMP1/DCLN.hpp"
#include "DNAMP1/MREA.hpp"
#include "DNAMP1/ANCS.hpp"
#include "DNAMP1/AGSC.hpp"
#include "DNAMP1/CSNG.hpp"
#include "DNAMP1/MAPA.hpp"
#include "DNAMP1/PATH.hpp"
#include "DNAMP1/FRME.hpp"
#include "DNAMP1/AFSM.hpp"
#include "DNACommon/ATBL.hpp"
#include "DNACommon/FONT.hpp"
#include "DNACommon/PART.hpp"
#include "DNACommon/SWHC.hpp"
#include "DNACommon/ELSC.hpp"
#include "DNACommon/WPSC.hpp"
#include "DNACommon/CRSC.hpp"
#include "DNACommon/DPSC.hpp"
#include "DNACommon/DGRP.hpp"
#include "DNACommon/MAPU.hpp"
#include "DNACommon/URDEVersionInfo.hpp"
#include "DNACommon/Tweaks/TweakWriter.hpp"
#include "DNAMP1/Tweaks/CTweakPlayerRes.hpp"
#include "DNAMP1/Tweaks/CTweakGunRes.hpp"
#include "DNAMP1/Tweaks/CTweakSlideShow.hpp"
#include "DNAMP1/Tweaks/CTweakPlayer.hpp"
#include "DNAMP1/Tweaks/CTweakCameraBob.hpp"
#include "DNAMP1/Tweaks/CTweakGame.hpp"
#include "DNAMP1/Tweaks/CTweakTargeting.hpp"
#include "DNAMP1/Tweaks/CTweakAutoMapper.hpp"
#include "DNAMP1/Tweaks/CTweakGui.hpp"
#include "DNAMP1/Tweaks/CTweakPlayerControl.hpp"
#include "DNAMP1/Tweaks/CTweakBall.hpp"
#include "DNAMP1/Tweaks/CTweakParticle.hpp"
#include "DNAMP1/Tweaks/CTweakGuiColors.hpp"
#include "DNAMP1/Tweaks/CTweakPlayerGun.hpp"
#include "DNAMP1/MazeSeeds.hpp"
#include "DNAMP1/SnowForces.hpp"
#include "hecl/ClientProcess.hpp"
#include "hecl/MultiProgressPrinter.hpp"
#include "hecl/Blender/Connection.hpp"
#include "hecl/Blender/SDNARead.hpp"
#include "nod/nod.hpp"

namespace DataSpec {

using namespace std::literals;

static logvisor::Module Log("urde::SpecMP1");
extern hecl::Database::DataSpecEntry SpecEntMP1;
extern hecl::Database::DataSpecEntry SpecEntMP1PC;
extern hecl::Database::DataSpecEntry SpecEntMP1ORIG;

struct TextureCache {
  static void Generate(PAKRouter<DNAMP1::PAKBridge>& pakRouter, hecl::Database::Project& project,
                       const hecl::ProjectPath& pakPath) {
    hecl::ProjectPath texturePath(pakPath, _SYS_STR("texture_cache.yaml"));
    hecl::ProjectPath catalogPath(pakPath, _SYS_STR("!catalog.yaml"));
    texturePath.makeDirChain(false);

    if (const auto fp = hecl::FopenUnique(catalogPath.getAbsolutePath().data(), _SYS_STR("a"))) {
      fmt::print(fp.get(), FMT_STRING("TextureCache: {}\n"), texturePath.getRelativePathUTF8());
    }

    Log.report(logvisor::Level::Info, FMT_STRING("Gathering Texture metadata (this can take up to 10 seconds)..."));
    std::unordered_map<UniqueID32, TXTR::Meta> metaMap;

    pakRouter.enumerateResources([&](const DNAMP1::PAK::Entry* ent) {
      if (ent->type == FOURCC('TXTR') && metaMap.find(ent->id) == metaMap.end()) {
        PAKEntryReadStream rs = pakRouter.beginReadStreamForId(ent->id);
        metaMap[ent->id] = TXTR::GetMetaData(rs);
      }
      return true;
    });

    athena::io::YAMLDocWriter yamlW("MP1TextureCache");
    for (const auto& pair : metaMap) {
      hecl::ProjectPath path = pakRouter.getWorking(pair.first);
      auto rec = yamlW.enterSubRecord(path.getRelativePathUTF8());
      pair.second.write(yamlW);
    }

    athena::io::FileWriter fileW(texturePath.getAbsolutePath());
    yamlW.finish(&fileW);
    Log.report(logvisor::Level::Info, FMT_STRING("Done..."));
  }

  static void Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath) {
    hecl::Database::Project& project = inPath.getProject();
    athena::io::YAMLDocReader r;
    athena::io::FileReader fr(inPath.getAbsolutePath());
    if (!fr.isOpen() || !r.parse(&fr))
      return;

    std::vector<std::pair<UniqueID32, TXTR::Meta>> metaPairs;
    metaPairs.reserve(r.getRootNode()->m_mapChildren.size());
    for (const auto& node : r.getRootNode()->m_mapChildren) {
      hecl::ProjectPath projectPath(project, node.first);
      auto rec = r.enterSubRecord(node.first.c_str());
      TXTR::Meta meta;
      meta.read(r);
      metaPairs.emplace_back(projectPath.parsedHash32(), meta);
    }

    std::sort(metaPairs.begin(), metaPairs.end(),
              [](const auto& a, const auto& b) -> bool { return a.first < b.first; });

    athena::io::FileWriter w(outPath.getAbsolutePath());
    w.writeUint32Big(metaPairs.size());
    for (const auto& pair : metaPairs) {
      pair.first.write(w);
      pair.second.write(w);
    }
  }
};

struct SpecMP1 : SpecBase {
  bool checkStandaloneID(const char* id) const override { return !memcmp(id, "GM8", 3); }

  std::vector<const nod::Node*> m_nonPaks;
  std::vector<DNAMP1::PAKBridge> m_paks;
  std::map<std::string, DNAMP1::PAKBridge*, hecl::CaseInsensitiveCompare> m_orderedPaks;

  hecl::ProjectPath m_workPath;
  hecl::ProjectPath m_cookPath;
  PAKRouter<DNAMP1::PAKBridge> m_pakRouter;

  std::unique_ptr<uint8_t[]> m_dolBuf;

  std::unordered_map<hecl::Hash, hecl::blender::Matrix4f> m_mreaPathToXF;

  SpecMP1(const hecl::Database::DataSpecEntry* specEntry, hecl::Database::Project& project, bool pc)
  : SpecBase(specEntry, project, pc)
  , m_workPath(project.getProjectWorkingPath(), _SYS_STR("MP1"))
  , m_cookPath(project.getProjectCookedPath(SpecEntMP1), _SYS_STR("MP1"))
  , m_pakRouter(*this, m_workPath, m_cookPath) {
    m_game = EGame::MetroidPrime1;
    SpecBase::setThreadProject();
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

          m_paks.emplace_back(child, good);
        }
      }

      if (!isPak)
        m_nonPaks.push_back(&child);
    }

    /* Sort PAKs alphabetically */
    m_orderedPaks.clear();
    for (DNAMP1::PAKBridge& dpak : m_paks) {
      m_orderedPaks[std::string(dpak.getName())] = &dpak;
    }

    /* Assemble extract report */
    rep.childOpts.reserve(m_orderedPaks.size());
    for (const auto& item : m_orderedPaks) {
      if (!item.second->m_doExtract) {
        continue;
      }

      ExtractReport& childRep = rep.childOpts.emplace_back();
      hecl::SystemStringConv nameView(item.first);
      childRep.name = nameView.sys_str();
      childRep.desc = item.second->getLevelString();
    }
  }

  bool checkFromStandaloneDisc(nod::DiscBase& disc, const hecl::SystemString& regstr,
                               const std::vector<hecl::SystemString>& args, std::vector<ExtractReport>& reps) override {
    nod::IPartition* partition = disc.getDataPartition();
    m_dolBuf = partition->getDOLBuf();
    const char* buildInfo =
        static_cast<char*>(memmem(m_dolBuf.get(), partition->getDOLSize(), "MetroidBuildInfo", 16)) + 19;

    if (buildInfo == nullptr)
      return false;

    m_version = std::string(buildInfo);
    /* Root Report */
    ExtractReport& rep = reps.emplace_back();
    rep.name = _SYS_STR("MP1");
    rep.desc = _SYS_STR("Metroid Prime ") + regstr;
    if (buildInfo) {
      hecl::SystemStringConv buildView(m_version);
      rep.desc += _SYS_STR(" (") + buildView + _SYS_STR(")");
    }

    /* Iterate PAKs and build level options */
    nod::Node& root = partition->getFSTRoot();
    buildPaks(root, args, rep);

    return true;
  }

  bool checkFromTrilogyDisc(nod::DiscBase& disc, const hecl::SystemString& regstr,
                            const std::vector<hecl::SystemString>& args, std::vector<ExtractReport>& reps) override {
    std::vector<hecl::SystemString> mp1args;
    bool doExtract = false;
    if (args.size()) {
      /* Needs filter */
      for (const hecl::SystemString& arg : args) {
        hecl::SystemString lowerArg = arg;
        hecl::ToLower(lowerArg);
        if (!lowerArg.compare(0, 3, _SYS_STR("mp1"))) {
          doExtract = true;
          mp1args.reserve(args.size());
          size_t slashPos = arg.find(_SYS_STR('/'));
          if (slashPos == hecl::SystemString::npos)
            slashPos = arg.find(_SYS_STR('\\'));
          if (slashPos != hecl::SystemString::npos)
            mp1args.emplace_back(hecl::SystemString(arg.begin() + slashPos + 1, arg.end()));
        }
      }
    } else
      doExtract = true;

    if (!doExtract)
      return false;

    nod::IPartition* partition = disc.getDataPartition();
    nod::Node& root = partition->getFSTRoot();
    nod::Node::DirectoryIterator dolIt = root.find("rs5mp1_p.dol");
    if (dolIt == root.end()) {
      dolIt = root.find("rs5mp1jpn_p.dol");
      if (dolIt == root.end())
        return false;
    }

    m_dolBuf = dolIt->getBuf();
    const char* buildInfo = static_cast<char*>(memmem(m_dolBuf.get(), dolIt->size(), "MetroidBuildInfo", 16)) + 19;

    /* Root Report */
    ExtractReport& rep = reps.emplace_back();
    rep.name = _SYS_STR("MP1");
    rep.desc = _SYS_STR("Metroid Prime ") + regstr;
    if (buildInfo != nullptr) {
      m_version = std::string(buildInfo);
      hecl::SystemStringConv buildView(m_version);
      rep.desc += _SYS_STR(" (") + buildView + _SYS_STR(")");
    }

    /* Iterate PAKs and build level options */
    nod::Node::DirectoryIterator mp1It = root.find("MP1");
    if (mp1It == root.end()) {
      mp1It = root.find("MP1JPN");
      if (mp1It == root.end())
        return false;
    }
    buildPaks(*mp1It, mp1args, rep);

    return true;
  }

  bool extractFromDisc(nod::DiscBase& disc, bool force, const hecl::MultiProgressPrinter& progress) override {
    m_project.enableDataSpecs({_SYS_STR("MP1-PC")});

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
    hecl::ProjectPath mp1OutPath(outPath, m_standalone ? _SYS_STR("files") : _SYS_STR("files/MP1"));
    mp1OutPath.makeDirChain(true);

    /* Extract non-pak files */
    progress.startNewLine();
    progress.print(_SYS_STR("MP1 Root"), _SYS_STR(""), 0.0);
    int prog = 0;
    ctx.progressCB = [&](std::string_view name, float) {
      hecl::SystemStringConv nameView(name);
      progress.print(_SYS_STR("MP1 Root"), nameView.c_str(), prog / (float)m_nonPaks.size());
    };
    for (const nod::Node* node : m_nonPaks) {
      node->extractToDirectory(mp1OutPath.getAbsolutePath(), ctx);
      prog++;
    }
    progress.print(_SYS_STR("MP1 Root"), _SYS_STR(""), 1.0);

    /* Extract unique resources */
    hecl::ClientProcess process;
    progress.startNewLine();
    for (std::pair<const std::string, DNAMP1::PAKBridge*>& pair : m_orderedPaks) {
#if 0
      const DNAMP1::PAK::Entry* ent = pair.second->getPAK().lookupEntry(UniqueID32(0xE39FC9A1));
      if (ent) {
        PAKEntryReadStream rs = ent->beginReadStream(pair.second->getNode());
        DNAMP1::CMDL::Extract(*this, rs, hecl::ProjectPath(m_project.getProjectWorkingPath(), "MP1/Metroid1/!1IntroLevel1027/CMDL_E39FC9A1.blend"), m_pakRouter, *ent, true, hecl::blender::SharedBlenderToken, {});

        ent = pair.second->getPAK().lookupEntry(UniqueID32(0xC1AE2B4A));
        rs = ent->beginReadStream(pair.second->getNode());
        DNAMP1::CMDL::Extract(*this, rs, hecl::ProjectPath(m_project.getProjectWorkingPath(), "MP1/Metroid1/!1IntroLevel1027/CMDL_C1AE2B4A.blend"), m_pakRouter, *ent, true, hecl::blender::SharedBlenderToken, {});

        exit(0);
      }
      else
        continue;
#endif

      DNAMP1::PAKBridge& pak = *pair.second;
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

    /* Extract part of .dol for RandomStatic entropy */
    hecl::ProjectPath noAramPath(m_project.getProjectWorkingPath(), _SYS_STR("MP1/URDE"));
    extractRandomStaticEntropy(m_dolBuf.get() + 0x4f60, noAramPath);

    /* Generate Texture Cache containing meta data for every texture file */
    TextureCache::Generate(m_pakRouter, m_project, noAramPath);

    /* Write version data */
    hecl::ProjectPath versionPath;
    if (m_standalone) {
      versionPath = hecl::ProjectPath(m_project.getProjectWorkingPath(), _SYS_STR("out/files"));
    } else {
      versionPath = hecl::ProjectPath(m_project.getProjectWorkingPath(), _SYS_STR("out/files/MP1"));
    }
    WriteVersionInfo(m_project, versionPath);
    return true;
  }

  const hecl::Database::DataSpecEntry& getOriginalSpec() const override { return SpecEntMP1; }

  const hecl::Database::DataSpecEntry& getUnmodifiedSpec() const override { return SpecEntMP1ORIG; }

  hecl::ProjectPath getWorking(class UniqueID32& id) override { return m_pakRouter.getWorking(id); }

  bool checkPathPrefix(const hecl::ProjectPath& path) const override {
    return path.getRelativePath().compare(0, 4, _SYS_STR("MP1/")) == 0;
  }

  bool validateYAMLDNAType(athena::io::IStreamReader& fp) const override {
    athena::io::YAMLDocReader reader;
    yaml_parser_set_input(reader.getParser(), (yaml_read_handler_t*)athena::io::YAMLAthenaReader, &fp);
    return reader.ClassTypeOperation([](std::string_view classType) {
      if (classType == DNAMP1::MLVL::DNAType())
        return true;
      else if (classType == DNAMP1::STRG::DNAType())
        return true;
      else if (classType == DNAMP1::SCAN::DNAType())
        return true;
      else if (classType == DNAParticle::GPSM<UniqueID32>::DNAType())
        return true;
      else if (classType == DNAParticle::SWSH<UniqueID32>::DNAType())
        return true;
      else if (classType == DNAParticle::ELSM<UniqueID32>::DNAType())
        return true;
      else if (classType == DNAParticle::WPSM<UniqueID32>::DNAType())
        return true;
      else if (classType == DNAParticle::CRSM<UniqueID32>::DNAType())
        return true;
      else if (classType == DNAParticle::DPSM<UniqueID32>::DNAType())
        return true;
      else if (classType == DNADGRP::DGRP<UniqueID32>::DNAType())
        return true;
      else if (classType == DNAFont::FONT<UniqueID32>::DNAType())
        return true;
      else if (classType == DNAMP1::CTweakPlayerRes<true>::DNAType())
        return true;
      else if (classType == DNAMP1::CTweakPlayerRes<false>::DNAType())
        return true;
      else if (classType == DNAMP1::CTweakGunRes::DNAType())
        return true;
      else if (classType == DNAMP1::CTweakSlideShow::DNAType())
        return true;
      else if (classType == DNAMP1::CTweakPlayer::DNAType())
        return true;
      else if (classType == DNAMP1::CTweakCameraBob::DNAType())
        return true;
      else if (classType == DNAMP1::CTweakGame::DNAType())
        return true;
      else if (classType == DNAMP1::CTweakAutoMapper::DNAType())
        return true;
      else if (classType == DNAMP1::CTweakTargeting<true>::DNAType())
        return true;
      else if (classType == DNAMP1::CTweakTargeting<false>::DNAType())
        return true;
      else if (classType == DNAMP1::CTweakGui::DNAType())
        return true;
      else if (classType == DNAMP1::CTweakPlayerControl::DNAType())
        return true;
      else if (classType == DNAMP1::CTweakBall::DNAType())
        return true;
      else if (classType == DNAMP1::CTweakParticle::DNAType())
        return true;
      else if (classType == DNAMP1::CTweakGuiColors::DNAType())
        return true;
      else if (classType == DNAMP1::CTweakPlayerGun::DNAType())
        return true;
      else if (classType == DNAMP1::HINT::DNAType())
        return true;
      else if (classType == DNAMP1::EVNT::DNAType())
        return true;
      else if (classType == DNAMP1::MazeSeeds::DNAType())
        return true;
      else if (classType == DNAMP1::SnowForces::DNAType())
        return true;
      else if (classType == "ATBL")
        return true;
      else if (classType == DNAMP1::AFSM::DNAType())
        return true;
      else if (classType == "MP1TextureCache")
        return true;
      return false;
    });
  }

  urde::SObjectTag buildTagFromPath(const hecl::ProjectPath& path) const override {
    if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _SYS_STR(".CSKR")))
      return {SBIG('CSKR'), path.parsedHash32()};
    else if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _SYS_STR(".ANIM")))
      return {SBIG('ANIM'), path.parsedHash32()};
    else if (const hecl::SystemChar* ext = path.getLastComponentExt().data()) {
      if (ext[0] == _SYS_STR('*') || !hecl::StrCmp(ext, _SYS_STR("mid"))) {
        if (path.getWithExtension(_SYS_STR(".mid"), true).isFile() &&
            path.getWithExtension(_SYS_STR(".yaml"), true).isFile()) {
          hecl::ProjectPath glob = path.getWithExtension(_SYS_STR(".*"), true);
          return {SBIG('CSNG'), glob.parsedHash32()};
        }
      }
    }

    if (path.getPathType() == hecl::ProjectPath::Type::Directory) {
      if (hecl::ProjectPath(path, _SYS_STR("!project.yaml")).isFile() &&
          hecl::ProjectPath(path, _SYS_STR("!pool.yaml")).isFile())
        return {SBIG('AGSC'), path.parsedHash32()};
    }

    hecl::ProjectPath asBlend;
    if (path.getPathType() == hecl::ProjectPath::Type::Glob)
      asBlend = path.getWithExtension(_SYS_STR(".blend"), true);
    else
      asBlend = path;

    if (hecl::IsPathBlend(asBlend)) {
      switch (hecl::blender::GetBlendType(asBlend.getAbsolutePath())) {
      case hecl::blender::BlendType::Mesh:
        return {SBIG('CMDL'), path.parsedHash32()};
      case hecl::blender::BlendType::ColMesh:
        return {SBIG('DCLN'), path.parsedHash32()};
      case hecl::blender::BlendType::Armature:
        return {SBIG('CINF'), path.parsedHash32()};
      case hecl::blender::BlendType::PathMesh:
        return {SBIG('PATH'), path.parsedHash32()};
      case hecl::blender::BlendType::Actor:
        if (path.getAuxInfo().size()) {
          if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _SYS_STR(".CSKR")))
            return {SBIG('CSKR'), path.getWithExtension(_SYS_STR(".*"), true).parsedHash32()};
          else if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _SYS_STR(".ANIM")))
            return {SBIG('ANIM'), path.getWithExtension(_SYS_STR(".*"), true).parsedHash32()};
        }
        return {SBIG('ANCS'), path.getWithExtension(_SYS_STR(".*"), true).parsedHash32()};
      case hecl::blender::BlendType::Area:
        return {SBIG('MREA'), path.parsedHash32()};
      case hecl::blender::BlendType::World:
        return {SBIG('MLVL'), path.getWithExtension(_SYS_STR(".*"), true).parsedHash32()};
      case hecl::blender::BlendType::MapArea:
        return {SBIG('MAPA'), path.parsedHash32()};
      case hecl::blender::BlendType::MapUniverse:
        return {SBIG('MAPU'), path.parsedHash32()};
      case hecl::blender::BlendType::Frame:
        return {SBIG('FRME'), path.parsedHash32()};
      default:
        return {};
      }
    } else if (hecl::IsPathPNG(path)) {
      return {SBIG('TXTR'), path.parsedHash32()};
    } else if (hecl::IsPathYAML(path)) {
      auto fp = hecl::FopenUnique(path.getAbsolutePath().data(), _SYS_STR("r"));
      if (fp == nullptr) {
        return {};
      }

      athena::io::YAMLDocReader reader;
      yaml_parser_set_input_file(reader.getParser(), fp.get());

      urde::SObjectTag resTag;
      if (reader.ClassTypeOperation([&](std::string_view className) {
            if (className == DNAParticle::GPSM<UniqueID32>::DNAType()) {
              resTag.type = SBIG('PART');
              return true;
            }
            if (className == DNAParticle::SWSH<UniqueID32>::DNAType()) {
              resTag.type = SBIG('SWHC');
              return true;
            }
            if (className == DNAParticle::ELSM<UniqueID32>::DNAType()) {
              resTag.type = SBIG('ELSC');
              return true;
            }
            if (className == DNAParticle::WPSM<UniqueID32>::DNAType()) {
              resTag.type = SBIG('WPSC');
              return true;
            }
            if (className == DNAParticle::CRSM<UniqueID32>::DNAType()) {
              resTag.type = SBIG('CRSC');
              return true;
            }
            if (className == DNAParticle::DPSM<UniqueID32>::DNAType()) {
              resTag.type = SBIG('DPSC');
              return true;
            } else if (className == DNAFont::FONT<UniqueID32>::DNAType()) {
              resTag.type = SBIG('FONT');
              return true;
            } else if (className == DNAMP1::EVNT::DNAType()) {
              resTag.type = SBIG('EVNT');
              return true;
            } else if (className == DNADGRP::DGRP<UniqueID32>::DNAType()) {
              resTag.type = SBIG('DGRP');
              return true;
            } else if (className == DataSpec::DNAMP1::STRG::DNAType()) {
              resTag.type = SBIG('STRG');
              return true;
            } else if (className == DataSpec::DNAMP1::SCAN::DNAType()) {
              resTag.type = SBIG('SCAN');
              return true;
            } else if (className == DataSpec::DNAMP1::CTweakPlayerRes<true>::DNAType() ||
                       className == DataSpec::DNAMP1::CTweakPlayerRes<false>::DNAType() ||
                       className == DataSpec::DNAMP1::CTweakGunRes::DNAType() ||
                       className == DataSpec::DNAMP1::CTweakSlideShow::DNAType() ||
                       className == DataSpec::DNAMP1::CTweakPlayer::DNAType() ||
                       className == DataSpec::DNAMP1::CTweakCameraBob::DNAType() ||
                       className == DataSpec::DNAMP1::CTweakGame::DNAType() ||
                       className == DataSpec::DNAMP1::CTweakTargeting<true>::DNAType() ||
                       className == DataSpec::DNAMP1::CTweakTargeting<false>::DNAType() ||
                       className == DataSpec::DNAMP1::CTweakAutoMapper::DNAType() ||
                       className == DataSpec::DNAMP1::CTweakGui::DNAType() ||
                       className == DataSpec::DNAMP1::CTweakPlayerControl::DNAType() ||
                       className == DataSpec::DNAMP1::CTweakBall::DNAType() ||
                       className == DataSpec::DNAMP1::CTweakParticle::DNAType() ||
                       className == DataSpec::DNAMP1::CTweakGuiColors::DNAType() ||
                       className == DataSpec::DNAMP1::CTweakPlayerGun::DNAType()) {
              resTag.type = SBIG('CTWK');
              return true;
            } else if (className == DataSpec::DNAMP1::MazeSeeds::DNAType() ||
                       className == DataSpec::DNAMP1::SnowForces::DNAType()) {
              resTag.type = SBIG('DUMB');
              return true;
            } else if (className == DataSpec::DNAMP1::HINT::DNAType()) {
              resTag.type = SBIG('HINT');
              return true;
            } else if (className == "ATBL") {
              resTag.type = SBIG('ATBL');
              return true;
            } else if (className == DataSpec::DNAMP1::AFSM::DNAType()) {
              resTag.type = SBIG('AFSM');
              return true;
            } else if (className == "MP1TextureCache") {
              resTag.type = SBIG('TMET');
              return true;
            } else if (className == "DataSpec::DNAMP1::SAVW") {
              resTag.type = SBIG('SAVW');
              return true;
            } else if (className == "DataSpec::DNAMP1::MAPW") {
              resTag.type = SBIG('MAPW');
              return true;
            }

            return false;
          })) {
        resTag.id = path.parsedHash32();
        fp.reset();
        return resTag;
      }
    }
    return {};
  }

  void getTagListForFile(const char* pakName, std::vector<urde::SObjectTag>& out) const override {
    std::string pathPrefix("MP1/");
    pathPrefix += pakName;
    pathPrefix += '/';

    std::unique_lock lk(m_backgroundIndexMutex);
    for (const auto& tag : m_tagToPath)
      if (!tag.second.getRelativePathUTF8().compare(0, pathPrefix.size(), pathPrefix))
        out.push_back(tag.first);
  }

  void cookMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                hecl::blender::Token& btok, FCookProgress progress) override {
    Mesh mesh = ds.compileMesh(fast ? hecl::HMDLTopology::Triangles : hecl::HMDLTopology::TriStrips, m_pc ? 16 : -1);

    if (m_pc)
      DNAMP1::CMDL::HMDLCook(out, in, mesh);
    else
      DNAMP1::CMDL::Cook(out, in, mesh);
  }

  void cookColMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                   hecl::blender::Token& btok, FCookProgress progress) override {
    std::vector<ColMesh> mesh = ds.compileColMeshes();
    ds.close();
    DNAMP1::DCLN::Cook(out, mesh);
  }

  void cookArmature(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                    hecl::blender::Token& btok, FCookProgress progress) override {
    Armature armature = ds.compileArmature();
    ds.close();
    DNAMP1::CINF::Cook(out, in, armature);
  }

  void cookPathMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                    hecl::blender::Token& btok, FCookProgress progress) override {
    PathMesh mesh = ds.compilePathMesh();
    ds.close();
    DNAMP1::PATH::Cook(out, in, mesh, btok);
  }

  void cookActor(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                 hecl::blender::Token& btok, FCookProgress progress) override {
    if (hecl::StringUtils::EndsWith(in.getAuxInfo(), _SYS_STR(".CSKR"))) {
      Actor actor = ds.compileActorCharacterOnly();
      ds.close();
      if (m_pc) {
        DNAMP1::ANCS::CookCSKRPC(out, in, actor, [&](const hecl::ProjectPath& modelPath) {
          hecl::ProjectPath cooked = modelPath.getCookedPath(SpecEntMP1PC);
          doCook(modelPath, cooked, fast, btok, progress);
          return true;
        });
      } else {
        DNAMP1::ANCS::CookCSKR(out, in, actor, [&](const hecl::ProjectPath& modelPath) {
          hecl::ProjectPath cooked = modelPath.getCookedPath(SpecEntMP1);
          doCook(modelPath, cooked, fast, btok, progress);
          return true;
        });
      }
    } else if (hecl::StringUtils::EndsWith(in.getAuxInfo(), _SYS_STR(".ANIM"))) {
      Actor actor = ds.compileActorCharacterOnly();
      DNAMP1::ANCS::CookANIM(out, in, actor, ds, m_pc);
    } else {
      Actor actor = ds.compileActor();
      DNAMP1::ANCS::Cook(out, in, actor);
    }
  }

  void buildAreaXFs(hecl::blender::Token& btok) {
    hecl::blender::Connection& conn = btok.getBlenderConnection();
    for (const auto& ent : m_workPath.enumerateDir()) {
      if (ent.m_isDir) {
        hecl::ProjectPath pakPath(m_workPath, ent.m_name);
        for (const auto& ent2 : pakPath.enumerateDir()) {
          if (ent2.m_isDir) {
            hecl::ProjectPath wldDir(pakPath, ent2.m_name);
            for (const auto& ent3 : wldDir.enumerateDir()) {
              if (hecl::StringUtils::BeginsWith(ent3.m_name, _SYS_STR("!world_")) &&
                  hecl::StringUtils::EndsWith(ent3.m_name, _SYS_STR(".blend"))) {
                hecl::ProjectPath wldPath(wldDir, ent3.m_name);
                if (wldPath.isFile()) {
                  if (!conn.openBlend(wldPath))
                    continue;
                  hecl::blender::DataStream ds = conn.beginData();
                  hecl::blender::World world = ds.compileWorld();
                  for (const auto& area : world.areas)
                    m_mreaPathToXF[area.path.hash()] = area.transform;
                }
                break;
              }
            }
          }
        }
      }
    }
  }

  void cookArea(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                hecl::blender::Token& btok, FCookProgress progress) override {
    std::vector<std::string> meshes = ds.getMeshList();
    std::vector<Mesh> meshCompiles;
    meshCompiles.reserve(meshes.size());

    std::optional<ColMesh> colMesh;

    for (const std::string& mesh : meshes) {
      hecl::SystemStringConv meshSys(mesh);
      if (mesh == "CMESH") {
        colMesh = ds.compileColMesh(mesh);
        progress(_SYS_STR("Collision Mesh"));
        continue;
      }
      meshCompiles.push_back(
          ds.compileMesh(mesh, fast ? hecl::HMDLTopology::Triangles : hecl::HMDLTopology::TriStrips, -1, !m_pc));
    }

    if (!colMesh)
      Log.report(logvisor::Fatal, FMT_STRING(_SYS_STR("unable to find mesh named 'CMESH' in {}")),
                 in.getAbsolutePath());

    std::vector<Light> lights = ds.compileLights();

    ds.close();

    if (m_mreaPathToXF.empty())
      buildAreaXFs(btok);

    const hecl::blender::Matrix4f* xf = nullptr;
    auto xfSearch = m_mreaPathToXF.find(in.getParentPath().hash());
    if (xfSearch != m_mreaPathToXF.cend())
      xf = &xfSearch->second;
    DNAMP1::MREA::Cook(out, in, meshCompiles, *colMesh, lights, btok, xf, m_pc);
  }

  void cookWorld(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                 hecl::blender::Token& btok, FCookProgress progress) override {
    hecl::blender::World world = ds.compileWorld();
    ds.close();
    DNAMP1::MLVL::Cook(out, in, world, btok);
  }

  void cookGuiFrame(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds,
                    hecl::blender::Token& btok, FCookProgress progress) override {
    auto data = ds.compileGuiFrame(0);
    athena::io::MemoryReader r(data.data(), data.size());
    DNAMP1::FRME frme;
    frme.read(r);
    athena::io::FileWriter w(out.getAbsolutePath());
    frme.write(w);
  }

  void cookYAML(const hecl::ProjectPath& out, const hecl::ProjectPath& in, athena::io::IStreamReader& fin,
                hecl::blender::Token& btok, FCookProgress progress) override {
    athena::io::YAMLDocReader reader;
    if (reader.parse(&fin)) {
      std::string classStr = reader.readString("DNAType");
      if (classStr.empty())
        return;

      if (classStr == DNAMP1::STRG::DNAType()) {
        DNAMP1::STRG strg;
        strg.read(reader);
        DNAMP1::STRG::Cook(strg, out);
      } else if (classStr == DNAMP1::SCAN::DNAType()) {
        DNAMP1::SCAN scan;
        scan.read(reader);
        DNAMP1::SCAN::Cook(scan, out);
      } else if (classStr == DNAParticle::GPSM<UniqueID32>::DNAType()) {
        DNAParticle::GPSM<UniqueID32> gpsm;
        gpsm.read(reader);
        DNAParticle::WriteGPSM(gpsm, out);
      } else if (classStr == DNAParticle::SWSH<UniqueID32>::DNAType()) {
        DNAParticle::SWSH<UniqueID32> swsh;
        swsh.read(reader);
        DNAParticle::WriteSWSH(swsh, out);
      } else if (classStr == DNAParticle::ELSM<UniqueID32>::DNAType()) {
        DNAParticle::ELSM<UniqueID32> elsm;
        elsm.read(reader);
        DNAParticle::WriteELSM(elsm, out);
      } else if (classStr == DNAParticle::WPSM<UniqueID32>::DNAType()) {
        DNAParticle::WPSM<UniqueID32> wpsm;
        wpsm.read(reader);
        DNAParticle::WriteWPSM(wpsm, out);
      } else if (classStr == DNAParticle::CRSM<UniqueID32>::DNAType()) {
        DNAParticle::CRSM<UniqueID32> crsm;
        crsm.read(reader);
        DNAParticle::WriteCRSM(crsm, out);
      } else if (classStr == DNAParticle::DPSM<UniqueID32>::DNAType()) {
        DNAParticle::DPSM<UniqueID32> dpsm;
        dpsm.read(reader);
        DNAParticle::WriteDPSM(dpsm, out);
      } else if (classStr == DNADGRP::DGRP<UniqueID32>::DNAType()) {
        DNADGRP::DGRP<UniqueID32> dgrp;
        dgrp.read(reader);
        dgrp.validateDeps();
        DNADGRP::WriteDGRP(dgrp, out);
      } else if (classStr == DNAFont::FONT<UniqueID32>::DNAType()) {
        DNAFont::FONT<UniqueID32> font;
        font.read(reader);
        DNAFont::WriteFONT(font, out);
      } else if (classStr == DNAMP1::CTweakPlayerRes<true>::DNAType()) {
        DNAMP1::CTweakPlayerRes<true> playerRes;
        playerRes.read(reader);
        WriteTweak(playerRes, out);
      } else if (classStr == DNAMP1::CTweakPlayerRes<false>::DNAType()) {
        DNAMP1::CTweakPlayerRes<false> playerRes;
        playerRes.read(reader);
        WriteTweak(playerRes, out);
      } else if (classStr == DNAMP1::CTweakGunRes::DNAType()) {
        DNAMP1::CTweakGunRes gunRes;
        gunRes.read(reader);
        WriteTweak(gunRes, out);
      } else if (classStr == DNAMP1::CTweakSlideShow::DNAType()) {
        DNAMP1::CTweakSlideShow slideShow;
        slideShow.read(reader);
        WriteTweak(slideShow, out);
      } else if (classStr == DNAMP1::CTweakPlayer::DNAType()) {
        DNAMP1::CTweakPlayer player;
        player.read(reader);
        WriteTweak(player, out);
      } else if (classStr == DNAMP1::CTweakCameraBob::DNAType()) {
        DNAMP1::CTweakCameraBob cBob;
        cBob.read(reader);
        WriteTweak(cBob, out);
      } else if (classStr == DNAMP1::CTweakGame::DNAType()) {
        DNAMP1::CTweakGame cGame;
        cGame.read(reader);
        WriteTweak(cGame, out);
      } else if (classStr == DNAMP1::CTweakAutoMapper::DNAType()) {
        DNAMP1::CTweakAutoMapper autoMapper;
        autoMapper.read(reader);
        WriteTweak(autoMapper, out);
      } else if (classStr == DNAMP1::CTweakTargeting<true>::DNAType()) {
        DNAMP1::CTweakTargeting<false> targeting;
        targeting.read(reader);
        WriteTweak(targeting, out);
      } else if (classStr == DNAMP1::CTweakTargeting<false>::DNAType()) {
        DNAMP1::CTweakTargeting<false> targeting;
        targeting.read(reader);
        WriteTweak(targeting, out);
      } else if (classStr == DNAMP1::CTweakGui::DNAType()) {
        DNAMP1::CTweakGui gui;
        gui.read(reader);
        WriteTweak(gui, out);
      } else if (classStr == DNAMP1::CTweakPlayerControl::DNAType()) {
        DNAMP1::CTweakPlayerControl pc;
        pc.read(reader);
        WriteTweak(pc, out);
      } else if (classStr == DNAMP1::CTweakBall::DNAType()) {
        DNAMP1::CTweakBall ball;
        ball.read(reader);
        WriteTweak(ball, out);
      } else if (classStr == DNAMP1::CTweakParticle::DNAType()) {
        DNAMP1::CTweakParticle part;
        part.read(reader);
        WriteTweak(part, out);
      } else if (classStr == DNAMP1::CTweakGuiColors::DNAType()) {
        DNAMP1::CTweakGuiColors gColors;
        gColors.read(reader);
        WriteTweak(gColors, out);
      } else if (classStr == DNAMP1::CTweakPlayerGun::DNAType()) {
        DNAMP1::CTweakPlayerGun pGun;
        pGun.read(reader);
        WriteTweak(pGun, out);
      } else if (classStr == DNAMP1::CTweakPlayerControl::DNAType()) {
        DNAMP1::CTweakPlayerControl pControl;
        pControl.read(reader);
        WriteTweak(pControl, out);
      } else if (classStr == DNAMP1::MazeSeeds::DNAType()) {
        DNAMP1::MazeSeeds mSeeds;
        mSeeds.read(reader);
        WriteTweak(mSeeds, out);
      } else if (classStr == DNAMP1::SnowForces::DNAType()) {
        DNAMP1::SnowForces sForces;
        sForces.read(reader);
        WriteTweak(sForces, out);
      } else if (classStr == DNAMP1::HINT::DNAType()) {
        DNAMP1::HINT::Cook(in, out);
      } else if (classStr == DNAMP1::EVNT::DNAType()) {
        DNAMP1::EVNT::Cook(in, out);
      } else if (classStr == "ATBL") {
        DNAAudio::ATBL::Cook(in, out);
      } else if (classStr == DNAMP1::AFSM::DNAType()) {
        DNAMP1::AFSM::Cook(in, out);
      } else if (classStr == "MP1TextureCache") {
        TextureCache::Cook(in, out);
      }
    }
    progress(_SYS_STR("Done"));
  }

  void flattenDependenciesYAML(athena::io::IStreamReader& fin, std::vector<hecl::ProjectPath>& pathsOut) override {
    athena::io::YAMLDocReader reader;
    if (reader.parse(&fin)) {
      std::string classStr = reader.readString("DNAType"sv);
      if (classStr.empty())
        return;

      if (classStr == DNAMP1::STRG::DNAType()) {
        DNAMP1::STRG strg;
        strg.read(reader);
        strg.gatherDependencies(pathsOut);
      }
      if (classStr == DNAMP1::SCAN::DNAType()) {
        DNAMP1::SCAN scan;
        scan.read(reader);
        scan.gatherDependencies(pathsOut);
      } else if (classStr == DNAParticle::GPSM<UniqueID32>::DNAType()) {
        DNAParticle::GPSM<UniqueID32> gpsm;
        gpsm.read(reader);
        gpsm.gatherDependencies(pathsOut);
      } else if (classStr == DNAParticle::SWSH<UniqueID32>::DNAType()) {
        DNAParticle::SWSH<UniqueID32> swsh;
        swsh.read(reader);
        swsh.gatherDependencies(pathsOut);
      } else if (classStr == DNAParticle::ELSM<UniqueID32>::DNAType()) {
        DNAParticle::ELSM<UniqueID32> elsm;
        elsm.read(reader);
        elsm.gatherDependencies(pathsOut);
      } else if (classStr == DNAParticle::WPSM<UniqueID32>::DNAType()) {
        DNAParticle::WPSM<UniqueID32> wpsm;
        wpsm.read(reader);
        wpsm.gatherDependencies(pathsOut);
      } else if (classStr == DNAParticle::CRSM<UniqueID32>::DNAType()) {
        DNAParticle::CRSM<UniqueID32> crsm;
        crsm.read(reader);
        crsm.gatherDependencies(pathsOut);
      } else if (classStr == DNAParticle::DPSM<UniqueID32>::DNAType()) {
        DNAParticle::DPSM<UniqueID32> dpsm;
        dpsm.read(reader);
        dpsm.gatherDependencies(pathsOut);
      } else if (classStr == DNAFont::FONT<UniqueID32>::DNAType()) {
        DNAFont::FONT<UniqueID32> font;
        font.read(reader);
        font.gatherDependencies(pathsOut);
      } else if (classStr == DNAMP1::EVNT::DNAType()) {
        DNAMP1::EVNT evnt;
        evnt.read(reader);
        evnt.gatherDependencies(pathsOut);
      }
    }
  }

  void flattenDependenciesANCSYAML(athena::io::IStreamReader& fin, std::vector<hecl::ProjectPath>& pathsOut,
                                   int charIdx) override {
    athena::io::YAMLDocReader reader;
    if (reader.parse(&fin)) {
      std::string classStr = reader.readString("DNAType");
      if (classStr == DNAMP1::ANCS::DNAType()) {
        DNAMP1::ANCS ancs;
        ancs.read(reader);
        ancs.gatherDependencies(pathsOut, charIdx);
      }
    }
  }

  void buildWorldPakList(const hecl::ProjectPath& worldPath, const hecl::ProjectPath& worldPathCooked,
                         hecl::blender::Token& btok, athena::io::FileWriter& w, std::vector<urde::SObjectTag>& listOut,
                         atUint64& resTableOffset,
                         std::unordered_map<urde::CAssetId, std::vector<uint8_t>>& mlvlData) override {
    DNAMP1::MLVL mlvl;
    {
      athena::io::FileReader r(worldPathCooked.getAbsolutePath());
      if (r.hasError())
        Log.report(logvisor::Fatal, FMT_STRING(_SYS_STR("Unable to open world {}")), worldPathCooked.getRelativePath());
      mlvl.read(r);
    }

    size_t count = 5;
    for (const auto& area : mlvl.areas) {
      auto lazyIt = area.lazyDeps.cbegin();
      auto it = area.deps.cbegin();
      while (it != area.deps.cend()) {
        if (it->id != lazyIt->id)
          ++count;
        ++lazyIt;
        ++it;
      }
    }
    listOut.reserve(count);

    urde::SObjectTag worldTag = tagFromPath(worldPath.getWithExtension(_SYS_STR(".*"), true));

    w.writeUint32Big(m_pc ? 0x80030005 : 0x00030005);
    w.writeUint32Big(0);

    w.writeUint32Big(1);
    DNAMP1::PAK::NameEntry nameEnt;
    hecl::ProjectPath parentDir = worldPath.getParentPath();
    nameEnt.type = worldTag.type;
    nameEnt.id = worldTag.id.Value();
    nameEnt.nameLen = atUint32(parentDir.getLastComponent().size());
    nameEnt.name = parentDir.getLastComponentUTF8();
    nameEnt.write(w);

    std::unordered_set<urde::CAssetId> addedTags;
    for (auto& area : mlvl.areas) {
      urde::SObjectTag areaTag(FOURCC('MREA'), area.areaMREAId.toUint64());

      bool dupeRes = false;
      if (hecl::ProjectPath areaDir = pathFromTag(areaTag).getParentPath())
        dupeRes = hecl::ProjectPath(areaDir, _SYS_STR("!duperes")).isFile();

      urde::SObjectTag nameTag(FOURCC('STRG'), area.areaNameId.toUint64());
      if (nameTag)
        listOut.push_back(nameTag);
      for (const auto& dep : area.deps) {
        urde::CAssetId newId = dep.id.toUint64();
        if (dupeRes || addedTags.find(newId) == addedTags.end()) {
          listOut.emplace_back(dep.type, newId);
          addedTags.insert(newId);
        }
      }
      if (areaTag)
        listOut.push_back(areaTag);

      std::vector<DNAMP1::MLVL::Area::Dependency> strippedDeps;
      strippedDeps.reserve(area.deps.size());
      std::vector<atUint32> strippedDepLayers;
      strippedDepLayers.reserve(area.depLayers.size());
      auto lazyIt = area.lazyDeps.cbegin();
      auto it = area.deps.cbegin();
      auto layerIt = area.depLayers.cbegin();
      while (it != area.deps.cend()) {
        while (layerIt != area.depLayers.cend() && it - area.deps.cbegin() == *layerIt) {
          strippedDepLayers.push_back(atUint32(strippedDeps.size()));
          ++layerIt;
        }
        if (it->id != lazyIt->id)
          strippedDeps.push_back(*it);
        ++lazyIt;
        ++it;
      }

      area.lazyDepCount = 0;
      area.lazyDeps.clear();
      area.depCount = strippedDeps.size();
      area.deps = std::move(strippedDeps);
      area.depLayerCount = strippedDepLayers.size();
      area.depLayers = std::move(strippedDepLayers);
    }

    urde::SObjectTag nameTag(FOURCC('STRG'), mlvl.worldNameId.toUint64());
    if (nameTag)
      listOut.push_back(nameTag);

    urde::SObjectTag savwTag(FOURCC('SAVW'), mlvl.saveWorldId.toUint64());
    if (savwTag) {
      if (hecl::ProjectPath savwPath = pathFromTag(savwTag))
        m_project.cookPath(savwPath, {}, false, true);
      listOut.push_back(savwTag);
    }

    urde::SObjectTag mapTag(FOURCC('MAPW'), mlvl.worldMap.toUint64());
    if (mapTag) {
      if (hecl::ProjectPath mapPath = pathFromTag(mapTag)) {
        m_project.cookPath(mapPath, {}, false, true);
        if (hecl::ProjectPath mapCookedPath = getCookedPath(mapPath, true)) {
          athena::io::FileReader r(mapCookedPath.getAbsolutePath());
          if (r.hasError())
            Log.report(logvisor::Fatal, FMT_STRING(_SYS_STR("Unable to open {}")), mapCookedPath.getRelativePath());

          if (r.readUint32Big() != 0xDEADF00D)
            Log.report(logvisor::Fatal, FMT_STRING(_SYS_STR("Corrupt MAPW {}")), mapCookedPath.getRelativePath());
          r.readUint32Big();
          atUint32 mapaCount = r.readUint32Big();
          for (atUint32 i = 0; i < mapaCount; ++i) {
            UniqueID32 id;
            id.read(r);
            listOut.emplace_back(FOURCC('MAPA'), id.toUint64());
          }
        }
      }
      listOut.push_back(mapTag);
    }

    urde::SObjectTag skyboxTag(FOURCC('CMDL'), mlvl.worldSkyboxId.toUint64());
    if (skyboxTag) {
      listOut.push_back(skyboxTag);
      hecl::ProjectPath skyboxPath = pathFromTag(skyboxTag);
      if (btok.getBlenderConnection().openBlend(skyboxPath)) {
        auto data = btok.getBlenderConnection().beginData();
        std::vector<hecl::ProjectPath> textures = data.getTextures();
        for (const auto& tex : textures) {
          urde::SObjectTag texTag = tagFromPath(tex);
          if (!texTag)
            Log.report(logvisor::Fatal, FMT_STRING(_SYS_STR("Unable to resolve {}")), tex.getRelativePath());
          listOut.push_back(texTag);
        }
      }
    }

    listOut.push_back(worldTag);

    w.writeUint32Big(atUint32(listOut.size()));
    resTableOffset = w.position();
    for (const auto& item : listOut) {
      DNAMP1::PAK::Entry ent;
      ent.compressed = 0;
      ent.type = item.type;
      ent.id = item.id.Value();
      ent.size = 0;
      ent.offset = 0;
      ent.write(w);
    }

    {
      std::vector<uint8_t>& mlvlOut = mlvlData[worldTag.id];
      size_t mlvlSize = 0;
      mlvl.binarySize(mlvlSize);
      mlvlOut.resize(mlvlSize);
      athena::io::MemoryWriter mw(&mlvlOut[0], mlvlSize);
      mlvl.write(mw);
    }
  }

  void buildPakList(hecl::blender::Token& btok, athena::io::FileWriter& w, const std::vector<urde::SObjectTag>& list,
                    const std::vector<std::pair<urde::SObjectTag, std::string>>& nameList,
                    atUint64& resTableOffset) override {
    w.writeUint32Big(m_pc ? 0x80030005 : 0x00030005);
    w.writeUint32Big(0);

    w.writeUint32Big(atUint32(nameList.size()));
    for (const auto& item : nameList) {
      DNAMP1::PAK::NameEntry nameEnt;
      nameEnt.type = item.first.type;
      nameEnt.id = item.first.id.Value();
      nameEnt.nameLen = atUint32(item.second.size());
      nameEnt.name = item.second;
      nameEnt.write(w);
    }

    w.writeUint32Big(atUint32(list.size()));
    resTableOffset = w.position();
    for (const auto& item : list) {
      DNAMP1::PAK::Entry ent;
      ent.compressed = 0;
      ent.type = item.type;
      ent.id = item.id.Value();
      ent.size = 0;
      ent.offset = 0;
      ent.write(w);
    }
  }

  void writePakFileIndex(athena::io::FileWriter& w, const std::vector<urde::SObjectTag>& tags,
                         const std::vector<std::tuple<size_t, size_t, bool>>& index, atUint64 resTableOffset) override {
    w.seek(resTableOffset, athena::SeekOrigin::Begin);

    auto it = tags.begin();
    for (const auto& item : index) {
      const urde::SObjectTag& tag = *it++;
      DNAMP1::PAK::Entry ent;
      ent.compressed = atUint32(std::get<2>(item));
      ent.type = tag.type;
      ent.id = tag.id.Value();
      ent.size = atUint32(std::get<1>(item));
      ent.offset = atUint32(std::get<0>(item));
      ent.write(w);
    }
  }

  std::pair<std::unique_ptr<uint8_t[]>, size_t> compressPakData(const urde::SObjectTag& tag, const uint8_t* data,
                                                                size_t len) override {
    bool doCompress = false;
    switch (tag.type.toUint32()) {
    case SBIG('TXTR'):
    case SBIG('CMDL'):
    case SBIG('CSKR'):
    case SBIG('ANCS'):
    case SBIG('ANIM'):
    case SBIG('FONT'):
      doCompress = true;
      break;
    case SBIG('PART'):
    case SBIG('ELSC'):
    case SBIG('SWHC'):
    case SBIG('WPSC'):
    case SBIG('DPSC'):
    case SBIG('CRSC'):
      doCompress = len >= 0x400;
      break;
    default:
      break;
    }
    if (!doCompress)
      return {};

    uLong destLen = compressBound(len);
    std::pair<std::unique_ptr<uint8_t[]>, size_t> ret;
    ret.first.reset(new uint8_t[destLen]);
    compress2(ret.first.get(), &destLen, data, len, Z_BEST_COMPRESSION);
    ret.second = destLen;
    return ret;
  };

  void cookAudioGroup(const hecl::ProjectPath& out, const hecl::ProjectPath& in, FCookProgress progress) override {
    DNAMP1::AGSC::Cook(in, out);
    progress(_SYS_STR("Done"));
  }

  void cookSong(const hecl::ProjectPath& out, const hecl::ProjectPath& in, FCookProgress progress) override {
    DNAMP1::CSNG::Cook(in, out);
    progress(_SYS_STR("Done"));
  }

  void cookMapArea(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds,
                   hecl::blender::Token& btok, FCookProgress progress) override {
    hecl::blender::MapArea mapa = ds.compileMapArea();
    ds.close();
    DNAMP1::MAPA::Cook(mapa, out);
    progress(_SYS_STR("Done"));
  }

  void cookMapUniverse(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds,
                       hecl::blender::Token& btok, FCookProgress progress) override {
    hecl::blender::MapUniverse mapu = ds.compileMapUniverse();
    ds.close();
    DNAMAPU::MAPU::Cook(mapu, out);
    progress(_SYS_STR("Done"));
  }
};

hecl::Database::DataSpecEntry SpecEntMP1 = {
    _SYS_STR("MP1"sv), _SYS_STR("Data specification for original Metroid Prime engine"sv), _SYS_STR(".pak"sv),
    [](hecl::Database::Project& project, hecl::Database::DataSpecTool) -> std::unique_ptr<hecl::Database::IDataSpec> {
      return std::make_unique<SpecMP1>(&SpecEntMP1, project, false);
    }};

hecl::Database::DataSpecEntry SpecEntMP1PC = {
    _SYS_STR("MP1-PC"sv), _SYS_STR("Data specification for PC-optimized Metroid Prime engine"sv), _SYS_STR(".upak"sv),
    [](hecl::Database::Project& project,
       hecl::Database::DataSpecTool tool) -> std::unique_ptr<hecl::Database::IDataSpec> {
      if (tool != hecl::Database::DataSpecTool::Extract)
        return std::make_unique<SpecMP1>(&SpecEntMP1PC, project, true);
      return {};
    }};

hecl::Database::DataSpecEntry SpecEntMP1ORIG = {
    _SYS_STR("MP1-ORIG"sv), _SYS_STR("Data specification for unmodified Metroid Prime resources"sv), {}, {}};
} // namespace DataSpec
