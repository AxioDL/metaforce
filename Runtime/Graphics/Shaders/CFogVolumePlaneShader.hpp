#ifndef __URDE_CFOGVOLUMEPLANESHADER_HPP__
#define __URDE_CFOGVOLUMEPLANESHADER_HPP__

#include "TShader.hpp"
#include "zeus/CMatrix4f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CRectangle.hpp"
#include "zeus/CVector4f.hpp"

namespace urde
{

class CFogVolumePlaneShader
{
    friend struct CFogVolumePlaneShaderGLDataBindingFactory;
    friend struct CFogVolumePlaneShaderVulkanDataBindingFactory;
    friend struct CFogVolumePlaneShaderMetalDataBindingFactory;
    friend struct CFogVolumePlaneShaderD3DDataBindingFactory;

    boo::GraphicsDataToken m_token;
    boo::IGraphicsBufferD* m_vbo;
    boo::IShaderDataBinding* m_dataBinds[4];
    std::vector<zeus::CVector4f> m_verts;
    size_t m_vertCapacity = 0;

    void CommitResources(size_t capacity);

public:
    static const zeus::CRectangle DefaultRect;
    void reset(int numVerts) { m_verts.clear(); m_verts.reserve(numVerts); }
    void addFan(const zeus::CVector3f* verts, int numVerts);
    void draw(int pass);

    using _CLS = CFogVolumePlaneShader;
#include "TShaderDecl.hpp"
};

}

#endif // __URDE_CFOGVOLUMEPLANESHADER_HPP__
