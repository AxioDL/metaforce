if(WIN32)
  set(PLAT_SRCS Shaders/CLineRendererShadersHLSL.cpp Shaders/CModelShadersHLSL.cpp Shaders/CThermalColdFilterHLSL.cpp)
elseif(APPLE)
  set(PLAT_SRCS Shaders/CLineRendererShadersMetal.cpp Shaders/CModelShadersMetal.cpp Shaders/CThermalColdFilterMetal.cpp)
endif()

set(GRAPHICS_SOURCES
    IRenderer.hpp
    IWeaponRenderer.hpp IWeaponRenderer.cpp
    CBooRenderer.hpp CBooRenderer.cpp
    CDrawable.hpp CDrawable.cpp
    CDrawablePlaneObject.hpp CDrawablePlaneObject.cpp
    CLineRenderer.hpp CLineRenderer.cpp
    CMetroidModelInstance.hpp CMetroidModelInstance.cpp
    CLight.hpp CLight.cpp
    CTexture.hpp CTextureBoo.cpp
    CModel.hpp CModelBoo.cpp
    CSkinnedModel.hpp CSkinnedModel.cpp
    CVertexMorphEffect.hpp CVertexMorphEffect.cpp
    CMoviePlayer.hpp CMoviePlayer.cpp
    CFrustumPlanes.hpp CFrustumPlanes.cpp
    CGraphicsPalette.hpp CGraphicsPalette.cpp
    CPVSVisSet.hpp CPVSVisSet.cpp
    CPVSVisOctree.hpp CPVSVisOctree.cpp
    CPVSBounds.hpp CPVSBounds.cpp
    CPVSAreaSet.hpp CPVSAreaSet.cpp
    CGraphics.hpp CGraphics.cpp
    Shaders/TShader.hpp Shaders/TShaderDecl.hpp
    Shaders/CLineRendererShaders.hpp Shaders/CLineRendererShaders.cpp Shaders/CLineRendererShadersGLSL.cpp
    Shaders/CModelShaders.hpp Shaders/CModelShaders.cpp Shaders/CModelShadersGLSL.cpp
    Shaders/CXrayOutlineFilter.hpp Shaders/CXrayOutlineFilter.cpp Shaders/CXrayOutlineFilterGLSL.cpp
    Shaders/CThermalColdFilter.hpp Shaders/CThermalColdFilter.cpp Shaders/CThermalColdFilterGLSL.cpp
    Shaders/CThermalHotFilter.hpp Shaders/CThermalHotFilter.cpp Shaders/CThermalHotFilterGLSL.cpp
    ${PLAT_SRCS})

runtime_add_list(Graphics GRAPHICS_SOURCES)
