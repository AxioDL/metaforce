#include <utility>
#include <stdio.h>
#include <string.h>

#include "SpecBase.hpp"
#include "DNAMP1/DNAMP1.hpp"

#include "DNAMP1/HINT.hpp"
#include "DNAMP1/MLVL.hpp"
#include "DNAMP1/STRG.hpp"
#include "DNAMP1/SCAN.hpp"
#include "DNAMP1/CMDL.hpp"
#include "DNAMP1/MREA.hpp"
#include "DNAMP1/ANCS.hpp"
#include "DNAMP1/AGSC.hpp"
#include "DNAMP1/CSNG.hpp"
#include "DNACommon/ATBL.hpp"
#include "DNACommon/FONT.hpp"
#include "DNACommon/PART.hpp"
#include "DNACommon/SWHC.hpp"
#include "DNACommon/ELSC.hpp"
#include "DNACommon/WPSC.hpp"
#include "DNACommon/CRSC.hpp"
#include "DNACommon/DPSC.hpp"
#include "DNACommon/DGRP.hpp"
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

#include "hecl/ClientProcess.hpp"

namespace DataSpec
{

static logvisor::Module Log("urde::SpecMP1");
extern hecl::Database::DataSpecEntry SpecEntMP1;
extern hecl::Database::DataSpecEntry SpecEntMP1PC;
extern hecl::Database::DataSpecEntry SpecEntMP1ORIG;

struct OriginalIDs
{
    static void Generate(PAKRouter<DNAMP1::PAKBridge>& pakRouter, hecl::Database::Project& project)
    {
        std::unordered_set<UniqueID32> addedIDs;
        std::vector<UniqueID32> originalIDs;
        pakRouter.enumerateResources([&](const DNAMP1::PAK::Entry* ent) -> bool {
            if (ent->type == FOURCC('MLVL') || ent->type == FOURCC('SCAN') ||
                ent->id.toUint32() == 0xB7BBD0B4 || ent->id.toUint32() == 0x1F9DA858)
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
                  [](const std::pair<UniqueID32, UniqueID32>& a, const std::pair<UniqueID32, UniqueID32>& b) -> bool {
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
                  [](const std::pair<UniqueID32, UniqueID32>& a, const std::pair<UniqueID32, UniqueID32>& b) -> bool {
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
        if (!memcmp(id, "GM8", 3))
            return true;
        return false;
    }

    std::vector<const nod::Node*> m_nonPaks;
    std::vector<DNAMP1::PAKBridge> m_paks;
    std::map<std::string, DNAMP1::PAKBridge*, hecl::CaseInsensitiveCompare> m_orderedPaks;

    hecl::ProjectPath m_workPath;
    hecl::ProjectPath m_cookPath;
    PAKRouter<DNAMP1::PAKBridge> m_pakRouter;

    SpecMP1(const hecl::Database::DataSpecEntry* specEntry, hecl::Database::Project& project, bool pc)
    : SpecBase(specEntry, project, pc)
    , m_workPath(project.getProjectWorkingPath(), _S("MP1"))
    , m_cookPath(project.getProjectCookedPath(SpecEntMP1), _S("MP1"))
    , m_pakRouter(*this, m_workPath, m_cookPath)
    {
    }

    void buildPaks(nod::Node& root, const std::vector<hecl::SystemString>& args, ExtractReport& rep)
    {
        m_nonPaks.clear();
        m_paks.clear();
        for (const nod::Node& child : root)
        {
            bool isPak = false;
            const std::string& name = child.getName();
            std::string lowerName = name;
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
                                std::string lowerArg = hecl::SystemUTF8View(arg).str();
                                std::transform(lowerArg.begin(), lowerArg.end(), lowerArg.begin(), tolower);
                                if (!lowerArg.compare(0, lowerBase.size(), lowerBase))
                                    good = true;
                            }
                        }
                    }

                    m_paks.emplace_back(m_project, child, good);
                }
            }

            if (!isPak)
                m_nonPaks.push_back(&child);
        }

