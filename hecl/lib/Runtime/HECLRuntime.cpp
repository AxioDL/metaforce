#include "HECL/Runtime.hpp"

namespace HECL
{
namespace Runtime
{

Runtime::Runtime(const HECL::SystemString& hlpkDirectory)
{
}

Runtime::~Runtime()
{
}

std::shared_ptr<Entity> Runtime::loadSync(const Hash& pathHash)
{
}

std::shared_ptr<Entity> Runtime::loadAsync(const Hash& pathHash,
                                           SGroupLoadStatus* statusOut)
{
}

}
}
