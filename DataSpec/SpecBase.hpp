#ifndef SPECBASE_HPP
#define SPECBASE_HPP

#include <functional>

#include <HECL/Database.hpp>
#include <NOD/NOD.hpp>

namespace Retro
{

struct SpecBase : HECL::Database::IDataSpec
{
    bool canExtract(const ExtractPassInfo& info, std::list<ExtractReport>& reps);
    void doExtract(const ExtractPassInfo& info, FProgress progress);

    bool canCook(const HECL::ProjectPath& path);
    void doCook(const HECL::ProjectPath& path, const HECL::ProjectPath& cookedPath);

    bool canPackage(const PackagePassInfo& info);
    void gatherDependencies(const PackagePassInfo& info,
                            std::unordered_set<HECL::ProjectPath>& implicitsOut);
    void doPackage(const PackagePassInfo& info);

    virtual bool checkStandaloneID(const char* id) const=0;
    virtual bool checkFromStandaloneDisc(NOD::DiscBase& disc,
                                         const HECL::SystemString& regstr,
                                         const std::list<HECL::SystemString>& args,
                                         std::list<ExtractReport>& reps)=0;
    virtual bool checkFromTrilogyDisc(NOD::DiscBase& disc,
                                      const HECL::SystemString& regstr,
                                      const std::list<HECL::SystemString>& args,
                                      std::list<ExtractReport>& reps)=0;
    virtual bool extractFromDisc(NOD::DiscBase& disc, bool force,
                                 FProgress progress)=0;

    virtual bool validateYAMLDNAType(FILE* fp) const=0;

    const HECL::ProjectPath& getMasterShaderPath() const {return m_masterShader;}

    SpecBase(HECL::Database::Project& project)
    : m_project(project),
      m_masterShader(project.getProjectWorkingPath(), ".hecl/RetroMasterShader.blend") {}
protected:
    HECL::Database::Project& m_project;
    HECL::ProjectPath m_masterShader;
private:
    std::unique_ptr<NOD::DiscBase> m_disc;
    bool m_isWii;
    bool m_standalone;
};

}

#endif // SPECBASE_HPP

