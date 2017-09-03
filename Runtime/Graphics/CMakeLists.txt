if(WIN32)
  set(PLAT_SRCS
      Shaders/CLineRendererShadersHLSL.cpp
      Shaders/CTexturedQuadFilterHLSL.cpp
      Shaders/CColoredQuadFilterHLSL.cpp
      Shaders/CModelShadersHLSL.cpp
      Shaders/CThermalColdFilterHLSL.cpp
      Shaders/CThermalHotFilterHLSL.cpp
      Shaders/CSpaceWarpFilterHLSL.cpp
      Shaders/CCameraBlurFilterHLSL.cpp
      Shaders/CXRayBlurFilterHLSL.cpp
      Shaders/CTextSupportShaderHLSL.cpp
      Shaders/CFogVolumePlaneShaderHLSL.cpp
      Shaders/CFogVolumeFilterHLSL.cpp
      Shaders/CEnergyBarShaderHLSL.cpp
      Shaders/CRadarPaintShaderHLSL.cpp
      Shaders/CMapSurfaceShaderHLSL.cpp
      Shaders/CPhazonSuitFilterHLSL.cpp
      Shaders/CScanLinesFilterHLSL.cpp
      Shaders/CRandomStaticFilterHLSL.cpp
      Shaders/CElementGenShadersHLSL.cpp
      Shaders/CParticleSwooshShadersHLSL.cpp
      Shaders/CFluidPlaneShaderHLSL.cpp
      Shaders/CAABoxShaderHLSL.cpp)
elseif(BOO_HAS_METAL)
  set(PLAT_SRCS
      Shaders/CLineRendererShadersMetal.cpp
      Shaders/CTexturedQuadFilterMetal.cpp
      Shaders/CColoredQuadFilterMetal.cpp
      Shaders/CModelShadersMetal.cpp
      Shaders/CThermalColdFilterMetal.cpp
      Shaders/CThermalHotFilterMetal.cpp
      Shaders/CSpaceWarpFilterMetal.cpp
      Shaders/CCameraBlurFilterMetal.cpp
      Shaders/CXRayBlurFilterMetal.cpp
      Shaders/CTextSupportShaderMetal.cpp
      Shaders/CFogVolumePlaneShaderMetal.cpp
      Shaders/CFogVolumeFilterMetal.cpp
      Shaders/CEnergyBarShaderMetal.cpp
      Shaders/CRadarPaintShaderMetal.cpp
      Shaders/CMapSurfaceShaderMetal.cpp
      Shaders/CPhazonSuitFilterMetal.cpp
      Shaders/CScanLinesFilterMetal.cpp
      Shaders/CRandomStaticFilterMetal.cpp
      Shaders/CElementGenShadersMetal.cpp
      Shaders/CParticleSwooshShadersMetal.cpp
      Shaders/CFluidPlaneShaderMetal.cpp
      Shaders/CAABoxShaderMetal.cpp)
endif()

set(GRAPHICS_SOURCES
    IRenderer.hpp
    IWeaponRenderer.hpp IWeaponRenderer.cpp
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
    CGraphicsPalette.hpp CGraphicsPalette.cpp
    CPVSVisSet.hpp CPVSVisSet.cpp
    CPVSVisOctree.hpp CPVSVisOctree.cpp
    CPVSAreaSet.hpp CPVSAreaSet.cpp
    CGraphics.hpp CGraphics.cpp
    CSimpleShadow.hpp CSimpleShadow.cpp
    CRainSplashGenerator.hpp CRainSplashGenerator.cpp
    Shaders/TShader.hpp Shaders/TMultiBlendShader.hpp Shaders/TShaderDecl.hpp Shaders/TMultiBlendShaderDecl.hpp
    Shaders/CLineRendererShaders.hpp Shaders/CLineRendererShaders.cpp Shaders/CLineRendererShadersGLSL.cpp
    Shaders/CTexturedQuadFilter.hpp Shaders/CTexturedQuadFilter.cpp Shaders/CTexturedQuadFilterGLSL.cpp
    Shaders/CColoredQuadFilter.hpp Shaders/CColoredQuadFilter.cpp Shaders/CColoredQuadFilterGLSL.cpp
    Shaders/CModelShaders.hpp Shaders/CModelShaders.cpp Shaders/CModelShadersGLSL.cpp
    Shaders/CThermalColdFilter.hpp Shaders/CThermalColdFilter.cpp Shaders/CThermalColdFilterGLSL.cpp
    Shaders/CThermalHotFilter.hpp Shaders/CThermalHotFilter.cpp Shaders/CThermalHotFilterGLSL.cpp
    Shaders/CSpaceWarpFilter.hpp Shaders/CSpaceWarpFilter.cpp Shaders/CSpaceWarpFilterGLSL.cpp
    Shaders/CCameraBlurFilter.hpp Shaders/CCameraBlurFilter.cpp Shaders/CCameraBlurFilterGLSL.cpp
    Shaders/CXRayBlurFilter.hpp Shaders/CXRayBlurFilter.cpp Shaders/CXRayBlurFilterGLSL.cpp
    Shaders/CTextSupportShader.hpp Shaders/CTextSupportShader.cpp Shaders/CTextSupportShaderGLSL.cpp
    Shaders/CFogVolumePlaneShader.hpp Shaders/CFogVolumePlaneShader.cpp Shaders/CFogVolumePlaneShaderGLSL.cpp
    Shaders/CFogVolumeFilter.hpp Shaders/CFogVolumeFilter.cpp Shaders/CFogVolumeFilterGLSL.cpp
    Shaders/CEnergyBarShader.hpp Shaders/CEnergyBarShader.cpp Shaders/CEnergyBarShaderGLSL.cpp
    Shaders/CRadarPaintShader.hpp Shaders/CRadarPaintShader.cpp Shaders/CRadarPaintShaderGLSL.cpp
    Shaders/CMapSurfaceShader.hpp Shaders/CMapSurfaceShader.cpp Shaders/CMapSurfaceShaderGLSL.cpp
    Shaders/CPhazonSuitFilter.hpp Shaders/CPhazonSuitFilter.cpp Shaders/CPhazonSuitFilterGLSL.cpp
    Shaders/CScanLinesFilter.hpp Shaders/CScanLinesFilter.cpp Shaders/CScanLinesFilterGLSL.cpp
    Shaders/CRandomStaticFilter.hpp Shaders/CRandomStaticFilter.cpp Shaders/CRandomStaticFilterGLSL.cpp
    Shaders/CElementGenShaders.hpp Shaders/CElementGenShaders.cpp Shaders/CElementGenShadersGLSL.cpp
    Shaders/CParticleSwooshShaders.hpp Shaders/CParticleSwooshShaders.cpp Shaders/CParticleSwooshShadersGLSL.cpp
    Shaders/CFluidPlaneShader.hpp Shaders/CFluidPlaneShader.cpp Shaders/CFluidPlaneShaderGLSL.cpp
    Shaders/CAABoxShader.hpp Shaders/CAABoxShader.cpp Shaders/CAABoxShaderGLSL.cpp
    ${PLAT_SRCS})

runtime_add_list(Graphics GRAPHICS_SOURCES)
