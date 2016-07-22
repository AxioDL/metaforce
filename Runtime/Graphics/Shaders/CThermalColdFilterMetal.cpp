#include "CThermalColdFilter.hpp"

namespace urde
{

TFilterShader<CThermalColdFilter>::IDataBindingFactory* CThermalColdFilter::Initialize(boo::MetalDataFactory::Context& ctx,
                                                                                       boo::IShaderPipeline*& pipeOut,
                                                                                       boo::IVertexFormat*& vtxFmtOut)
{
    return nullptr;
}

}
