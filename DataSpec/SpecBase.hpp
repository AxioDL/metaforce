#ifndef SPECBASE_HPP
#define SPECBASE_HPP

#include <functional>

#include <HECL/Database.hpp>
#include <NOD/NOD.hpp>

namespace Retro
{

extern LogVisor::LogModule LogModule;

struct SpecBase : HECL::Database::IDataSpec
{
    bool canExtract(const ExtractPassInfo& info, std::vector<ExtractReport>& reps);
    void doExtract(const HECL::Database::Project& project, const ExtractPassInfo& info);

    bool canCook(const HECL::Database::Project& project, const CookTaskInfo& info);
    void doCook(const HECL::Database::Project& project, const CookTaskInfo& info);

    bool canPackage(const HECL::Database::Project& project, const PackagePassInfo& info);
    void gatherDependencies(const HECL::Database::Project& project, const PackagePassInfo& info,
                            std::unordered_set<HECL::ProjectPath>& implicitsOut);
    void doPackage(const HECL::Database::Project& project, const PackagePassInfo& info);

    virtual bool checkFromStandaloneDisc(NOD::DiscBase& disc,
                                         const std::string& regstr,
                                         const std::vector<const HECL::SystemString*>& args,
                                         std::vector<ExtractReport>& reps)=0;
    virtual bool checkFromTrilogyDisc(NOD::DiscBase& disc,
                                      const std::string& regstr,
                                      const std::vector<const HECL::SystemString*>& args,
                                      std::vector<ExtractReport>& reps)=0;
    virtual bool extractFromDisc()=0;

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
};

}

#endif // SPECBASE_HPP

