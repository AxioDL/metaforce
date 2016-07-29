#include "CSpaceWarpFilter.hpp"

namespace urde
{

TShader<CSpaceWarpFilter>::IDataBindingFactory* CSpaceWarpFilter::Initialize(boo::MetalDataFactory::Context& ctx,
                                                                             boo::IShaderPipeline*& pipeOut,
                                                                             boo::IVertexFormat*& vtxFmtOut)
{
    return nullptr;
}

}
