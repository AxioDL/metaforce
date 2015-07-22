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
    return std::shared_ptr<Entity>();
}

std::shared_ptr<Entity> Runtime::loadAsync(const Hash& pathHash,
                                           SGroupLoadStatus* statusOut)
{
    return std::shared_ptr<Entity>();
}

}
}
