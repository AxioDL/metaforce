#include <utility>
#include <set>

#include "SpecBase.hpp"
#include "DNAMP3/DNAMP3.hpp"

#include "DNAMP3/MLVL.hpp"
#include "DNAMP3/STRG.hpp"
#include "DNAMP3/MAPA.hpp"
#include "DNAMP2/STRG.hpp"

#include "hecl/ClientProcess.hpp"

#include "Runtime/RetroTypes.hpp"

namespace DataSpec
{

static logvisor::Module Log("urde::SpecMP3");
extern hecl::Database::DataSpecEntry SpecEntMP3;
extern hecl::Database::DataSpecEntry SpecEntMP3ORIG;

struct SpecMP3 : SpecBase
{
    bool checkStandaloneID(const char* id) const
    {
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
    : SpecBase(specEntry, project, pc),
      m_workPath(project.getProjectWorkingPath(), _S("MP3")),
      m_cookPath(project.getProjectCookedPath(SpecEntMP3), _S("MP3")),
      m_pakRouter(*this, m_workPath, m_cookPath),
      m_feWorkPath(project.getProjectWorkingPath(), _S("fe")),
      m_feCookPath(project.getProjectCookedPath(SpecEntMP3), _S("fe")),
      m_fePakRouter(*this, m_feWorkPath, m_feCookPath) {}

    void buildPaks(nod::Node& root,
                   const std::vector<hecl::SystemString>& args,
                   ExtractReport& rep,
                   bool fe)
    {
        if (fe)
        {
            m_feNonPaks.clear();
            m_fePaks.clear();
        }
        else
        {
            m_nonPaks.clear();
            m_paks.clear();
        }
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

                    if (fe)
                        m_fePaks.emplace_back(m_project, child, good);
                    else
                        m_paks.emplace_back(m_project, child, good);
                }
            }

            if (!isPak)
            {
                if (fe)
                    m_feNonPaks.push_back(&child);
                else
                    m_nonPaks.push_back(&child);
            }
        }

        /* Sort PAKs alphabetically */
        if (fe)
        {
            m_feOrderedPaks.clear();
            for (DNAMP3::PAKBridge& dpak : m_fePaks)
                m_feOrderedPaks[dpak.getName()] = &dpak;
        }
        else
        {
            m_orderedPaks.clear();
            for (DNAMP3::PAKBridge& dpak : m_paks)
                m_orderedPaks[dpak.getName()] = &dpak;
        }

