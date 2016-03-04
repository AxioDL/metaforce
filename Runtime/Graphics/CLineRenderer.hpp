#ifndef __PSHAG_CLINERENDERER_HPP__
#define __PSHAG_CLINERENDERER_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CColor.hpp"
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"

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

public:
    boo::GraphicsDataToken m_gfxToken;
    boo::IGraphicsBufferD* m_vertBuf;
    boo::IGraphicsBufferD* m_uniformBuf;
    boo::IShaderDataBinding* m_shaderBind = nullptr;

    CLineRenderer(EPrimitiveMode mode, u32 maxVerts, boo::ITexture* texture, bool additive);

    void Reset();
    void AddVertex(const zeus::CVector3f& position, const zeus::CColor& color, float width,
                   const zeus::CVector2f& uv=zeus::CVector2f::skZero);
    void Render(const zeus::CColor& moduColor=zeus::CColor::skWhite);

    static void Initialize();
    static void Shutdown();
};

}

#endif // __PSHAG_CLINERENDERER_HPP__
