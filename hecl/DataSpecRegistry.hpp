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

/* An overzealous optimizing compiler/linker may not init the specs if
 * there's no in-code reference.. this is a simple hack to solve that */
extern "C" void HECLDataSpecs()
{
    HECL::Printf(_S("%s\n"), Retro::SpecEntMP1.m_name);
    HECL::Printf(_S("%s\n"), Retro::SpecEntMP2.m_name);
    HECL::Printf(_S("%s\n"), Retro::SpecEntMP3.m_name);
}