        /* Sort PAKs alphabetically */
        m_orderedPaks.clear();
        for (DNAMP1::PAKBridge& dpak : m_paks)
            m_orderedPaks[dpak.getName()] = &dpak;

        /* Assemble extract report */
        rep.childOpts.reserve(m_orderedPaks.size());
        for (const std::pair<std::string, DNAMP1::PAKBridge*>& item : m_orderedPaks)
        {
            if (!item.second->m_doExtract)
                continue;
            rep.childOpts.emplace_back();
            ExtractReport& childRep = rep.childOpts.back();
            hecl::SystemStringView nameView(item.first);
            childRep.name = nameView;
            childRep.desc = item.second->getLevelString();
        }
    }

    bool checkFromStandaloneDisc(nod::DiscBase& disc, const hecl::SystemString& regstr,
                                 const std::vector<hecl::SystemString>& args, std::vector<ExtractReport>& reps)
    {
        nod::Partition* partition = disc.getDataPartition();
        std::unique_ptr<uint8_t[]> dolBuf = partition->getDOLBuf();
        const char* buildInfo = (char*)memmem(dolBuf.get(), partition->getDOLSize(), "MetroidBuildInfo", 16) + 19;

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
            hecl::SystemStringView buildView(buildStr);
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

        nod::Partition* partition = disc.getDataPartition();
        nod::Node& root = partition->getFSTRoot();
        nod::Node::DirectoryIterator dolIt = root.find("rs5mp1_p.dol");
        if (dolIt == root.end())
            return false;

        std::unique_ptr<uint8_t[]> dolBuf = dolIt->getBuf();
        const char* buildInfo = (char*)memmem(dolBuf.get(), dolIt->size(), "MetroidBuildInfo", 16) + 19;

        /* Root Report */
        reps.emplace_back();
        ExtractReport& rep = reps.back();
        rep.name = _S("MP1");
        rep.desc = _S("Metroid Prime ") + regstr;
        if (buildInfo)
        {
            std::string buildStr(buildInfo);
            hecl::SystemStringView buildView(buildStr);
            rep.desc += _S(" (") + buildView + _S(")");
        }

        /* Iterate PAKs and build level options */
        nod::Node::DirectoryIterator mp1It = root.find("MP1");
        if (mp1It == root.end())
            return false;
        buildPaks(*mp1It, mp1args, rep);

        return true;
    }

    bool extractFromDisc(nod::DiscBase&, bool force, FProgress progress)
    {
        m_project.enableDataSpecs({_S("MP1-PC")});

        nod::ExtractionContext ctx = {true, force, nullptr};

        m_workPath.makeDir();

        progress(_S("Indexing PAKs"), _S(""), 2, 0.0);
        m_pakRouter.build(m_paks, [&progress](float factor) { progress(_S("Indexing PAKs"), _S(""), 2, factor); });
        progress(_S("Indexing PAKs"), _S(""), 2, 1.0);

        hecl::ProjectPath outPath(m_project.getProjectWorkingPath(), _S("out"));
        outPath.makeDir();
        hecl::ProjectPath mp1OutPath(outPath, _S("MP1"));
        mp1OutPath.makeDir();

        /* Generate original ID mapping for MLVL and SCAN entries */
        OriginalIDs::Generate(m_pakRouter, m_project);

        /* Extract non-pak files */
        progress(_S("MP1 Root"), _S(""), 3, 0.0);
        int prog = 0;
        ctx.progressCB = [&](const std::string& name) {
            hecl::SystemStringView nameView(name);
            progress(_S("MP1 Root"), nameView.c_str(), 3, prog / (float)m_nonPaks.size());
        };
        for (const nod::Node* node : m_nonPaks)
        {
            node->extractToDirectory(mp1OutPath.getAbsolutePath(), ctx);
            prog++;
        }
        progress(_S("MP1 Root"), _S(""), 3, 1.0);

        /* Extract unique resources */
        std::mutex msgLock;
        hecl::ClientProcess process;
        int compIdx = 4;
        prog = 0;
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

            const std::string& name = pak.getName();
            hecl::SystemStringView sysName(name);

            {
                std::unique_lock<std::mutex> lk(msgLock);
                progress(sysName.c_str(), _S(""), compIdx, 0.0);
            }
            hecl::SystemString pakName = sysName.sys_str();
            process.addLambdaTransaction([&, pakName](hecl::BlenderToken& btok) {
                m_pakRouter.extractResources(pak, force, btok, [&](const hecl::SystemChar* substr, float factor) {
                    std::unique_lock<std::mutex> lk(msgLock);
                    progress(pakName.c_str(), substr, compIdx, factor);
                });
            });
        }

