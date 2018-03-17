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
      Shaders/CDecalShadersHLSL.cpp
      Shaders/CParticleSwooshShadersHLSL.cpp
      Shaders/CFluidPlaneShaderHLSL.cpp
      Shaders/CAABoxShaderHLSL.cpp
      Shaders/CWorldShadowShaderHLSL.cpp)
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
      Shaders/CDecalShadersMetal.cpp
      Shaders/CParticleSwooshShadersMetal.cpp
      Shaders/CFluidPlaneShaderMetal.cpp
      Shaders/CAABoxShaderMetal.cpp
      Shaders/CWorldShadowShaderMetal.cpp)
endif()

if(NOT WINDOWS_STORE)
  list(APPEND PLAT_SRCS
      Shaders/CLineRendererShadersGLSL.cpp
      Shaders/CTexturedQuadFilterGLSL.cpp
      Shaders/CColoredQuadFilterGLSL.cpp
      Shaders/CModelShadersGLSL.cpp
      Shaders/CThermalColdFilterGLSL.cpp
      Shaders/CThermalHotFilterGLSL.cpp
      Shaders/CSpaceWarpFilterGLSL.cpp
      Shaders/CCameraBlurFilterGLSL.cpp
      Shaders/CXRayBlurFilterGLSL.cpp
      Shaders/CTextSupportShaderGLSL.cpp
      Shaders/CFogVolumePlaneShaderGLSL.cpp
      Shaders/CFogVolumeFilterGLSL.cpp
      Shaders/CEnergyBarShaderGLSL.cpp
      Shaders/CRadarPaintShaderGLSL.cpp
      Shaders/CMapSurfaceShaderGLSL.cpp
      Shaders/CPhazonSuitFilterGLSL.cpp
      Shaders/CScanLinesFilterGLSL.cpp
      Shaders/CRandomStaticFilterGLSL.cpp
      Shaders/CElementGenShadersGLSL.cpp
      Shaders/CDecalShadersGLSL.cpp
      Shaders/CParticleSwooshShadersGLSL.cpp
      Shaders/CFluidPlaneShaderGLSL.cpp
      Shaders/CAABoxShaderGLSL.cpp
      Shaders/CWorldShadowShaderGLSL.cpp)
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
    Shaders/TShader.hpp Shaders/TMultiBlendShader.hpp
    Shaders/TShaderDecl.hpp Shaders/TMultiBlendShaderDecl.hpp
    Shaders/CLineRendererShaders.hpp Shaders/CLineRendererShaders.cpp
    Shaders/CTexturedQuadFilter.hpp Shaders/CTexturedQuadFilter.cpp
    Shaders/CColoredQuadFilter.hpp Shaders/CColoredQuadFilter.cpp
    Shaders/CModelShaders.hpp Shaders/CModelShaders.cpp
    Shaders/CThermalColdFilter.hpp Shaders/CThermalColdFilter.cpp
    Shaders/CThermalHotFilter.hpp Shaders/CThermalHotFilter.cpp
    Shaders/CSpaceWarpFilter.hpp Shaders/CSpaceWarpFilter.cpp
    Shaders/CCameraBlurFilter.hpp Shaders/CCameraBlurFilter.cpp
    Shaders/CXRayBlurFilter.hpp Shaders/CXRayBlurFilter.cpp
    Shaders/CTextSupportShader.hpp Shaders/CTextSupportShader.cpp
    Shaders/CFogVolumePlaneShader.hpp Shaders/CFogVolumePlaneShader.cpp
    Shaders/CFogVolumeFilter.hpp Shaders/CFogVolumeFilter.cpp
    Shaders/CEnergyBarShader.hpp Shaders/CEnergyBarShader.cpp
    Shaders/CRadarPaintShader.hpp Shaders/CRadarPaintShader.cpp
    Shaders/CMapSurfaceShader.hpp Shaders/CMapSurfaceShader.cpp
    Shaders/CPhazonSuitFilter.hpp Shaders/CPhazonSuitFilter.cpp
    Shaders/CScanLinesFilter.hpp Shaders/CScanLinesFilter.cpp
    Shaders/CRandomStaticFilter.hpp Shaders/CRandomStaticFilter.cpp
    Shaders/CElementGenShaders.hpp Shaders/CElementGenShaders.cpp
    Shaders/CDecalShaders.hpp Shaders/CDecalShaders.cpp
    Shaders/CParticleSwooshShaders.hpp Shaders/CParticleSwooshShaders.cpp
    Shaders/CFluidPlaneShader.hpp Shaders/CFluidPlaneShader.cpp
    Shaders/CAABoxShader.hpp Shaders/CAABoxShader.cpp
    Shaders/CWorldShadowShader.hpp Shaders/CWorldShadowShader.cpp
    ${PLAT_SRCS})

runtime_add_list(Graphics GRAPHICS_SOURCES)
