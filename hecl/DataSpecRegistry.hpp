/* Include this file once in the main translation unit of any executable file
 * using HECL's database functionality (see driver/main.cpp)
 */
#ifdef DATA_SPEC_REGISTRY_HPP
#error DataSpecRegistry.hpp may only be included once
#endif
#define DATA_SPEC_REGISTRY_HPP

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

/* Please Call Me! */
void HECLRegisterDataSpecs()
{
    HECL::Database::DATA_SPEC_REGISTRY.push_back(&Retro::SpecEntMP1);
    HECL::Database::DATA_SPEC_REGISTRY.push_back(&Retro::SpecEntMP2);
    HECL::Database::DATA_SPEC_REGISTRY.push_back(&Retro::SpecEntMP3);
}
