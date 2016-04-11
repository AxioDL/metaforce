#include "DNACommon.hpp"
#include "PAK.hpp"
#include "boo/ThreadLocalPtr.hpp"

namespace DataSpec
{

logvisor::Module LogDNACommon("urde::DNACommon");
ThreadLocalPtr<SpecBase> g_curSpec;
ThreadLocalPtr<PAKRouterBase> g_PakRouter;
ThreadLocalPtr<hecl::Database::Project> UniqueIDBridge::s_Project;
UniqueID32 UniqueID32::kInvalidId;

}
