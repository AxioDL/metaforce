#include "HECL/Database.hpp"

namespace HECL
{
namespace Database
{

/* Centralized registry for DataSpec lookup */
std::vector<const struct DataSpecEntry*> DATA_SPEC_REGISTRY;

}
}
