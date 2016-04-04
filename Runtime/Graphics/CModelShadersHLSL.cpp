#include "CModelShaders.hpp"

namespace urde
{

hecl::Runtime::ShaderCacheExtensions
CModelShaders::GetShaderExtensionsHLSL(boo::IGraphicsDataFactory::Platform plat)
{
    hecl::Runtime::ShaderCacheExtensions ext(plat);
    return ext;
}

}
