#ifndef __PSHAG_CELEMENTGENSHADERS_HPP__
#define __PSHAG_CELEMENTGENSHADERS_HPP__

#include "CGraphics.hpp"
#include "boo/graphicsdev/GL.hpp"
#include "boo/graphicsdev/D3D.hpp"
#include "boo/graphicsdev/Metal.hpp"

namespace pshag
{
class CElementGen;

class CElementGenShaders
{
public:
    struct IDataBindingFactory
    {
        virtual void BuildShaderDataBinding(CElementGen& gen,
                                            boo::IShaderPipeline* regPipeline,
                                            boo::IShaderPipeline* redToAlphaPipeline)=0;
    };

    enum class EShaderClass
    {
        Tex,
        IndTex,
        NoTex
    };

private:
    static boo::IShaderPipeline* m_texZTestZWrite;
    static boo::IShaderPipeline* m_texNoZTestZWrite;
    static boo::IShaderPipeline* m_texZTestNoZWrite;
    static boo::IShaderPipeline* m_texNoZTestNoZWrite;
    static boo::IShaderPipeline* m_texAdditiveZTest;
    static boo::IShaderPipeline* m_texAdditiveNoZTest;
    static boo::IShaderPipeline* m_texRedToAlphaZTest;
    static boo::IShaderPipeline* m_texRedToAlphaNoZTest;

    static boo::IShaderPipeline* m_indTexZWrite;
    static boo::IShaderPipeline* m_indTexNoZWrite;
    static boo::IShaderPipeline* m_indTexAdditive;

    static boo::IShaderPipeline* m_cindTexZWrite;
    static boo::IShaderPipeline* m_cindTexNoZWrite;
    static boo::IShaderPipeline* m_cindTexAdditive;

    static boo::IShaderPipeline* m_noTexZTestZWrite;
    static boo::IShaderPipeline* m_noTexNoZTestZWrite;
    static boo::IShaderPipeline* m_noTexZTestNoZWrite;
    static boo::IShaderPipeline* m_noTexNoZTestNoZWrite;
    static boo::IShaderPipeline* m_noTexAdditiveZTest;
    static boo::IShaderPipeline* m_noTexAdditiveNoZTest;

    static boo::IVertexFormat* m_vtxFormatTex; /* No OpenGL */
    static boo::IVertexFormat* m_vtxFormatIndTex; /* No OpenGL */
    static boo::IVertexFormat* m_vtxFormatNoTex; /* No OpenGL */

    static std::unique_ptr<IDataBindingFactory> m_bindFactory;
    static boo::GraphicsDataToken m_gfxToken;

public:
    static IDataBindingFactory* Initialize(boo::GLDataFactory& factory);
#if _WIN32
    static IDataBindingFactory* Initialize(boo::ID3DDataFactory& factory);
#elif __APPLE__
    static IDataBindingFactory* Initialize(boo::MetalDataFactory& factory);
#endif

    static void Initialize();
    static EShaderClass GetShaderClass(CElementGen& gen);
    static void BuildShaderDataBinding(CElementGen& gen);
};

}

#endif // __PSHAG_CELEMENTGENSHADERS_HPP__
