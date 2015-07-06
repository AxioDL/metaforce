#include "SpecBase.hpp"

namespace Retro
{

struct SpecMP1 : public SpecBase
{
    bool checkFromGCNDisc(const NOD::DiscGCN& disc)
    {
    }
    bool readFromGCNDisc(const NOD::DiscGCN& disc)
    {
    }

    bool checkFromWiiDisc(const NOD::DiscWii& disc)
    {
    }
    bool readFromWiiDisc(const NOD::DiscWii& disc)
    {
    }

    bool checkFromProject(const HECL::Database::Project& proj)
    {
    }
    bool readFromProject(const HECL::Database::Project& proj)
    {
    }

    bool visitGameObjects(std::function<bool(const HECL::Database::ObjectBase&)>)
    {
    }
    struct LevelSpec : public ILevelSpec
    {
        bool visitLevelObjects(std::function<bool(const HECL::Database::ObjectBase&)>)
        {
        }
        struct AreaSpec : public IAreaSpec
        {
            bool visitAreaObjects(std::function<bool(const HECL::Database::ObjectBase&)>)
            {
            }
        };
        bool visitAreas(std::function<bool(const IAreaSpec&)>)
        {
        }
    };
    bool visitLevels(std::function<bool(const ILevelSpec&)>)
    {
    }
};

HECL::Database::DataSpecEntry SpecMP1 =
{
    _S("MP1"),
    _S("Data specification for original Metroid Prime engine"),
    [](HECL::Database::DataSpecTool) -> HECL::Database::IDataSpec* {return new struct SpecMP1;}
};

}


