#ifndef __PSHAG_CLINERENDERER_HPP__
#define __PSHAG_CLINERENDERER_HPP__

#include "RetroTypes.hpp"
#include "CVector3f.hpp"
#include "CColor.hpp"
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"

namespace pshag
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

public:
    EPrimitiveMode m_mode;
    u32 m_maxVerts;
    u32 m_nextVert = 0;
    bool m_final = false;
    bool m_textured;

    Zeus::CVector3f m_firstPos;
    Zeus::CVector3f m_secondPos;
    Zeus::CVector2f m_firstUV;
    Zeus::CColor m_firstColor;
    float m_firstWidth;

    Zeus::CVector3f m_lastPos;
    Zeus::CVector3f m_lastPos2;
    Zeus::CVector2f m_lastUV;
    Zeus::CColor m_lastColor;
    float m_lastWidth;

public:
    boo::GraphicsDataToken m_gfxToken;
    boo::IGraphicsBufferD* m_vertBuf;
    boo::IGraphicsBufferD* m_uniformBuf;
    boo::IShaderDataBinding* m_shaderBind = nullptr;

    CLineRenderer(EPrimitiveMode mode, u32 maxVerts, boo::ITexture* texture, bool additive);

    void Reset();
    void AddVertex(const Zeus::CVector3f& position, const Zeus::CColor& color, float width,
                   const Zeus::CVector2f& uv=Zeus::CVector2f::skZero);
    void Render(const Zeus::CColor& moduColor=Zeus::CColor::skWhite);

    static void Initialize();
    static void Shutdown();
};

}

#endif // __PSHAG_CLINERENDERER_HPP__
