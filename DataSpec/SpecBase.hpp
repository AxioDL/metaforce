#ifndef SPECBASE_HPP
#define SPECBASE_HPP

#include <functional>

#include <HECL/Database.hpp>
#include <NOD/NOD.hpp>

namespace Retro
{

struct SpecBase : HECL::Database::IDataSpec
{
    bool canExtract(const ExtractPassInfo& info, std::vector<ExtractReport>& reps);
    void doExtract(const ExtractPassInfo& info, FExtractProgress progress);

    bool canCook(const CookTaskInfo& info);
    void doCook(const CookTaskInfo& info);

    bool canPackage(const PackagePassInfo& info);
    void gatherDependencies(const PackagePassInfo& info,
                            std::unordered_set<HECL::ProjectPath>& implicitsOut);
    void doPackage(const PackagePassInfo& info);

    virtual bool checkStandaloneID(const char* id) const=0;
    virtual bool checkFromStandaloneDisc(NOD::DiscBase& disc,
                                         const HECL::SystemString& regstr,
                                         const std::vector<HECL::SystemString>& args,
                                         std::vector<ExtractReport>& reps)=0;
    virtual bool checkFromTrilogyDisc(NOD::DiscBase& disc,
                                      const HECL::SystemString& regstr,
                                      const std::vector<HECL::SystemString>& args,
                                      std::vector<ExtractReport>& reps)=0;
    virtual bool extractFromDisc(NOD::DiscBase& disc, bool force,
                                 FExtractProgress progress)=0;

    virtual bool checkFromProject()=0;
    virtual bool readFromProject()=0;

    virtual bool visitGameObjects(std::function<bool(const HECL::Database::ObjectBase&)>)=0;
    struct ILevelSpec
    {
        virtual bool visitLevelObjects(std::function<bool(const HECL::Database::ObjectBase&)>)=0;
        struct IAreaSpec
        {
            virtual bool visitAreaObjects(std::function<bool(const HECL::Database::ObjectBase&)>)=0;
        };
        virtual bool visitAreas(std::function<bool(const IAreaSpec&)>)=0;
    };
    virtual bool visitLevels(std::function<bool(const ILevelSpec&)>)=0;

    inline const HECL::ProjectPath& getMasterShaderPath() const {return m_masterShader;}

    SpecBase(HECL::Database::Project& project)
    : m_project(project),
      m_masterShader(project.getProjectRootPath(), ".hecl/RetroMasterShader.blend") {}
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

