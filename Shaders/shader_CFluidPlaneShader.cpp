#include "shader_CFluidPlaneShader.hpp"

const boo::VertexElementDescriptor Shader_CFluidPlaneShader::VtxFmtElems[5] = {{boo::VertexSemantic::Position4},
                                                                               {boo::VertexSemantic::Normal4, 0},
                                                                               {boo::VertexSemantic::Normal4, 1},
                                                                               {boo::VertexSemantic::Normal4, 2},
                                                                               {boo::VertexSemantic::Color}};

const boo::VertexElementDescriptor Shader_CFluidPlaneShader::TessVtxFmtElems[3] = {
    {boo::VertexSemantic::Position4}, {boo::VertexSemantic::UV4, 0}, {boo::VertexSemantic::UV4, 1}};
