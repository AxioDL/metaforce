#include "HECLDatabase.hpp"

#include "DUMB.hpp"
#include "HMDL.hpp"
#include "MATR.hpp"
#include "STRG.hpp"
#include "TXTR.hpp"

const HECLDatabase::RegistryEntry DATASPEC_TYPES[]
{
    REGISTRY_ENTRY("DUMB", CDUMBProject, CDUMBRuntime),
    REGISTRY_ENTRY("HMDL", CHMDLProject, CHMDLRuntime),
    REGISTRY_ENTRY("MATR", CMATRProject, CMATRRuntime),
    REGISTRY_ENTRY("STRG", CSTRGProject, CSTRGRuntime),
    REGISTRY_ENTRY("TXTR", CTXTRProject, CTXTRRuntime),
    REGISTRY_SENTINEL()
};
