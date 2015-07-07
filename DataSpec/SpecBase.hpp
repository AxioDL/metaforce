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
    bool canExtract(const ExtractPassInfo& info);
    void doExtract(const HECL::Database::Project& project, const ExtractPassInfo& info);

    bool canCook(const HECL::Database::Project& project, const CookTaskInfo& info);
    void doCook(const HECL::Database::Project& project, const CookTaskInfo& info);

    bool canPackage(const HECL::Database::Project& project, const PackagePassInfo& info);
    void gatherDependencies(const HECL::Database::Project& project, const PackagePassInfo& info,
                            std::unordered_set<HECL::ProjectPath>& implicitsOut);
    void doPackage(const HECL::Database::Project& project, const PackagePassInfo& info);

    virtual bool checkFromGCNDisc(NOD::DiscGCN& disc, ExtractOption& opts)=0;
    virtual bool readFromGCNDisc(NOD::DiscGCN& disc)=0;

    virtual bool checkFromWiiDisc(NOD::DiscWii& disc, ExtractOption& opts)=0;
    virtual bool readFromWiiDisc(NOD::DiscWii& disc)=0;

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

