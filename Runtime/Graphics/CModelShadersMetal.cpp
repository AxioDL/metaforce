#include "CModelShaders.hpp"

namespace urde
{

hecl::Runtime::ShaderCacheExtensions
CModelShaders::GetShaderExtensionsMetal(boo::IGraphicsDataFactory::Platform plat)
{
    hecl::Runtime::ShaderCacheExtensions ext(plat);
    return ext;
}

}
