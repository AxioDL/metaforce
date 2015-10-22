#include <utility>
#include <set>

#include "SpecBase.hpp"
#include "DNAMP3/DNAMP3.hpp"

#include "DNAMP3/MLVL.hpp"
#include "DNAMP3/STRG.hpp"
#include "DNAMP2/STRG.hpp"

namespace Retro
{

static LogVisor::LogModule Log("Retro::SpecMP3");
extern HECL::Database::DataSpecEntry SpecEntMP3;

struct SpecMP3 : SpecBase
{
    bool checkStandaloneID(const char* id) const
    {
        if (!memcmp(id, "RM3", 3))
            return true;
        return false;
    }

    bool doMP3 = false;
    std::vector<const NOD::Node*> m_nonPaks;
    std::vector<DNAMP3::PAKBridge> m_paks;
    std::map<std::string, DNAMP3::PAKBridge*, CaseInsensitiveCompare> m_orderedPaks;

    HECL::ProjectPath m_workPath;
    HECL::ProjectPath m_cookPath;
    PAKRouter<DNAMP3::PAKBridge> m_pakRouter;

    /* These are populated when extracting MPT's frontend (uses MP3's DataSpec) */
    bool doMPTFE = false;
    std::vector<const NOD::Node*> m_feNonPaks;
    std::vector<DNAMP3::PAKBridge> m_fePaks;
    std::map<std::string, DNAMP3::PAKBridge*, CaseInsensitiveCompare> m_feOrderedPaks;

    HECL::ProjectPath m_feWorkPath;
    HECL::ProjectPath m_feCookPath;
    PAKRouter<DNAMP3::PAKBridge> m_fePakRouter;

    SpecMP3(HECL::Database::Project& project)
    : SpecBase(project),
      m_workPath(project.getProjectWorkingPath(), _S("MP3")),
      m_cookPath(project.getProjectCookedPath(SpecEntMP3), _S("MP3")),
      m_pakRouter(*this, m_workPath, m_cookPath),
      m_feWorkPath(project.getProjectWorkingPath(), _S("fe")),
      m_feCookPath(project.getProjectCookedPath(SpecEntMP3), _S("fe")),
      m_fePakRouter(*this, m_feWorkPath, m_feCookPath) {}

