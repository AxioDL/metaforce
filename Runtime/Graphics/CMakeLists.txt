if(WIN32)
  set(PLAT_SRCS
      Shaders/CLineRendererShaderHLSL.cpp
      Shaders/CTexturedQuadFilterHLSL.cpp
      Shaders/CColoredQuadFilterHLSL.cpp
      Shaders/CModelShadersHLSL.cpp
      Shaders/CThermalColdFilterHLSL.cpp
      Shaders/CThermalHotFilterHLSL.cpp
      Shaders/CSpaceWarpFilterHLSL.cpp)
elseif(APPLE)
  set(PLAT_SRCS
      Shaders/CLineRendererShadersMetal.cpp
      Shaders/CTexturedQuadFilterMetal.cpp
      Shaders/CColoredQuadFilterMetal.cpp
      Shaders/CModelShadersMetal.cpp
      Shaders/CThermalColdFilterMetal.cpp
      Shaders/CThermalHotFilterMetal.cpp
      Shaders/CSpaceWarpFilterMetal.cpp)
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
    Shaders/TShader.hpp Shaders/TMultiBlendShader.hpp Shaders/TShaderDecl.hpp Shaders/TMultiBlendShaderDecl.hpp
    Shaders/CLineRendererShaders.hpp Shaders/CLineRendererShaders.cpp Shaders/CLineRendererShadersGLSL.cpp
    Shaders/CTexturedQuadFilter.hpp Shaders/CTexturedQuadFilter.cpp Shaders/CTexturedQuadFilterGLSL.cpp
    Shaders/CColoredQuadFilter.hpp Shaders/CColoredQuadFilter.cpp Shaders/CColoredQuadFilterGLSL.cpp
    Shaders/CModelShaders.hpp Shaders/CModelShaders.cpp Shaders/CModelShadersGLSL.cpp
    Shaders/CXrayOutlineFilter.hpp Shaders/CXrayOutlineFilter.cpp Shaders/CXrayOutlineFilterGLSL.cpp
    Shaders/CThermalColdFilter.hpp Shaders/CThermalColdFilter.cpp Shaders/CThermalColdFilterGLSL.cpp
    Shaders/CThermalHotFilter.hpp Shaders/CThermalHotFilter.cpp Shaders/CThermalHotFilterGLSL.cpp
    Shaders/CSpaceWarpFilter.hpp Shaders/CSpaceWarpFilter.cpp Shaders/CSpaceWarpFilterGLSL.cpp
    ${PLAT_SRCS})

runtime_add_list(Graphics GRAPHICS_SOURCES)
