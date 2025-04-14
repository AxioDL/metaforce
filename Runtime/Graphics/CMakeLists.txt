set(GRAPHICS_SOURCES
        IRenderer.hpp
        IWeaponRenderer.hpp IWeaponRenderer.cpp
        CCubeMaterial.cpp CCubeMaterial.hpp
        CCubeModel.cpp CCubeModel.hpp
        CCubeRenderer.cpp CCubeRenderer.hpp
        CCubeSurface.cpp CCubeSurface.hpp
        CDrawable.hpp
        CDrawablePlaneObject.hpp
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
)

runtime_add_list(Graphics GRAPHICS_SOURCES)
