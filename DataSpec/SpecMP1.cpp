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
#include "nod/nod.hpp"

namespace DataSpec
{

using namespace std::literals;

static logvisor::Module Log("urde::SpecMP1");
extern hecl::Database::DataSpecEntry SpecEntMP1;
extern hecl::Database::DataSpecEntry SpecEntMP1PC;
extern hecl::Database::DataSpecEntry SpecEntMP1ORIG;

static const std::unordered_set<uint32_t> IndividualOrigIDs =
{
    0xB7BBD0B4,
    0x1F9DA858,
    0x2A13C23E,
    0xF13452F8,
    0xA91A7703,
    0xC042EC91,
    0x12A12131,
    0x5F556002,
    0xA9798329,
    0xB306E26F,
    0xCD7B1ACA,
    0x8ADA8184,
    0x1A29C0E6,
    0x5D9F9796,
    0x951546A8,
    0x7946C4C5,
    0x409AA72E,
};

struct OriginalIDs
{
    static void Generate(PAKRouter<DNAMP1::PAKBridge>& pakRouter, hecl::Database::Project& project)
    {
        std::unordered_set<UniqueID32> addedIDs;
        std::vector<UniqueID32> originalIDs;

        pakRouter.enumerateResources([&](const DNAMP1::PAK::Entry* ent) {
            if (ent->type == FOURCC('MLVL') ||
                ent->type == FOURCC('SCAN') ||
                ent->type == FOURCC('MREA') ||
                IndividualOrigIDs.find(ent->id.toUint32()) != IndividualOrigIDs.end())
            {
                if (addedIDs.find(ent->id) == addedIDs.cend())
                {
                    addedIDs.insert(ent->id);
                    originalIDs.push_back(ent->id);
                }
            }
            return true;
        });
        std::sort(originalIDs.begin(), originalIDs.end());

        athena::io::YAMLDocWriter yamlW("MP1OriginalIDs");
        for (const UniqueID32& id : originalIDs)
        {
            hecl::ProjectPath path = pakRouter.getWorking(id);
            yamlW.writeString(id.toString().c_str(), path.getRelativePathUTF8());
        }
        hecl::ProjectPath path(project.getProjectWorkingPath(), "MP1/!original_ids.yaml");
        path.makeDirChain(false);
        athena::io::FileWriter fileW(path.getAbsolutePath());
        yamlW.finish(&fileW);
    }

    static void Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath)
    {
        hecl::Database::Project& project = inPath.getProject();
        athena::io::YAMLDocReader r;
        athena::io::FileReader fr(inPath.getAbsolutePath());
        if (!fr.isOpen() || !r.parse(&fr))
            return;

        std::vector<std::pair<UniqueID32, UniqueID32>> originalIDs;
        originalIDs.reserve(r.getRootNode()->m_mapChildren.size());
        for (const auto& node : r.getRootNode()->m_mapChildren)
        {
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
        for (const auto& idPair : originalIDs)
        {
            idPair.first.write(w);
            idPair.second.write(w);
        }

        std::sort(originalIDs.begin(), originalIDs.end(),
                  [](const std::pair<UniqueID32, UniqueID32>& a, const std::pair<UniqueID32, UniqueID32>& b) {
                      return a.second < b.second;
                  });
        for (const auto& idPair : originalIDs)
        {
            idPair.second.write(w);
            idPair.first.write(w);
        }
    }
};

struct SpecMP1 : SpecBase
{
    bool checkStandaloneID(const char* id) const
    {
        return !memcmp(id, "GM8", 3);
    }

    std::vector<const nod::Node*> m_nonPaks;
    std::vector<DNAMP1::PAKBridge> m_paks;
    std::map<std::string, DNAMP1::PAKBridge*, hecl::CaseInsensitiveCompare> m_orderedPaks;

    hecl::ProjectPath m_workPath;
    hecl::ProjectPath m_cookPath;
    PAKRouter<DNAMP1::PAKBridge> m_pakRouter;

    std::unique_ptr<uint8_t[]> m_dolBuf;

    IDRestorer<UniqueID32> m_idRestorer;

    std::unordered_map<hecl::Hash, hecl::blender::Matrix4f> m_mreaPathToXF;

    void setThreadProject()
    {
        SpecBase::setThreadProject();
        UniqueIDBridge::SetIDRestorer(&m_idRestorer);
    }

    SpecMP1(const hecl::Database::DataSpecEntry* specEntry, hecl::Database::Project& project, bool pc)
    : SpecBase(specEntry, project, pc)
    , m_workPath(project.getProjectWorkingPath(), _S("MP1"))
    , m_cookPath(project.getProjectCookedPath(SpecEntMP1), _S("MP1"))
    , m_pakRouter(*this, m_workPath, m_cookPath)
    , m_idRestorer({project.getProjectWorkingPath(), "MP1/!original_ids.yaml"}, project)
    {
        setThreadProject();
    }

