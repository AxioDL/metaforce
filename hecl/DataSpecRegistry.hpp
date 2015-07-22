#include "HECL/Database.hpp"

namespace HECL
{
namespace Database
{
/* Centralized registry for DataSpec lookup */
std::vector<const struct DataSpecEntry*> DATA_SPEC_REGISTRY;
}
}

/* RetroCommon specs */
namespace Retro
{
    extern HECL::Database::DataSpecEntry SpecEntMP1;
    extern HECL::Database::DataSpecEntry SpecEntMP2;
    extern HECL::Database::DataSpecEntry SpecEntMP3;
}

extern "C" void HECLDataSpecs()
{
    HECL::Printf(Retro::SpecEntMP1.m_name);
    HECL::Printf(Retro::SpecEntMP2.m_name);
    HECL::Printf(Retro::SpecEntMP3.m_name);
}
