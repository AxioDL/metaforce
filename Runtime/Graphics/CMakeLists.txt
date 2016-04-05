if(WIN32)
  set(PLAT_SRCS CLineRendererShadersHLSL.cpp CModelShadersHLSL.cpp)
elseif(APPLE)
  set(PLAT_SRCS CLineRendererShadersMetal.cpp CModelShadersMetal.cpp)
endif()

include_directories(${LIBJPEG_INCLUDE_DIR})

add_library(RuntimeCommonGraphics
            IRenderer.hpp
            CBooRenderer.hpp CBooRenderer.cpp
            CLineRenderer.hpp CLineRenderer.cpp
            CLineRendererShaders.hpp CLineRendererShadersGLSL.cpp
            CMetroidModelInstance.hpp
            CLight.hpp CLight.cpp
            CTexture.hpp CTextureBoo.cpp
            CModel.hpp CModelBoo.cpp
            CSkinnedModel.hpp CSkinnedModel.cpp
            CModelShaders.hpp CModelShadersGLSL.cpp
            CVertexMorphEffect.hpp CVertexMorphEffect.cpp
            CMoviePlayer.hpp CMoviePlayer.cpp
            CGraphicsPalette.hpp CGraphicsPalette.cpp
            CGraphics.hpp CGraphics.cpp
            ${PLAT_SRCS})
