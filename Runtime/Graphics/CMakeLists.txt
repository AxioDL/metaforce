set(GRAPHICS_SOURCES
        IRenderer.hpp
        IWeaponRenderer.hpp IWeaponRenderer.cpp
        CCubeMaterial.cpp CCubeMaterial.hpp
        CCubeModel.cpp CCubeModel.hpp
        CCubeRenderer.cpp CCubeRenderer.hpp
        CCubeSurface.cpp CCubeSurface.hpp
        CDrawable.hpp
        CDrawablePlaneObject.hpp
        CLineRenderer.hpp CLineRenderer.cpp
        CMetroidModelInstance.cpp CMetroidModelInstance.hpp
        CLight.hpp CLight.cpp
        CTevCombiners.cpp CTevCombiners.hpp
        CTexture.hpp CTexture.cpp
        CModel.cpp CModel.hpp
        CSkinnedModel.hpp CSkinnedModel.cpp
        CVertexMorphEffect.hpp CVertexMorphEffect.cpp
        CMoviePlayer.hpp CMoviePlayer.cpp
        CGraphicsPalette.hpp CGraphicsPalette.cpp
        CGX.hpp CGX.cpp
        CPVSVisSet.hpp CPVSVisSet.cpp
        CPVSVisOctree.hpp CPVSVisOctree.cpp
        CPVSAreaSet.hpp CPVSAreaSet.cpp
        CGraphics.hpp CGraphics.cpp
        CSimpleShadow.hpp CSimpleShadow.cpp
        CRainSplashGenerator.hpp CRainSplashGenerator.cpp
        CFont.hpp CFont.cpp
        Shaders/CLineRendererShaders.hpp Shaders/CLineRendererShaders.cpp
        Shaders/CColoredStripShader.hpp Shaders/CColoredStripShader.cpp
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
        Shaders/CEnvFxShaders.hpp Shaders/CEnvFxShaders.cpp)

runtime_add_list(Graphics GRAPHICS_SOURCES)
