#ifndef SPECBASE_HPP
#define SPECBASE_HPP

#include <functional>

#include <HECL/Database.hpp>
#include <NOD/NOD.hpp>

namespace Retro
{

struct SpecBase : HECL::Database::IDataSpec
{
    bool canExtract(HECL::Database::Project& project, const ExtractPassInfo& info,
                    std::vector<ExtractReport>& reps);
    void doExtract(HECL::Database::Project& project, const ExtractPassInfo& info);

    bool canCook(const HECL::Database::Project& project, const CookTaskInfo& info);
    void doCook(const HECL::Database::Project& project, const CookTaskInfo& info);

    bool canPackage(const HECL::Database::Project& project, const PackagePassInfo& info);
    void gatherDependencies(const HECL::Database::Project& project, const PackagePassInfo& info,
                            std::unordered_set<HECL::ProjectPath>& implicitsOut);
    void doPackage(const HECL::Database::Project& project, const PackagePassInfo& info);

    virtual bool checkStandaloneID(const char* id) const=0;
    virtual bool checkFromStandaloneDisc(HECL::Database::Project& project,
                                         NOD::DiscBase& disc,
                                         const HECL::SystemString& regstr,
                                         const std::vector<HECL::SystemString>& args,
                                         std::vector<ExtractReport>& reps)=0;
    virtual bool checkFromTrilogyDisc(HECL::Database::Project& project,
                                      NOD::DiscBase& disc,
                                      const HECL::SystemString& regstr,
                                      const std::vector<HECL::SystemString>& args,
                                      std::vector<ExtractReport>& reps)=0;
    virtual bool extractFromDisc(HECL::Database::Project& project, NOD::DiscBase& disc)=0;

    virtual bool checkFromProject(HECL::Database::Project& proj)=0;
    virtual bool readFromProject(HECL::Database::Project& proj)=0;

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

private:
    std::unique_ptr<NOD::DiscBase> m_disc;
};

}

#endif // SPECBASE_HPP

