#include "CThermalColdFilter.hpp"

namespace urde
{

TShader<CThermalColdFilter>::IDataBindingFactory* CThermalColdFilter::Initialize(boo::ID3DDataFactory::Context& ctx,
                                                                                       boo::IShaderPipeline*& pipeOut,
                                                                                       boo::IVertexFormat*& vtxFmtOut)
{
    return nullptr;
}

}
