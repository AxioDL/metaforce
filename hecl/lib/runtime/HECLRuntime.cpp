#include "HECLRuntime.hpp"

namespace HECLRuntime
{

HECLRuntime::HECLRuntime(const HECL::SystemString& hlpkDirectory)
{
}

HECLRuntime::~HECLRuntime()
{
}

std::shared_ptr<RuntimeEntity> HECLRuntime::loadSync(const HECL::Hash& pathHash)
{
}

std::shared_ptr<RuntimeEntity> HECLRuntime::loadAsync(const HECL::Hash& pathHash,
                                                      SGroupLoadStatus* statusOut)
{
}

}