        process.waitUntilComplete();

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
            else if (!strcmp(classType, "ATBL"))
                return true;
            else if (!strcmp(classType, "MP1OriginalIDs"))
                return true;
            return false;
        });
    }

    urde::SObjectTag BuildTagFromPath(const hecl::ProjectPath& path, hecl::BlenderToken& btok) const
    {
        if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".CINF")))
            return {SBIG('CINF'), path.hash().val32()};
        else if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".CSKR")))
            return {SBIG('CSKR'), path.hash().val32()};
        else if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".ANIM")))
            return {SBIG('ANIM'), path.hash().val32()};
        else if (const hecl::SystemChar* ext = path.getLastComponentExt())
        {
            if (ext[0] == _S('*') || !hecl::StrCmp(ext, _S("proj")))
            {
                if (path.getWithExtension(_S(".proj"), true).isFile() &&
                    path.getWithExtension(_S(".pool"), true).isFile() &&
                    path.getWithExtension(_S(".sdir"), true).isFile() &&
                    path.getWithExtension(_S(".samp"), true).isFile())
                {
                    hecl::ProjectPath glob = path.getWithExtension(_S(".*"), true);
                    return {SBIG('AGSC'), glob.hash().val32()};
                }
            }
        }

        hecl::ProjectPath asBlend;
        if (path.getPathType() == hecl::ProjectPath::Type::Glob)
            asBlend = path.getWithExtension(_S(".blend"), true);
        else
            asBlend = path;

        if (hecl::IsPathBlend(asBlend))
        {
            hecl::BlenderConnection& conn = btok.getBlenderConnection();
            if (!conn.openBlend(asBlend))
                return {};

            switch (conn.getBlendType())
            {
            case hecl::BlenderConnection::BlendType::Mesh:
                return {SBIG('CMDL'), path.hash().val32()};
            case hecl::BlenderConnection::BlendType::Actor:
                if (path.getAuxInfo().size())
                {
                    if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".CINF")))
                        return {SBIG('CINF'), path.hash().val32()};
                    else if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".CSKR")))
                        return {SBIG('CSKR'), path.hash().val32()};
                    else if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S(".ANIM")))
                        return {SBIG('ANIM'), path.hash().val32()};
                }
                return {SBIG('ANCS'), path.hash().val32()};
            case hecl::BlenderConnection::BlendType::Area:
                return {SBIG('MREA'), path.hash().val32()};
            case hecl::BlenderConnection::BlendType::World:
            {
                if (path.getAuxInfo().size())
                {
                    if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S("MAPW")))
                        return {SBIG('MAPW'), path.hash().val32()};
                    else if (hecl::StringUtils::EndsWith(path.getAuxInfo(), _S("SAVW")))
                        return {SBIG('SAVW'), path.hash().val32()};
                }
                return {SBIG('MLVL'), path.hash().val32()};
            }
            case hecl::BlenderConnection::BlendType::MapArea:
                return {SBIG('MAPA'), path.hash().val32()};
            case hecl::BlenderConnection::BlendType::MapUniverse:
                return {SBIG('MAPU'), path.hash().val32()};
            case hecl::BlenderConnection::BlendType::Frame:
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
            FILE* fp = hecl::Fopen(path.getAbsolutePath().c_str(), _S("r"));
            if (!fp)
                return {};

            athena::io::YAMLDocReader reader;
            yaml_parser_set_input_file(reader.getParser(), fp);

            urde::SObjectTag resTag;
            if (reader.ClassTypeOperation([&](const char* className) -> bool {
                    if (!strcmp(className, "GPSM"))
                    {
                        resTag.type = SBIG('PART');
                        return true;
                    }
                    if (!strcmp(className, "SWSH"))
                    {
                        resTag.type = SBIG('SWHC');
                        return true;
                    }
                    if (!strcmp(className, "ELSM"))
                    {
                        resTag.type = SBIG('ELSC');
                        return true;
                    }
                    if (!strcmp(className, "WPSM"))
                    {
                        resTag.type = SBIG('WPSC');
                        return true;
                    }
                    if (!strcmp(className, "CRSM"))
                    {
                        resTag.type = SBIG('CRSC');
                        return true;
                    }
                    if (!strcmp(className, "DPSM"))
                    {
                        resTag.type = SBIG('DPSC');
                        return true;
                    }
                    else if (!strcmp(className, "FONT"))
                    {
                        resTag.type = SBIG('FONT');
                        return true;
                    }
                    else if (!strcmp(className, "urde::DNAMP1::EVNT"))
                    {
                        resTag.type = SBIG('EVNT');
                        return true;
                    }
                    else if (!strcmp(className, "urde::DGRP"))
                    {
                        resTag.type = SBIG('DGRP');
                        return true;
                    }
                    else if (!strcmp(className, "urde::DNAMP1::STRG"))
                    {
                        resTag.type = SBIG('STRG');
                        return true;
                    }
                    else if (!strcmp(className, "urde::DNAMP1::SCAN"))
                    {
                        resTag.type = SBIG('SCAN');
                        return true;
                    }
                    else if (!strcmp(className, "DataSpec::DNAMP1::CTweakPlayerRes") ||
                             !strcmp(className, "DataSpec::DNAMP1::CTweakGunRes") ||
                             !strcmp(className, "DataSpec::DNAMP1::CTweakSlideShow") ||
                             !strcmp(className, "DataSpec::DNAMP1::CTweakPlayer") ||
                             !strcmp(className, "DataSpec::DNAMP1::CTweakCameraBob") ||
                             !strcmp(className, "DataSpec::DNAMP1::CTweakGame") ||
                             !strcmp(className, "DataSpec::DNAMP1::CTweakTargeting") ||
                             !strcmp(className, "DataSpec::DNAMP1::CTweakAutoMapper") ||
                             !strcmp(className, "DataSpec::DNAMP1::CTweakGui") ||
                             !strcmp(className, "DataSpec::DNAMP1::CTweakPlayerControl") ||
                             !strcmp(className, "DataSpec::DNAMP1::CTweakBall") ||
                             !strcmp(className, "DataSpec::DNAMP1::CTweakParticle") ||
                             !strcmp(className, "DataSpec::DNAMP1::CTweakGuiColors") ||
                             !strcmp(className, "DataSpec::DNAMP1::CTweakPlayerGun"))
                    {
                        resTag.type = SBIG('CTWK');
                        return true;
                    }
                    else if (!strcmp(className, "DataSpec::DNAMP1::HINT"))
                    {
                        resTag.type = SBIG('HINT');
                        return true;
                    }
                    else if (!strcmp(className, "ATBL"))
                    {
                        resTag.type = SBIG('ATBL');
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

    void cookMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                  hecl::BlenderToken& btok, FCookProgress progress)
    {
        Mesh mesh =
            ds.compileMesh(fast ? hecl::HMDLTopology::Triangles : hecl::HMDLTopology::TriStrips, m_pc ? 16 : -1,
                           [&progress](int surfCount) { progress(hecl::SysFormat(_S("%d"), surfCount).c_str()); });

        if (m_pc)
            DNAMP1::CMDL::HMDLCook(out, in, mesh);
        else
            DNAMP1::CMDL::Cook(out, in, mesh);
    }

    void cookActor(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                   hecl::BlenderToken& btok, FCookProgress progress)
    {
        Actor actor = ds.compileActor();
        DNAMP1::ANCS::Cook(out, in, actor, ds, m_pc, [&](const hecl::ProjectPath& modelPath) -> bool {
            hecl::ProjectPath cooked;
            if (m_pc)
                cooked = modelPath.getCookedPath(SpecEntMP1PC);
            else
                cooked = modelPath.getCookedPath(SpecEntMP1);
            doCook(modelPath, cooked, fast, btok, progress);
            return true;
        });
    }

    void cookArea(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                  hecl::BlenderToken& btok, FCookProgress progress)
    {
        std::vector<std::string> meshes = ds.getMeshList();
        std::vector<Mesh> meshCompiles;
        meshCompiles.reserve(meshes.size());

        std::experimental::optional<ColMesh> colMesh;

        for (const std::string& mesh : meshes)
        {
            hecl::SystemStringView meshSys(mesh);
            if (!mesh.compare("CMESH"))
            {
                colMesh = ds.compileColMesh(mesh);
                progress(_S("Collision Mesh"));
                continue;
            }
            meshCompiles.push_back(ds.compileMesh(
                mesh, fast ? hecl::HMDLTopology::Triangles : hecl::HMDLTopology::TriStrips, -1,
                [&](int surfCount) { progress(hecl::SysFormat(_S("%s %d"), meshSys.c_str(), surfCount).c_str()); }));
        }

        if (!colMesh)
            Log.report(logvisor::Fatal, _S("unable to find mesh named 'CMESH' in %s"), in.getAbsolutePath().c_str());

        std::vector<Light> lights = ds.compileLights();

        if (m_pc)
            DNAMP1::MREA::PCCook(out, in, meshCompiles, *colMesh, lights);
        else
            DNAMP1::MREA::Cook(out, in, meshCompiles, *colMesh, lights);
    }

    void cookWorld(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds, bool fast,
                   hecl::BlenderToken& btok, FCookProgress progress)
    {
        BlendStream::World world = ds.compileWorld();
        ds.close();
        DNAMP1::MLVL::Cook(out, in, world, btok);
    }

    void cookGuiFrame(const hecl::ProjectPath& out, const hecl::ProjectPath& in, BlendStream& ds,
                      hecl::BlenderToken& btok, FCookProgress progress)
    {
        ds.compileGuiFrame(out.getAbsolutePathUTF8(), 0);
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
            else if (!classStr.compare(DNAMP1::HINT::DNAType()))
            {
                DNAMP1::HINT::Cook(in, out);
            }
            else if (!classStr.compare("ATBL"))
            {
                DNAAudio::ATBL::Cook(in, out);
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
        }
    }

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
};

hecl::Database::DataSpecEntry SpecEntMP1 = {
    _S("MP1"), _S("Data specification for original Metroid Prime engine"),
    [](hecl::Database::Project& project, hecl::Database::DataSpecTool) -> hecl::Database::IDataSpec* {
        return new struct SpecMP1(&SpecEntMP1, project, false);
    }};

hecl::Database::DataSpecEntry SpecEntMP1PC = {
    _S("MP1-PC"), _S("Data specification for PC-optimized Metroid Prime engine"),
    [](hecl::Database::Project& project, hecl::Database::DataSpecTool tool) -> hecl::Database::IDataSpec* {
        if (tool != hecl::Database::DataSpecTool::Extract)
            return new struct SpecMP1(&SpecEntMP1PC, project, true);
        return nullptr;
    }};

hecl::Database::DataSpecEntry SpecEntMP1ORIG = {
    _S("MP1-ORIG"), _S("Data specification for unmodified Metroid Prime resources"), {}};
}
