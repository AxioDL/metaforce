#ifndef __URDE_CLINERENDERER_HPP__
#define __URDE_CLINERENDERER_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CVector4f.hpp"
#include "zeus/CColor.hpp"
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"
#include "hecl/VertexBufferPool.hpp"
#include "hecl/UniformBufferPool.hpp"

namespace urde
{

class CLineRenderer
{
public:
    enum class EPrimitiveMode
    {
        Lines,
        LineStrip,
        LineLoop
    };

    struct SDrawVertTex
    {
        zeus::CVector4f pos;
        zeus::CColor color;
        zeus::CVector2f uv;
    };

    struct SDrawVertNoTex
    {
        zeus::CVector4f pos;
        zeus::CColor color;
    };

    struct SDrawUniform
    {
        zeus::CColor moduColor;
    };

private:
    EPrimitiveMode m_mode;
    u32 m_maxVerts;
    u32 m_nextVert = 0;
    bool m_final = false;
    bool m_textured;

    zeus::CVector3f m_firstPos;
    zeus::CVector3f m_secondPos;
    zeus::CVector2f m_firstUV;
    zeus::CColor m_firstColor;
    float m_firstWidth;

    zeus::CVector3f m_lastPos;
    zeus::CVector3f m_lastPos2;
    zeus::CVector2f m_lastUV;
    zeus::CColor m_lastColor;
    float m_lastWidth;

    static rstl::reserved_vector<SDrawVertTex, 1024> g_StaticLineVertsTex;
    static rstl::reserved_vector<SDrawVertNoTex, 1024> g_StaticLineVertsNoTex;

    static hecl::VertexBufferPool<SDrawVertTex> s_vertPoolTex;
    static hecl::VertexBufferPool<SDrawVertNoTex> s_vertPoolNoTex;
    static hecl::UniformBufferPool<SDrawUniform> s_uniformPool;

public:
    hecl::VertexBufferPool<SDrawVertTex>::Token m_vertBufTex;
    hecl::VertexBufferPool<SDrawVertNoTex>::Token m_vertBufNoTex;
    hecl::UniformBufferPool<SDrawUniform>::Token m_uniformBuf;
    boo::ObjToken<boo::IShaderDataBinding> m_shaderBind;

    CLineRenderer(boo::IGraphicsDataFactory::Context& ctx,
                  EPrimitiveMode mode, u32 maxVerts, const boo::ObjToken<boo::ITexture>& texture,
                  bool additive, bool zTest = false);
    CLineRenderer(EPrimitiveMode mode, u32 maxVerts, const boo::ObjToken<boo::ITexture>& texture,
                  bool additive, bool zTest = false);
    CLineRenderer(CLineRenderer&&) = default;

    void Reset();
    void AddVertex(const zeus::CVector3f& position, const zeus::CColor& color, float width,
                   const zeus::CVector2f& uv=zeus::CVector2f::skZero);
    void Render(const zeus::CColor& moduColor=zeus::CColor::skWhite);

    static void UpdateBuffers()
    {
        s_vertPoolTex.updateBuffers();
        s_vertPoolNoTex.updateBuffers();
        s_uniformPool.updateBuffers();
    }

    static void Initialize();
    static void Shutdown();
};

}

#endif // __URDE_CLINERENDERER_HPP__
