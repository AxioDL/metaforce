if(WIN32)
  set(PLAT_SRCS Shaders/CLineRendererShadersHLSL.cpp Shaders/CModelShadersHLSL.cpp)
elseif(APPLE)
  set(PLAT_SRCS Shaders/CLineRendererShadersMetal.cpp Shaders/CModelShadersMetal.cpp)
endif()

set(GRAPHICS_SOURCES
    IRenderer.hpp
    CBooRenderer.hpp CBooRenderer.cpp
    CDrawable.hpp CDrawable.cpp
    CDrawablePlaneObject.hpp CDrawablePlaneObject.cpp
    CLineRenderer.hpp CLineRenderer.cpp
    CMetroidModelInstance.hpp
    CLight.hpp CLight.cpp
    CTexture.hpp CTextureBoo.cpp
    CModel.hpp CModelBoo.cpp
    CSkinnedModel.hpp CSkinnedModel.cpp
    CVertexMorphEffect.hpp CVertexMorphEffect.cpp
    CMoviePlayer.hpp CMoviePlayer.cpp
    CFrustumPlanes.hpp CFrustumPlanes.cpp
    CGraphicsPalette.hpp CGraphicsPalette.cpp
    CGraphics.hpp CGraphics.cpp
    Shaders/CLineRendererShaders.hpp Shaders/CLineRendererShadersGLSL.cpp
    Shaders/CModelShaders.hpp Shaders/CModelShadersGLSL.cpp
    Shaders/CXrayOutlineFilter.hpp Shaders/CXrayOutlineFilterGLSL.cpp
    Shaders/CThermalColdFilter.hpp Shaders/CThermalColdFilterGLSL.cpp
    Shaders/CThermalHotFilter.hpp Shaders/CThermalHotFilterGLSL.cpp
    ${PLAT_SRCS})

runtime_add_list(Graphics GRAPHICS_SOURCES)
