#ifndef __URDE_CRADARPAINTSHADER_HPP__
#define __URDE_CRADARPAINTSHADER_HPP__

#include "TShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CRectangle.hpp"
#include "Camera/CCameraFilter.hpp"

namespace urde
{

class CRadarPaintShader
{
    friend struct CRadarPaintShaderGLDataBindingFactory;
    friend struct CRadarPaintShaderVulkanDataBindingFactory;
    friend struct CRadarPaintShaderMetalDataBindingFactory;
    friend struct CRadarPaintShaderD3DDataBindingFactory;

public:
    struct Instance
    {
        zeus::CVector3f pos[4];
        zeus::CVector2f uv[4];
        zeus::CColor color;
    };

private:
    boo::ObjToken<boo::IGraphicsBufferD> m_vbo;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
    boo::ObjToken<boo::IShaderDataBinding> m_dataBind;
    const CTexture* m_tex = nullptr;
    size_t m_maxInsts = 0;

public:
    void draw(const std::vector<Instance>& instances, const CTexture* tex);

    using _CLS = CRadarPaintShader;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CRADARPAINTSHADER_HPP__