        /* Assemble extract report */
        for (const std::pair<std::string, DNAMP3::PAKBridge*>& item : fe ? m_feOrderedPaks : m_orderedPaks)
        {
            if (!item.second->m_doExtract)
                continue;
            rep.childOpts.emplace_back();
            ExtractReport& childRep = rep.childOpts.back();
            hecl::SystemStringView nameView(item.first);
            childRep.name = nameView;
            if (!item.first.compare("Worlds.pak"))
                continue;
            else if (!item.first.compare("Metroid6.pak"))
            {
                /* Phaaze doesn't have a world name D: */
                childRep.desc = _S("Phaaze");
                continue;
            }
            else if (!item.first.compare("Metroid8.pak"))
            {
                /* Space world is misnamed */
                childRep.desc = _S("Space");
                continue;
            }
            childRep.desc = item.second->getLevelString();
        }
    }

    bool checkFromStandaloneDisc(nod::DiscBase& disc,
                                 const hecl::SystemString& regstr,
                                 const std::vector<hecl::SystemString>& args,
                                 std::vector<ExtractReport>& reps)
    {
        doMP3 = true;
        nod::Partition* partition = disc.getDataPartition();
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
        rep.name = _S("MP3");
        rep.desc = _S("Metroid Prime 3 ") + regstr;
        std::string buildStr(buildInfo);
        hecl::SystemStringView buildView(buildStr);
        rep.desc += _S(" (") + buildView + _S(")");

        /* Iterate PAKs and build level options */
        nod::Node& root = partition->getFSTRoot();
        buildPaks(root, args, rep, false);

        return true;
    }

    bool checkFromTrilogyDisc(nod::DiscBase& disc,
                              const hecl::SystemString& regstr,
                              const std::vector<hecl::SystemString>& args,
                              std::vector<ExtractReport>& reps)
    {
        std::vector<hecl::SystemString> mp3args;
        std::vector<hecl::SystemString> feargs;
        if (args.size())
        {
            /* Needs filter */
            for (const hecl::SystemString& arg : args)
            {
                hecl::SystemString lowerArg = arg;
                hecl::ToLower(lowerArg);
                if (!lowerArg.compare(0, 3, _S("mp3")))
                {
                    doMP3 = true;
                    mp3args.reserve(args.size());
                    size_t slashPos = arg.find(_S('/'));
                    if (slashPos == hecl::SystemString::npos)
                        slashPos = arg.find(_S('\\'));
                    if (slashPos != hecl::SystemString::npos)
                        mp3args.emplace_back(hecl::SystemString(arg.begin() + slashPos + 1, arg.end()));
                }
            }

            for (const hecl::SystemString& arg : args)
            {
                hecl::SystemString lowerArg = arg;
                hecl::ToLower(lowerArg);
                if (!lowerArg.compare(0, 2, _S("fe")))
                {
                    doMPTFE = true;
                    feargs.reserve(args.size());
                    size_t slashPos = arg.find(_S('/'));
                    if (slashPos == hecl::SystemString::npos)
                        slashPos = arg.find(_S('\\'));
                    if (slashPos != hecl::SystemString::npos)
                        feargs.emplace_back(hecl::SystemString(arg.begin() + slashPos + 1, arg.end()));
                }
            }
        }
        else
        {
            doMP3 = true;
            doMPTFE = true;
        }

        if (!doMP3 && !doMPTFE)
            return false;

        nod::Partition* partition = disc.getDataPartition();
        nod::Node& root = partition->getFSTRoot();

        /* MP3 extract */
        if (doMP3)
        {
            nod::Node::DirectoryIterator dolIt = root.find("rs5mp3_p.dol");
            if (dolIt == root.end())
                return false;

            std::unique_ptr<uint8_t[]> dolBuf = dolIt->getBuf();
            const char* buildInfo = (char*)memmem(dolBuf.get(), dolIt->size(), "MetroidBuildInfo", 16) + 19;

            if (!buildInfo)
                return false;

            /* We don't want no stinking demo dammit */
            if (!strcmp(buildInfo, "Build v3.068 3/2/2006 14:55:13"))
                return false;

            /* Root Report */
            reps.emplace_back();
            ExtractReport& rep = reps.back();
            rep.name = _S("MP3");
            rep.desc = _S("Metroid Prime 3 ") + regstr;

            std::string buildStr(buildInfo);
            hecl::SystemStringView buildView(buildStr);
            rep.desc += _S(" (") + buildView + _S(")");


            /* Iterate PAKs and build level options */
            nod::Node::DirectoryIterator mp3It = root.find("MP3");
            if (mp3It == root.end())
                return false;
            buildPaks(*mp3It, mp3args, rep, false);
        }

        /* MPT Frontend extract */
        if (doMPTFE)
        {
            nod::Node::DirectoryIterator dolIt = root.find("rs5fe_p.dol");
            if (dolIt == root.end())
                return false;

            std::unique_ptr<uint8_t[]> dolBuf = dolIt->getBuf();
            const char* buildInfo = (char*)memmem(dolBuf.get(), dolIt->size(), "MetroidBuildInfo", 16) + 19;

            /* Root Report */
            reps.emplace_back();
            ExtractReport& rep = reps.back();
            rep.name = _S("fe");
            rep.desc = _S("Metroid Prime Trilogy Frontend ") + regstr;
            if (buildInfo)
            {
                std::string buildStr(buildInfo);
                hecl::SystemStringView buildView(buildStr);
                rep.desc += _S(" (") + buildView + _S(")");
            }

            /* Iterate PAKs and build level options */
            nod::Node::DirectoryIterator feIt = root.find("fe");
            if (feIt == root.end())
                return false;
            buildPaks(*feIt, feargs, rep, true);
        }

        return true;
    }

    bool extractFromDisc(nod::DiscBase&, bool force, FProgress progress)
    {
        int compIdx = 2;
        hecl::SystemString currentTarget = _S("");
        size_t nodeCount = 0;
        int prog = 0;
        nod::ExtractionContext ctx = {true, force,
        [&](const std::string& name, float)
        {
            hecl::SystemStringView nameView(name);
            progress(currentTarget.c_str(), nameView.c_str(), compIdx, prog / (float)nodeCount);
        }};
        if (doMP3)
        {
            m_workPath.makeDir();

            progress(_S("Indexing PAKs"), _S(""), compIdx, 0.0);
            m_pakRouter.build(m_paks, [&progress, &compIdx](float factor)
            {
                progress(_S("Indexing PAKs"), _S(""), compIdx, factor);
            });
            progress(_S("Indexing PAKs"), _S(""), compIdx++, 1.0);

            hecl::ProjectPath outPath(m_project.getProjectWorkingPath(), _S("out"));
            outPath.makeDir();
            hecl::ProjectPath mp3OutPath(outPath, _S("MP3"));
            mp3OutPath.makeDir();
            currentTarget = _S("MP3 Root");
            progress(currentTarget.c_str(), _S(""), compIdx, 0.0);
            prog = 0;

            nodeCount = m_nonPaks.size();
            // TODO: Make this more granular
            for (const nod::Node* node : m_nonPaks)
            {
                node->extractToDirectory(mp3OutPath.getAbsolutePath(), ctx);
                prog++;
            }
            ctx.progressCB = nullptr;

            progress(currentTarget.c_str(), _S(""), compIdx++, 1.0);

            std::mutex msgLock;
            hecl::ClientProcess process;
            prog = 0;
            for (std::pair<const std::string, DNAMP3::PAKBridge*>& pair : m_orderedPaks)
            {
                DNAMP3::PAKBridge& pak = *pair.second;
                if (!pak.m_doExtract)
                    continue;

                const std::string& name = pak.getName();
                hecl::SystemStringView sysName(name);

                {
                    std::unique_lock<std::mutex> lk(msgLock);
                    progress(sysName.c_str(), _S(""), compIdx, 0.0);
                }
                hecl::SystemString pakName = sysName.sys_str();
                process.addLambdaTransaction([&, pakName](hecl::BlenderToken& btok)
                {
                    m_pakRouter.extractResources(pak, force, btok,
                    [&](const hecl::SystemChar* substr, float factor)
                    {
                        std::unique_lock<std::mutex> lk(msgLock);
                        progress(pakName.c_str(), substr, compIdx, factor);
                    });
                });
            }

            process.waitUntilComplete();
        }

        if (doMPTFE)
        {
            m_feWorkPath.makeDir();

            progress(_S("Indexing PAKs"), _S(""), compIdx, 0.0);
            m_fePakRouter.build(m_fePaks, [&progress, &compIdx](float factor)
            {
                progress(_S("Indexing PAKs"), _S(""), compIdx, factor);
            });
            progress(_S("Indexing PAKs"), _S(""), compIdx++, 1.0);

            hecl::ProjectPath outPath(m_project.getProjectWorkingPath(), _S("out"));
            outPath.makeDir();
            hecl::ProjectPath feOutPath(outPath, _S("fe"));
            feOutPath.makeDir();
            currentTarget = _S("fe Root");
            progress(currentTarget.c_str(), _S(""), compIdx, 0.0);
            prog = 0;
            nodeCount = m_feNonPaks.size();

            // TODO: Make this more granular
            for (const nod::Node* node : m_feNonPaks)
            {
                node->extractToDirectory(feOutPath.getAbsolutePath(), ctx);
                prog++;
            }
            progress(currentTarget.c_str(), _S(""), compIdx++, 1.0);

            std::mutex msgLock;
            hecl::ClientProcess process;
            prog = 0;
            for (std::pair<std::string, DNAMP3::PAKBridge*> pair : m_feOrderedPaks)
            {
                DNAMP3::PAKBridge& pak = *pair.second;
                if (!pak.m_doExtract)
                    continue;

                const std::string& name = pak.getName();
                hecl::SystemStringView sysName(name);

                {
                    std::unique_lock<std::mutex> lk(msgLock);
                    progress(sysName.c_str(), _S(""), compIdx, 0.0);
                }
                hecl::SystemString pakName = sysName.sys_str();
                process.addLambdaTransaction([&, pakName](hecl::BlenderToken& btok)
                {
                    m_fePakRouter.extractResources(pak, force, btok,
                    [&](const hecl::SystemChar* substr, float factor)
                    {
                        std::unique_lock<std::mutex> lk(msgLock);
                        progress(pakName.c_str(), substr, compIdx, factor);
                    });
                });
            }

            process.waitUntilComplete();
        }
        return true;
    }

    const hecl::Database::DataSpecEntry& getOriginalSpec() const
    {
        return SpecEntMP3;
    }

    const hecl::Database::DataSpecEntry& getUnmodifiedSpec() const
    {
        return SpecEntMP3ORIG;
    }

    hecl::ProjectPath getWorking(class UniqueID64& id)
    {
        return m_pakRouter.getWorking(id);
    }

    bool checkPathPrefix(const hecl::ProjectPath& path) const
    {
        return path.getRelativePath().compare(0, 4, _S("MP3/")) == 0;
    }

    bool validateYAMLDNAType(athena::io::IStreamReader& fp) const
    {
        if (BigYAML::ValidateFromYAMLStream<DNAMP3::MLVL>(fp))
            return true;
        if (BigYAML::ValidateFromYAMLStream<DNAMP3::STRG>(fp))
            return true;
        if (BigYAML::ValidateFromYAMLStream<DNAMP2::STRG>(fp))
            return true;
        return false;
    }

    urde::SObjectTag BuildTagFromPath(const hecl::ProjectPath& path, hecl::BlenderToken& btok) const
    {
        return {};
    }

    void cookMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                  BlendStream& ds, bool fast, hecl::BlenderToken& btok,
                  FCookProgress progress)
    {
    }

    void cookActor(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                   BlendStream& ds, bool fast, hecl::BlenderToken& btok,
                   FCookProgress progress)
    {
    }

    void cookArea(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                  BlendStream& ds, bool fast, hecl::BlenderToken& btok,
                  FCookProgress progress)
    {
    }

    void cookWorld(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                   BlendStream& ds, bool fast, hecl::BlenderToken& btok,
                   FCookProgress progress)
    {
    }

    void cookGuiFrame(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                      BlendStream& ds, hecl::BlenderToken& btok,
                      FCookProgress progress)
    {
    }

    void cookYAML(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                  athena::io::IStreamReader& fin, FCookProgress progress)
    {
    }

    void flattenDependenciesYAML(athena::io::IStreamReader& fin, std::vector<hecl::ProjectPath>& pathsOut)
    {
    }

    void cookAudioGroup(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                        FCookProgress progress)
    {
    }

    void cookSong(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                  FCookProgress progress)
    {
    }

    void cookMapArea(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                     BlendStream& ds, hecl::BlenderToken& btok,
                     FCookProgress progress)
    {
        BlendStream::MapArea mapa = ds.compileMapArea();
        ds.close();
        DNAMP3::MAPA::Cook(mapa, out);
        progress(_S("Done"));
    }

    void cookMapUniverse(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                         BlendStream& ds, hecl::BlenderToken& btok,
                         FCookProgress progress)
    {
    }
};

hecl::Database::DataSpecEntry SpecEntMP3
(
    _S("MP3"),
    _S("Data specification for original Metroid Prime 3 engine"),
    [](hecl::Database::Project& project, hecl::Database::DataSpecTool)
    -> hecl::Database::IDataSpec* {return new struct SpecMP3(&SpecEntMP3, project, false);}
);

hecl::Database::DataSpecEntry SpecEntMP3PC =
{
    _S("MP3-PC"),
    _S("Data specification for PC-optimized Metroid Prime 3 engine"),
    [](hecl::Database::Project& project, hecl::Database::DataSpecTool tool)
    -> hecl::Database::IDataSpec*
    {
        if (tool != hecl::Database::DataSpecTool::Extract)
            return new struct SpecMP3(&SpecEntMP3PC, project, true);
        return nullptr;
    }
};

hecl::Database::DataSpecEntry SpecEntMP3ORIG =
{
    _S("MP3-ORIG"),
    _S("Data specification for unmodified Metroid Prime 3 resources"),
    {}
};

}