    void buildPaks(NOD::Node& root,
                   const std::vector<HECL::SystemString>& args,
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
        for (const NOD::Node& child : root)
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
                            HECL::SystemChar idxChar = lowerName[7];
                            for (const HECL::SystemString& arg : args)
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
                            for (const HECL::SystemString& arg : args)
                            {
#if HECL_UCS2
                                std::string lowerArg = HECL::WideToUTF8(arg);
#else
                                std::string lowerArg = arg;
#endif
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
            HECL::SystemStringView nameView(item.first);
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

    bool checkFromStandaloneDisc(NOD::DiscBase& disc,
                                 const HECL::SystemString& regstr,
                                 const std::vector<HECL::SystemString>& args,
                                 std::vector<ExtractReport>& reps)
    {
        doMP3 = true;
        NOD::Partition* partition = disc.getDataPartition();
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
        HECL::SystemStringView buildView(buildStr);
        rep.desc += _S(" (") + buildView + _S(")");

        /* Iterate PAKs and build level options */
        NOD::Node& root = partition->getFSTRoot();
        buildPaks(root, args, rep, false);

        return true;
    }

    bool checkFromTrilogyDisc(NOD::DiscBase& disc,
                              const HECL::SystemString& regstr,
                              const std::vector<HECL::SystemString>& args,
                              std::vector<ExtractReport>& reps)
    {
        std::vector<HECL::SystemString> mp3args;
        std::vector<HECL::SystemString> feargs;
        if (args.size())
        {
            /* Needs filter */
            for (const HECL::SystemString& arg : args)
            {
                HECL::SystemString lowerArg = arg;
                HECL::ToLower(lowerArg);
                if (!lowerArg.compare(0, 3, _S("mp3")))
                {
                    doMP3 = true;
                    mp3args.reserve(args.size());
                    size_t slashPos = arg.find(_S('/'));
                    if (slashPos == HECL::SystemString::npos)
                        slashPos = arg.find(_S('\\'));
                    if (slashPos != HECL::SystemString::npos)
                        mp3args.emplace_back(HECL::SystemString(arg.begin() + slashPos + 1, arg.end()));
                }
            }

            for (const HECL::SystemString& arg : args)
            {
                HECL::SystemString lowerArg = arg;
                HECL::ToLower(lowerArg);
                if (!lowerArg.compare(0, 2, _S("fe")))
                {
                    doMPTFE = true;
                    feargs.reserve(args.size());
                    size_t slashPos = arg.find(_S('/'));
                    if (slashPos == HECL::SystemString::npos)
                        slashPos = arg.find(_S('\\'));
                    if (slashPos != HECL::SystemString::npos)
                        feargs.emplace_back(HECL::SystemString(arg.begin() + slashPos + 1, arg.end()));
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

        NOD::Partition* partition = disc.getDataPartition();
        NOD::Node& root = partition->getFSTRoot();

        /* MP3 extract */
        if (doMP3)
        {
            NOD::Node::DirectoryIterator dolIt = root.find("rs5mp3_p.dol");
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
            HECL::SystemStringView buildView(buildStr);
            rep.desc += _S(" (") + buildView + _S(")");


            /* Iterate PAKs and build level options */
            NOD::Node::DirectoryIterator mp3It = root.find("MP3");
            if (mp3It == root.end())
                return false;
            buildPaks(*mp3It, mp3args, rep, false);
        }

        /* MPT Frontend extract */
        if (doMPTFE)
        {
            NOD::Node::DirectoryIterator dolIt = root.find("rs5fe_p.dol");
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
                HECL::SystemStringView buildView(buildStr);
                rep.desc += _S(" (") + buildView + _S(")");
            }

            /* Iterate PAKs and build level options */
            NOD::Node::DirectoryIterator feIt = root.find("fe");
            if (feIt == root.end())
                return false;
            buildPaks(*feIt, feargs, rep, true);
        }

        return true;
    }

    bool extractFromDisc(NOD::DiscBase&, bool force, FProgress progress)
    {
        int compIdx = 2;
        HECL::SystemString currentTarget = _S("");
        size_t nodeCount = 0;
        int prog = 0;
        NOD::ExtractionContext ctx = {true, force,
                                      [&](const std::string& name)
                                      {
                                          HECL::SystemStringView nameView(name);
                                          progress(currentTarget.c_str(), nameView.sys_str().c_str(), compIdx, prog / (float)nodeCount);
                                      }};
        if (doMP3)
        {
            progress(_S("Indexing PAKs"), _S(""), compIdx, 0.0);
            m_pakRouter.build(m_paks, [&progress, &compIdx](float factor)
            {
                progress(_S("Indexing PAKs"), _S(""), compIdx, factor);
            });
            progress(_S("Indexing PAKs"), _S(""), compIdx++, 1.0);

            HECL::ProjectPath mp3WorkPath(m_project.getProjectWorkingPath(), "MP3");
            mp3WorkPath.makeDir();
            currentTarget = _S("MP3 Root");
            progress(currentTarget.c_str(), _S(""), compIdx, 0.0);
            prog = 0;

            nodeCount = m_nonPaks.size();
            // TODO: Make this more granular
            for (const NOD::Node* node : m_nonPaks)
            {
                node->extractToDirectory(mp3WorkPath.getAbsolutePath(), ctx);
                prog++;
            }
            ctx.progressCB = nullptr;

            progress(currentTarget.c_str(), _S(""), compIdx++, 1.0);

            const HECL::ProjectPath& cookPath = m_project.getProjectCookedPath(SpecEntMP3);
            cookPath.makeDir();
            HECL::ProjectPath mp3CookPath(cookPath, "MP3");
            mp3CookPath.makeDir();

            prog = 0;
            for (std::pair<std::string, DNAMP3::PAKBridge*> pair : m_orderedPaks)
            {
                DNAMP3::PAKBridge& pak = *pair.second;
                if (!pak.m_doExtract)
                    continue;

                const std::string& name = pak.getName();
                HECL::SystemStringView sysName(name);

                progress(sysName.sys_str().c_str(), _S(""), compIdx, 0.0);
                m_pakRouter.extractResources(pak, force,
                [&progress, &sysName, &compIdx](const HECL::SystemChar* substr, float factor)
                {
                    progress(sysName.sys_str().c_str(), substr, compIdx, factor);
                });
                progress(sysName.sys_str().c_str(), _S(""), compIdx++, 1.0);
            }
        }

        if (doMPTFE)
        {
            progress(_S("Indexing PAKs"), _S(""), compIdx, 0.0);
            m_fePakRouter.build(m_fePaks, [&progress, &compIdx](float factor)
            {
                progress(_S("Indexing PAKs"), _S(""), compIdx, factor);
            });
            progress(_S("Indexing PAKs"), _S(""), compIdx++, 1.0);

            m_feWorkPath.makeDir();
            currentTarget = _S("fe Root");
            progress(currentTarget.c_str(), _S(""), compIdx, 0.0);
            prog = 0;
            nodeCount = m_feNonPaks.size();

            // TODO: Make this more granular
            for (const NOD::Node* node : m_feNonPaks)
            {
                node->extractToDirectory(m_feWorkPath.getAbsolutePath(), ctx);
                prog++;
            }
            progress(currentTarget.c_str(), _S(""), compIdx++, 1.0);

            const HECL::ProjectPath& cookPath = m_project.getProjectCookedPath(SpecEntMP3);
            cookPath.makeDir();
            m_feCookPath.makeDir();

            prog = 0;
            for (std::pair<std::string, DNAMP3::PAKBridge*> pair : m_feOrderedPaks)
            {
                DNAMP3::PAKBridge& pak = *pair.second;
                if (!pak.m_doExtract)
                    continue;

                const std::string& name = pak.getName();
                HECL::SystemStringView sysName(name);

                progress(sysName.sys_str().c_str(), _S(""), compIdx, 0.0);
                m_fePakRouter.extractResources(pak, force,
                [&progress, &sysName, &compIdx](const HECL::SystemChar* substr, float factor)
                {
                    progress(sysName.sys_str().c_str(), substr, compIdx, factor);
                });
                progress(sysName.sys_str().c_str(), _S(""), compIdx++, 1.0);
            }
        }
        return true;
    }

    bool checkPathPrefix(const HECL::ProjectPath& path)
    {
        return path.getRelativePath().compare(0, 4, _S("MP3/")) == 0;
    }

    bool validateYAMLDNAType(FILE* fp) const
    {
        if (BigYAML::ValidateFromYAMLFile<DNAMP3::MLVL>(fp))
            return true;
        if (BigYAML::ValidateFromYAMLFile<DNAMP3::STRG>(fp))
            return true;
        if (BigYAML::ValidateFromYAMLFile<DNAMP2::STRG>(fp))
            return true;
        return false;
    }

    void cookMesh(const HECL::ProjectPath& out, const HECL::ProjectPath& in,
                  BlendStream& ds, bool fast, FCookProgress progress) const
    {
    }

    void cookActor(const HECL::ProjectPath& out, const HECL::ProjectPath& in,
                   BlendStream& ds, bool fast, FCookProgress progress) const
    {
    }

    void cookArea(const HECL::ProjectPath& out, const HECL::ProjectPath& in,
                  BlendStream& ds, bool fast, FCookProgress progress) const
    {
    }

    void cookYAML(const HECL::ProjectPath& out, const HECL::ProjectPath& in,
                  FILE* fin, FCookProgress progress) const
    {
    }
};

HECL::Database::DataSpecEntry SpecEntMP3
(
    _S("MP3"),
    _S("Data specification for original Metroid Prime 3 engine"),
    [](HECL::Database::Project& project, HECL::Database::DataSpecTool)
    -> HECL::Database::IDataSpec* {return new struct SpecMP3(project);}
);

}