    void buildPaks(nod::Node& root, const std::vector<hecl::SystemString>& args, ExtractReport& rep)
    {
        m_nonPaks.clear();
        m_paks.clear();
        for (const nod::Node& child : root)
        {
            bool isPak = false;
            auto name = child.getName();
            std::string lowerName(name);
            std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), tolower);
            if (name.size() > 4)
            {
                std::string::iterator extit = lowerName.end() - 4;
                if (!std::string(extit, lowerName.end()).compare(".pak"))
                {
                    /* This is a pak */
                    isPak = true;
                    std::string lowerBase(lowerName.begin(), extit);

                    /* Needs filter */
                    bool good = true;
                    if (args.size())
                    {
                        good = false;
                        if (!lowerName.compare(0, 7, "metroid"))
                        {
                            hecl::SystemChar idxChar = lowerName[7];
                            for (const hecl::SystemString& arg : args)
                            {
                                if (arg.size() == 1 && iswdigit(arg[0]))
                                    if (arg[0] == idxChar)
                                        good = true;
                            }
                        }
                        else
                            good = true;

                        if (!good)
                        {
                            for (const hecl::SystemString& arg : args)
                            {
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
        for (DNAMP1::PAKBridge& dpak : m_paks)
            m_orderedPaks[std::string(dpak.getName())] = &dpak;

        /* Assemble extract report */
        rep.childOpts.reserve(m_orderedPaks.size());
        for (const std::pair<std::string, DNAMP1::PAKBridge*>& item : m_orderedPaks)
        {
            if (!item.second->m_doExtract)
                continue;
            rep.childOpts.emplace_back();
            ExtractReport& childRep = rep.childOpts.back();
            hecl::SystemStringConv nameView(item.first);
            childRep.name = nameView.sys_str();
            childRep.desc = item.second->getLevelString();
        }
    }

    bool checkFromStandaloneDisc(nod::DiscBase& disc, const hecl::SystemString& regstr,
                                 const std::vector<hecl::SystemString>& args, std::vector<ExtractReport>& reps)
    {
        nod::IPartition* partition = disc.getDataPartition();
        m_dolBuf = partition->getDOLBuf();
        const char* buildInfo = (char*)memmem(m_dolBuf.get(), partition->getDOLSize(), "MetroidBuildInfo", 16) + 19;

        if (!buildInfo)
            return false;

        /* Root Report */
        reps.emplace_back();
        ExtractReport& rep = reps.back();
        rep.name = _S("MP1");
        rep.desc = _S("Metroid Prime ") + regstr;
        if (buildInfo)
        {
            std::string buildStr(buildInfo);
            hecl::SystemStringConv buildView(buildStr);
            rep.desc += _S(" (") + buildView + _S(")");
        }

        /* Iterate PAKs and build level options */
        nod::Node& root = partition->getFSTRoot();
        buildPaks(root, args, rep);

        return true;
    }

    bool checkFromTrilogyDisc(nod::DiscBase& disc, const hecl::SystemString& regstr,
                              const std::vector<hecl::SystemString>& args, std::vector<ExtractReport>& reps)
    {
        std::vector<hecl::SystemString> mp1args;
        bool doExtract = false;
        if (args.size())
        {
            /* Needs filter */
            for (const hecl::SystemString& arg : args)
            {
                hecl::SystemString lowerArg = arg;
                hecl::ToLower(lowerArg);
                if (!lowerArg.compare(0, 3, _S("mp1")))
                {
                    doExtract = true;
                    mp1args.reserve(args.size());
                    size_t slashPos = arg.find(_S('/'));
                    if (slashPos == hecl::SystemString::npos)
                        slashPos = arg.find(_S('\\'));
                    if (slashPos != hecl::SystemString::npos)
                        mp1args.emplace_back(hecl::SystemString(arg.begin() + slashPos + 1, arg.end()));
                }
            }
        }
        else
            doExtract = true;

        if (!doExtract)
            return false;

        nod::IPartition* partition = disc.getDataPartition();
        nod::Node& root = partition->getFSTRoot();
        nod::Node::DirectoryIterator dolIt = root.find("rs5mp1_p.dol");
        if (dolIt == root.end())
        {
            dolIt = root.find("rs5mp1jpn_p.dol");
            if (dolIt == root.end())
                return false;
        }

        m_dolBuf = dolIt->getBuf();
        const char* buildInfo = (char*)memmem(m_dolBuf.get(), dolIt->size(), "MetroidBuildInfo", 16) + 19;

        /* Root Report */
        reps.emplace_back();
        ExtractReport& rep = reps.back();
        rep.name = _S("MP1");
        rep.desc = _S("Metroid Prime ") + regstr;
        if (buildInfo)
        {
            std::string buildStr(buildInfo);
            hecl::SystemStringConv buildView(buildStr);
            rep.desc += _S(" (") + buildView + _S(")");
        }

        /* Iterate PAKs and build level options */
        nod::Node::DirectoryIterator mp1It = root.find("MP1");
        if (mp1It == root.end())
        {
            mp1It = root.find("MP1JPN");
            if (mp1It == root.end())
                return false;
        }
        buildPaks(*mp1It, mp1args, rep);

        return true;
    }

    bool extractFromDisc(nod::DiscBase& disc, bool force, const hecl::MultiProgressPrinter& progress)
    {
        m_project.enableDataSpecs({_S("MP1-PC")});

        nod::ExtractionContext ctx = {force, nullptr};

        m_workPath.makeDir();

        progress.startNewLine();
        progress.print(_S("Indexing PAKs"), _S(""), 0.0);
        m_pakRouter.build(m_paks, [&progress](float factor) { progress.print(_S("Indexing PAKs"), _S(""), factor); });
        progress.print(_S("Indexing PAKs"), _S(""), 1.0);

        hecl::ProjectPath outPath(m_project.getProjectWorkingPath(), _S("out"));
        outPath.makeDir();
        disc.getDataPartition()->extractSysFiles(outPath.getAbsolutePath(), ctx);
        hecl::ProjectPath mp1OutPath(outPath, m_standalone ? _S("files") : _S("files/MP1"));
        mp1OutPath.makeDirChain(true);

        /* Extract non-pak files */
        progress.startNewLine();
        progress.print(_S("MP1 Root"), _S(""), 0.0);
        int prog = 0;
        ctx.progressCB = [&](std::string_view name, float) {
            hecl::SystemStringConv nameView(name);
            progress.print(_S("MP1 Root"), nameView.c_str(), prog / (float)m_nonPaks.size());
        };
        for (const nod::Node* node : m_nonPaks)
        {
            node->extractToDirectory(mp1OutPath.getAbsolutePath(), ctx);
            prog++;
        }
        progress.print(_S("MP1 Root"), _S(""), 1.0);

        /* Extract unique resources */
        hecl::ClientProcess process;
        progress.startNewLine();
        for (std::pair<const std::string, DNAMP1::PAKBridge*>& pair : m_orderedPaks)
        {
#if 0
            const DNAMP1::PAK::Entry* ent = pair.second->getPAK().lookupEntry(UniqueID32("A4DFCAD6"));
            if (ent)
            {
                DNAMP1::ANIM anim;
                PAKEntryReadStream rs = ent->beginReadStream(pair.second->getNode());
                anim.read(rs);
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
        hecl::ProjectPath noAramPath(m_project.getProjectWorkingPath(), _S("MP1/NoARAM"));
        extractRandomStaticEntropy(m_dolBuf.get() + 0x4f60, noAramPath);

        /* Generate original ID mapping for MLVL and SCAN entries - marks complete project */
        OriginalIDs::Generate(m_pakRouter, m_project);

        return true;
    }

    const hecl::Database::DataSpecEntry& getOriginalSpec() const { return SpecEntMP1; }

    const hecl::Database::DataSpecEntry& getUnmodifiedSpec() const { return SpecEntMP1ORIG; }

    hecl::ProjectPath getWorking(class UniqueID32& id) { return m_pakRouter.getWorking(id); }

    bool checkPathPrefix(const hecl::ProjectPath& path) const
    {
        return path.getRelativePath().compare(0, 4, _S("MP1/")) == 0;
    }

    bool validateYAMLDNAType(athena::io::IStreamReader& fp) const
    {
        athena::io::YAMLDocReader reader;
        yaml_parser_set_input(reader.getParser(), (yaml_read_handler_t*)athena::io::YAMLAthenaReader, &fp);
        return reader.ClassTypeOperation([](const char* classType) {
            if (!strcmp(classType, DNAMP1::MLVL::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::STRG::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::SCAN::DNAType()))
                return true;
            else if (!strcmp(classType, DNAParticle::GPSM<UniqueID32>::DNAType()))
                return true;
            else if (!strcmp(classType, DNAParticle::SWSH<UniqueID32>::DNAType()))
                return true;
            else if (!strcmp(classType, DNAParticle::ELSM<UniqueID32>::DNAType()))
                return true;
            else if (!strcmp(classType, DNAParticle::WPSM<UniqueID32>::DNAType()))
                return true;
            else if (!strcmp(classType, DNAParticle::CRSM<UniqueID32>::DNAType()))
                return true;
            else if (!strcmp(classType, DNAParticle::DPSM<UniqueID32>::DNAType()))
                return true;
            else if (!strcmp(classType, DNADGRP::DGRP<UniqueID32>::DNAType()))
                return true;
            else if (!strcmp(classType, DNAFont::FONT<UniqueID32>::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::CTweakPlayerRes::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::CTweakGunRes::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::CTweakSlideShow::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::CTweakPlayer::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::CTweakCameraBob::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::CTweakGame::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::CTweakAutoMapper::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::CTweakTargeting::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::CTweakGui::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::CTweakPlayerControl::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::CTweakBall::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::CTweakParticle::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::CTweakGuiColors::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::CTweakPlayerGun::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::HINT::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::EVNT::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::MazeSeeds::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::SnowForces::DNAType()))
                return true;
            else if (!strcmp(classType, "ATBL"))
                return true;
            else if (!strcmp(classType, DNAMP1::AFSM::DNAType()))
                return true;
            else if (!strcmp(classType, "MP1OriginalIDs"))
                return true;
            return false;
        });
    }

    urde::SObjectTag buildTagFromPath(const hecl::ProjectPath& path, hecl::blender::Token& btok) const
    {
        if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".CINF")))
            return {SBIG('CINF'), path.hash().val32()};
        else if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".CSKR")))
            return {SBIG('CSKR'), path.hash().val32()};
        else if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".ANIM")))
            return {SBIG('ANIM'), path.hash().val32()};
        else if (const hecl::SystemChar* ext = path.getLastComponentExt().data())
        {
            if (ext[0] == _S('*') || !hecl::StrCmp(ext, _S("mid")))
            {
                if (path.getWithExtension(_S(".mid"), true).isFile() &&
                    path.getWithExtension(_S(".yaml"), true).isFile())
                {
                    hecl::ProjectPath glob = path.getWithExtension(_S(".*"), true);
                    return {SBIG('CSNG'), glob.hash().val32()};
                }
            }
        }

        if (path.getPathType() == hecl::ProjectPath::Type::Directory)
        {
            if (hecl::ProjectPath(path, _S("!project.yaml")).isFile() &&
                hecl::ProjectPath(path, _S("!pool.yaml")).isFile())
                return {SBIG('AGSC'), path.hash().val32()};
        }

        hecl::ProjectPath asBlend;
        if (path.getPathType() == hecl::ProjectPath::Type::Glob)
            asBlend = path.getWithExtension(_S(".blend"), true);
        else
            asBlend = path;

        if (hecl::IsPathBlend(asBlend))
        {
            hecl::blender::Connection& conn = btok.getBlenderConnection();
            if (!conn.openBlend(asBlend))
                return {};

            switch (conn.getBlendType())
            {
            case hecl::blender::BlendType::Mesh:
                return {SBIG('CMDL'), path.hash().val32()};
            case hecl::blender::BlendType::ColMesh:
                return {SBIG('DCLN'), path.hash().val32()};
            case hecl::blender::BlendType::PathMesh:
                return {SBIG('PATH'), path.hash().val32()};
            case hecl::blender::BlendType::Actor:
                if (path.getAuxInfo().size())
                {
                    if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".CINF")))
                        return {SBIG('CINF'), path.getWithExtension(_S(".*"), true).hash().val32()};
                    else if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".CSKR")))
                        return {SBIG('CSKR'), path.getWithExtension(_S(".*"), true).hash().val32()};
                    else if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".ANIM")))
                        return {SBIG('ANIM'), path.getWithExtension(_S(".*"), true).hash().val32()};
                }
                return {SBIG('ANCS'), path.getWithExtension(_S(".*"), true).hash().val32()};
            case hecl::blender::BlendType::Area:
                return {SBIG('MREA'), path.hash().val32()};
            case hecl::blender::BlendType::World:
            {
                if (path.getAuxInfo().size())
                {
                    if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S("MAPW")))
                        return {SBIG('MAPW'), path.getWithExtension(_S(".*"), true).hash().val32()};
                    else if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S("SAVW")))
                        return {SBIG('SAVW'), path.getWithExtension(_S(".*"), true).hash().val32()};
                }
                return {SBIG('MLVL'), path.getWithExtension(_S(".*"), true).hash().val32()};
            }
            case hecl::blender::BlendType::MapArea:
                return {SBIG('MAPA'), path.hash().val32()};
            case hecl::blender::BlendType::MapUniverse:
                return {SBIG('MAPU'), path.hash().val32()};
            case hecl::blender::BlendType::Frame:
                return {SBIG('FRME'), path.hash().val32()};
            default:
                return {};
            }
        }
        else if (hecl::IsPathPNG(path))
        {
            return {SBIG('TXTR'), path.hash().val32()};
        }
        else if (hecl::IsPathYAML(path))
        {
            FILE* fp = hecl::Fopen(path.getAbsolutePath().data(), _S("r"));
            if (!fp)
                return {};

            athena::io::YAMLDocReader reader;
            yaml_parser_set_input_file(reader.getParser(), fp);

            urde::SObjectTag resTag;
            if (reader.ClassTypeOperation([&](const char* className) -> bool {
                if (!strcmp(className, DNAParticle::GPSM<UniqueID32>::DNAType()))
                {
                    resTag.type = SBIG('PART');
                    return true;
                }
                if (!strcmp(className, DNAParticle::SWSH<UniqueID32>::DNAType()))
                {
                    resTag.type = SBIG('SWHC');
                    return true;
                }
                if (!strcmp(className, DNAParticle::ELSM<UniqueID32>::DNAType()))
                {
                    resTag.type = SBIG('ELSC');
                    return true;
                }
                if (!strcmp(className, DNAParticle::WPSM<UniqueID32>::DNAType()))
                {
                    resTag.type = SBIG('WPSC');
                    return true;
                }
                if (!strcmp(className, DNAParticle::CRSM<UniqueID32>::DNAType()))
                {
                    resTag.type = SBIG('CRSC');
                    return true;
                }
                if (!strcmp(className, DNAParticle::DPSM<UniqueID32>::DNAType()))
                {
                    resTag.type = SBIG('DPSC');
                    return true;
                }
                else if (!strcmp(className, DNAFont::FONT<UniqueID32>::DNAType()))
                {
                    resTag.type = SBIG('FONT');
                    return true;
                }
                else if (!strcmp(className, DNAMP1::EVNT::DNAType()))
                {
                    resTag.type = SBIG('EVNT');
                    return true;
                }
                else if (!strcmp(className, DNADGRP::DGRP<UniqueID32>::DNAType()))
                {
                    resTag.type = SBIG('DGRP');
                    return true;
                }
                else if (!strcmp(className, DataSpec::DNAMP1::STRG::DNAType()))
                {
                    resTag.type = SBIG('STRG');
                    return true;
                }
                else if (!strcmp(className, DataSpec::DNAMP1::SCAN::DNAType()))
                {
                    resTag.type = SBIG('SCAN');
                    return true;
                }
                else if (!strcmp(className, DataSpec::DNAMP1::CTweakPlayerRes::DNAType()) ||
                         !strcmp(className, DataSpec::DNAMP1::CTweakGunRes::DNAType()) ||
                         !strcmp(className, DataSpec::DNAMP1::CTweakSlideShow::DNAType()) ||
                         !strcmp(className, DataSpec::DNAMP1::CTweakPlayer::DNAType()) ||
                         !strcmp(className, DataSpec::DNAMP1::CTweakCameraBob::DNAType()) ||
                         !strcmp(className, DataSpec::DNAMP1::CTweakGame::DNAType()) ||
                         !strcmp(className, DataSpec::DNAMP1::CTweakTargeting::DNAType()) ||
                         !strcmp(className, DataSpec::DNAMP1::CTweakAutoMapper::DNAType()) ||
                         !strcmp(className, DataSpec::DNAMP1::CTweakGui::DNAType()) ||
                         !strcmp(className, DataSpec::DNAMP1::CTweakPlayerControl::DNAType()) ||
                         !strcmp(className, DataSpec::DNAMP1::CTweakBall::DNAType()) ||
                         !strcmp(className, DataSpec::DNAMP1::CTweakParticle::DNAType()) ||
                         !strcmp(className, DataSpec::DNAMP1::CTweakGuiColors::DNAType()) ||
                         !strcmp(className, DataSpec::DNAMP1::CTweakPlayerGun::DNAType()))
                {
                    resTag.type = SBIG('CTWK');
                    return true;
                }
                else if (!strcmp(className, DataSpec::DNAMP1::MazeSeeds::DNAType()) ||
                         !strcmp(className, DataSpec::DNAMP1::SnowForces::DNAType()))
                {
                    resTag.type = SBIG('DUMB');
                    return true;
                }
                else if (!strcmp(className, DataSpec::DNAMP1::HINT::DNAType()))
                {
                    resTag.type = SBIG('HINT');
                    return true;
                }
                else if (!strcmp(className, "ATBL"))
                {
                    resTag.type = SBIG('ATBL');
                    return true;
                }
                else if (!strcmp(className, DataSpec::DNAMP1::AFSM::DNAType()))
                {
                    resTag.type = SBIG('AFSM');
                    return true;
                }
                else if (!strcmp(className, "MP1OriginalIDs"))
                {
                    resTag.type = SBIG('OIDS');
                    return true;
                }

                return false;
            }))
            {
                resTag.id = path.hash().val32();
                fclose(fp);
                return resTag;
            }
            fclose(fp);
        }
        return {};
    }

    void getTagListForFile(const char* pakName, std::vector<urde::SObjectTag>& out) const
    {
        std::string pathPrefix("MP1/");
        pathPrefix += pakName;
        pathPrefix += '/';

        std::unique_lock<std::mutex> lk(const_cast<SpecMP1&>(*this).m_backgroundIndexMutex);
        for (const auto& tag : m_tagToPath)
            if (!tag.second.getRelativePathUTF8().compare(0, pathPrefix.size(), pathPrefix))
                out.push_back(tag.first);
    }

    void cookMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                  hecl::blender::Token& btok, FCookProgress progress)
    {
        Mesh mesh =
            ds.compileMesh(fast ? hecl::HMDLTopology::Triangles : hecl::HMDLTopology::TriStrips, m_pc ? 16 : -1,
                           [&progress](int surfCount) { progress(hecl::SysFormat(_S("%d"), surfCount).c_str()); });

        if (m_pc)
            DNAMP1::CMDL::HMDLCook(out, in, mesh);
        else
            DNAMP1::CMDL::Cook(out, in, mesh);
    }

    void cookColMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                          hecl::blender::Token& btok, FCookProgress progress)
    {
        std::vector<ColMesh> mesh = ds.compileColMeshes();
        ds.close();
        DNAMP1::DCLN::Cook(out, mesh);
    }

    void cookPathMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                      hecl::blender::Token& btok, FCookProgress progress)
    {
        PathMesh mesh = ds.compilePathMesh();
        ds.close();
        DNAMP1::PATH::Cook(out, mesh);
    }

    void cookActor(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                   hecl::blender::Token& btok, FCookProgress progress)
    {
        if (hecl::StringUtils::EndsWith(in.getAuxInfo(), _S(".CINF")))
        {
            Actor actor = ds.compileActorCharacterOnly();
            DNAMP1::ANCS::CookCINF(out, in, actor);
        }
        else if (hecl::StringUtils::EndsWith(in.getAuxInfo(), _S(".CSKR")))
        {
            Actor actor = ds.compileActorCharacterOnly();
            ds.close();
            if (m_pc)
            {
                DNAMP1::ANCS::CookCSKRPC(out, in, actor, [&](const hecl::ProjectPath& modelPath) {
                    hecl::ProjectPath cooked = modelPath.getCookedPath(SpecEntMP1PC);
                    doCook(modelPath, cooked, fast, btok, progress);
                    return true;
                });
            }
            else
            {
                DNAMP1::ANCS::CookCSKR(out, in, actor, [&](const hecl::ProjectPath& modelPath) {
                    hecl::ProjectPath cooked = modelPath.getCookedPath(SpecEntMP1);
                    doCook(modelPath, cooked, fast, btok, progress);
                    return true;
                });
            }
        }
        else if (hecl::StringUtils::EndsWith(in.getAuxInfo(), _S(".ANIM")))
        {
            Actor actor = ds.compileActorCharacterOnly();
            DNAMP1::ANCS::CookANIM(out, in, actor, ds, m_pc);
        }
        else
        {
            Actor actor = ds.compileActor();
            DNAMP1::ANCS::Cook(out, in, actor);
        }
    }

    void buildAreaXFs(hecl::blender::Token& btok)
    {
        hecl::blender::Connection& conn = btok.getBlenderConnection();
        for (const auto& ent : m_workPath.enumerateDir())
        {
            if (ent.m_isDir)
            {
                hecl::ProjectPath pakPath(m_workPath, ent.m_name);
                for (const auto& ent2 : pakPath.enumerateDir())
                {
                    if (ent2.m_isDir)
                    {
                        hecl::ProjectPath wldPath(pakPath, ent2.m_name + _S("/!world.blend"));
                        if (wldPath.isFile())
                        {
                            if (!conn.openBlend(wldPath))
                                continue;
                            hecl::blender::DataStream ds = conn.beginData();
                            hecl::blender::World world = ds.compileWorld();
                            for (const auto& area : world.areas)
                                m_mreaPathToXF[area.path.hash()] = area.transform;
                        }
                    }
                }
            }
        }
    }

    void cookArea(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                  hecl::blender::Token& btok, FCookProgress progress)
    {
        std::vector<std::string> meshes = ds.getMeshList();
        std::vector<Mesh> meshCompiles;
        meshCompiles.reserve(meshes.size());

        std::experimental::optional<ColMesh> colMesh;

        for (const std::string& mesh : meshes)
        {
            hecl::SystemStringConv meshSys(mesh);
            if (!mesh.compare("CMESH"))
            {
                colMesh = ds.compileColMesh(mesh);
                progress(_S("Collision Mesh"));
                continue;
            }
            meshCompiles.push_back(ds.compileMesh(
                mesh, fast ? hecl::HMDLTopology::Triangles : hecl::HMDLTopology::TriStrips, -1, !m_pc,
                [&](int surfCount) { progress(hecl::SysFormat(_S("%s %d"), meshSys.c_str(), surfCount).c_str()); }));
        }

        if (!colMesh)
            Log.report(logvisor::Fatal, _S("unable to find mesh named 'CMESH' in %s"), in.getAbsolutePath().data());

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
                   hecl::blender::Token& btok, FCookProgress progress)
    {
        if (hecl::StringUtils::EndsWith(in.getAuxInfo(), _S("MAPW")))
        {
            hecl::blender::World world = ds.compileWorld();
            ds.close();
            DNAMP1::MLVL::CookMAPW(out, world, btok);
        }
        else if (hecl::StringUtils::EndsWith(in.getAuxInfo(), _S("SAVW")))
        {
            hecl::blender::World world = ds.compileWorld();
            ds.close();
            DNAMP1::MLVL::CookSAVW(out, world);
        }
        else
        {
            hecl::blender::World world = ds.compileWorld();
            ds.close();
            DNAMP1::MLVL::Cook(out, in, world, btok);
        }
    }

    void cookGuiFrame(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds,
                      hecl::blender::Token& btok, FCookProgress progress)
    {
        auto data = ds.compileGuiFrame(0);
        athena::io::MemoryReader r(data.data(), data.size());
        DNAMP1::FRME frme;
        frme.read(r);
        athena::io::FileWriter w(out.getAbsolutePath());
        frme.write(w);
    }

    void cookYAML(const hecl::ProjectPath& out, const hecl::ProjectPath& in, athena::io::IStreamReader& fin,
                  FCookProgress progress)
    {
        athena::io::YAMLDocReader reader;
        if (reader.parse(&fin))
        {
            std::string classStr = reader.readString("DNAType");
            if (classStr.empty())
                return;

            if (!classStr.compare(DNAMP1::STRG::DNAType()))
            {
                DNAMP1::STRG strg;
                strg.read(reader);
                DNAMP1::STRG::Cook(strg, out);
            }
            else if (!classStr.compare(DNAMP1::SCAN::DNAType()))
            {
                DNAMP1::SCAN scan;
                scan.read(reader);
                DNAMP1::SCAN::Cook(scan, out);
            }
            else if (!classStr.compare(DNAParticle::GPSM<UniqueID32>::DNAType()))
            {
                DNAParticle::GPSM<UniqueID32> gpsm;
                gpsm.read(reader);
                DNAParticle::WriteGPSM(gpsm, out);
            }
            else if (!classStr.compare(DNAParticle::SWSH<UniqueID32>::DNAType()))
            {
                DNAParticle::SWSH<UniqueID32> swsh;
                swsh.read(reader);
                DNAParticle::WriteSWSH(swsh, out);
            }
            else if (!classStr.compare(DNAParticle::ELSM<UniqueID32>::DNAType()))
            {
                DNAParticle::ELSM<UniqueID32> elsm;
                elsm.read(reader);
                DNAParticle::WriteELSM(elsm, out);
            }
            else if (!classStr.compare(DNAParticle::WPSM<UniqueID32>::DNAType()))
            {
                DNAParticle::WPSM<UniqueID32> wpsm;
                wpsm.read(reader);
                DNAParticle::WriteWPSM(wpsm, out);
            }
            else if (!classStr.compare(DNAParticle::CRSM<UniqueID32>::DNAType()))
            {
                DNAParticle::CRSM<UniqueID32> crsm;
                crsm.read(reader);
                DNAParticle::WriteCRSM(crsm, out);
            }
            else if (!classStr.compare(DNAParticle::DPSM<UniqueID32>::DNAType()))
            {
                DNAParticle::DPSM<UniqueID32> dpsm;
                dpsm.read(reader);
                DNAParticle::WriteDPSM(dpsm, out);
            }
            else if (!classStr.compare(DNADGRP::DGRP<UniqueID32>::DNAType()))
            {
                DNADGRP::DGRP<UniqueID32> dgrp;
                dgrp.read(reader);
                dgrp.validateDeps();
                DNADGRP::WriteDGRP(dgrp, out);
            }
            else if (!classStr.compare(DNAFont::FONT<UniqueID32>::DNAType()))
            {
                DNAFont::FONT<UniqueID32> font;
                font.read(reader);
                DNAFont::WriteFONT(font, out);
            }
            else if (!classStr.compare(DNAMP1::CTweakPlayerRes::DNAType()))
            {
                DNAMP1::CTweakPlayerRes playerRes;
                playerRes.read(reader);
                WriteTweak(playerRes, out);
            }
            else if (!classStr.compare(DNAMP1::CTweakGunRes::DNAType()))
            {
                DNAMP1::CTweakGunRes gunRes;
                gunRes.read(reader);
                WriteTweak(gunRes, out);
            }
            else if (!classStr.compare(DNAMP1::CTweakSlideShow::DNAType()))
            {
                DNAMP1::CTweakSlideShow slideShow;
                slideShow.read(reader);
                WriteTweak(slideShow, out);
            }
            else if (!classStr.compare(DNAMP1::CTweakPlayer::DNAType()))
            {
                DNAMP1::CTweakPlayer player;
                player.read(reader);
                WriteTweak(player, out);
            }
            else if (!classStr.compare(DNAMP1::CTweakCameraBob::DNAType()))
            {
                DNAMP1::CTweakCameraBob cBob;
                cBob.read(reader);
                WriteTweak(cBob, out);
            }
            else if (!classStr.compare(DNAMP1::CTweakGame::DNAType()))
            {
                DNAMP1::CTweakGame cGame;
                cGame.read(reader);
                WriteTweak(cGame, out);
            }
            else if (!classStr.compare(DNAMP1::CTweakAutoMapper::DNAType()))
            {
                DNAMP1::CTweakAutoMapper autoMapper;
                autoMapper.read(reader);
                WriteTweak(autoMapper, out);
            }
            else if (!classStr.compare(DNAMP1::CTweakTargeting::DNAType()))
            {
                DNAMP1::CTweakTargeting targeting;
                targeting.read(reader);
                WriteTweak(targeting, out);
            }
            else if (!classStr.compare(DNAMP1::CTweakGui::DNAType()))
            {
                DNAMP1::CTweakGui gui;
                gui.read(reader);
                WriteTweak(gui, out);
            }
            else if (!classStr.compare(DNAMP1::CTweakPlayerControl::DNAType()))
            {
                DNAMP1::CTweakPlayerControl pc;
                pc.read(reader);
                WriteTweak(pc, out);
            }
            else if (!classStr.compare(DNAMP1::CTweakBall::DNAType()))
            {
                DNAMP1::CTweakBall ball;
                ball.read(reader);
                WriteTweak(ball, out);
            }
            else if (!classStr.compare(DNAMP1::CTweakParticle::DNAType()))
            {
                DNAMP1::CTweakParticle part;
                part.read(reader);
                WriteTweak(part, out);
            }
            else if (!classStr.compare(DNAMP1::CTweakGuiColors::DNAType()))
            {
                DNAMP1::CTweakGuiColors gColors;
                gColors.read(reader);
                WriteTweak(gColors, out);
            }
            else if (!classStr.compare(DNAMP1::CTweakPlayerGun::DNAType()))
            {
                DNAMP1::CTweakPlayerGun pGun;
                pGun.read(reader);
                WriteTweak(pGun, out);
            }
            else if (!classStr.compare(DNAMP1::CTweakPlayerControl::DNAType()))
            {
                DNAMP1::CTweakPlayerControl pControl;
                pControl.read(reader);
                WriteTweak(pControl, out);
            }
            else if (!classStr.compare(DNAMP1::MazeSeeds::DNAType()))
            {
                DNAMP1::MazeSeeds mSeeds;
                mSeeds.read(reader);
                WriteTweak(mSeeds, out);
            }
            else if (!classStr.compare(DNAMP1::SnowForces::DNAType()))
            {
                DNAMP1::SnowForces sForces;
                sForces.read(reader);
                WriteTweak(sForces, out);
            }
            else if (!classStr.compare(DNAMP1::HINT::DNAType()))
            {
                DNAMP1::HINT::Cook(in, out);
            }
            else if (!classStr.compare(DNAMP1::EVNT::DNAType()))
            {
                DNAMP1::EVNT::Cook(in, out);
            }
            else if (!classStr.compare("ATBL"))
            {
                DNAAudio::ATBL::Cook(in, out);
            }
            else if (!classStr.compare(DNAMP1::AFSM::DNAType()))
            {
                DNAMP1::AFSM::Cook(in, out);
            }
            else if (!classStr.compare("MP1OriginalIDs"))
            {
                OriginalIDs::Cook(in, out);
            }
        }
        progress(_S("Done"));
    }

    void flattenDependenciesYAML(athena::io::IStreamReader& fin, std::vector<hecl::ProjectPath>& pathsOut)
    {
        athena::io::YAMLDocReader reader;
        if (reader.parse(&fin))
        {
            std::string classStr = reader.readString("DNAType");
            if (classStr.empty())
                return;

            if (!classStr.compare(DNAMP1::STRG::DNAType()))
            {
                DNAMP1::STRG strg;
                strg.read(reader);
                strg.gatherDependencies(pathsOut);
            }
            if (!classStr.compare(DNAMP1::SCAN::DNAType()))
            {
                DNAMP1::SCAN scan;
                scan.read(reader);
                scan.gatherDependencies(pathsOut);
            }
            else if (!classStr.compare(DNAParticle::GPSM<UniqueID32>::DNAType()))
            {
                DNAParticle::GPSM<UniqueID32> gpsm;
                gpsm.read(reader);
                gpsm.gatherDependencies(pathsOut);
            }
            else if (!classStr.compare(DNAParticle::SWSH<UniqueID32>::DNAType()))
            {
                DNAParticle::SWSH<UniqueID32> swsh;
                swsh.read(reader);
                swsh.gatherDependencies(pathsOut);
            }
            else if (!classStr.compare(DNAParticle::ELSM<UniqueID32>::DNAType()))
            {
                DNAParticle::ELSM<UniqueID32> elsm;
                elsm.read(reader);
                elsm.gatherDependencies(pathsOut);
            }
            else if (!classStr.compare(DNAParticle::WPSM<UniqueID32>::DNAType()))
            {
                DNAParticle::WPSM<UniqueID32> wpsm;
                wpsm.read(reader);
                wpsm.gatherDependencies(pathsOut);
            }
            else if (!classStr.compare(DNAParticle::CRSM<UniqueID32>::DNAType()))
            {
                DNAParticle::CRSM<UniqueID32> crsm;
                crsm.read(reader);
                crsm.gatherDependencies(pathsOut);
            }
            else if (!classStr.compare(DNAParticle::DPSM<UniqueID32>::DNAType()))
            {
                DNAParticle::DPSM<UniqueID32> dpsm;
                dpsm.read(reader);
                dpsm.gatherDependencies(pathsOut);
            }
            else if (!classStr.compare(DNAFont::FONT<UniqueID32>::DNAType()))
            {
                DNAFont::FONT<UniqueID32> font;
                font.read(reader);
                font.gatherDependencies(pathsOut);
            }
            else if (!classStr.compare(DNAMP1::EVNT::DNAType()))
            {
                DNAMP1::EVNT evnt;
                evnt.read(reader);
                evnt.gatherDependencies(pathsOut);
            }
        }
    }

    void flattenDependenciesANCSYAML(athena::io::IStreamReader& fin,
                                     std::vector<hecl::ProjectPath>& pathsOut,
                                     int charIdx)
    {
        athena::io::YAMLDocReader reader;
        if (reader.parse(&fin))
        {
            std::string classStr = reader.readString("DNAType");
            if (!classStr.compare(DNAMP1::ANCS::DNAType()))
            {
                DNAMP1::ANCS ancs;
                ancs.read(reader);
                ancs.gatherDependencies(pathsOut, charIdx);
            }
        }
    }

    UniqueID32 newToOriginal(urde::CAssetId id) const
    {
        if (UniqueID32 origId = m_idRestorer.newToOriginal({uint32_t(id.Value()), true}))
            return {origId.toUint32(), true};
        return {uint32_t(id.Value()), true};
    }

    urde::CAssetId originalToNew(UniqueID32 id) const
    {
        if (UniqueID32 newId = m_idRestorer.originalToNew(id))
            return newId.toUint32();
        return id.toUint32();
    }

    void buildWorldPakList(const hecl::ProjectPath& worldPath,
                           const hecl::ProjectPath& worldPathCooked,
                           hecl::blender::Token& btok,
                           athena::io::FileWriter& w,
                           std::vector<urde::SObjectTag>& listOut,
                           atUint64& resTableOffset,
                           std::unordered_map<urde::CAssetId, std::vector<uint8_t>>& mlvlData)
    {
        DNAMP1::MLVL mlvl;
        {
            athena::io::FileReader r(worldPathCooked.getAbsolutePath());
            if (r.hasError())
                Log.report(logvisor::Fatal, _S("Unable to open world %s"), worldPathCooked.getRelativePath().data());
            mlvl.read(r);
        }

        size_t count = 5;
        for (const auto& area : mlvl.areas)
        {
            auto lazyIt = area.lazyDeps.cbegin();
            auto it = area.deps.cbegin();
            while (it != area.deps.cend())
            {
                if (it->id != lazyIt->id)
                    ++count;
                ++lazyIt;
                ++it;
            }
        }
        listOut.reserve(count);

        urde::SObjectTag worldTag = tagFromPath(worldPath.getWithExtension(_S(".*"), true), btok);

        w.writeUint32Big(m_pc ? 0x80030005 : 0x00030005);
        w.writeUint32Big(0);

        w.writeUint32Big(1);
        DNAMP1::PAK::NameEntry nameEnt;
        hecl::ProjectPath parentDir = worldPath.getParentPath();
        nameEnt.type = worldTag.type;
        nameEnt.id = newToOriginal(worldTag.id);
        nameEnt.nameLen = atUint32(parentDir.getLastComponent().size());
        nameEnt.name = parentDir.getLastComponentUTF8();
        nameEnt.write(w);

        std::unordered_set<urde::CAssetId> addedTags;
        for (auto& area : mlvl.areas)
        {
            urde::SObjectTag areaTag(FOURCC('MREA'), originalToNew(area.areaMREAId));

            bool dupeRes = false;
            if (hecl::ProjectPath areaDir = pathFromTag(areaTag).getParentPath())
                dupeRes = hecl::ProjectPath(areaDir, _S("!duperes")).isFile();

            urde::SObjectTag nameTag(FOURCC('STRG'), originalToNew(area.areaNameId));
            if (nameTag)
                listOut.push_back(nameTag);
            for (const auto& dep : area.deps)
            {
                urde::CAssetId newId = originalToNew(dep.id);
                if (dupeRes || addedTags.find(newId) == addedTags.end())
                {
                    listOut.push_back({dep.type, newId});
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
            while (it != area.deps.cend())
            {
                while (layerIt != area.depLayers.cend() && it - area.deps.cbegin() == *layerIt)
                {
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

        urde::SObjectTag nameTag(FOURCC('STRG'), originalToNew(mlvl.worldNameId));
        if (nameTag)
            listOut.push_back(nameTag);

        urde::SObjectTag savwTag(FOURCC('SAVW'), originalToNew(mlvl.saveWorldId));
        if (savwTag)
        {
            if (hecl::ProjectPath savwPath = pathFromTag(savwTag))
                m_project.cookPath(savwPath, {}, false, true);
            listOut.push_back(savwTag);
        }

        urde::SObjectTag mapTag(FOURCC('MAPW'), originalToNew(mlvl.worldMap));
        if (mapTag)
        {
            if (hecl::ProjectPath mapPath = pathFromTag(mapTag))
            {
                m_project.cookPath(mapPath, {}, false, true);
                if (hecl::ProjectPath mapCookedPath = getCookedPath(mapPath, true))
                {
                    athena::io::FileReader r(mapCookedPath.getAbsolutePath());
                    if (r.hasError())
                        Log.report(logvisor::Fatal, _S("Unable to open %s"), mapCookedPath.getRelativePath().data());

                    if (r.readUint32Big() != 0xDEADF00D)
                        Log.report(logvisor::Fatal, _S("Corrupt MAPW %s"), mapCookedPath.getRelativePath().data());
                    r.readUint32Big();
                    atUint32 mapaCount = r.readUint32Big();
                    for (int i=0 ; i<mapaCount ; ++i)
                    {
                        UniqueID32 id;
                        id.read(r);
                        listOut.push_back({FOURCC('MAPA'), originalToNew(id)});
                    }
                }
            }
            listOut.push_back(mapTag);
        }

        urde::SObjectTag skyboxTag(FOURCC('CMDL'), originalToNew(mlvl.worldSkyboxId));
        if (skyboxTag)
        {
            listOut.push_back(skyboxTag);
            hecl::ProjectPath skyboxPath = pathFromTag(skyboxTag);
            if (btok.getBlenderConnection().openBlend(skyboxPath))
            {
                auto data = btok.getBlenderConnection().beginData();
                std::vector<hecl::ProjectPath> textures = data.getTextures();
                for (const auto& tex : textures)
                {
                    urde::SObjectTag texTag = tagFromPath(tex, btok);
                    if (!texTag)
                        Log.report(logvisor::Fatal, _S("Unable to resolve %s"), tex.getRelativePath().data());
                    listOut.push_back(texTag);
                }
            }
        }

        listOut.push_back(worldTag);

        w.writeUint32Big(atUint32(listOut.size()));
        resTableOffset = w.position();
        for (const auto& item : listOut)
        {
            DNAMP1::PAK::Entry ent;
            ent.compressed = 0;
            ent.type = item.type;
            ent.id = newToOriginal(item.id.Value());
            ent.size = 0;
            ent.offset = 0;
            ent.write(w);
        }

        {
            std::vector<uint8_t>& mlvlOut = mlvlData[worldTag.id];
            size_t mlvlSize = 0;
            mlvl.binarySize(mlvlSize);
            mlvlOut.resize(mlvlSize);
            athena::io::MemoryWriter w(&mlvlOut[0], mlvlSize);
            mlvl.write(w);
        }
    }

    void buildPakList(hecl::blender::Token& btok,
                      athena::io::FileWriter& w,
                      const std::vector<urde::SObjectTag>& list,
                      const std::vector<std::pair<urde::SObjectTag, std::string>>& nameList,
                      atUint64& resTableOffset)
    {
        w.writeUint32Big(m_pc ? 0x80030005 : 0x00030005);
        w.writeUint32Big(0);

        w.writeUint32Big(atUint32(nameList.size()));
        for (const auto& item : nameList)
        {
            DNAMP1::PAK::NameEntry nameEnt;
            nameEnt.type = item.first.type;
            nameEnt.id = newToOriginal(item.first.id);
            nameEnt.nameLen = atUint32(item.second.size());
            nameEnt.name = item.second;
            nameEnt.write(w);
        }

        w.writeUint32Big(atUint32(list.size()));
        resTableOffset = w.position();
        for (const auto& item : list)
        {
            DNAMP1::PAK::Entry ent;
            ent.compressed = 0;
            ent.type = item.type;
            ent.id = newToOriginal(item.id);
            ent.size = 0;
            ent.offset = 0;
            ent.write(w);
        }
    }

    void writePakFileIndex(athena::io::FileWriter& w,
                           const std::vector<urde::SObjectTag>& tags,
                           const std::vector<std::tuple<size_t, size_t, bool>>& index,
                           atUint64 resTableOffset)
    {
        w.seek(resTableOffset, athena::Begin);

        auto it = tags.begin();
        for (const auto& item : index)
        {
            const urde::SObjectTag& tag = *it++;
            DNAMP1::PAK::Entry ent;
            ent.compressed = atUint32(std::get<2>(item));
            ent.type = tag.type;
            ent.id = newToOriginal(tag.id);
            ent.size = atUint32(std::get<1>(item));
            ent.offset = atUint32(std::get<0>(item));
            ent.write(w);
        }
    }

    std::pair<std::unique_ptr<uint8_t[]>, size_t>
    compressPakData(const urde::SObjectTag& tag, const uint8_t* data, size_t len)
    {
        bool doCompress = false;
        switch (tag.type)
        {
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

    void cookAudioGroup(const hecl::ProjectPath& out, const hecl::ProjectPath& in, FCookProgress progress)
    {
        DNAMP1::AGSC::Cook(in, out);
        progress(_S("Done"));
    }

    void cookSong(const hecl::ProjectPath& out, const hecl::ProjectPath& in, FCookProgress progress)
    {
        DNAMP1::CSNG::Cook(in, out);
        progress(_S("Done"));
    }

    void cookMapArea(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                     BlendStream& ds, hecl::blender::Token& btok,
                     FCookProgress progress)
    {
        hecl::blender::MapArea mapa = ds.compileMapArea();
        ds.close();
        DNAMP1::MAPA::Cook(mapa, out);
        progress(_S("Done"));
    }

    void cookMapUniverse(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                         BlendStream& ds, hecl::blender::Token& btok,
                         FCookProgress progress)
    {
        hecl::blender::MapUniverse mapu = ds.compileMapUniverse();
        ds.close();
        DNAMAPU::MAPU::Cook(mapu, out);
        progress(_S("Done"));
    }
};

hecl::Database::DataSpecEntry SpecEntMP1 = {
    _S("MP1"sv), _S("Data specification for original Metroid Prime engine"sv), _S(".pak"sv), 2,
    [](hecl::Database::Project& project, hecl::Database::DataSpecTool) -> std::unique_ptr<hecl::Database::IDataSpec> {
        return std::make_unique<SpecMP1>(&SpecEntMP1, project, false);
    }};

hecl::Database::DataSpecEntry SpecEntMP1PC = {
    _S("MP1-PC"sv), _S("Data specification for PC-optimized Metroid Prime engine"sv), _S(".upak"sv), 2,
    [](hecl::Database::Project& project, hecl::Database::DataSpecTool tool) -> std::unique_ptr<hecl::Database::IDataSpec> {
        if (tool != hecl::Database::DataSpecTool::Extract)
            return std::make_unique<SpecMP1>(&SpecEntMP1PC, project, true);
        return {};
    }};

hecl::Database::DataSpecEntry SpecEntMP1ORIG = {
    _S("MP1-ORIG"sv), _S("Data specification for unmodified Metroid Prime resources"sv), {}, 2, {}};
}
