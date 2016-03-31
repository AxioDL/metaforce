#include <utility>
#include <stdio.h>
#include <string.h>

#include "SpecBase.hpp"
#include "DNAMP1/DNAMP1.hpp"

#include "DNAMP1/MLVL.hpp"
#include "DNAMP1/STRG.hpp"
#include "DNAMP1/CMDL.hpp"
#include "DNAMP1/ANCS.hpp"
#include "DNACommon/PART.hpp"
#include "DNACommon/SWHC.hpp"
#include "DNACommon/ELSC.hpp"
#include "DNACommon/WPSC.hpp"
#include "DNACommon/CRSC.hpp"
#include "DNACommon/DPSC.hpp"

namespace DataSpec
{

static logvisor::Module Log("urde::SpecMP1");
extern hecl::Database::DataSpecEntry SpecEntMP1;

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
    : SpecBase(specEntry, project, pc),
      m_workPath(project.getProjectWorkingPath(), _S("MP1")),
      m_cookPath(project.getProjectCookedPath(SpecEntMP1), _S("MP1")),
      m_pakRouter(*this, m_workPath, m_cookPath) {}

    void buildPaks(nod::Node& root,
                   const std::vector<hecl::SystemString>& args,
                   ExtractReport& rep)
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

    bool checkFromStandaloneDisc(nod::DiscBase& disc,
                                 const hecl::SystemString& regstr,
                                 const std::vector<hecl::SystemString>& args,
                                 std::vector<ExtractReport>& reps)
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

    bool checkFromTrilogyDisc(nod::DiscBase& disc,
                              const hecl::SystemString& regstr,
                              const std::vector<hecl::SystemString>& args,
                              std::vector<ExtractReport>& reps)
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
        const hecl::ProjectPath& cookPath = m_project.getProjectCookedPath(SpecEntMP1);
        cookPath.makeDir();
        m_cookPath.makeDir();

        progress(_S("Indexing PAKs"), _S(""), 2, 0.0);
        m_pakRouter.build(m_paks, [&progress](float factor)
        {
            progress(_S("Indexing PAKs"), _S(""), 2, factor);
        });
        progress(_S("Indexing PAKs"), _S(""), 2, 1.0);

        hecl::ProjectPath outPath(m_project.getProjectWorkingPath(), _S("out"));
        outPath.makeDir();
        hecl::ProjectPath mp1OutPath(outPath, _S("MP1"));
        mp1OutPath.makeDir();
        progress(_S("MP1 Root"), _S(""), 3, 0.0);
        int prog = 0;
        ctx.progressCB = [&](const std::string& name) {
            hecl::SystemStringView nameView(name);
            progress(_S("MP1 Root"), nameView.sys_str().c_str(), 3, prog / (float)m_nonPaks.size());
        };
        for (const nod::Node* node : m_nonPaks)
        {
            node->extractToDirectory(mp1OutPath.getAbsolutePath(), ctx);
            prog++;
        }
        progress(_S("MP1 Root"), _S(""), 3, 1.0);

        int compIdx = 4;
        prog = 0;
        for (std::pair<std::string, DNAMP1::PAKBridge*> pair : m_orderedPaks)
        {
            DNAMP1::PAKBridge& pak = *pair.second;
            if (!pak.m_doExtract)
                continue;

            const std::string& name = pak.getName();
            hecl::SystemStringView sysName(name);

            progress(sysName.sys_str().c_str(), _S(""), compIdx, 0.0);
            m_pakRouter.extractResources(pak, force,
            [&progress, &sysName, &compIdx](const hecl::SystemChar* substr, float factor)
            {
                progress(sysName.sys_str().c_str(), substr, compIdx, factor);
            });
            progress(sysName.sys_str().c_str(), _S(""), compIdx++, 1.0);
        }

        return true;
    }

    const hecl::Database::DataSpecEntry* getOriginalSpec() const
    {
        return &SpecEntMP1;
    }

    hecl::ProjectPath getWorking(class UniqueID32& id)
    {
        return m_pakRouter.getWorking(id);
    }

    bool checkPathPrefix(const hecl::ProjectPath& path)
    {
        return path.getRelativePath().compare(0, 4, _S("MP1/")) == 0;
    }

    bool validateYAMLDNAType(FILE* fp) const
    {
        athena::io::YAMLDocReader reader;
        yaml_parser_set_input_file(reader.getParser(), fp);
        return reader.ClassTypeOperation([](const char* classType)
        {
            if (!strcmp(classType, DNAMP1::MLVL::DNAType()))
                return true;
            else if (!strcmp(classType, DNAMP1::STRG::DNAType()))
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
            return false;
        });
    }

    void cookMesh(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                  BlendStream& ds, bool fast, FCookProgress progress) const
    {
        Mesh mesh = ds.compileMesh(fast ? hecl::HMDLTopology::Triangles : hecl::HMDLTopology::TriStrips, -1,
        [&progress](int surfCount)
        {
            progress(hecl::SysFormat(_S("%d"), surfCount).c_str());
        });
        if (m_pc)
            DNAMP1::CMDL::HMDLCook(out, in, mesh);
        else
            DNAMP1::CMDL::Cook(out, in, mesh);
    }

    void cookActor(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                   BlendStream& ds, bool fast, FCookProgress progress) const
    {
        Actor actor = ds.compileActor();
        DNAMP1::ANCS::Cook(out, in, actor);
    }

    void cookArea(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                  BlendStream& ds, bool fast, FCookProgress progress) const
    {
    }

    void cookYAML(const hecl::ProjectPath& out, const hecl::ProjectPath& in,
                  FILE* fin, FCookProgress progress) const
    {
        athena::io::YAMLDocReader reader;
        yaml_parser_set_input_file(reader.getParser(), fin);
        if (reader.parse())
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
        }
        progress(_S("Done"));
    }
};

hecl::Database::DataSpecEntry SpecEntMP1 =
{
    _S("MP1"),
    _S("Data specification for original Metroid Prime engine"),
    [](hecl::Database::Project& project, hecl::Database::DataSpecTool)
    -> hecl::Database::IDataSpec* {return new struct SpecMP1(&SpecEntMP1, project, false);}
};

hecl::Database::DataSpecEntry SpecEntMP1PC =
{
    _S("MP1-PC"),
    _S("Data specification for PC-optimized Metroid Prime engine"),
    [](hecl::Database::Project& project, hecl::Database::DataSpecTool tool)
    -> hecl::Database::IDataSpec*
    {
        if (tool != hecl::Database::DataSpecTool::Extract)
            return new struct SpecMP1(&SpecEntMP1PC, project, true);
        return nullptr;
    }
};

}


