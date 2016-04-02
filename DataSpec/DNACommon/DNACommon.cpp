#include "DNACommon.hpp"
#include "PAK.hpp"

namespace DataSpec
{

logvisor::Module LogDNACommon("urde::DNACommon");
SpecBase* g_curSpec = nullptr;
PAKRouterBase* g_PakRouter = nullptr;
hecl::Database::Project* UniqueIDBridge::s_Project = nullptr;

}
